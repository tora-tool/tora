/** \file
 * Basic type and constant definitions for ANTLR3 Runtime.
 */
#ifndef	_ANTLR3DEFS_HPP
#define	_ANTLR3DEFS_HPP

// [The "BSD licence"]
// Copyright (c) 2005-2009 Gokulakannan Somasundaram, ElectronDB

//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/* Following are for generated code, they are not referenced internally!!!
 */
#if !defined(ANTLR_HUGE) && !defined(ANTLR_AVERAGE) && !defined(ANTLR_SMALL)
#define	ANTLR_AVERAGE
#endif

#ifdef	ANTLR_HUGE
#ifndef	ANTLR_SIZE_HINT
#define	ANTLR_SIZE_HINT        2049
#endif
#ifndef	ANTLR_LIST_SIZE_HINT
#define	ANTLR_LIST_SIZE_HINT   127
#endif
#endif

#ifdef	ANTLR_AVERAGE
#ifndef	ANTLR_SIZE_HINT
#define	ANTLR_SIZE_HINT        1025
#define	ANTLR_LIST_SIZE_HINT   63
#endif
#endif

#ifdef	ANTLR_SMALL
#ifndef	ANTLR_SIZE_HINT
#define	ANTLR_SIZE_HINT        211
#define	ANTLR_LIST_SIZE_HINT   31
#endif
#endif

// Definitions that indicate the encoding scheme character streams and strings etc
//
/// Indicates Big Endian for encodings where this makes sense
///
#define ANTLR_BE           1

/// Indicates Little Endian for encoidngs where this makes sense
///
#define ANTLR_LE           2

/// General latin-1 or other 8 bit encoding scheme such as straight ASCII
///
#define ANTLR_ENC_8BIT     4

/// UTF-8 encoding scheme
///
#define ANTLR_ENC_UTF8     8

/// UTF-16 encoding scheme (which also covers UCS2 as that does not have surrogates)
///
#define ANTLR_ENC_UTF16        16
#define ANTLR_ENC_UTF16BE      16 + ANTLR_BE
#define ANTLR_ENC_UTF16LE      16 + ANTLR_LE

/// UTF-32 encoding scheme (basically straight 32 bit)
///
#define ANTLR_ENC_UTF32        32
#define ANTLR_ENC_UTF32BE      32 + ANTLR_BE
#define ANTLR_ENC_UTF32LE      32 + ANTLR_LE

/// Input is 8 bit EBCDIC (which we convert to 8 bit ASCII on the fly
///
#define ANTLR_ENC_EBCDIC       64

#define ANTLR_BEGIN_NAMESPACE() namespace antlr3 {
#define ANTLR_END_NAMESPACE() }

#define ANTLR_USE_64BIT

/* Common definitions come first
 */
#include    <antlr3errors.hpp>

/* Work out what operating system/compiler this is. We just do this once
 * here and use an internal symbol after this.
 */
#ifdef	_WIN64

# ifndef	ANTLR_WINDOWS
#   define	ANTLR_WINDOWS
# endif
# define	ANTLR_WIN64
# define	ANTLR_USE_64BIT

#else

#ifdef	_WIN32
# ifndef	ANTLR_WINDOWS
#  define	ANTLR_WINDOWS
# endif

#define	ANTLR_WIN32
#endif

#endif

#ifdef	ANTLR_WINDOWS 

#ifndef WIN32_LEAN_AND_MEAN
#define	WIN32_LEAN_AND_MEAN
#endif

/* Allow VC 8 (vs2005) and above to use 'secure' versions of various functions such as sprintf
 */
#ifndef	_CRT_SECURE_NO_DEPRECATE 
#define	_CRT_SECURE_NO_DEPRECATE 
#endif

#include    <stdlib.h>
#include    <winsock2.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <stdarg.h>

#define	ANTLR_API      __declspec(dllexport)
#define	ANTLR_CDECL    __cdecl
#define ANTLR_FASTCALL __fastcall


#ifndef __MINGW32__
// Standard Windows types
//
typedef	INT32	ANTLR_CHAR;
typedef	UINT32	ANTLR_UCHAR;

typedef	INT8	ANTLR_INT8;
typedef	INT16	ANTLR_INT16;
typedef	INT32	ANTLR_INT32;
typedef	INT64	ANTLR_INT64;
typedef	UINT8	ANTLR_UINT8;
typedef	UINT16	ANTLR_UINT16;
typedef	UINT32	ANTLR_UINT32;
typedef	UINT64	ANTLR_UINT64;
typedef UINT64  ANTLR_BITWORD;

#else
// Mingw uses stdint.h and fails to define standard Microsoft typedefs
// such as UINT16, hence we must use stdint.h for Mingw.
//
#include <stdint.h>
typedef int32_t     ANTLR_CHAR;
typedef uint32_t    ANTLR_UCHAR;

typedef int8_t	    ANTLR_INT8;
typedef int16_t	    ANTLR_INT16;
typedef int32_t	    ANTLR_INT32;
typedef int64_t	    ANTLR_INT64;

typedef uint8_t	    ANTLR_UINT8;
typedef uint16_t    ANTLR_UINT16;
typedef uint32_t    ANTLR_UINT32;
typedef uint64_t    ANTLR_UINT64;
typedef uint64_t    ANTLR_BITWORD;

#endif



#define	ANTLR_UINT64_LIT(lit)  lit##ULL

#define	ANTLR_INLINE	        __inline

typedef FILE *	    ANTLR_FDSC;
typedef	struct stat ANTLR_FSTAT_STRUCT;



#ifdef	ANTLR_USE_64BIT
#define ANTLR_UINT64_CAST(ptr) ((ANTLR_UINT64)(ptr))
#define	ANTLR_UINT32_CAST(ptr)	(ANTLR_UINT32)((ANTLR_UINT64)(ptr))
typedef ANTLR_INT64		ANTLR_MARKER;			
typedef ANTLR_UINT64		ANTLR_INTKEY;
#else
#define ANTLR_UINT64_CAST(ptr) (ANTLR_UINT64)((ANTLR_UINT32)(ptr))
#define	ANTLR_UINT32_CAST(ptr)	(ANTLR_UINT32)(ptr)
typedef	ANTLR_INT32		ANTLR_MARKER;
typedef ANTLR_UINT32	ANTLR_INTKEY;
#endif

#ifdef	ANTLR_WIN32
#endif

#ifdef	ANTLR_WIN64
#endif


typedef	int			ANTLR_SALENT;								// Type used for size of accept structure
typedef struct sockaddr_in	ANTLR_SOCKADDRT, * pANTLR_SOCKADDRT;	// Type used for socket address declaration
typedef struct sockaddr		ANTLR_SOCKADDRC, * pANTLR_SOCKADDRC;	// Type used for cast on accept()

#define	ANTLR_CLOSESOCKET	closesocket

/* Warnings that are over-zealous such as complaining about strdup, we
 * can turn off.
 */

/* Don't complain about "deprecated" functions such as strdup
 */
#pragma warning( disable : 4996 )

#else

#ifdef __LP64__
#define ANTLR_USE_64BIT
#endif

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdarg.h>
#include <inttypes.h>
#include <unistd.h>

#define _stat stat

typedef int SOCKET;

/* Inherit type definitions for autoconf
 */
typedef int32_t	    ANTLR_CHAR;
typedef uint32_t    ANTLR_UCHAR;

typedef int8_t	    ANTLR_INT8;
typedef int16_t	    ANTLR_INT16;
typedef int32_t	    ANTLR_INT32;
typedef int64_t	    ANTLR_INT64;

typedef uint8_t	    ANTLR_UINT8;
typedef uint16_t    ANTLR_UINT16;
typedef uint32_t    ANTLR_UINT32;
typedef uint64_t    ANTLR_UINT64;
typedef uint64_t    ANTLR_BITWORD;

#define ANTLR_INLINE   inline
#define	ANTLR_API

typedef FILE *	    ANTLR_FDSC;
typedef	struct stat ANTLR_FSTAT_STRUCT;

#ifdef	ANTLR_USE_64BIT
#define	ANTLR_FUNC_PTR(ptr)    (void *)((ANTLR_UINT64)(ptr))
#define ANTLR_UINT64_CAST(ptr)	(ANTLR_UINT64)(ptr))
#define	ANTLR_UINT32_CAST(ptr) (ANTLR_UINT32)((ANTLR_UINT64)(ptr))
typedef ANTLR_INT64		ANTLR_MARKER;
typedef ANTLR_UINT64		ANTLR_INTKEY;
#else
#define	ANTLR_FUNC_PTR(ptr)	(void *)((ANTLR_UINT32)(ptr))
#define ANTLR_UINT64_CAST(ptr) (ANTLR_UINT64)((ANTLR_UINT32)(ptr))
#define	ANTLR_UINT32_CAST(ptr)	(ANTLR_UINT32)(ptr)
typedef	ANTLR_INT32		ANTLR_MARKER;
typedef ANTLR_UINT32		ANTLR_INTKEY;
#endif
#define	ANTLR_UINT64_LIT(lit)	lit##ULL

#endif

#ifdef ANTLR_USE_64BIT
#define ANTLR_TRIE_DEPTH 63
#else
#define ANTLR_TRIE_DEPTH 31
#endif
/* Pre declare the typedefs for all the interfaces, then 
 * they can be inter-dependant and we will let the linker
 * sort it out for us.
 */
#include    <antlr3interfaces.hpp>

// Include the unicode.org conversion library header.
//
#include    <antlr3convertutf.hpp>

enum ChannelType
{
	/** Default channel for a token
	*/
	TOKEN_DEFAULT_CHANNEL	 = 0
	/** Reserved channel number for a HIDDEN token - a token that
		*  is hidden from the parser.
		*/
	,	HIDDEN		=		99
};

#endif	/* _ANTLR3DEFS_H	*/
