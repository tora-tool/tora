// This defines the interface to the QsciLexerIDL class.
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


#ifndef QSCILEXERIDL_H
#define QSCILEXERIDL_H

extern "C++" {

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexercpp.h>


//! \brief The QsciLexerIDL class encapsulates the Scintilla IDL
//! lexer.
class QSCINTILLA_EXPORT QsciLexerIDL : public QsciLexerCPP
{
    Q_OBJECT

public:
    //! Construct a QsciLexerIDL with parent \a parent.  \a parent is typically
    //! the QsciScintilla instance.
    QsciLexerIDL(QObject *parent = 0);

    //! Destroys the QsciLexerIDL instance.
    virtual ~QsciLexerIDL();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the foreground colour of the text for style number \a style.
    QColor defaultColor(int style) const;

    //! Returns the set of keywords for the keyword set \a set recognised
    //! by the lexer as a space separated string.
    const char *keywords(int set) const;

    //! Returns the descriptive name for style number \a style.  If the
    //! style is invalid for this language then an empty QString is returned.
    //! This is intended to be used in user preference dialogs.
    QString description(int style) const;

private:
    QsciLexerIDL(const QsciLexerIDL &);
    QsciLexerIDL &operator=(const QsciLexerIDL &);
};

}

#endif
