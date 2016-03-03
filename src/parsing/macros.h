#ifndef __MACROS_ANT__
#define __MACROS_ANT__

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif

#if   defined(__GNUC__)
//#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
//#pragma GCC diagnostic pop
#endif

#if   defined(__GNUC__)
#define __weak_symbol__ __attribute__ ((weak))
#elif defined(_MSC_VER)
#define __weak_symbol__ __declspec(selectany)
#else
#define __weak_symbol__
#endif

#endif //__MACROS_ANT__
