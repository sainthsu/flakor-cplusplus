
#ifndef _FK_MACROS_H_
#define _FK_MACROS_H_

	// namespace flakor {}
	#ifdef __cplusplus
    	#define FLAKOR_NS_BEGIN                     namespace flakor {
    	#define FLAKOR_NS_END                       }
    	#define USING_FLAKOR_NS                    using namespace flakor
	#else
    	#define FLAKOR_NS_BEGIN 
    	#define FLAKOR_NS_END 
    	#define USING_FLAKOR_NS   
	#endif 

	/**
 	* define a create function for a specific type, such as FKLayer
 	* @__TYPE__ class type to add create(), such as FKLayer
 	*/
	#define CREATE_FUNC(__TYPE__) \
	static __TYPE__* create() \
	{ \
    	__TYPE__ *pRet = new __TYPE__(); \
    	if (pRet && pRet->init()) \
    	{ \
        	pRet->autorelease(); \
        	return pRet; \
    	} \
    	else \
    	{ \
        	delete pRet; \
        	pRet = NULL; \
        	return NULL; \
    	} \
	}

	/** FK_PROPERTY_READONLY is used to declare a protected variable.
 	We can use getter to read the variable.
	能够getter的proteted成员变量
 	@param varType : the type of variable.
 	@param varName : variable name.
 	@param funName : "get + funName" is the name of the getter.
 	@warning : The getter is a public virtual function, you should rewrite it first.
 	The variables and methods declared after FK_PROPERTY_READONLY are all public.
 	If you need protected or private, please declare.
 	*/
	#define FK_PROPERTY_READONLY(varType, varName, funName)\
	protected: varType varName;\
	public: virtual varType get##funName(void);

	#define FK_PROPERTY_READONLY_PASS_BY_REF(varType, varName, funName)\
	protected: varType varName;\
	public: virtual const varType& get##funName(void);

	/** FK_PROPERTY is used to declare a protected variable.
 	We can use getter to read the variable, and use the setter to change the variable
	FK_PROPERTY创建protected成员变量，可以getter，setter
 	@param varType : the type of variable.
 	@param varName : variable name.
 	@param funName : "get + funName" is the name of the getter.
 	"set + funName" is the name of the setter.
 	@warning : The getter and setter are public virtual functions, you should rewrite them first.
 	The variables and methods declared after CC_PROPERTY are all public.
 	If you need protected or private, please declare.
 	*/
	#define FK_PROPERTY(varType, varName, funName)\
	protected: varType varName;\
	public: virtual varType get##funName(void);\
	public: virtual void set##funName(varType var);

	#define FK_PROPERTY_PASS_BY_REF(varType, varName, funName)\
	protected: varType varName;\
	public: virtual const varType& get##funName(void);\
	public: virtual void set##funName(const varType& var);

	/** FK_SYNTHESIZE_READONLY is used to declare a protected variable.
 	We can use getter to read the variable.
 	@param varType : the type of variable.
 	@param varName : variable name.
 	@param funName : "get + funName" is the name of the getter.
 	@warning : The getter is a public inline function.
 	The variables and methods declared after CC_SYNTHESIZE_READONLY are all public.
 	If you need protected or private, please declare.
 	*/
	#define FK_SYNTHESIZE_READONLY(varType, varName, funName)\
	protected: varType varName;\
	public: virtual varType get##funName(void) const { return varName; }

	#define FK_SYNTHESIZE_READONLY_PASS_BY_REF(varType, varName, funName)\
	protected: varType varName;\
	public: virtual const varType& get##funName(void) const { return varName; }

	/** FK_SYNTHESIZE is used to declare a protected variable.
 	We can use getter to read the variable, and use the setter to change the variable.
 	@param varType : the type of variable.
 	@param varName : variable name.
 	@param funName : "get + funName" is the name of the getter.
 	"set + funName" is the name of the setter.
 	@warning : The getter and setter are public  inline functions.
 	The variables and methods declared after CC_SYNTHESIZE are all public.
 	If you need protected or private, please declare.
 	*/
	#define FK_SYNTHESIZE(varType, varName, funName)\
	protected: varType varName;\
	public: virtual varType get##funName(void) const { return varName; }\
	public: virtual void set##funName(varType var){ varName = var; }

	#define FK_SYNTHESIZE_PASS_BY_REF(varType, varName, funName)\
	protected: varType varName;\
	public: virtual const varType& get##funName(void) const { return varName; }\
	public: virtual void set##funName(const varType& var){ varName = var; }

	#define FK_SYNTHESIZE_RETAIN(varType, varName, funName)    \
	private: varType varName; \
	public: virtual varType get##funName(void) const { return varName; } \
	public: virtual void set##funName(varType var)   \
	{ \
    	if (varName != var) \
    	{ \
        	CC_SAFE_RETAIN(var); \
        	CC_SAFE_RELEASE(varName); \
        	varName = var; \
    	} \
	} 

	#define FK_SAFE_DELETE(p)            do { if(p) { delete (p); (p) = 0; } } while(0)
	#define FK_SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = 0; } } while(0)
	#define FK_SAFE_FREE(p)                do { if(p) { free(p); (p) = 0; } } while(0)
	//#define FK_SAFE_RELEASE(p)            do { if(p) { (p)->release(); } } while(0)
	//#define FK_SAFE_RELEASE_NULL(p)        do { if(p) { (p)->release(); (p) = 0; } } while(0)
	//#define FK_SAFE_RETAIN(p)            do { if(p) { (p)->retain(); } } while(0)
	#define FK_BREAK_IF(cond)            if(cond) break

	#define __CCLOGWITHFUNCTION(s, ...) \
    	CCLog("%s : %s",__FUNCTION__, CCString::createWithFormat(s, ##__VA_ARGS__)->getCString())

	// flakor debug
	#if !defined(FLAKOR_DEBUG) || FLAKO_DEBUG == 0
	#define CCLOG(...)       do {} while (0)
	#define CCLOGINFO(...)   do {} while (0)
	#define CCLOGERROR(...)  do {} while (0)
	#define CCLOGWARN(...)   do {} while (0)

	#elif FLAKOR_DEBUG == 1
	#define FKLOG(format, ...)      cocos2d::CCLog(format, ##__VA_ARGS__)
	#define FKLOGERROR(format,...)  cocos2d::CCLog(format, ##__VA_ARGS__)
	#define FKLOGINFO(format,...)   do {} while (0)
	#define FKLOGWARN(...) __CCLOGWITHFUNCTION(__VA_ARGS__)

	#elif FLAKOR_DEBUG > 1
	#define FKLOG(format, ...)      cocos2d::CCLog(format, ##__VA_ARGS__)
	#define FKLOGERROR(format,...)  cocos2d::CCLog(format, ##__VA_ARGS__)
	#define FKLOGINFO(format,...)   cocos2d::CCLog(format, ##__VA_ARGS__)
	#define FKLOGWARN(...) __CCLOGWITHFUNCTION(__VA_ARGS__)
	#endif // FLAKOR_DEBUG

	// Lua engine debug
	#if !defined(COCOS2D_DEBUG) || COCOS2D_DEBUG == 0 || CC_LUA_ENGINE_DEBUG == 0
	#define LUALOG(...)
	#else
	#define LUALOG(format, ...)     cocos2d::CCLog(format, ##__VA_ARGS__)
	#endif 
   // Lua engine debug

	#if defined(__GNUC__) && ((__GNUC__ >= 5) || ((__GNUG__ == 4) && (__GNUC_MINOR__ >= 4))) \
    	|| (defined(__clang__) && (__clang_major__ >= 3))
	#define CC_DISABLE_COPY(Class) \
	private: \
    	Class(const Class &) = delete; \
    	Class &operator =(const Class &) = delete;
	#else
	#define CC_DISABLE_COPY(Class) \
	private: \
    	Class(const Class &); \
    	Class &operator =(const Class &);
	#endif

/*
 * only certain compilers support __attribute__((deprecated))
 */
#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
    #define CC_DEPRECATED_ATTRIBUTE __attribute__((deprecated))
#elif _MSC_VER >= 1400 //vs 2005 or higher
    #define CC_DEPRECATED_ATTRIBUTE __declspec(deprecated) 
#else
    #define CC_DEPRECATED_ATTRIBUTE
#endif 

/*
 * only certain compiler support __attribute__((format))
 * formatPos - 1-based position of format string argument
 * argPos - 1-based position of first format-dependent argument
 */
#if defined(__GNUC__) && (__GNUC__ >= 4)
#define CC_FORMAT_PRINTF(formatPos, argPos) __attribute__((__format__(printf, formatPos, argPos)))
#elif defined(__has_attribute)
  #if __has_attribute(format)
  #define CC_FORMAT_PRINTF(formatPos, argPos) __attribute__((__format__(printf, formatPos, argPos)))
  #endif // __has_attribute(format)
#else
#define CC_FORMAT_PRINTF(formatPos, argPos)
#endif

#if defined(_MSC_VER)
#define CC_FORMAT_PRINTF_SIZE_T "%08lX"
#else
#define CC_FORMAT_PRINTF_SIZE_T "%08zX"
#endif

#ifdef __GNUC__
#define CC_UNUSED __attribute__ ((unused))
#else
#define CC_UNUSED
#endif

#endif 