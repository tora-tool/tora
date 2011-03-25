// This module implements the QsciLexerProperties class.
//
// Copyright (c) 2010 Riverbank Computing Limited <info@riverbankcomputing.com>
// 
// This file is part of QScintilla.
// 
// This file may be used under the terms of the GNU General Public
// License versions 2.0 or 3.0 as published by the Free Software
// Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
// included in the packaging of this file.  Alternatively you may (at
// your option) use any later version of the GNU General Public
// License if such license has been publicly approved by Riverbank
// Computing Limited (or its successors, if any) and the KDE Free Qt
// Foundation. In addition, as a special exception, Riverbank gives you
// certain additional rights. These rights are described in the Riverbank
// GPL Exception version 1.1, which can be found in the file
// GPL_EXCEPTION.txt in this package.
// 
// Please review the following information to ensure GNU General
// Public Licensing requirements will be met:
// http://trolltech.com/products/qt/licenses/licensing/opensource/. If
// you are unsure which license is appropriate for your use, please
// review the following information:
// http://trolltech.com/products/qt/licenses/licensing/licensingoverview
// or contact the sales department at sales@riverbankcomputing.com.
// 
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


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

    case Key:
        return tr("Key");
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
