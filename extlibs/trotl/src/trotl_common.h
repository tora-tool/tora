// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008 Martin Fuchs <martin-fuchs@gmx.net>
//
// forked from OCIPL Version 1.3
// by Ivan Brezina <ivan@cvut.cz>

/*

  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright
	notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in
	the documentation and/or other materials provided with the
	distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef TROTL_COMMON_H_
#define TROTL_COMMON_H_

#include "trotl_export.h"
/* First of all include windows.h on windows
   boost::spirit (and others) do include windows.h which includes RpcNdr.h
   RpcNdr.h (162):typedef unsigned char boolean;
   while oratypes.h does:
   #ifndef boolean
   # define boolean int
   #endif
   Therefore windows.h MUST be included before oci.h
   We never use datatype boolean in trotl. (see bOOlean)
*/
#if defined(_MSC_VER)
#include <stdio.h>
#include <wtypes.h>
#include <windows.h>
#define snprintf _snprintf
#endif

#include <oci.h>
#define bOOlean int

#ifdef __GNUC__
#include "loki/TypeTraits.h"
#include <stdlib.h> // atoi
#endif

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <string.h> // strrchr
#include <stdio.h>

// "multiplatform" sleep - used in trotl_lob.cpp
#if defined(_MSC_VER)
#define MSLEEP(x) Sleep(x)
//#define gmtime gmtime_s
//#define localtime localtime_s
#define __NORETURN __declspec(noreturn)
#else
#include <unistd.h>
#define MSLEEP(x) usleep(1000 * x)
#define __NORETURN  __attribute__ ((noreturn))
#endif

namespace trotl
{
typedef TROTL_EXPORT ::std::string tstring;
typedef TROTL_EXPORT ::std::ostringstream tostream;

/// ten bytes (SID,SERIAL#) encoded
typedef struct  UB10 { ub1 bytes[10]; } UB10;

//TODO
//#include <fstream>
//#define USE_BOOST_THREAD_MANAGER
//#include "ts_log/critical_section.h"
//#include "ts_log/ts_log.h"
//thread_safe_log get_log();
//#define OCICALL(x)  x; do { std::cerr << __TROTL_HERE__ << #x << std::endl; } while(0)
//#define OCICALL(x)  x; do { std::cerr << #x << std::endl; } while(0)
#define OCICALL(x)  x

#ifdef __GNUC__

template <class T>
typename ::Loki::TypeTraits<T>::ParameterType min
(
        T &a,
        T &b
)
{
	return (a < b) ? a : b;
};

template <class T>
typename ::Loki::TypeTraits<T>::ParameterType min
(
        T a,
        T b
)
{
	return (a < b) ? a : b;
};

template <class T>
typename ::Loki::TypeTraits<T>::ParameterType min
(
        typename ::Loki::TypeTraits<T>::ConstParameterType a,
        typename ::Loki::TypeTraits<T>::ConstParameterType b
)
{
	return (a < b) ? a : b;
};
#else
//template <class T>
//T min(T a, T b)
//{
//	return (a < b) ? a : b;
//};
#ifndef min
#define min(a, b) ( (a < b) ? a : b )
#endif
#endif

};

// MSVC Debug build
//#if defined(_DEBUG) && !defined(DEBUG)
//#define DEBUG
//#endif

#ifdef DEBUG
#ifdef __GNUC__
#define __TROTL_HERE__ ::trotl::tstring(((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)) + ":" __TROTL_HERE1__(__LINE__)
//	#define __HERE_SHORT__ ::trotl::tstring(((strrchr(__FILE__, '/') ?: __FILE__ - 1) + 1)) + ":"__HERE1__(__LINE__)
//	#define __TROTL_HERE__ ::trotl::str_backtrace() + __HERE3__(__LINE__, __FILE__)
#define __TROTL_HERE1__(x)   STR(x)"\t" + __PRETTY_FUNCTION__
//	#define __HERE2__(x,y) ::trotl::tstring("("y":" STR(x)"(") +  __PRETTY_FUNCTION__ +")"
//	#define __HERE3__(x,y) ::trotl::tstring("\n(") + __PRETTY_FUNCTION__ + ") " y ":" STR(x) + "\n"
#else
//	#define __TROTL_HERE__ __HERE3__(__LINE__, __FILE__)
//	#define __HERE_SHORT__ __HERE3__(__LINE__, __FILE__)
//	#define __TROTL_HERE__ ::trotl::str_backtrace() + __HERE3__(__LINE__, __FILE__)
//	#define __HERE2__(x,y) ::trotl::tstring("("y":" STR(x)"(") +  __FUNCTION__ +")"
//	#define __HERE3__(x,y) ::trotl::tstring("\n(") + __FUNCTION__ + ") " y ":" STR(x) + "\n"
#define __TROTL_HERE__ ::trotl::tstring(__FILE__) + ":"__TROTL_HERE1__(__LINE__)
#define __TROTL_HERE1__(x)   STR(x)"\t" +  __FUNCSIG__
#endif //__GNUC__
#else  //DEBUG
#define __TROTL_HERE__ ""
#endif

#define STR(a) #a

#endif
