// This module implements the QsciLexerDiff class.
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


#include "Qsci/qscilexerdiff.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>


// The ctor.
QsciLexerDiff::QsciLexerDiff(QObject *parent)
    : QsciLexer(parent)
{
}


// The dtor.
QsciLexerDiff::~QsciLexerDiff()
{
}


// Returns the language name.
const char *QsciLexerDiff::language() const
{
    return "Diff";
}


// Returns the lexer name.
const char *QsciLexerDiff::lexer() const
{
    return "diff";
}


// Return the string of characters that comprise a word.
const char *QsciLexerDiff::wordCharacters() const
{
    return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-";
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerDiff::defaultColor(int style) const
{
    switch (style)
    {
    case Default:
        return QColor(0x00,0x00,0x00);

    case Comment:
        return QColor(0x00,0x7f,0x00);

    case Command:
        return QColor(0x7f,0x7f,0x00);

    case Header:
        return QColor(0x7f,0x00,0x00);

    case Position:
        return QColor(0x7f,0x00,0x7f);

    case LineRemoved:
        return QColor(0x00,0x7f,0x7f);

    case LineAdded:
        return QColor(0x00,0x00,0x7f);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the user name of a style.
QString QsciLexerDiff::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case Command:
        return tr("Command");

    case Header:
        return tr("Header");

    case Position:
        return tr("Position");

    case LineRemoved:
        return tr("Removed line");

    case LineAdded:
        return tr("Added line");
    }

    return QString();
}
