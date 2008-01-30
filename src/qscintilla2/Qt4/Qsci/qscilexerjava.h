// This defines the interface to the QsciLexerJava class.
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


#ifndef QSCILEXERJAVA_H
#define QSCILEXERJAVA_H

extern "C++" {

#include <qobject.h>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscilexercpp.h>


//! \brief The QsciLexerJava class encapsulates the Scintilla Java lexer.
class QSCINTILLA_EXPORT QsciLexerJava : public QsciLexerCPP
{
    Q_OBJECT

public:
    //! Construct a QsciLexerJava with parent \a parent.  \a parent is
    //! typically the QsciScintilla instance.
    QsciLexerJava(QObject *parent = 0);

    //! Destroys the QsciLexerJava instance.
    virtual ~QsciLexerJava();

    //! Returns the name of the language.
    const char *language() const;

    //! Returns the set of keywords for the keyword set \a set recognised
    //! by the lexer as a space separated string.
    const char *keywords(int set) const;

private:
    QsciLexerJava(const QsciLexerJava &);
    QsciLexerJava &operator=(const QsciLexerJava &);
};

}

#endif
