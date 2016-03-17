
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "tools/toparamget.h"

#include "core/utils.h"
#include "editor/tomemoeditor.h"

#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QCheckBox>

QHash<QString, QStringList> toParamGet::DefaultCache;
QHash<QString, QStringList> toParamGet::Cache;


toParamGet::toParamGet(QWidget *parent, const char *name)
    : QDialog(parent),
      toHelpContext(QString::fromLatin1("common.html#param"))
{

    setModal(true);
    toHelp::connectDialog(this);
    resize(500, 480);
    setWindowTitle(tr("Define binding variables"));

    QHBoxLayout *hlayout = new QHBoxLayout;

    // for buttons on right
    QWidget *vbox = new QWidget;
    QVBoxLayout *vlayout = new QVBoxLayout;

    View = new QScrollArea(this);
    View->setSizePolicy(QSizePolicy::MinimumExpanding,
                        QSizePolicy::MinimumExpanding);
    hlayout->addWidget(View);

    // a widget to set as central to scroll area
    QWidget *ext = new QWidget(View);
    View->setWidget(ext);
    View->setWidgetResizable(true);

    Container = new QGridLayout;
    Container->setSpacing(10);
    ext->setLayout(Container);

    QPushButton *OkButton = new QPushButton("OkButton", this);
    OkButton->setText(tr("&Ok"));
    OkButton->setDefault(true);
    vlayout->addWidget(OkButton);

    QPushButton *CancelButton = new QPushButton("CancelButton", this);
    CancelButton->setText(tr("Cancel"));
    CancelButton->setDefault(false);
    vlayout->addWidget(CancelButton);

    QSpacerItem* spacer = new QSpacerItem(20,
                                          20,
                                          QSizePolicy::Minimum,
                                          QSizePolicy::Expanding);
    vlayout->addItem(spacer);

    connect(OkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));

    vbox->setLayout(vlayout);
    hlayout->addWidget(vbox);
    setLayout(hlayout);
}

toQueryParams toParamGet::getParam(toConnection &conn,
                                   QWidget *parent,
                                   QString &str,
                                   bool interactive)
{
    std::map<QString, bool> parameters;
    QStringList names;
    toParamGet *widget = NULL;

    enum
    {
        afterName,
        inString,
        normal,
        comment,
        multiComment,
        name,
        specification,
        endInput
    } state;
    state = normal;

    QChar endString;
    QString fname;
    QString direction;
    QString res;

    QString def = QString::fromLatin1("<char[4000]>");

    int colon = 1;

    int num = 0;
    for (int i = 0; i < str.length() + 1; i++)
    {
        QChar c;
        QChar nc;
        QChar pc;

        c = nc = pc = '\n';

        if (i < str.length())
            c = str.at(i);

        if (i < str.length() - 1)
            nc = str.at(i + 1);

        if (i - 1 > 0)
            pc = str.at(i - 1);

        if (state == normal && c == '-' && nc == '-')
            state = comment;
        else if (state == normal && c == '/' && nc == '*')
            state = multiComment;
        else
        {
            switch (state)
            {
                case inString:
                    if (c == endString)
                        state = normal;
                    break;
                case comment:
                    if (c == '\n')
                        state = normal;
                    break;
                case multiComment:
                    if (c == '*' && nc == '/')
                        state = normal;
                    break;
                case normal:
                    switch (c.toLatin1())
                    {
                        case '\'':
                        case '\"':
                            endString = c;
                            state = inString;
                            break;
                        case ':':
                            // ignore ::
                            // this is a type cast for postgres, not a parameter.
                            if (nc == ':' || pc == ':')
                                break;

                            if (nc != '=')
                                state = name;
                            direction = "";
                            fname = "";
                            break;
                        case '?':
                            fname = QString::fromLatin1("f");
                            fname += QString::number(colon);
                            colon++;
                            res += QString::fromLatin1(":");
                            res += fname;
                            res += def.mid(0, def.length() - 1);
                            c = def.at(def.length() - 1);
                            break;
                    }
                    break;
                case name:
                    if (c.isLetterOrNumber() || c == '_')
                    {
                        fname += c;
                        break;
                    }
                    if (fname.isEmpty() //conn && !conn.providerIs("QMYSQL")
                       )
                    {
                        Utils::toStatusMessage(tr("Missing field name"));
                        throw tr("Missing field name");
                    }
                    state = afterName;
                case afterName:
                    if (c == '<')
                        state = specification;
                    else
                    {
                        state = normal;
                        res += def;
                    }
                    break;
                case specification:
                    if (c == ',')
                        state = endInput;
                    else if (c == '>')
                        state = normal;
                    break;
                case endInput:
                    if (c == '>')
                        state = normal;
                    else
                        direction += c;
                    break;
            }
        }

        if (state == normal && !fname.isEmpty())
        {
            if (direction.isEmpty() ||
                    direction == "in" ||
                    direction == "inout")
            {

                if (!parameters[fname])
                {
                    parameters[fname] = true;

                    if (!widget)
                        widget = new toParamGet(parent);

                    QLabel *id = new QLabel(fname, widget);
                    widget->Container->addWidget(id, num, 0);

                    QComboBox *edit = new QComboBox(widget);
                    // to memo finds child by widget name (row)
                    edit->setObjectName(QString::number(num));
                    edit->setEditable(true);
                    edit->setSizePolicy(QSizePolicy::MinimumExpanding,
                                        QSizePolicy::MinimumExpanding);
                    edit->setMinimumContentsLength(30);
                    // take away the completer - it causes:
                    // #3061131: problem when using parameters in query
                    //when doing a "select id, name from table1 where name = :pname" TORA asks me for the value of pname. When i enter e.g. "Lars" the query runs.
                    // When doing the same query again answering "LARS" for the value TORA runs the query again, but not with the second value, it uses the forst one.
                    edit->setCompleter(0);

                    widget->Container->addWidget(edit, num, 1);

                    if (Cache.contains(fname))
                    {
                        edit->addItems(Cache[fname]);
                    }

                    if (DefaultCache.contains(fname))
                    {
                        edit->addItems(DefaultCache[fname]);
                    }

                    QCheckBox *box = new QCheckBox(tr("NULL"), widget);
                    connect(box,
                            SIGNAL(toggled(bool)),
                            edit,
                            SLOT(setDisabled(bool)));
                    if (edit->currentText().isNull())
                    {
                        box->setChecked(true);
                    }
                    widget->Container->addWidget(box, num, 2);

                    toParamGetButton *btn = new toParamGetButton(
                        num,
                        widget);
                    btn->setText(tr("Edit"));
                    connect(btn, SIGNAL(clicked(int)),
                            widget, SLOT(showMemo(int)));
                    connect(box, SIGNAL(toggled(bool)),
                            btn, SLOT(setDisabled(bool)));
                    widget->Container->addWidget(btn, num, 3);
                    widget->Value.insert(widget->Value.end(), edit);

                    names.append(fname);

                    widget->Container->setRowMinimumHeight(num, 30);
                    num++;
                }
            }
            fname = "";
        }

        if (i < str.length())
            res += c;
    } // for

    if (widget && num > 0)
    {
        // set edit column to stretch
        widget->Container->setColumnStretch(1, 1);
        // add widget at bottom of grid that can resize
        widget->Container->addWidget(new QLabel(widget), num, 0);
        widget->Container->setRowStretch(num, 1);
    }

    toQueryParams ret;
    if (widget)
    {
        (*widget->Value.begin())->setFocus();
        if (!interactive || widget->exec())
        {
            int ix = 0;
            foreach(QComboBox * current, widget->Value)
            {
                QString val;
                if (current)
                {
                    if (current->isEnabled())
                        val = current->currentText();
                    else
                        val = QString::null;
                }

                QString keyName = names.at(ix);
                ++ix;

                QStringList lst = Cache[keyName];
                lst.removeAll(val);
                lst.prepend(val);
                Cache[keyName] = lst;

                lst = DefaultCache[keyName];
                lst.removeAll(val);

                ret << val;
            }
            delete widget;
        }
        else
        {
            delete widget;
            Utils::toStatusMessage(tr("Aborted execution"), false, false);
            throw tr("Aborted execution");
        }
    }
    str = res;
    return ret;
}

void toParamGet::setDefault(toConnection &, const QString &name, const QString &val)
{
    if (Cache[name].contains(val))
        return;

    QStringList lst = DefaultCache[name];
    lst.removeAll(val);
    lst.prepend(val);
    DefaultCache[name] = lst;
}

void toParamGet::showMemo(int row)
{
    QComboBox *obj = findChild<QComboBox *>(QString::number(row));
    if (obj)
    {
        toMemoEditor *memo = new toMemoEditor(this,
                                              obj->currentText(),
                                              row,
                                              0,
                                              false,
                                              true);
        if (memo->exec())
            obj->setItemText(obj->currentIndex(), memo->text());
    }
}
