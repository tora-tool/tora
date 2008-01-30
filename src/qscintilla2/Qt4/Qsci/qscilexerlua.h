// This defines the interface to the QsciLexerLua class.
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


#ifndef QSCILEXERLUA_H
#define QSCILEXERLUA_H

extern "C++" {

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexer.h>


//! \brief The QsciLexerLua class encapsulates the Scintilla Lua
//! lexer.
class QSCINTILLA_EXPORT QsciLexerLua : public QsciLexer
{
    Q_OBJECT

public:
    //! This enum defines the meanings of the different styles used by the
    //! Lua lexer.
    enum {
        //! The default.
        Default = 0,

        //! A block comment.
        Comment = 1,

        //! A line comment.
        LineComment = 2,

        //! A number.
        Number = 4,

        //! A keyword.
        Keyword = 5,

        //! A string.
        String = 6,

        //! A character.
        Character = 7,

        //! A literal string.
        LiteralString = 8,

        //! Preprocessor
        Preprocessor = 9,

        //! An operator.
        Operator = 10,

        //! An identifier
        Identifier = 11,

        //! The end of a line where a string is not closed.
        UnclosedString = 12,

        //! Basic functions.
        BasicFunctions = 13,

        //! String, table and maths functions.
        StringTableMathsFunctions = 14,

        //! Coroutines, I/O and system facilities.
        CoroutinesIOSystemFacilities = 15
    };

    //! Construct a QsciLexerLua with parent \a parent.  \a parent is typically
    //! the QsciScintilla instance.
    QsciLexerLua(QObject *parent = 0);

    //! Destroys the QsciLexerLua instance.
    virtual ~QsciLexerLua();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the name of the lexer.  Some lexers support a number of
    //! languages.
    const char *lexer() const;

    //! \internal Returns a space separated list of words or characters in
    //! a particular style that define the start of a block for
    //! auto-indentation.  The styles is returned via \a style.
    const char *blockStart(int *style = 0) const;

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

    //! Returns true if trailing blank lines are included in a fold block.
    //!
    //! \sa setFoldCompact()
    bool foldCompact() const;

public slots:
    //! If \a fold is true then trailing blank lines are included in a fold
    //! block. The default is true.
    //!
    //! \sa foldCompact()
    virtual void setFoldCompact(bool fold);

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
    void setCompactProp();

    bool fold_compact;

    QsciLexerLua(const QsciLexerLua &);
    QsciLexerLua &operator=(const QsciLexerLua &);
};

}

#endif
