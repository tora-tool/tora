
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
#include "parsing/tsqllexer.h"

#include <QtCore/QDebug>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QCheckBox>

QHash<QString, QStringList> toParamGet::DefaultCache;
QHash<QString, QStringList> toParamGet::Cache;

toParamGet::toParamGet(QWidget *parent, const char *name)
    : QDialog(parent)
    , toHelpContext(QString::fromLatin1("common.html#param"))
{
    setupUi(this);
    toHelp::connectDialog(this);

    Container = qobject_cast<QGridLayout*>(leftWidget->layout());
    Q_ASSERT_X(Container, qPrintable(__QHERE__), "Invalid layout for leftWidget");

    connect(OkButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(CancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

toQueryParams toParamGet::getParam(toConnection &conn, QWidget *parent, QString &str)
{
    const QString def = QString::fromLatin1("<char[4000]>");

    toQueryParams ret;

    std::unique_ptr <SQLLexer::Lexer> lexer;
    if (conn.providerIs("Oracle"))
            lexer = LexerFactTwoParmSing::Instance().create("OracleGuiLexer", str, "toParamGet::getParam");
    else if(conn.providerIs("QMYSQL"))
        lexer = LexerFactTwoParmSing::Instance().create("MySQLGuiLexer", str, "toParamGet::getParam");
    else if(conn.providerIs("QPSQL"))
        lexer = LexerFactTwoParmSing::Instance().create("PostreSQLGuiLexer", str, "toParamGet::getParam");
    else
        throw QString("Unsupported sql dialect: %1").arg(conn.provider());

    QString retvalsql;
    QStringList BindParamList;        // Keep order of BindParam occurrences
    SQLLexer::Lexer::token_const_iterator iter = lexer->begin();
    while (iter->getTokenType() != SQLLexer::Token::X_EOF)
    {
        switch (iter->getTokenType())
        {
        case SQLLexer::Token::L_BIND_VAR:
        {
            QString name = iter->getText();
			retvalsql += name;
            if (conn.providerIs("Oracle"))
                retvalsql+=def;
            if (!BindParamList.contains(name))
                BindParamList << name;
        }
        break;
        case SQLLexer::Token::L_BIND_VAR_WITH_PARAMS:
        {
            QString l1 = iter->getText();
            QString name = l1.left( l1.indexOf('<'));
            if (!BindParamList.contains(name))
                BindParamList << name;
            retvalsql += l1;
        }
        break;
        default:
            retvalsql += iter->getText();
            //no break here
        }
        iter++;
    }

    // No bind parameters in the SQL query
    if (BindParamList.empty())
        return ret;

    toParamGet *widget = new toParamGet(parent);

    int num = 0;
    foreach(QString BindParam, BindParamList)
    {
        widget->createWidgetRow(BindParam, num);
        num++;
    }

    // set edit column to stretch
    widget->Container->setColumnStretch(1, 1);
    // add widget at bottom of grid that can resize
    widget->Container->addWidget(new QLabel(widget), num, 0);
    widget->Container->setRowStretch(num, 1);

    (*widget->Values.begin())->setFocus();
    if (widget->exec())
    {
        int ix = 0;
        foreach(QComboBox * current, widget->Values)
        {
            QString val;
            if (current->isEnabled())
                val = current->currentText();
            else
                val = QString::null;

            QString bindName = BindParamList.at(ix);
            ++ix;

            QStringList lst = Cache[bindName.toLower()];
            lst.removeAll(val);
            lst.prepend(val);
            Cache[bindName.toLower()] = lst;

            lst = DefaultCache[bindName.toLower()];
            lst.removeAll(val);

            ret << val;
        }
    }
    else
    {
        Utils::toStatusMessage(tr("Aborted execution"), false, false);
        throw tr("Aborted execution");
    }
    delete widget;
    str = retvalsql;
    return ret;
}

void toParamGet::createWidgetRow(const QString &fname, int rownum)
{
    //int rownum = Container->rowCount();

    QLabel *id = new QLabel(fname, this);
    Container->addWidget(id, rownum, 0);

    QComboBox *edit = new QComboBox(this);
    // to memo finds child by widget name (row)
    edit->setObjectName(QString::number(rownum));
    edit->setEditable(true);
    edit->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    edit->setMinimumContentsLength(30);
    // take away the completer - it causes:
    // #3061131: problem when using parameters in query
    //when doing a "select id, name from table1 where name = :pname" TORA asks me for the value of pname. When i enter e.g. "Lars" the query runs.
    // When doing the same query again answering "LARS" for the value TORA runs the query again, but not with the second value, it uses the forst one.
    edit->setCompleter(0);

    Container->addWidget(edit, rownum, 1);

    if (Cache.contains(fname.toLower()))
    {
        edit->addItems(Cache[fname.toLower()]);
    }

    if (DefaultCache.contains(fname.toLower()))
    {
        edit->addItems(DefaultCache[fname.toLower()]);
    }

    QCheckBox *box = new QCheckBox(tr("NULL"), this);
    connect(box, SIGNAL(toggled(bool)), edit, SLOT(setDisabled(bool)));
    if (edit->currentText().isNull())
    {
        box->setChecked(true);
    }
    Container->addWidget(box, rownum, 2);

    toParamGetButton *btn = new toParamGetButton(rownum, this);
    btn->setText(tr("Edit"));
    btn->setFocusPolicy(Qt::ClickFocus);
    connect(btn, SIGNAL(clicked(int)), this, SLOT(showMemo(int)));
    connect(box, SIGNAL(toggled(bool)), btn, SLOT(setDisabled(bool)));
    Container->addWidget(btn, rownum, 3);
    Values.insert(Values.end(), edit);

    Container->setRowMinimumHeight(rownum, 30);
}

void toParamGet::setDefault(toConnection &, const QString &name, const QString &val)
{
    if (Cache[name.toLower()].contains(val))
        return;

    QStringList lst = DefaultCache[name.toLower()];
    lst.removeAll(val);
    lst.prepend(val);
    DefaultCache[name.toLower()] = lst;
}

void toParamGet::setDefault(toConnection &, const QString &name, const QStringList &values)
{
    DefaultCache[name.toLower()] = values;
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
