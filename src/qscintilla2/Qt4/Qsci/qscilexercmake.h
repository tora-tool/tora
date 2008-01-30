// This defines the interface to the QsciLexerCMake class.
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


#ifndef QSCILEXERCMAKE_H
#define QSCILEXERCMAKE_H

extern "C++" {

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexer.h>


//! \brief The QsciLexerCMake class encapsulates the Scintilla CMake lexer.
class QSCINTILLA_EXPORT QsciLexerCMake : public QsciLexer
{
    Q_OBJECT

public:
    //! This enum defines the meanings of the different styles used by the
    //! CMake lexer.
    enum {
        //! The default.
        Default = 0,

        //! A comment.
        Comment = 1,

        //! A string.
        String = 2,

        //! A left quoted string.
        StringLeftQuote = 3,

        //! A right quoted string.
        StringRightQuote = 4,

        //! A function.  (Defined by keyword set number 1.)
        Function = 5,

        //! A variable. (Defined by keyword set number 2.)
        Variable = 6,

        //! A label.
        Label = 7,

        //! A keyword defined in keyword set number 3.  The class must be
        //! sub-classed and re-implement keywords() to make use of this style.
        KeywordSet3 = 8,

        //! A WHILE block.
        BlockWhile = 9,

        //! A FOREACH block.
        BlockForeach = 10,

        //! An IF block.
        BlockIf = 11,

        //! A MACRO block.
        BlockMacro = 12,

        //! A variable within a string.
        StringVariable = 13,

        //! A number.
        Number = 14
    };

    //! Construct a QsciLexerCMake with parent \a parent.  \a parent is
    //! typically the QsciScintilla instance.
    QsciLexerCMake(QObject *parent = 0);

    //! Destroys the QsciLexerCMake instance.
    virtual ~QsciLexerCMake();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the name of the lexer.  Some lexers support a number of
    //! languages.
    const char *lexer() const;

    //! Returns the foreground colour of the text for style number \a style.
    //!
    //! \sa defaultPaper()
    QColor defaultColor(int style) const;

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

    //! Causes all properties to be refreshed by emitting the propertyChanged()
    //! signal as required.
    void refreshProperties();

    //! Returns true if ELSE blocks can be folded.
    //!
    //! \sa setFoldAtElse()
    bool foldAtElse() const;

public slots:
    //! If \a fold is true then ELSE blocks can be folded.  The default is
    //! false.
    //!
    //! \sa foldAtElse()
    virtual void setFoldAtElse(bool fold);

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
    void setAtElseProp();

    bool fold_atelse;

    QsciLexerCMake(const QsciLexerCMake &);
    QsciLexerCMake &operator=(const QsciLexerCMake &);
};

}

#endif
