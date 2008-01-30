// This module implements the QsciLexerMakefile class.
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


#include "Qsci/qscilexermakefile.h"

#include <qcolor.h>
#include <qfont.h>


// The ctor.
QsciLexerMakefile::QsciLexerMakefile(QObject *parent)
    : QsciLexer(parent)
{
}


// The dtor.
QsciLexerMakefile::~QsciLexerMakefile()
{
}


// Returns the language name.
const char *QsciLexerMakefile::language() const
{
    return "Makefile";
}


// Returns the lexer name.
const char *QsciLexerMakefile::lexer() const
{
    return "makefile";
}


// Return the string of characters that comprise a word.
const char *QsciLexerMakefile::wordCharacters() const
{
    return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-";
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerMakefile::defaultColor(int style) const
{
    switch (style)
    {
    case Default:
    case Operator:
        return QColor(0x00,0x00,0x00);

    case Comment:
        return QColor(0x00,0x7f,0x00);

    case Preprocessor:
        return QColor(0x7f,0x7f,0x00);

    case Variable:
        return QColor(0x00,0x00,0x80);

    case Target:
        return QColor(0xa0,0x00,0x00);

    case Error:
        return QColor(0xff,0xff,0x00);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the end-of-line fill for a style.
bool QsciLexerMakefile::defaultEolFill(int style) const
{
    if (style == Error)
        return true;

    return QsciLexer::defaultEolFill(style);
}


// Returns the font of the text for a style.
QFont QsciLexerMakefile::defaultFont(int style) const
{
    QFont f;

    if (style == Comment)
#if defined(Q_OS_WIN)
        f = QFont("Comic Sans MS",9);
#else
        f = QFont("Bitstream Vera Serif",9);
#endif
    else
        f = QsciLexer::defaultFont(style);

    return f;
}


// Returns the user name of a style.
QString QsciLexerMakefile::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case Preprocessor:
        return tr("Preprocessor");

    case Variable:
        return tr("Variable");

    case Operator:
        return tr("Operator");

    case Target:
        return tr("Target");

    case Error:
        return tr("Error");
    }

    return QString();
}


// Returns the background colour of the text for a style.
QColor QsciLexerMakefile::defaultPaper(int style) const
{
    if (style == Error)
        return QColor(0xff,0x00,0x00);

    return QsciLexer::defaultPaper(style);
}
