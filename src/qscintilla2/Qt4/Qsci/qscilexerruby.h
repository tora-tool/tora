// This defines the interface to the QsciLexerRuby class.
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


#ifndef QSCILEXERRUBY_H
#define QSCILEXERRUBY_H

extern "C++" {

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexer.h>


//! \brief The QsciLexerRuby class encapsulates the Scintilla Ruby lexer.
class QSCINTILLA_EXPORT QsciLexerRuby : public QsciLexer
{
    Q_OBJECT

public:
    //! This enum defines the meanings of the different styles used by the
    //! Ruby lexer.
    enum {
        //! The default.
        Default = 0,

        //! An error.
        Error = 1,

        //! A comment.
        Comment = 2,

        //! A POD.
        POD = 3,

        //! A number.
        Number = 4,

        //! A keyword.
        Keyword = 5,

        //! A double-quoted string.
        DoubleQuotedString = 6,

        //! A single-quoted string.
        SingleQuotedString = 7,

        //! The name of a class.
        ClassName = 8,

        //! The name of a function or method.
        FunctionMethodName = 9,

        //! An operator.
        Operator = 10,

        //! An identifier
        Identifier = 11,

        //! A regular expression.
        Regex = 12,

        //! A global.
        Global = 13,

        //! A symbol.
        Symbol = 14,

        //! The name of a module.
        ModuleName = 15,

        //! An instance variable.
        InstanceVariable = 16,

        //! A class variable.
        ClassVariable = 17,

        //! Backticks.
        Backticks = 18,

        //! A data section.
        DataSection = 19,

        //! A here document delimiter.
        HereDocumentDelimiter = 20,

        //! A here document.
        HereDocument = 21,

        //! A %q string.
        PercentStringq = 24,

        //! A %Q string.
        PercentStringQ = 25,

        //! A %x string.
        PercentStringx = 26,

        //! A %r string.
        PercentStringr = 27,

        //! A %w string.
        PercentStringw = 28,

        //! A demoted keyword.
        DemotedKeyword = 29,

        //! stdin.
        Stdin = 30,

        //! stdout.
        Stdout = 31,

        //! stderr.
        Stderr = 40
    };

    //! Construct a QsciLexerRuby with parent \a parent.  \a parent is
    //! typically the QsciScintilla instance.
    QsciLexerRuby(QObject *parent = 0);

    //! Destroys the QsciLexerRuby instance.
    virtual ~QsciLexerRuby();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the name of the lexer.  Some lexers support a number of
    //! languages.
    const char *lexer() const;

    //! \internal Returns a space separated list of words or characters in
    //! a particular style that define the end of a block for
    //! auto-indentation.  The style is returned via \a style.
    const char *blockEnd(int *style = 0) const;

    //! \internal Returns a space separated list of words or characters in
    //! a particular style that define the start of a block for
    //! auto-indentation.  The styles is returned via \a style.
    const char *blockStart(int *style = 0) const;

    //! \internal Returns a space separated list of keywords in a
    //! particular style that define the start of a block for
    //! auto-indentation.  The style is returned via \a style.
    const char *blockStartKeyword(int *style = 0) const;

    //! \internal Returns the style used for braces for brace matching.
    int braceStyle() const;

    //! Returns the foreground colour of the text for style number \a style.
    //!
    //! \sa defaultpaper()
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

    //! Returns the descriptive name for style number \a style.  If the style
    //! is invalid for this language then an empty QString is returned.  This
    //! is intended to be used in user preference dialogs.
    QString description(int style) const;

private:
    QsciLexerRuby(const QsciLexerRuby &);
    QsciLexerRuby &operator=(const QsciLexerRuby &);
};

}

#endif
