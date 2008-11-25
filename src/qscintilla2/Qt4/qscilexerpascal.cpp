// This module implements the QsciLexerPascal class.
//
// Copyright (c) 2008 Riverbank Computing Limited <info@riverbankcomputing.com>
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
// This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
// INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
// granted herein.
// 
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.


#include "Qsci/qscilexerpascal.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>


// The ctor.
QsciLexerPascal::QsciLexerPascal(QObject *parent)
    : QsciLexer(parent),
      fold_comments(false), fold_compact(true), fold_preproc(false)
{
}


// The dtor.
QsciLexerPascal::~QsciLexerPascal()
{
}


// Returns the language name.
const char *QsciLexerPascal::language() const
{
    return "Pascal";
}


// Returns the lexer name.
const char *QsciLexerPascal::lexer() const
{
    return "pascal";
}


// Return the set of character sequences that can separate auto-completion
// words.
QStringList QsciLexerPascal::autoCompletionWordSeparators() const
{
    QStringList wl;

    wl << "." << "^";

    return wl;
}


// Return the list of keywords that can start a block.
const char *QsciLexerPascal::blockStartKeyword(int *style) const
{
    if (style)
        *style = Keyword;

    return
        "case catch class default do else for then private protected public "
        "struct try union while type";
}


// Return the list of characters that can start a block.
const char *QsciLexerPascal::blockStart(int *style) const
{
    if (style)
        *style = Operator;

    return "begin";
}


// Return the list of characters that can end a block.
const char *QsciLexerPascal::blockEnd(int *style) const
{
    if (style)
        *style = Operator;

    return "end";
}


// Return the style used for braces.
int QsciLexerPascal::braceStyle() const
{
    return Operator;
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerPascal::defaultColor(int style) const
{
    switch (style)
    {
    case Default:
    case Operator:
        return QColor(0x00,0x00,0x00);

    case Comment:
    case CommentLine:
        return QColor(0x00,0x7f,0x00);

    case CommentDoc:
        return QColor(0x7f,0x7f,0x7f);

    case Number:
        return QColor(0x00,0x7f,0x7f);

    case Keyword:
        return QColor(0x00,0x00,0x7f);

    case SingleQuotedString:
        return QColor(0x7f,0x00,0x7f);

    case PreProcessor:
        return QColor(0x7f,0x7f,0x00);

    case Identifier:
        break;

    case Asm:
        return QColor(0x00,0x80,0x80);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the font of the text for a style.
QFont QsciLexerPascal::defaultFont(int style) const
{
    QFont f;

    switch (style)
    {
    case Comment:
    case CommentLine:
#if defined(Q_OS_WIN)
        f = QFont("Comic Sans MS",9);
#else
        f = QFont("Bitstream Vera Serif",9);
#endif
        break;

    case Keyword:
        f = QsciLexer::defaultFont(style);
        f.setBold(true);
        break;

    case SingleQuotedString:
#if defined(Q_OS_WIN)
        f = QFont("Times New Roman", 11);
#else
        f = QFont("Bitstream Charter", 10);
#endif
        f.setItalic(true);
        break;

    default:
        f = QsciLexer::defaultFont(style);
    }

    return f;
}


// Returns the set of keywords.
const char *QsciLexerPascal::keywords(int set) const
{
    if (set == 1)
        return
            "and array asm begin case cdecl class const constructor contains "
            "default destructor div do downto else end end. except exit "
            "exports external far file finalization finally for function goto "
            "if implementation in index inherited initialization inline "
            "interface label library message mod near nil not object of on or "
            "out overload override package packed pascal private procedure "
            "program property protected public published raise read record "
            "register repeat requires resourcestring safecall set shl shr "
            "stdcall stored string then threadvar to try type unit until uses "
            "var virtual while with write xor";

    if (set == 2)
        return
            "write read default public protected private property published "
            "stored";

    return 0;
}


// Returns the user name of a style.
QString QsciLexerPascal::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Comment:
        return tr("Comment");

    case CommentLine:
        return tr("Line comment");

    case CommentDoc:
        return tr("JavaDoc style comment");

    case Number:
        return tr("Number");

    case Keyword:
        return tr("Keyword");

    case SingleQuotedString:
        return tr("Single-quoted string");

    case PreProcessor:
        return tr("Pre-processor block");

    case Operator:
        return tr("Operator");

    case Identifier:
        return tr("Identifier");

    case Asm:
        return tr("Inline asm");
    }

    return QString();
}


// Refresh all properties.
void QsciLexerPascal::refreshProperties()
{
    setCommentProp();
    setCompactProp();
    setPreprocProp();
}


// Read properties from the settings.
bool QsciLexerPascal::readProperties(QSettings &qs,const QString &prefix)
{
    int rc = true;

    fold_comments = qs.value(prefix + "foldcomments", false).toBool();
    fold_compact = qs.value(prefix + "foldcompact", true).toBool();
    fold_preproc = qs.value(prefix + "foldpreprocessor", true).toBool();

    return rc;
}


// Write properties to the settings.
bool QsciLexerPascal::writeProperties(QSettings &qs,const QString &prefix) const
{
    int rc = true;

    qs.setValue(prefix + "foldcomments", fold_comments);
    qs.setValue(prefix + "foldcompact", fold_compact);
    qs.setValue(prefix + "foldpreprocessor", fold_preproc);

    return rc;
}


// Return true if comments can be folded.
bool QsciLexerPascal::foldComments() const
{
    return fold_comments;
}


// Set if comments can be folded.
void QsciLexerPascal::setFoldComments(bool fold)
{
    fold_comments = fold;

    setCommentProp();
}


// Set the "fold.comment" property.
void QsciLexerPascal::setCommentProp()
{
    emit propertyChanged("fold.comment",(fold_comments ? "1" : "0"));
}


// Return true if folds are compact.
bool QsciLexerPascal::foldCompact() const
{
    return fold_compact;
}


// Set if folds are compact
void QsciLexerPascal::setFoldCompact(bool fold)
{
    fold_compact = fold;

    setCompactProp();
}


// Set the "fold.compact" property.
void QsciLexerPascal::setCompactProp()
{
    emit propertyChanged("fold.compact",(fold_compact ? "1" : "0"));
}


// Return true if preprocessor blocks can be folded.
bool QsciLexerPascal::foldPreprocessor() const
{
    return fold_preproc;
}


// Set if preprocessor blocks can be folded.
void QsciLexerPascal::setFoldPreprocessor(bool fold)
{
    fold_preproc = fold;

    setPreprocProp();
}


// Set the "fold.preprocessor" property.
void QsciLexerPascal::setPreprocProp()
{
    emit propertyChanged("fold.preprocessor",(fold_preproc ? "1" : "0"));
}
