// The definition of various Qt version independent classes used by the rest of
// the port.
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


#ifndef _SCICLASSES_H
#define _SCICLASSES_H

#include <qglobal.h>
#include <qwidget.h>


class QMouseEvent;
class QPaintEvent;
class ScintillaQt;


// A simple QWidget sub-class to implement a call tip.
class SciCallTip : public QWidget
{
    Q_OBJECT

public:
    SciCallTip(QWidget *parent, ScintillaQt *sci_);
    ~SciCallTip();

protected:
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);

private:
    ScintillaQt *sci;
};


// A popup menu where options correspond to a numeric command.

#include <QMenu>
#include <QSignalMapper>

class SciPopup : public QMenu
{
    Q_OBJECT

public:
    SciPopup();

    void addItem(const QString &label, int cmd, bool enabled,
            ScintillaQt *sci_);

private slots:
    void on_triggered(int cmd);

private:
    ScintillaQt *sci;
    QSignalMapper mapper;
};



// This sub-class of QListBox is needed to provide slots from which we can call
// ListBox's double-click callback.  (And you thought this was a C++ program.)

class ListBoxQt;


#include <QListWidget>

class SciListBox : public QListWidget
{
    Q_OBJECT

public:
    SciListBox(QWidget *parent, ListBoxQt *lbx_);
    virtual ~SciListBox();

    void addItemPixmap(const QPixmap &pm, const QString &txt);

    int find(const QString &prefix);
    QString text(int n);

private slots:
    void handleDoubleClick();

private:
    ListBoxQt *lbx;
};


#endif
