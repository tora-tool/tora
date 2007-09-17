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
#ifndef FOO_FOO_EXPORT_H
#define FOO_FOO_EXPORT_H

// $Id: foo_export.h 760 2006-10-17 20:36:13Z syntheticpp $


#include <loki/LokiExport.h>

#if (defined(FOO_MAKE_DLL) && defined(FOO_DLL)) || \
    (defined(FOO_MAKE_DLL) && defined(FOO_STATIC)) || \
    (defined(FOO_DLL) && defined(FOO_STATIC))
#error export macro error: you could not build AND use the library
#endif

#ifdef FOO_MAKE_DLL
#define FOO_EXPORT LOKI_EXPORT_SPEC //__declspec(dllexport)
#endif

#ifdef FOO_DLL
#define FOO_EXPORT LOKI_IMPORT_SPEC //__declspec(dllimport)
#endif

#ifdef FOO_STATIC
#define FOO_EXPORT
#endif

#if !defined(FOO_EXPORT) && !defined(EXPLICIT_EXPORT)
#define FOO_EXPORT
#endif

#ifndef FOO_EXPORT
#error export macro error: FOO_EXPORT was not defined, disable EXPLICIT_EXPORT or define a export specification
#endif


#endif
