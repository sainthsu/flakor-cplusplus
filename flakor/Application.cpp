#include <jni.h>

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>

#include <android/log.h>
#include "Application.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "threaded_app", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "threaded_app", __VA_ARGS__))

/* For debug builds, always enable the debug traces in this library */
#ifndef NDEBUG
#  define LOGV(...)  ((void)__android_log_print(ANDROID_LOG_VERBOSE, "threaded_app", __VA_ARGS__))
#else
#  define LOGV(...)  ((void)0)
#endif

int8_t Application::readCmd()
{
    int8_t cmd;
    if (read(this->msgread, &cmd, sizeof(cmd)) == sizeof(cmd))
    {
        switch (cmd)
        {
            case APP_CMD_SAVE_STATE:
                this->freeSavedState();
                break;
        }
        return cmd;
    }
    else
    {
        LOGE("No data on command pipe!");
    }
    return -1;
}

void Application::preExecCmd(int8_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_INPUT_CHANGED:
            LOGV("APP_CMD_INPUT_CHANGED\n");
            pthread_mutex_lock(&this->mutex);
            if (this->inputQueue != NULL) {
                AInputQueue_detachLooper(this->inputQueue);
            }
            
            this->inputQueue = this->pendingInputQueue;
            if (this->inputQueue != NULL)
            {
                LOGV("Attaching input queue to looper");
                AInputQueue_attachLooper(this->inputQueue,
                                         this->looper, LOOPER_ID_INPUT, NULL,
                                         &this->inputPollSource);
            }
            pthread_cond_broadcast(&this->cond);
            pthread_mutex_unlock(&this->mutex);
            break;
            
        case APP_CMD_INIT_WINDOW:
            LOGV("APP_CMD_INIT_WINDOW\n");
            pthread_mutex_lock(&this->mutex);
            this->window = this->pendingWindow;
            pthread_cond_broadcast(&this->cond);
            pthread_mutex_unlock(&this->mutex);
            break;
            
        case APP_CMD_TERM_WINDOW:
            LOGV("APP_CMD_TERM_WINDOW\n");
            pthread_cond_broadcast(&this->cond);
            break;
            
        case APP_CMD_RESUME:
        case APP_CMD_START:
        case APP_CMD_PAUSE:
        case APP_CMD_STOP:
            LOGV("activityState=%d\n", cmd);
            pthread_mutex_lock(&this->mutex);
            this->activityState = cmd;
            pthread_cond_broadcast(&this->cond);
            pthread_mutex_unlock(&this->mutex);
            break;
            
        case APP_CMD_CONFIG_CHANGED:
            LOGV("APP_CMD_CONFIG_CHANGED\n");
            AConfiguration_fromAssetManager(this->config,
                                            this->activity->assetManager);
            this->printConfig();
            break;
            
        case APP_CMD_DESTROY:
            LOGV("APP_CMD_DESTROY\n");
            this->destroyRequested = 1;
            break;
    }
}

void Application::onAppCmd(int32_t cmd)
{
    if(this->engine != NULL)
    {
        this->engine->handleCMD(cmd);
    }
}

void Application::postExecCmd(int8_t cmd)
{
    switch (cmd)
    {
        case APP_CMD_TERM_WINDOW:
            LOGV("APP_CMD_TERM_WINDOW\n");
            pthread_mutex_lock(&this->mutex);
            this->window = NULL;
            pthread_cond_broadcast(&this->cond);
            pthread_mutex_unlock(&this->mutex);
            break;
            
        case APP_CMD_SAVE_STATE:
            LOGV("APP_CMD_SAVE_STATE\n");
            pthread_mutex_lock(&this->mutex);
            this->stateSaved = 1;
            pthread_cond_broadcast(&this->cond);
            pthread_mutex_unlock(&this->mutex);
            break;
            
        case APP_CMD_RESUME:
            this->freeSavedState();
            break;
    }
}

int32_t Application::onInputEvent(AInputEvent* event)
{
    if (this->engine != NULL)
    {
        return this->engine->handleInput(event);
    }
    return 0;
}

void Application::freeSavedState()
{
    pthread_mutex_lock(&this->mutex);
    if (this->savedState != NULL)
    {
        free(this->savedState);
        this->savedState = NULL;
        this->savedStateSize = 0;
    }
    pthread_mutex_unlock(&this->mutex);
}

void Application::destroy()
{
    LOGV("android_app_destroy!");
    this->freeSavedState();
    pthread_mutex_lock(&this->mutex);
    if (this->inputQueue != NULL) {
        AInputQueue_detachLooper(this->inputQueue);
    }
    AConfiguration_delete(this->config);
    this->destroyed = 1;
    pthread_cond_broadcast(&this->cond);
    pthread_mutex_unlock(&this->mutex);
    // Can't touch android_app object after this.
}


void Application::writeCmd(int8_t cmd)
{
    if (write(this->msgwrite, &cmd, sizeof(cmd)) != sizeof(cmd)) {
        LOGE("Failure writing android application cmd: %s\n", strerror(errno));
    }
}

void Application::setInput(AInputQueue* inputQueue)
{
    pthread_mutex_lock(&this->mutex);
    android_app->pendingInputQueue = inputQueue;
    this->writeCmd(APP_CMD_INPUT_CHANGED);
    
    while (this->inputQueue != this->pendingInputQueue)
    {
        pthread_cond_wait(&this->cond, &this->mutex);
    }
    pthread_mutex_unlock(&this->mutex);
}

void Application::setWindow(ANativeWindow* window)
{
    pthread_mutex_lock(&this->mutex);
    if (this->pendingWindow != NULL)
    {
        this->writeCmd(APP_CMD_TERM_WINDOW);
    }
    this->pendingWindow = window;
    if (window != NULL)
    {
        this->writeCmd(APP_CMD_INIT_WINDOW);
    }
    while (this->window != this->pendingWindow) {
        pthread_cond_wait(&this->cond, &this->mutex);
    }
    pthread_mutex_unlock(&this->mutex);
}

void Application::setActivityState(int8_t cmd)
{
    pthread_mutex_lock(&this->mutex);
    this->writeCmd(cmd);
    while (this->activityState != cmd)
    {
        pthread_cond_wait(&this->cond, &this->mutex);
    }
    pthread_mutex_unlock(&this->mutex);
}

void Application::free()
{
    pthread_mutex_lock(&this->mutex);
    this->writeCmd(APP_CMD_DESTROY);
    while (!this->destroyed)
    {
        pthread_cond_wait(&this->cond, &this->mutex);
    }
    pthread_mutex_unlock(&this->mutex);
    
    close(this->msgread);
    close(this->msgwrite);
    pthread_cond_destroy(&this->cond);
    pthread_mutex_destroy(&this->mutex);
    free(android_app);
}

void Application::printConfig()
{
    char lang[2], country[2];
    AConfiguration_getLanguage(android_app->config, lang);
    AConfiguration_getCountry(android_app->config, country);
    
    LOGV("Config: mcc=%d mnc=%d lang=%c%c cnt=%c%c orien=%d touch=%d dens=%d "
         "keys=%d nav=%d keysHid=%d navHid=%d sdk=%d size=%d long=%d "
         "modetype=%d modenight=%d",
         AConfiguration_getMcc(android_app->config),
         AConfiguration_getMnc(android_app->config),
         lang[0], lang[1], country[0], country[1],
         AConfiguration_getOrientation(android_app->config),
         AConfiguration_getTouchscreen(android_app->config),
         AConfiguration_getDensity(android_app->config),
         AConfiguration_getKeyboard(android_app->config),
         AConfiguration_getNavigation(android_app->config),
         AConfiguration_getKeysHidden(android_app->config),
         AConfiguration_getNavHidden(android_app->config),
         AConfiguration_getSdkVersion(android_app->config),
         AConfiguration_getScreenSize(android_app->config),
         AConfiguration_getScreenLong(android_app->config),
         AConfiguration_getUiModeType(android_app->config),
         AConfiguration_getUiModeNight(android_app->config));
}

// --------------------------------------------------------------------
// Native activity interaction (called from main thread)
// --------------------------------------------------------------------
Application* Application::create(ANativeActivity* activity,
                                 void* savedState, size_t savedStateSize)
{
    Application* app = new Application();
    app->activity = activity;
    
    pthread_mutex_init(&app->mutex, NULL);
    pthread_cond_init(&app->cond, NULL);
    
    if (savedState != NULL)
    {
        app->savedState = malloc(savedStateSize);
        app->savedStateSize = savedStateSize;
        memcpy(app->savedState, savedState, savedStateSize);
    }
    
    int msgpipe[2];
    if (pipe(msgpipe))
    {
        LOGE("could not create pipe: %s", strerror(errno));
        return NULL;
    }
    
    app->msgread = msgpipe[0];
    app->msgwrite = msgpipe[1];
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&app->thread, &attr, Application::ThreadEntry, app);
    
    // Wait for thread to start.
    pthread_mutex_lock(&app->mutex);
    while (!app->running) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }
    pthread_mutex_unlock(&app->mutex);
    
    return app;
}

/* new thread entry */
void* Application::ThreadEntry(void* param)
{
    Application* app = (Application*)param;
    
    app->config = AConfiguration_new();
    AConfiguration_fromAssetManager(app->config, app->activity->assetManager);
    
    //print config info
    app->printConfig();
    
    /*
     *设置主循环和输入命令控制
     */
    app->cmdPollSource.id = LOOPER_ID_MAIN;
    app->cmdPollSource.app = app;
    app->cmdPollSource.process = process_cmd;
    app->inputPollSource.id = LOOPER_ID_INPUT;
    app->inputPollSource.app = app;
    app->inputPollSource.process = process_input;
    
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, app->msgread, LOOPER_ID_MAIN, ALOOPER_EVENT_INPUT, NULL,
                  &app->cmdPollSource);
    app->looper = looper;
    
    pthread_mutex_lock(&app->mutex);
    app->running = 1;
    pthread_cond_broadcast(&app->cond);
    pthread_mutex_unlock(&app->mutex);
    
    app->main();
    app->destroy();
    return NULL;
}

/**
 * This is the main entry point of a native application that is using
 *  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void Application::main()
{
    
    Engine *engine = new Engine();
    // Make sure glue isn't stripped.
    this->dummy();
    
    this->engine = engine;
    this->userData = NULL;
    
    //app->onAppCmd = engine_handle_cmd;
    //app->onInputEvent = engine_handle_input;
    
    engine->app = this;
    
    // Prepare to monitor accelerometer
    engine->sensorManager = ASensorManager_getInstance();
    engine->accelerometerSensor = ASensorManager_getDefaultSensor(engine->sensorManager,
                                                                  ASENSOR_TYPE_ACCELEROMETER);
    engine->sensorEventQueue = ASensorManager_createEventQueue(engine->sensorManager,
                                                               this->looper, LOOPER_ID_USER, NULL, NULL);
    
    if (this->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine->app = *(Application *)this->savedState;
    }
    
    // loop waiting for stuff to do.
    
    while (1)
    {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;
        
        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine->animating ? 0 : -1, NULL, &events,
                                      (void**)&source)) >= 0)
        {
            
            // Process this event.
            if (source != NULL) {
                source->process(this, source);
            }
            
            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER)
            {
                if (engine->accelerometerSensor != NULL)
                {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine->sensorEventQueue,
                                                       &event, 1) > 0)
                    {
                        LOGI("accelerometer: x=%f y=%f z=%f",
                             event.acceleration.x, event.acceleration.y,
                             event.acceleration.z);
                    }
                }
            }
            
            // Check if we are exiting.
            if (this->destroyRequested != 0) {
                engine->termDisplay();
                return;
            }
        }
        
        if (engine->animating)
        {
            // Done with events; draw next animation frame.
            engine.state.angle += .01f;
            if (engine.state.angle > 1) {
                engine.state.angle = 0;
            }
            
            // Drawing is throttled to the screen update rate, so there
            // is no need to do timing here.
            engine->drawFrame();
        }
    }
}


static void process_input(Application* app, struct android_poll_source* source) {
    AInputEvent* event = NULL;
    while (AInputQueue_getEvent(app->inputQueue, &event) >= 0)
    {
        LOGV("New input event: type=%d\n", AInputEvent_getType(event));
        if (AInputQueue_preDispatchEvent(app->inputQueue, event)) {
            continue;
        }
        int32_t handled = 0;
        handled = app->onInputEvent(event);
        AInputQueue_finishEvent(app->inputQueue, event, handled);
    }
}

static void process_cmd(Application* app, struct android_poll_source* source)
{
    int8_t cmd = android_app_read_cmd(app);
    app->preExecCmd(cmd);
    app->onAppCmd(cmd);
    app->postExecCmd(cmd);
}

/**
 * nativeActivity lifecycle
 */

static void onStart(ANativeActivity* activity)
{
    LOGV("Start: %p\n", activity);
    Application* app = (Application*)activity->instance;
    app->setActivityState(APP_CMD_START);
}

static void onPause(ANativeActivity* activity)
{
    LOGV("Pause: %p\n", activity);
    Application* app = (Application*)activity->instance;
    app->setActivityState(APP_CMD_PAUSE);
}

static void onStop(ANativeActivity* activity)
{
    LOGV("Stop: %p\n", activity);
    Application* app = (Application*)activity->instance;
    app->setActivityState(APP_CMD_STOP);
}

static void onResume(ANativeActivity* activity)
{
    LOGV("Resume: %p\n", activity);
    Application* app = (Application*)activity->instance;
    app->setActivityState(APP_CMD_RESUME);
}


static void onDestroy(ANativeActivity* activity) 
{
    LOGV("Destroy: %p\n", activity);
    Application* app = (Application*)activity->instance;
    app->free();
}

static void* onSaveInstanceState(ANativeActivity* activity, size_t* outLen)
{
    Application* app = (Application*)activity->instance;
    void* savedState = NULL;

    LOGV("SaveInstanceState: %p\n", activity);
    pthread_mutex_lock(&app->mutex);
    app->stateSaved = 0;
    app->writeCmd(APP_CMD_SAVE_STATE);
    
    while (!app->stateSaved) {
        pthread_cond_wait(&app->cond, &app->mutex);
    }

    if (app->savedState != NULL)
    {
        savedState = app->savedState;
        *outLen = app->savedStateSize;
        app->savedState = NULL;
        app->savedStateSize = 0;
    }

    pthread_mutex_unlock(&app->mutex);

    return savedState;
}

static void onConfigurationChanged(ANativeActivity* activity)
{
    Application* app = (Application*)activity->instance;
    LOGV("ConfigurationChanged: %p\n", activity);
    app->writeCmd(APP_CMD_CONFIG_CHANGED);
}

static void onLowMemory(ANativeActivity* activity)
{
    Application* app = (Application*)activity->instance;
    LOGV("LowMemory: %p\n", activity);
    app->writeCmd(APP_CMD_LOW_MEMORY);
}

static void onWindowFocusChanged(ANativeActivity* activity, int focused)
{
    LOGV("WindowFocusChanged: %p -- %d\n", activity, focused);
    Application* app = (Application*)activity->instance;
    app->writeCmd(focused ? APP_CMD_GAINED_FOCUS : APP_CMD_LOST_FOCUS);
}

static void onNativeWindowCreated(ANativeActivity* activity, ANativeWindow* window)
{
    LOGV("NativeWindowCreated: %p -- %p\n", activity, window);
    Application* app = (Application*)activity->instance;

    app->setWindow(window);
}

static void onNativeWindowDestroyed(ANativeActivity* activity, ANativeWindow* window)
{
    LOGV("NativeWindowDestroyed: %p -- %p\n", activity, window);
    Application* app = (Application*)activity->instance;

    app->setWindow(NULL);
}

static void onInputQueueCreated(ANativeActivity* activity, AInputQueue* queue)
{
    LOGV("InputQueueCreated: %p -- %p\n", activity, queue);
    Application* app = (Application*)activity->instance;

    app->setInput(queue);
}

static void onInputQueueDestroyed(ANativeActivity* activity, AInputQueue* queue)
{
    LOGV("InputQueueDestroyed: %p -- %p\n", activity, queue);
    Application* app = (Application*)activity->instance;
    
    app->setInput(NULL);
}

/*create activity*/
void ANativeActivity_onCreate(ANativeActivity* activity,
        void* savedState, size_t savedStateSize) 
{
    LOGV("Creating: %p\n", activity);
    
    activity->callbacks->onStart = onStart;
    activity->callbacks->onResume = onResume;
    activity->callbacks->onSaveInstanceState = onSaveInstanceState;
    activity->callbacks->onPause = onPause;
    activity->callbacks->onStop = onStop;
    activity->callbacks->onDestroy = onDestroy;
    
    activity->callbacks->onWindowFocusChanged = onWindowFocusChanged;
    activity->callbacks->onNativeWindowCreated = onNativeWindowCreated;
    activity->callbacks->onNativeWindowResized = NULL;
    activity->callbacks->onNativeWindowRedrawNeeded = NULL;
    activity->callbacks->onNativeWindowDestroyed = onNativeWindowDestroyed;
    
    activity->callbacks->onInputQueueCreated = onInputQueueCreated;
    activity->callbacks->onInputQueueDestroyed = onInputQueueDestroyed;

    activity->callbacks->onContentRectChanged = NULL;
    
    activity->callbacks->onConfigurationChanged = onConfigurationChanged;
    activity->callbacks->onLowMemory = onLowMemory;

    activity->instance = Application::create(activity, savedState, savedStateSize);
}

