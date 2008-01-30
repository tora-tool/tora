// This module implements the QsciLexerProperties class.
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


#include "Qsci/qscilexerproperties.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>


// The ctor.
QsciLexerProperties::QsciLexerProperties(QObject *parent)
    : QsciLexer(parent),
      fold_compact(true)
{
}


// The dtor.
QsciLexerProperties::~QsciLexerProperties()
{
}


// Returns the language name.
const char *QsciLexerProperties::language() const
{
    return "Properties";
}


// Returns the lexer name.
const char *QsciLexerProperties::lexer() const
{
    return "props";
}


// Return the string of characters that comprise a word.
const char *QsciLexerProperties::wordCharacters() const
{
    return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-";
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerProperties::defaultColor(int style) const
{
    switch (style)
    {
    case Comment:
        return QColor(0x00,0x7f,0x7f);

    case Section:
        return QColor(0x7f,0x00,0x7f);

    case Assignment:
        return QColor(0xb0,0x60,0x00);

    case DefaultValue:
        return QColor(0x7f,0x7f,0x00);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the end-of-line fill for a style.
bool QsciLexerProperties::defaultEolFill(int style) const
{
    if (style == Section)
        return true;

    return QsciLexer::defaultEolFill(style);
}


// Returns the font of the text for a style.
QFont QsciLexerProperties::defaultFont(int style) const
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
QString QsciLexerProperties::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case Section:
        return tr("Section");

    case Assignment:
        return tr("Assignment");

    case DefaultValue:
        return tr("Default value");
    }

    return QString();
}


// Returns the background colour of the text for a style.
QColor QsciLexerProperties::defaultPaper(int style) const
{
    if (style == Section)
        return QColor(0xe0,0xf0,0xf0);

    return QsciLexer::defaultPaper(style);
}


// Refresh all properties.
void QsciLexerProperties::refreshProperties()
{
    setCompactProp();
}


// Read properties from the settings.
bool QsciLexerProperties::readProperties(QSettings &qs,const QString &prefix)
{
    int rc = true;

    fold_compact = qs.value(prefix + "foldcompact", true).toBool();

    return rc;
}


// Write properties to the settings.
bool QsciLexerProperties::writeProperties(QSettings &qs,const QString &prefix) const
{
    int rc = true;

    qs.setValue(prefix + "foldcompact", fold_compact);

    return rc;
}


// Return true if folds are compact.
bool QsciLexerProperties::foldCompact() const
{
    return fold_compact;
}


// Set if folds are compact
void QsciLexerProperties::setFoldCompact(bool fold)
{
    fold_compact = fold;

    setCompactProp();
}


// Set the "fold.compact" property.
void QsciLexerProperties::setCompactProp()
{
    emit propertyChanged("fold.compact",(fold_compact ? "1" : "0"));
}
