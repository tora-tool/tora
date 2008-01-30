// This defines the interface to the QsciCommandSet class.
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


#ifndef QSCICOMMANDSET_H
#define QSCICOMMANDSET_H

extern "C++" {

#include <qglobal.h>

#include <QList>

#include <Qsci/qsciglobal.h>
#include <Qsci/qscicommand.h>


class QSettings;
class QsciScintilla;


//! \brief The QsciCommandSet class represents the set of all internal editor
//! commands that may have keys bound.
//!
//! Methods are provided to access the individual commands and to read and
//! write the current bindings from and to settings files.
class QSCINTILLA_EXPORT QsciCommandSet
{
public:
    //! The key bindings for each command in the set are read from the
    //! settings \a qs.  \a prefix is prepended to the key of each entry.
    //! true is returned if there was no error.
    //!
    //! \sa writeSettings()
    bool readSettings(QSettings &qs, const char *prefix = "/Scintilla");

    //! The key bindings for each command in the set are written to the
    //! settings \a qs.  \a prefix is prepended to the key of each entry.
    //! true is returned if there was no error.
    //!
    //! \sa readSettings()
    bool writeSettings(QSettings &qs, const char *prefix = "/Scintilla");

    //! The commands in the set are returned as a list.
    QList<QsciCommand *> &commands() {return cmds;}

    //! The primary keys bindings for all commands are removed.
    void clearKeys();

    //! The alternate keys bindings for all commands are removed.
    void clearAlternateKeys();

private:
    friend class QsciScintilla;

    QsciCommandSet(QsciScintilla *qs);
    ~QsciCommandSet();

    QsciScintilla *qsci;
    QList<QsciCommand *> cmds;

    QsciCommandSet(const QsciCommandSet &);
    QsciCommandSet &operator=(const QsciCommandSet &);
};

}

#endif
