// This module implements the QsciCommand class.
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


#include "Qsci/qscicommand.h"

#include <qnamespace.h>
#include <qapplication.h>

#include "Qsci/qsciscintilla.h"
#include "Qsci/qsciscintillabase.h"


static int convert(int key);


// The ctor.
QsciCommand::QsciCommand(QsciScintilla *qs, int msg, int key, int altkey,
        const char *desc)
    : qsCmd(qs), msgCmd(msg), qkey(key), qaltkey(altkey), descCmd(desc)
{
    scikey = convert(qkey);

    if (scikey)
        qsCmd->SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, scikey,
                msgCmd);

    scialtkey = convert(qaltkey);

    if (scialtkey)
        qsCmd->SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, scialtkey,
                msgCmd);
}


// Bind a key to a command.
void QsciCommand::setKey(int key)
{
    bindKey(key,qkey,scikey);
}


// Bind an alternate key to a command.
void QsciCommand::setAlternateKey(int altkey)
{
    bindKey(altkey,qaltkey,scialtkey);
}


// Do the hard work of binding a key.
void QsciCommand::bindKey(int key,int &qk,int &scik)
{
    int new_scikey;

    // Ignore if it is invalid, allowing for the fact that we might be
    // unbinding it.
    if (key)
    {
        new_scikey = convert(key);

        if (!new_scikey)
            return;
    }
    else
        new_scikey = 0;

    if (scik)
        qsCmd->SendScintilla(QsciScintillaBase::SCI_CLEARCMDKEY, scik);

    qk = key;
    scik = new_scikey;

    if (scik)
        qsCmd->SendScintilla(QsciScintillaBase::SCI_ASSIGNCMDKEY, scik, msgCmd);
}


// See if a key is valid.
bool QsciCommand::validKey(int key)
{
    return convert(key);
}


// Convert a Qt character to the Scintilla equivalent.  Return zero if it is
// invalid.
static int convert(int key)
{
    // Convert the modifiers.
    int sci_mod = 0;

    if (key & Qt::SHIFT)
        sci_mod |= QsciScintillaBase::SCMOD_SHIFT;

    if (key & Qt::CTRL)
        sci_mod |= QsciScintillaBase::SCMOD_CTRL;

    if (key & Qt::ALT)
        sci_mod |= QsciScintillaBase::SCMOD_ALT;

    key &= ~Qt::MODIFIER_MASK;

    // Convert the key.
    int sci_key;

    if (key > 0x7f)
        switch (key)
        {
        case Qt::Key_Down:
            sci_key = QsciScintillaBase::SCK_DOWN;
            break;

        case Qt::Key_Up:
            sci_key = QsciScintillaBase::SCK_UP;
            break;

        case Qt::Key_Left:
            sci_key = QsciScintillaBase::SCK_LEFT;
            break;

        case Qt::Key_Right:
            sci_key = QsciScintillaBase::SCK_RIGHT;
            break;

        case Qt::Key_Home:
            sci_key = QsciScintillaBase::SCK_HOME;
            break;

        case Qt::Key_End:
            sci_key = QsciScintillaBase::SCK_END;
            break;

        case Qt::Key_PageUp:
            sci_key = QsciScintillaBase::SCK_PRIOR;
            break;

        case Qt::Key_PageDown:
            sci_key = QsciScintillaBase::SCK_NEXT;
            break;

        case Qt::Key_Delete:
            sci_key = QsciScintillaBase::SCK_DELETE;
            break;

        case Qt::Key_Insert:
            sci_key = QsciScintillaBase::SCK_INSERT;
            break;

        case Qt::Key_Escape:
            sci_key = QsciScintillaBase::SCK_ESCAPE;
            break;

        case Qt::Key_Backspace:
            sci_key = QsciScintillaBase::SCK_BACK;
            break;

        case Qt::Key_Tab:
            sci_key = QsciScintillaBase::SCK_TAB;
            break;

        case Qt::Key_Return:
            sci_key = QsciScintillaBase::SCK_RETURN;
            break;

        default:
            sci_key = 0;
        }
    else
        sci_key = key;

    if (sci_key)
        sci_key |= (sci_mod << 16);

    return sci_key;
}


// Return the translated user friendly description.
QString QsciCommand::description() const
{
    return qApp->translate("QsciCommand", descCmd);
}
