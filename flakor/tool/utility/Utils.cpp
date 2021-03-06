/****************************************************************************
Copyright (c) 2013-2014 Saint Hsu(saint@aliyun.com)

http://www.flakor.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#include "tool/utility/Utils.h"

#include <stdlib.h>

FLAKOR_NS_BEGIN

int nextPOT(int x)
{
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    return x + 1;
}

namespace utils
{
/**
 * Capture screen implementation, don't use it directly.
 */
void onCaptureScreen(const std::function<void(bool, const std::string&)>& afterCaptured, const std::string& filename)
{
/*
    auto glView = Director::getInstance()->getOpenGLView();
    auto frameSize = glView->getFrameSize();
#if (FK_TARGET_PLATFORM == FK_PLATFORM_MAC) || (FK_TARGET_PLATFORM == FK_PLATFORM_WIN32) || (FK_TARGET_PLATFORM == FK_PLATFORM_LINUX)
    frameSize = frameSize * glView->getFrameZoomFactor() * glView->getRetinaFactor();
#endif
    
    int width = static_cast<int>(frameSize.width);
    int height = static_cast<int>(frameSize.height);
    
    bool succeed = false;
    std::string outputFile = "";
    
    do
    {
        std::shared_ptr<GLubyte> buffer(new GLubyte[width * height * 4], [](GLubyte* p){ CC_SAFE_DELETE_ARRAY(p); });
        if (!buffer)
        {
            break;
        }
        
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

#if (FK_TARGET_PLATFORM == FK_PLATFORM_WP8)
        // The frame buffer is always created with portrait orientation on WP8. 
        // So if the current device orientation is landscape, we need to rotate the frame buffer.  
        auto renderTargetSize = glView->getRenerTargetSize();
        FKASSERT(width * height == static_cast<int>(renderTargetSize.width * renderTargetSize.height), "The frame size is not matched");
        glReadPixels(0, 0, (int)renderTargetSize.width, (int)renderTargetSize.height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get());
#else
        glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get());
#endif
        
        std::shared_ptr<GLubyte> flippedBuffer(new GLubyte[width * height * 4], [](GLubyte* p) { CC_SAFE_DELETE_ARRAY(p); });
        if (!flippedBuffer)
        {
            break;
        }

#if (FK_TARGET_PLATFORM == FK_PLATFORM_WP8)
        if (width == static_cast<int>(renderTargetSize.width))
        {
            // The current device orientation is portrait.
            for (int row = 0; row < height; ++row)
            {
                memcpy(flippedBuffer.get() + (height - row - 1) * width * 4, buffer.get() + row * width * 4, width * 4);
            }
        }
        else
        {
            // The current device orientation is landscape.
            for (int row = 0; row < width; ++row)
            {
                for (int col = 0; col < height; ++col)
                {
                    *(int*)(flippedBuffer.get() + (height - col - 1) * width * 4 + row * 4) = *(int*)(buffer.get() + row * height * 4 + col * 4);
                }
            }     
        }
#else
        for (int row = 0; row < height; ++row)
        {
            memcpy(flippedBuffer.get() + (height - row - 1) * width * 4, buffer.get() + row * width * 4, width * 4);
        }
#endif

        std::shared_ptr<Image> image(new Image);
        if (image)
        {
            image->initWithRawData(flippedBuffer.get(), width * height * 4, width, height, 8);
            if (FileUtils::getInstance()->isAbsolutePath(filename))
            {
                outputFile = filename;
            }
            else
            {
                CCASSERT(filename.find("/") == std::string::npos, "The existence of a relative path is not guaranteed!");
                outputFile = FileUtils::getInstance()->getWritablePath() + filename;
            }
            succeed = image->saveToFile(outputFile);
        }
    }while(0);
    
    if (afterCaptured)
    {
        afterCaptured(succeed, outputFile);
    }*/
}
/*
 * Capture screen interface
 */
void captureScreen(const std::function<void(bool, const std::string&)>& afterCaptured, const std::string& filename)
{
    /*static CustomCommand captureScreenCommand;
    captureScreenCommand.init(std::numeric_limits<float>::max());
    captureScreenCommand.func = std::bind(onCaptureScreen, afterCaptured, filename);
    Director::getInstance()->getRenderer()->addCommand(&captureScreenCommand);*/
}
    
std::vector<Entity*> findChildren(const Entity &node, const std::string &name)
{
    return NULL;
}

#define MAX_ITOA_BUFFER_SIZE 256
double atof(const char* str)
{
    if (str == nullptr)
    {
        return 0.0;
    }
    
    char buf[MAX_ITOA_BUFFER_SIZE];
    strncpy(buf, str, MAX_ITOA_BUFFER_SIZE);
    
    // strip string, only remain 7 numbers after '.'
    char* dot = strchr(buf, '.');
    if (dot != nullptr && dot - buf + 8 <  MAX_ITOA_BUFFER_SIZE)
    {
        dot[8] = '\0';
    }
    
    return ::atof(buf);
}

double gettime()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    return (double)tv.tv_sec + (double)tv.tv_usec/1000000;
}
    
}

FLAKOR_NS_END
