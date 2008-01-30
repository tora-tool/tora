// This module defines various things common to all of the Scintilla Qt port.
//
// Copyright (c) 2007
// 	Phil Thompson <phil@river-bank.demon.co.uk>
// 
// This file is part of QScintilla.
// 
// This copy of QScintilla is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option) any
// later version.
// 
// QScintilla is supplied in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
// details.
// 
// You should have received a copy of the GNU General Public License along with
// QScintilla; see the file LICENSE.  If not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.


#ifndef QSCIGLOBAL_H
#define QSCIGLOBAL_H

extern "C++" {

#include <qglobal.h>


#define QSCINTILLA_VERSION      0x020100
#define QSCINTILLA_VERSION_STR  "2-snapshot-20070916"


// Under Windows, define QSCINTILLA_MAKE_DLL to create a Scintilla DLL, or
// define QSCINTILLA_DLL to link against a Scintilla DLL, or define neither
// to either build or link against a static Scintilla library.
#if defined(Q_WS_WIN)

#if defined(QSCINTILLA_DLL)
#define QSCINTILLA_EXPORT       __declspec(dllimport)
#elif defined(QSCINTILLA_MAKE_DLL)
#define QSCINTILLA_EXPORT       __declspec(dllexport)
#endif

#endif

#if !defined(QSCINTILLA_EXPORT)
#define QSCINTILLA_EXPORT
#endif

}

#endif
