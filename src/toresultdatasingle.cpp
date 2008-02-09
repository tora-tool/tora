/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation;  only version 2 of
* the License is valid for this program.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*      As a special exception, you have permission to link this program
*      with the Oracle Client libraries and distribute executables, as long
*      as you follow the requirements of the GNU GPL in regard to all of the
*      software in the executable aside from Oracle client libraries.
*
*      Specifically you are not permitted to link this program with the
*      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
*      And you are not permitted to distribute binaries compiled against
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "config.h"
#include "utils.h"
#include "toconf.h"
#include "toconfiguration.h"
#include "toresultdatasingle.h"
#include "toparamget.h"
#include "tomemoeditor.h"

#include <QScrollArea>
#include <QCheckBox>
#include <QLineEdit>
#include <QAction>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>


toResultDataSingle::toResultDataSingle(QWidget *parent)
    : QWidget(parent) {

    Row       = -1;
    Container = NULL;
    Model     = NULL;

    setBackgroundRole(QPalette::Window);

    // fscking qscrollarea won't resize this is added
    QHBoxLayout *h = new QHBoxLayout;
    h->setContentsMargins(0, 0, 0, 0);
    h->setSpacing(0);
    setLayout(h);

    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
}


void toResultDataSingle::changeSource(toResultModel *model, int _row) {
    delete Container;
    Container = new QScrollArea(this);
    layout()->addWidget(Container);

    Model = model;

    QWidget *ext = new QWidget(Container);
    Container->setWidget(ext);
    Container->setWidgetResizable(true);

    QGridLayout *grid = new QGridLayout;
    ext->setLayout(grid);

    Value.clear();
    Null.clear();

    const toResultModel::HeaderList Headers = Model->headers();

    int row = 1;
    int col = 0;

    for(; row < Headers.size(); row++, col = 0) {
        QLabel *name = new QLabel(Headers[row].name, this);
        grid->addWidget(name, row, col++);

        QLineEdit *edit = new QLineEdit(this);
        edit->setObjectName(QString::number(row));
        edit->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                        QSizePolicy::Minimum));
        grid->addWidget(edit, row, col);
        // edit widget should get the most space
        grid->setColumnStretch(col++, 1);

        QCheckBox *box = new QCheckBox(("NULL"), this);
        connect(box, SIGNAL(toggled(bool)), edit, SLOT(setDisabled(bool)));
        grid->addWidget(box, row, col++);

        toParamGetButton *btn = new toParamGetButton(row, this);
        btn->setText(tr("Edit"));
        btn->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed));
        connect(btn, SIGNAL(clicked(int)), this, SLOT(showMemo(int)));
        connect(box, SIGNAL(toggled(bool)), btn, SLOT(setDisabled(bool)));
        grid->addWidget(btn, row, col++);

        Value.append(edit);
        Null.append(box);
    }

    // add widget at bottom of grid that can resize
    grid->addWidget(new QLabel(this), row, 0);
    grid->setRowStretch(row, 1);

    Row = _row;
    changeRow(Row);
}

void toResultDataSingle::changeRow(int row) {
    bool any = false;

    QList<QCheckBox *>::iterator chk = Null.begin();
    QList<QLineEdit *>::iterator val = Value.begin();

    for(int i = 1;
        i < Model->columnCount() && chk != Null.end() && val != Value.end();
        i++, chk++, val++) {

        QVariant str = Model->data(row, i);
        if(!str.isNull())
            any = true;
        (*chk)->setChecked(str.isNull());
        (*val)->setText(str.toString());
    }

    if(!any) {
        for(chk = Null.begin(); chk != Null.end(); chk++)
            (*chk)->setChecked(false);
    }

    Row = row;
}


void toResultDataSingle::saveRow() {
    if(Row < 0)
        return;

    QList<QLineEdit *>::iterator val = Value.begin();
    for(int i = 1; i < Model->columnCount() && val != Value.end(); i++, val++) {
        QString v = QString::null;
        if((*val)->isEnabled())
            v = (*val)->text();

        QModelIndex index = Model->createIndex(Row, i);
        QVariant orig = Model->data(Row, i).toString();

        if(!((v.isNull() && orig.isNull()) || v == orig))
            Model->setData(index, v);
    }
}


void toResultDataSingle::showMemo(int row) {
    QLineEdit *obj = findChild<QLineEdit *>(QString::number(row));
    if(obj) {
        toMemoEditor *memo = new toMemoEditor(
            this,
            obj->text(),
            row,
            0,
            false,
            true);

        if(memo->exec())
            obj->setText(memo->text());
    }
}
