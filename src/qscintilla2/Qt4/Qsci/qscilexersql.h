// This defines the interface to the QsciLexerSQL class.
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


#ifndef QSCILEXERSQL_H
#define QSCILEXERSQL_H

extern "C++" {

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexer.h>


//! \brief The QsciLexerSQL class encapsulates the Scintilla SQL lexer.
class QSCINTILLA_EXPORT QsciLexerSQL : public QsciLexer
{
    Q_OBJECT

public:
    //! This enum defines the meanings of the different styles used by the
    //! SQL lexer.
    enum {
        //! The default.
        Default = 0,

        //! A comment.
        Comment = 1,

        //! A line comment.
        CommentLine = 2,

        //! A JavaDoc/Doxygen style comment.
        CommentDoc = 3,

        //! A number.
        Number = 4,

        //! A keyword.
        Keyword = 5,

        //! A double-quoted string.
        DoubleQuotedString = 6,

        //! A single-quoted string.
        SingleQuotedString = 7,

        //! An SQL*Plus keyword.
        PlusKeyword = 8,

        //! An SQL*Plus prompt.
        PlusPrompt = 9,

        //! An operator.
        Operator = 10,

        //! An identifier
        Identifier = 11,

        //! An SQL*Plus comment.
        PlusComment = 13,

        //! A '#' line comment.
        CommentLineHash = 15,

        //! A JavaDoc/Doxygen keyword.
        CommentDocKeyword = 17,

        //! A JavaDoc/Doxygen keyword error.
        CommentDocKeywordError = 18,

        //! A keyword defined in keyword set number 5.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet5 = 19,

        //! A keyword defined in keyword set number 6.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet6 = 20,

        //! A keyword defined in keyword set number 7.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet7 = 21,

        //! A keyword defined in keyword set number 8.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet8 = 22
    };

    //! Construct a QsciLexerSQL with parent \a parent.  \a parent is typically
    //! the QsciScintilla instance.
    QsciLexerSQL(QObject *parent = 0);

    //! Destroys the QsciLexerSQL instance.
    virtual ~QsciLexerSQL();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the name of the lexer.  Some lexers support a number of
    //! languages.
    const char *lexer() const;

    //! \internal Returns the style used for braces for brace matching.
    int braceStyle() const;

    //! Returns the foreground colour of the text for style number \a style.
    //!
    //! \sa defaultPaper()
    QColor defaultColor(int style) const;

    //! Returns the end-of-line fill for style number \a style.
    bool defaultEolFill(int style) const;

    //! Returns the font for style number \a style.
    QFont defaultFont(int style) const;

    //! Returns the background colour of the text for style number \a style.
    //!
    //! \sa defaultColor()
    QColor defaultPaper(int style) const;

    //! Returns the set of keywords for the keyword set \a set recognised
    //! by the lexer as a space separated string.
    const char *keywords(int set) const;

    //! Returns the descriptive name for style number \a style.  If the
    //! style is invalid for this language then an empty QString is returned.
    //! This is intended to be used in user preference dialogs.
    QString description(int style) const;

    //! Causes all properties to be refreshed by emitting the
    //! propertyChanged() signal as required.
    void refreshProperties();

    //! Returns true if multi-line comment blocks can be folded.
    //!
    //! \sa setFoldComments()
    bool foldComments() const;

    //! Returns true if trailing blank lines are included in a fold block.
    //!
    //! \sa setFoldCompact()
    bool foldCompact() const;

    //! Returns true if backslash escapes are enabled.
    //!
    //! \sa setBackslashEscapes()
    bool backslashEscapes() const;

public slots:
    //! If \a fold is true then multi-line comment blocks can be folded.
    //! The default is false.
    //!
    //! \sa foldComments()
    virtual void setFoldComments(bool fold);

    //! If \a fold is true then trailing blank lines are included in a fold
    //! block. The default is true.
    //!
    //! \sa foldCompact()
    virtual void setFoldCompact(bool fold);

    //! If \a enable is true then backslash escapes are enabled.  The
    //! default is false.
    //!
    //! \sa backslashEscapes()
    virtual void setBackslashEscapes(bool enable);

protected:
    //! The lexer's properties are read from the settings \a qs.  \a prefix
    //! (which has a trailing '/') should be used as a prefix to the key of
    //! each setting.  true is returned if there is no error.
    //!
    bool readProperties(QSettings &qs,const QString &prefix);

    //! The lexer's properties are written to the settings \a qs.
    //! \a prefix (which has a trailing '/') should be used as a prefix to
    //! the key of each setting.  true is returned if there is no error.
    //!
    bool writeProperties(QSettings &qs,const QString &prefix) const;

private:
    void setCommentProp();
    void setCompactProp();
    void setBackslashEscapesProp();

    bool fold_comments;
    bool fold_compact;
    bool backslash_escapes;

    QsciLexerSQL(const QsciLexerSQL &);
    QsciLexerSQL &operator=(const QsciLexerSQL &);
};

}

#endif
