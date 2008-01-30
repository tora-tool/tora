// This defines the interface to the QsciMacro class.
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


#ifndef QSCIMACRO_H
#define QSCIMACRO_H

extern "C++" {

#include <qobject.h>
#include <qstring.h>

#include <qlist.h>

#include <Qsci/qsciglobal.h>


class QsciScintilla;


//! \brief The QsciMacro class represents a sequence of recordable
//! editor commands.
//!
//! Methods are provided to convert convert a macro to and from a textual
//! representation so that they can be easily written to and read from
//! permanent storage.
class QSCINTILLA_EXPORT QsciMacro : public QObject
{
    Q_OBJECT

public:
    //! Construct a QsciMacro with parent \a parent.
    QsciMacro(QsciScintilla *parent);

    //! Construct a QsciMacro from the printable ASCII representation \a asc,
    //! with parent \a parent.
    QsciMacro(const QString &asc, QsciScintilla *parent);

    //! Destroy the QsciMacro instance.
    virtual ~QsciMacro();

    //! Clear the contents of the macro.
    void clear();

    //! Load the macro from the printable ASCII representation \a asc.  Returns
    //! true if there was no error.
    //!
    //! \sa save()
    bool load(const QString &asc);

    //! Return a printable ASCII representation of the macro.  It is guaranteed
    //! that only printable ASCII characters are used and that double quote
    //! characters will not be used.
    //!
    //! \sa load()
    QString save() const;

public slots:
    //! Play the macro.
    virtual void play();

    //! Start recording user commands and add them to the macro.
    virtual void startRecording();

    //! Stop recording user commands.
    virtual void endRecording();

private slots:
    void record(unsigned int msg, unsigned long wParam, long lParam);

private:
    struct Macro {
        unsigned int msg;
        unsigned long wParam;
        QByteArray text;
    };

    QsciScintilla *qsci;
    QList<Macro> macro;

    QsciMacro(const QsciMacro &);
    QsciMacro &operator=(const QsciMacro &);
};

}

#endif
