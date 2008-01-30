// This module implements the QsciLexerCPP class.
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


#include "Qsci/qscilexercpp.h"

#include <qcolor.h>
#include <qfont.h>
#include <qsettings.h>


// The ctor.
QsciLexerCPP::QsciLexerCPP(QObject *parent, bool caseInsensitiveKeywords)
    : QsciLexer(parent),
      fold_atelse(false), fold_comments(false), fold_compact(true),
      fold_preproc(true), style_preproc(false), nocase(caseInsensitiveKeywords)
{
}


// The dtor.
QsciLexerCPP::~QsciLexerCPP()
{
}


// Returns the language name.
const char *QsciLexerCPP::language() const
{
    return "C++";
}


// Returns the lexer name.
const char *QsciLexerCPP::lexer() const
{
    return (nocase ? "cppnocase" : "cpp");
}


// Return the set of character sequences that can separate auto-completion
// words.
QStringList QsciLexerCPP::autoCompletionWordSeparators() const
{
    QStringList wl;

    wl << "::" << "->" << ".";

    return wl;
}


// Return the list of keywords that can start a block.
const char *QsciLexerCPP::blockStartKeyword(int *style) const
{
    if (style)
        *style = Keyword;

    return "case catch class default do else finally for if private "
           "protected public struct try union while";
}


// Return the list of characters that can start a block.
const char *QsciLexerCPP::blockStart(int *style) const
{
    if (style)
        *style = Operator;

    return "{";
}


// Return the list of characters that can end a block.
const char *QsciLexerCPP::blockEnd(int *style) const
{
    if (style)
        *style = Operator;

    return "}";
}


// Return the style used for braces.
int QsciLexerCPP::braceStyle() const
{
    return Operator;
}


// Return the string of characters that comprise a word.
const char *QsciLexerCPP::wordCharacters() const
{
    return "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_#";
}


// Returns the foreground colour of the text for a style.
QColor QsciLexerCPP::defaultColor(int style) const
{
    switch (style)
    {
    case Default:
        return QColor(0x80,0x80,0x80);

    case Comment:
    case CommentLine:
        return QColor(0x00,0x7f,0x00);

    case CommentDoc:
    case CommentLineDoc:
        return QColor(0x3f,0x70,0x3f);

    case Number:
        return QColor(0x00,0x7f,0x7f);

    case Keyword:
        return QColor(0x00,0x00,0x7f);

    case DoubleQuotedString:
    case SingleQuotedString:
        return QColor(0x7f,0x00,0x7f);

    case PreProcessor:
        return QColor(0x7f,0x7f,0x00);

    case Operator:
    case UnclosedString:
        return QColor(0x00,0x00,0x00);

    case Identifier:
        break;

    case Regex:
        return QColor(0x3f,0x7f,0x3f);

    case CommentDocKeyword:
        return QColor(0x30,0x60,0xa0);

    case CommentDocKeywordError:
        return QColor(0x80,0x40,0x20);
    }

    return QsciLexer::defaultColor(style);
}


// Returns the end-of-line fill for a style.
bool QsciLexerCPP::defaultEolFill(int style) const
{
    if (style == UnclosedString)
        return true;

    return QsciLexer::defaultEolFill(style);
}


// Returns the font of the text for a style.
QFont QsciLexerCPP::defaultFont(int style) const
{
    QFont f;

    switch (style)
    {
    case Comment:
    case CommentLine:
    case CommentDoc:
    case CommentLineDoc:
    case CommentDocKeyword:
    case CommentDocKeywordError:
#if defined(Q_OS_WIN)
        f = QFont("Comic Sans MS",9);
#else
        f = QFont("Bitstream Vera Serif",9);
#endif
        break;

    case Keyword:
    case Operator:
        f = QsciLexer::defaultFont(style);
        f.setBold(true);
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

    default:
        f = QsciLexer::defaultFont(style);
    }

    return f;
}


// Returns the set of keywords.
const char *QsciLexerCPP::keywords(int set) const
{
    if (set == 1)
        return
            "and and_eq asm auto bitand bitor bool break case "
            "catch char class compl const const_cast continue "
            "default delete do double dynamic_cast else enum "
            "explicit export extern false float for friend goto if "
            "inline int long mutable namespace new not not_eq "
            "operator or or_eq private protected public register "
            "reinterpret_cast return short signed sizeof static "
            "static_cast struct switch template this throw true "
            "try typedef typeid typename union unsigned using "
            "virtual void volatile wchar_t while xor xor_eq";

    if (set == 3)
        return
            "a addindex addtogroup anchor arg attention author b "
            "brief bug c class code date def defgroup deprecated "
            "dontinclude e em endcode endhtmlonly endif "
            "endlatexonly endlink endverbatim enum example "
            "exception f$ f[ f] file fn hideinitializer "
            "htmlinclude htmlonly if image include ingroup "
            "internal invariant interface latexonly li line link "
            "mainpage name namespace nosubgrouping note overload "
            "p page par param post pre ref relates remarks return "
            "retval sa section see showinitializer since skip "
            "skipline struct subsection test throw todo typedef "
            "union until var verbatim verbinclude version warning "
            "weakgroup $ @ \\ & < > # { }";

    return 0;
}


// Returns the user name of a style.
QString QsciLexerCPP::description(int style) const
{
    switch (style)
    {
    case Default:
        return tr("Default");

    case Comment:
        return tr("C comment");

    case CommentLine:
        return tr("C++ comment");

    case CommentDoc:
        return tr("JavaDoc style C comment");

    case Number:
        return tr("Number");

    case Keyword:
        return tr("Keyword");

    case DoubleQuotedString:
        return tr("Double-quoted string");

    case SingleQuotedString:
        return tr("Single-quoted string");

    case PreProcessor:
        return tr("Pre-processor block");

    case Operator:
        return tr("Operator");

    case Identifier:
        return tr("Identifier");

    case UnclosedString:
        return tr("Unclosed string");

    case CommentLineDoc:
        return tr("JavaDoc style C++ comment");

    case KeywordSet2:
        return tr("Secondary keywords and identifiers");

    case CommentDocKeyword:
        return tr("JavaDoc keyword");

    case CommentDocKeywordError:
        return tr("JavaDoc keyword error");

    case GlobalClass:
        return tr("Global classes and typedefs");
    }

    return QString();
}


// Returns the background colour of the text for a style.
QColor QsciLexerCPP::defaultPaper(int style) const
{
    if (style == UnclosedString)
        return QColor(0xe0,0xc0,0xe0);

    return QsciLexer::defaultPaper(style);
}


// Refresh all properties.
void QsciLexerCPP::refreshProperties()
{
    setAtElseProp();
    setCommentProp();
    setCompactProp();
    setPreprocProp();
    setStylePreprocProp();
}


// Read properties from the settings.
bool QsciLexerCPP::readProperties(QSettings &qs,const QString &prefix)
{
    int rc = true;

    fold_atelse = qs.value(prefix + "foldatelse", false).toBool();
    fold_comments = qs.value(prefix + "foldcomments", false).toBool();
    fold_compact = qs.value(prefix + "foldcompact", true).toBool();
    fold_preproc = qs.value(prefix + "foldpreprocessor", true).toBool();
    style_preproc = qs.value(prefix + "stylepreprocessor", false).toBool();

    return rc;
}


// Write properties to the settings.
bool QsciLexerCPP::writeProperties(QSettings &qs,const QString &prefix) const
{
    int rc = true;

    qs.setValue(prefix + "foldatelse", fold_atelse);
    qs.setValue(prefix + "foldcomments", fold_comments);
    qs.setValue(prefix + "foldcompact", fold_compact);
    qs.setValue(prefix + "foldpreprocessor", fold_preproc);
    qs.setValue(prefix + "stylepreprocessor", style_preproc);

    return rc;
}


// Return true if else can be folded.
bool QsciLexerCPP::foldAtElse() const
{
    return fold_atelse;
}


// Set if else can be folded.
void QsciLexerCPP::setFoldAtElse(bool fold)
{
    fold_atelse = fold;

    setAtElseProp();
}


// Set the "fold.at.else" property.
void QsciLexerCPP::setAtElseProp()
{
    emit propertyChanged("fold.at.else",(fold_atelse ? "1" : "0"));
}


// Return true if comments can be folded.
bool QsciLexerCPP::foldComments() const
{
    return fold_comments;
}


// Set if comments can be folded.
void QsciLexerCPP::setFoldComments(bool fold)
{
    fold_comments = fold;

    setCommentProp();
}


// Set the "fold.comment" property.
void QsciLexerCPP::setCommentProp()
{
    emit propertyChanged("fold.comment",(fold_comments ? "1" : "0"));
}


// Return true if folds are compact.
bool QsciLexerCPP::foldCompact() const
{
    return fold_compact;
}


// Set if folds are compact
void QsciLexerCPP::setFoldCompact(bool fold)
{
    fold_compact = fold;

    setCompactProp();
}


// Set the "fold.compact" property.
void QsciLexerCPP::setCompactProp()
{
    emit propertyChanged("fold.compact",(fold_compact ? "1" : "0"));
}


// Return true if preprocessor blocks can be folded.
bool QsciLexerCPP::foldPreprocessor() const
{
    return fold_preproc;
}


// Set if preprocessor blocks can be folded.
void QsciLexerCPP::setFoldPreprocessor(bool fold)
{
    fold_preproc = fold;

    setPreprocProp();
}


// Set the "fold.preprocessor" property.
void QsciLexerCPP::setPreprocProp()
{
    emit propertyChanged("fold.preprocessor",(fold_preproc ? "1" : "0"));
}


// Return true if preprocessor lines are styled.
bool QsciLexerCPP::stylePreprocessor() const
{
    return style_preproc;
}


// Set if preprocessor lines are styled.
void QsciLexerCPP::setStylePreprocessor(bool style)
{
    style_preproc = style;

    setStylePreprocProp();
}


// Set the "style.within.preprocessor" property.
void QsciLexerCPP::setStylePreprocProp()
{
    emit propertyChanged("style.within.preprocessor",(style_preproc ? "1" : "0"));
}
