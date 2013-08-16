#error NOTUSED
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
#ifndef TROTL_PARSER_EXPORT_H
#define TROTL_PARSER_EXPORT_H

// $Id: foo_export.h 760 2006-10-17 20:36:13Z syntheticpp $

#include "loki/LokiExport.h"

//#if defined(parsing_EXPORTS)
//#   define TSQLPARSER_MAKE_DLL
//#endif

#if (defined(TSQLPARSER_MAKE_DLL) && defined(TSQLPARSER_DLL)) || \
    (defined(TSQLPARSER_MAKE_DLL) && defined(TSQLPARSER_STATIC)) || \
    (defined(TSQLPARSER_DLL) && defined(TSQLPARSER_STATIC))
#error export macro error: you could not build AND use the library
#endif

#ifdef TSQLPARSER_MAKE_DLL
#define TSQLPARSER_EXPORT LOKI_EXPORT_SPEC //__declspec(dllexport)
#endif

#ifdef TSQLPARSER_DLL
#define TSQLPARSER_EXPORT LOKI_IMPORT_SPEC //__declspec(dllimport)
#endif

#ifdef TSQLPARSER_STATIC
#define TSQLPARSER_EXPORT
#endif

#if !defined(TSQLPARSER_EXPORT) && !defined(EXPLICIT_EXPORT)
#define TSQLPARSER_EXPORT
#endif

#ifndef TSQLPARSER_EXPORT
#error export macro error: TSQLPARSER_EXPORT was not defined, disable EXPLICIT_EXPORT or define a export specification
#endif

#endif
