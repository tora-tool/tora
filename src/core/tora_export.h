////////////////////////////////////////////////////////////////////////////////
// The Loki Library
// Copyright (c) 2006 Peter Kümmel
// Permission to use, copy, modify, distribute and sell this software for any
//     purpose is hereby granted without fee, provided that the above copyright
//     notice appear in all copies and that both that copyright notice and this
//     permission notice appear in supporting documentation.
// The author makes no representations about the
//     suitability of this software for any purpose. It is provided "as is"
//     without express or implied warranty.
////////////////////////////////////////////////////////////////////////////////
#ifndef TORA_EXPORT_H
#define TORA_EXPORT_H

// $Id: foo_export.h 760 2006-10-17 20:36:13Z syntheticpp $


#include "loki/LokiExport.h"

// Theat all the test programs the same way as the main .EXE file
#if defined(test1_EXPORTS) \
    || defined(test2_EXPORTS) \
    || defined(test3_EXPORTS) \
    || defined(test4_EXPORTS) \
    || defined(test5_EXPORTS) \
    || defined(test6_EXPORTS) \
    || defined(test7_EXPORTS) \
    || defined(test8_EXPORTS) \
    || defined(test9_EXPORTS) \
    || defined(test10_EXPORTS) \
    || defined(test11_EXPORTS)
#define tora_EXPORTS
#endif

// Note: please don't be confused. We do not make any tora.so.
// On Windows we just need tora.lib to be generated.
// This file is needed by connection provider plugins to refer undefined symbols
// See: src/CMakeLists.txt:SET_TARGET_PROPERTIES(${EXE_NAME} PROPERTIES ENABLE_EXPORTS ON)
#if defined(tora_EXPORTS)
#   define TORA_MAKE_DLL
#else
#   define TORA_DLL
#endif

#if (defined(TORA_MAKE_DLL) && defined(TORA_DLL)) || \
    (defined(TORA_MAKE_DLL) && defined(TORA_STATIC)) || \
    (defined(TORA_DLL) && defined(TORA_STATIC))
#error export macro error: you could not build AND use the library
#endif

#ifdef TORA_MAKE_DLL
#define TORA_EXPORT LOKI_EXPORT_SPEC //__declspec(dllexport)
#endif

#ifdef TORA_DLL
#define TORA_EXPORT LOKI_IMPORT_SPEC //__declspec(dllimport)
#endif

#ifdef TORA_STATIC
#define TORA_EXPORT
#endif

#if !defined(TORA_EXPORT) && !defined(EXPLICIT_EXPORT)
#define TORA_EXPORT
#endif

#ifndef TORA_EXPORT
#error export macro error: TORA_EXPORT was not defined, disable EXPLICIT_EXPORT or define a export specification
#endif

#endif
