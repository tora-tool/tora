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
#ifndef TROTL_TROTL_EXPORT_H
#define TROTL_TROTL_EXPORT_H

// $Id: foo_export.h 760 2006-10-17 20:36:13Z syntheticpp $


#include "loki/LokiExport.h"

#if (defined(TROTL_MAKE_DLL) && defined(TROTL_DLL)) || \
    (defined(TROTL_MAKE_DLL) && defined(TROTL_STATIC)) || \
    (defined(TROTL_DLL) && defined(TROTL_STATIC))
#error export macro error: you could not build AND use the library
#endif

#ifdef TROTL_MAKE_DLL
#define TROTL_EXPORT LOKI_EXPORT_SPEC //__declspec(dllexport)
#endif

#ifdef TROTL_DLL
#define TROTL_EXPORT LOKI_IMPORT_SPEC //__declspec(dllimport)
#endif

#ifdef TROTL_STATIC
#define TROTL_EXPORT
#endif

#if !defined(TROTL_EXPORT) && !defined(EXPLICIT_EXPORT)
#define TROTL_EXPORT
#endif

#ifndef TROTL_EXPORT
#error export macro error: TROTL_EXPORT was not defined, disable EXPLICIT_EXPORT or define a export specification
#endif

namespace trotl
{
// Todo this needs to be fixed on windows
#if defined(TROTL_MAKE_DLL) || defined(__GNUC__)
extern int TROTL_EXPORT g_OCIPL_BULK_ROWS;
extern int TROTL_EXPORT g_OCIPL_MAX_LONG;
extern const char TROTL_EXPORT *g_TROTL_DEFAULT_NUM_FTM;
extern const char TROTL_EXPORT *g_TROTL_DEFAULT_DATE_FTM;
#else
int TROTL_EXPORT g_OCIPL_BULK_ROWS;
int TROTL_EXPORT g_OCIPL_MAX_LONG;
const char TROTL_EXPORT *g_TROTL_DEFAULT_NUM_FTM;
const char TROTL_EXPORT *g_TROTL_DEFAULT_DATE_FTM;
#endif
};

#endif
