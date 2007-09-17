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
#ifndef SINGLETONDLL_SINGLETONDLL_EXPORT_H
#define SINGLETONDLL_SINGLETONDLL_EXPORT_H

// $Id: singletondll_export.h 760 2006-10-17 20:36:13Z syntheticpp $


#include <loki/LokiExport.h>

#if (defined(SINGLETONDLL_MAKE_DLL) && defined(SINGLETONDLL_DLL)) || \
    (defined(SINGLETONDLL_MAKE_DLL) && defined(SINGLETONDLL_STATIC)) || \
    (defined(SINGLETONDLL_DLL) && defined(SINGLETONDLL_STATIC))
#error export macro error: you could not build AND use the library
#endif

#ifdef SINGLETONDLL_MAKE_DLL
#define SINGLETONDLL_EXPORT LOKI_EXPORT_SPEC //__declspec(dllexport)
#endif

#ifdef SINGLETONDLL_DLL
#define SINGLETONDLL_EXPORT LOKI_IMPORT_SPEC //__declspec(dllimport)
#endif

#ifdef SINGLETONDLL_STATIC
#define SINGLETONDLL_EXPORT
#endif

#if !defined(SINGLETONDLL_EXPORT) && !defined(EXPLICIT_EXPORT)
#define SINGLETONDLL_EXPORT
#endif

#ifndef SINGLETONDLL_EXPORT
#error export macro error: SINGLETONDLL_EXPORT was not defined, disable EXPLICIT_EXPORT or define a export specification
#endif


#endif
