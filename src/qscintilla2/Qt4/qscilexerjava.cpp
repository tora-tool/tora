// This module implements the QsciLexerJava class.
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


#include "Qsci/qscilexerjava.h"


// The ctor.
QsciLexerJava::QsciLexerJava(QObject *parent)
    : QsciLexerCPP(parent)
{
}


// The dtor.
QsciLexerJava::~QsciLexerJava()
{
}


// Returns the language name.
const char *QsciLexerJava::language() const
{
    return "Java";
}


// Returns the set of keywords.
const char *QsciLexerJava::keywords(int set) const
{
    if (set != 1)
        return 0;

    return "abstract assert boolean break byte case catch char class "
           "const continue default do double else extends final finally "
           "float for future generic goto if implements import inner "
           "instanceof int interface long native new null operator outer "
           "package private protected public rest return short static "
           "super switch synchronized this throw throws transient try var "
           "void volatile while";
}
