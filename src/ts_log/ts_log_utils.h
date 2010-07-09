#ifndef __TS_LOG_UTILS__
#define __TS_LOG_UTILS__

#ifndef __HERE__

#ifdef DEBUG

#ifdef __GNUC__
	#define __HERE__ std::string(((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)) + ":"__HERE1__(__LINE__)
	#define __HERE1__(x)   STR(x)"\t" + __PRETTY_FUNCTION__
	#define STR(a) #a
#else
	#define __HERE__ std::string(__FILE__) + ":"__HERE1__(__LINE__)
	#define __HERE1__(x)   STR(x)"\t" +  __FUNCSIG__
	#define STR(a) #a
#endif

#else
#define __HERE__ ""
#endif

#endif


#endif
