// This module implements the QsciDocument class.
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


#include "Qsci/qscidocument.h"
#include "Qsci/qsciscintillabase.h"


// This internal class encapsulates the underlying document and is shared by
// QsciDocument instances.
class QsciDocumentP
{
public:
    QsciDocumentP() : doc(0), nr_displays(0), nr_attaches(1) {}

    long doc;               // The Scintilla document.
    int nr_displays;        // The number of displays.
    int nr_attaches;        // The number of attaches.
};


// The ctor.
QsciDocument::QsciDocument()
{
    pdoc = new QsciDocumentP();
}


// The dtor.
QsciDocument::~QsciDocument()
{
    detach();
}


// The copy ctor.
QsciDocument::QsciDocument(const QsciDocument &that)
{
    attach(that);
}


// The assignment operator.
QsciDocument &QsciDocument::operator=(const QsciDocument &that)
{
    if (pdoc != that.pdoc)
    {
        detach();
        attach(that);
    }

    return *this;
}


// Attach an existing document to this one.
void QsciDocument::attach(const QsciDocument &that)
{
    ++that.pdoc->nr_attaches;
    pdoc = that.pdoc;
}


// Detach the underlying document.
void QsciDocument::detach()
{
    if (!pdoc)
        return;

    if (--pdoc->nr_attaches == 0)
    {
        if (pdoc->doc && pdoc->nr_displays == 0)
        {
            QsciScintillaBase *qsb = QsciScintillaBase::pool();

            // Release the explicit reference to the document.  If the pool is
            // empty then we just accept the memory leak.
            if (qsb)
                qsb->SendScintilla(QsciScintillaBase::SCI_RELEASEDOCUMENT, 0,
                        pdoc->doc);
        }

        delete pdoc;
    }

    pdoc = 0;
}


// Undisplay and detach the underlying document.
void QsciDocument::undisplay(QsciScintillaBase *qsb)
{
    if (--pdoc->nr_attaches == 0)
        delete pdoc;
    else if (--pdoc->nr_displays == 0)
    {
        // Create an explicit reference to the document to keep it alive.
        qsb->SendScintilla(QsciScintillaBase::SCI_ADDREFDOCUMENT, 0, pdoc->doc);
    }

    pdoc = 0;
}


// Display the underlying document.
void QsciDocument::display(QsciScintillaBase *qsb, const QsciDocument *from)
{
    long ndoc;

    if (from)
    {
        ndoc = from->pdoc->doc;
        qsb->SendScintilla(QsciScintillaBase::SCI_SETDOCPOINTER, 0, ndoc);
    }
    else
        ndoc = qsb->SendScintilla(QsciScintillaBase::SCI_GETDOCPOINTER);

    pdoc->doc = ndoc;
    ++pdoc->nr_displays;
}
