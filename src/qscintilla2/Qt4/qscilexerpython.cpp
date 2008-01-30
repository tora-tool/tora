// This module implements the QsciLexerPython class.
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


#include "Qsci/qscilexerpython.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>


// The list of Python keywords that can be used by other friendly lexers.
const char *QsciLexerPython::keywordClass =
    "and assert break class continue def del elif else except exec "
    "finally for from global if import in is lambda None not or pass "
    "print raise return try while yield";


// The ctor.
QsciLexerPython::QsciLexerPython(QObject *parent)
    : QsciLexer(parent),
      fold_comments(false), fold_quotes(false), indent_warn(NoWarning)
{
}


// The dtor.
QsciLexerPython::~QsciLexerPython()
{
}


// Returns the language name.
const char *QsciLexerPython::language() const
{
    return "Python";
}


// Returns the lexer name.
const char *QsciLexerPython::lexer() const
{
    return "python";
}


// Return the set of character sequences that can separate auto-completion
// words.
QStringList QsciLexerPython::autoCompletionWordSeparators() const
{
    QStringList wl;

    wl << ".";

    return wl;
}


// Return the list of characters that can start a block.
const char *QsciLexerPython::blockStart(int *style) const
{
    if (style)
        *style = Operator;

    return ":";
}


// Return the number of lines to look back when auto-indenting.
int QsciLexerPython::blockLookback() const
{
    // This must be 0 otherwise de-indenting a Python block gets very
    // difficult.
    return 0;
}


// Return the style used for braces.
int QsciLexerPython::braceStyle() const
{
    return Operator;
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerPython::defaultColor(int style) const
{
    switch (style)
    {
    case Default:
        return QColor(0x80,0x80,0x80);

    case Comment:
        return QColor(0x00,0x7f,0x00);

    case Number:
        return QColor(0x00,0x7f,0x7f);

    case DoubleQuotedString:
    case SingleQuotedString:
        return QColor(0x7f,0x00,0x7f);

    case Keyword:
        return QColor(0x00,0x00,0x7f);

    case TripleSingleQuotedString:
    case TripleDoubleQuotedString:
        return QColor(0x7f,0x00,0x00);

    case ClassName:
        return QColor(0x00,0x00,0xff);

    case FunctionMethodName:
        return QColor(0x00,0x7f,0x7f);

    case Operator:
    case Identifier:
        break;

    case CommentBlock:
        return QColor(0x7f,0x7f,0x7f);

    case UnclosedString:
        return QColor(0x00,0x00,0x00);

    case HighlightedIdentifier:
        return QColor(0x40,0x70,0x90);

    case Decorator:
        return QColor(0x80,0x50,0x00);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the end-of-line fill for a style.
bool QsciLexerPython::defaultEolFill(int style) const
{
    if (style == UnclosedString)
        return true;

    return QsciLexer::defaultEolFill(style);
}


// Returns the font of the text for a style.
QFont QsciLexerPython::defaultFont(int style) const
{
    QFont f;

    switch (style)
    {
    case Comment:
#if defined(Q_OS_WIN)
        f = QFont("Comic Sans MS",9);
#else
        f = QFont("Bitstream Vera Serif",9);
#endif
        break;

    case DoubleQuotedString:
    case SingleQuotedString:
    case UnclosedString:
#if defined(Q_OS_WIN)
        f = QFont("Courier New",10);
#else
        f = QFont("Bitstream Vera Sans Mono",9);
#endif
        break;

    case Keyword:
    case ClassName:
    case FunctionMethodName:
    case Operator:
        f = QsciLexer::defaultFont(style);
        f.setBold(true);
        break;

    default:
        f = QsciLexer::defaultFont(style);
    }

    return f;
}


// Returns the set of keywords.
const char *QsciLexerPython::keywords(int set) const
{
    if (set != 1)
        return 0;

    return keywordClass;
}


// Returns the user name of a style.
QString QsciLexerPython::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case Number:
        return tr("Number");

    case DoubleQuotedString:
        return tr("Double-quoted string");

    case SingleQuotedString:
        return tr("Single-quoted string");

    case Keyword:
        return tr("Keyword");

    case TripleSingleQuotedString:
        return tr("Triple single-quoted string");

    case TripleDoubleQuotedString:
        return tr("Triple double-quoted string");

    case ClassName:
        return tr("Class name");

    case FunctionMethodName:
        return tr("Function or method name");

    case Operator:
        return tr("Operator");

    case Identifier:
        return tr("Identifier");

    case CommentBlock:
        return tr("Comment block");

    case UnclosedString:
        return tr("Unclosed string");

    case HighlightedIdentifier:
        return tr("Highlighted identifier");

    case Decorator:
        return tr("Decorator");
    }

    return QString();
}


// Returns the background colour of the text for a style.
QColor QsciLexerPython::defaultPaper(int style) const
{
    if (style == UnclosedString)
        return QColor(0xe0,0xc0,0xe0);

    return QsciLexer::defaultPaper(style);
}


// Refresh all properties.
void QsciLexerPython::refreshProperties()
{
    setCommentProp();
    setQuotesProp();
    setTabWhingeProp();
}


// Read properties from the settings.
bool QsciLexerPython::readProperties(QSettings &qs,const QString &prefix)
{
    int rc = true, num;

    fold_comments = qs.value(prefix + "foldcomments", false).toBool();
    fold_quotes = qs.value(prefix + "foldquotes", false).toBool();
    indent_warn = (IndentationWarning)qs.value(prefix + "indentwarning", (int)NoWarning).toInt();

    return rc;
}


// Write properties to the settings.
bool QsciLexerPython::writeProperties(QSettings &qs,const QString &prefix) const
{
    int rc = true;

    qs.setValue(prefix + "foldcomments", fold_comments);
    qs.setValue(prefix + "foldquotes", fold_quotes);
    qs.setValue(prefix + "indentwarning", (int)indent_warn);

    return rc;
}


// Return true if comments can be folded.
bool QsciLexerPython::foldComments() const
{
    return fold_comments;
}


// Set if comments can be folded.
void QsciLexerPython::setFoldComments(bool fold)
{
    fold_comments = fold;

    setCommentProp();
}


// Set the "fold.comment.python" property.
void QsciLexerPython::setCommentProp()
{
    emit propertyChanged("fold.comment.python",(fold_comments ? "1" : "0"));
}


// Return true if quotes can be folded.
bool QsciLexerPython::foldQuotes() const
{
    return fold_quotes;
}


// Set if quotes can be folded.
void QsciLexerPython::setFoldQuotes(bool fold)
{
    fold_quotes = fold;

    setQuotesProp();
}


// Set the "fold.quotes.python" property.
void QsciLexerPython::setQuotesProp()
{
    emit propertyChanged("fold.quotes.python",(fold_quotes ? "1" : "0"));
}


// Return the indentation warning.
QsciLexerPython::IndentationWarning QsciLexerPython::indentationWarning() const
{
    return indent_warn;
}


// Set the indentation warning.
void QsciLexerPython::setIndentationWarning(QsciLexerPython::IndentationWarning warn)
{
    indent_warn = warn;

    setTabWhingeProp();
}


// Set the "tab.timmy.whinge.level" property.
void QsciLexerPython::setTabWhingeProp()
{
    emit propertyChanged("tab.timmy.whinge.level", QByteArray::number(indent_warn));
}
