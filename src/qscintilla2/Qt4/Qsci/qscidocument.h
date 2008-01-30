// This defines the interface to the QsciDocument class.
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


#ifndef QSCIDOCUMENT_H
#define QSCIDOCUMENT_H

extern "C++" {

#include <Qsci/qsciglobal.h>


class QsciScintillaBase;
class QsciDocumentP;


//! \brief The QsciDocument class represents a document to be edited.
//!
//! It is an opaque class that can be attached to multiple instances of
//! QsciScintilla to create different simultaneous views of the same document.
//! QsciDocument uses implicit sharing so that copying class instances is a
//! cheap operation.
class QSCINTILLA_EXPORT QsciDocument
{
public:
    //! Create a new unattached document.
    QsciDocument();
    virtual ~QsciDocument();

    QsciDocument(const QsciDocument &);
    QsciDocument &operator=(const QsciDocument &);

private:
    friend class QsciScintilla;

    void attach(const QsciDocument &that);
    void detach();
    void display(QsciScintillaBase *qsb, const QsciDocument *from);
    void undisplay(QsciScintillaBase *qsb);

    QsciDocumentP *pdoc;
};

}

#endif
