
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "tobrowserindex.h"
#include "toconnection.h"
#include "toresultcols.h"
#include "toextract.h"
#include "tomemoeditor.h"
#include "tosql.h"
#include "totableselect.h"

#include <qapplication.h>
#include <qcombobox.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qprogressdialog.h>
#include <qradiobutton.h>
#include <qtimer.h>


void toBrowserIndex::modifyIndex(toConnection &conn, const QString &owner, const QString &table, QWidget *parent, const QString &index)
{
    toBrowserIndex dialog(conn, owner, table, index, parent);
    if (dialog.exec())
    {
        dialog.changeIndex();
        dialog.execute();
    }
}

void toBrowserIndex::addIndex(toConnection &conn, const QString &owner, const QString &table, QWidget *parent)
{
    toBrowserIndex dialog(conn, owner, table, QString::null, parent);
    dialog.addIndex();
    if (dialog.exec())
    {
        dialog.changeIndex();
        dialog.execute();
    }
}

void toBrowserIndex::execute()
{
    std::list<toSQLParse::statement> statements = toSQLParse::parse(sql(), connection());
    try
    {
        QProgressDialog prog(tr("Executing index change script"),
                             tr("Stop"),
                             0,
                             statements.size(),
                             this);
        prog.setWindowTitle(tr("Performing index changes"));
        for (std::list<toSQLParse::statement>::iterator i = statements.begin();i != statements.end();i++)
        {
            QString sql = toSQLParse::indentStatement(*i, connection());
            int l = sql.length() - 1;
            while (l >= 0 && (sql.at(l) == ';' || sql.at(l).isSpace()))
                l--;
            if (l >= 0)
                connection().execute(sql.mid(0, l + 1));
            qApp->processEvents();
            if (prog.wasCanceled())
                throw tr("Canceled ongoing index modification, indexes might be corrupt");
        }
    }
    TOCATCH
}

void toBrowserIndex::registerIndex(const QString &type, const QString &name, std::list<QString>::iterator beg, std::list<QString>::iterator end)
{
    std::list<toExtract::columnInfo> columns = toExtract::parseColumnDescription(beg, end);
    QString val;
    for (std::list<toExtract::columnInfo>::iterator i = columns.begin();i != columns.end();i++)
    {
        if (i != columns.begin())
            val += ",";
        val += (*i).Name;
    }
    IndexType[name] = type;
    IndexCols[name] = val;
}

toBrowserIndex::toBrowserIndex(toConnection &conn, const QString &owner, const QString &table, const QString &index, QWidget *parent, const char *name)
        : QDialog(parent), toConnectionWidget(conn, this), Extractor(conn, NULL)
{
    setupUi(this);

    Extractor.setIndexes(true);
    Extractor.setConstraints(false);
    Extractor.setPrompt(false);
    Extractor.setHeading(false);

    if (toIsMySQL(connection()))
        Type->addItem("Fulltext Index");

    if (!owner.isEmpty() && !table.isEmpty())
    {
        QString fulltable = conn.quote(owner) + "." + conn.quote(table);
        TableSelect->setTable(fulltable);
        describeTable(fulltable);
    }
    else if (!owner.isEmpty())
        TableSelect->setTable(conn.quote(owner));

    connect(TableSelect, SIGNAL(selectTable(const QString &)), this, SLOT(changeTable(const QString &)));

    for (int i = 0;i < Name->count();i++)
        if (connection().unQuote(Name->itemText(i)) == index)
        {
            Name->setCurrentIndex(Name->count() - 1);
            break;
        }
}

QString toBrowserIndex::table(void)
{
    if (Owner.isEmpty() && Table.isEmpty())
        return QString::null;
    return connection().quote(Owner) + "." + connection().quote(Table);
}

void toBrowserIndex::describeTable(const QString &table)
{
    try
    {
        QStringList parts = table.split(".");
        if (parts.size() > 1)
        {
            Owner = connection().unQuote(parts[0]);
            Table = connection().unQuote(parts[1]);
        }
        else
        {
            Table = connection().unQuote(table);
            if (toIsOracle(connection()))
                Owner = connection().user();
            else
                Owner = connection().database();
        }

        Current = QString::null;
        if (!table.isEmpty())
        {
            if ( OriginalDescription[table].empty() )
            {
                std::list<QString> Objects;
                Objects.insert(Objects.end(), "TABLE:" + table);

                NewDescription[table] = Extractor.describe(Objects);
            }
            std::list<QString> &origDescription = NewDescription[table];
            std::list<QString>::iterator beg = origDescription.end();
            QString lname;
            QString ltype;
            QString realtype;

            IndexType.clear();
            IndexCols.clear();

            for (std::list<QString>::iterator i = origDescription.begin();i != origDescription.end();)
            {
                std::list<QString> ctx = toExtract::splitDescribe(*i);
                toShift(ctx);
                QString type = toShift(ctx);
                if (type == "INDEX")
                {
                    QString name = toShift(ctx);
                    if (name != lname)
                    {
                        if (beg != origDescription.end())
                            registerIndex(realtype, lname, beg, i);
                        beg = i;
                        ltype = type;
                        lname = name;
                    }
                    if (toShift(ctx) == "TYPE")
                        realtype = toShift(ctx);
                    i++;
                }
                else
                {
                    if (beg != origDescription.end())
                    {
                        registerIndex(realtype, lname, beg, i);
                        beg = origDescription.end();
                    }
                    i = origDescription.erase(i);  // Remove everything that isn't indexes
                }
            }
            if (beg != origDescription.end())
                registerIndex(realtype, lname, beg, origDescription.end());
        }
        if ( OriginalDescription[table].empty() )
            OriginalDescription[table] = NewDescription[table];

        Name->clear();
        for (std::map<QString, QString>::iterator i = IndexType.begin();i != IndexType.end();i++)
            Name->addItem((*i).first);

        ColList->displayHeader(false);
        ColList->changeParams(Owner, Table);

        show();
        changeIndex();
    }
    catch (const QString &exc)
    {
        toStatusMessage(exc);
        reject();
    }
}

void toBrowserIndex::saveChanges(void)
{
    if (table().isEmpty())
        return ;

    changeIndex();

    std::list<QString> migrateTable;

    for (std::map<QString, QString>::iterator i = IndexType.begin();i != IndexType.end();i++)
    {
        std::list<QString> ctx;
        toPush(ctx, Owner);
        toPush(ctx, QString("INDEX"));
        toPush(ctx, (*i).first);

        QStringList lst = IndexCols[(*i).first].split(",");
        for (int j = 0;j < lst.count();j++)
        {
            toExtract::addDescription(migrateTable, ctx, "COLUMN", lst[j]);
            toExtract::addDescription(migrateTable, ctx, "COLUMN", lst[j], "ORDER", QString::number(j + 1));
        }
        toExtract::addDescription(migrateTable, ctx);
        toExtract::addDescription(migrateTable, ctx, "ON", Table);
        toExtract::addDescription(migrateTable, ctx, "TYPE", (*i).second);
    }
    NewDescription[table()] = migrateTable;
}

QString toBrowserIndex::sql()
{
    saveChanges();

    std::list<QString> origTable;
    std::list<QString> migrateTable;

    {
        for (std::map<QString, std::list<QString> >::iterator i = OriginalDescription.begin();i != OriginalDescription.end();i++)
        {
            std::list<QString> &cur = (*i).second;
            for (std::list<QString>::iterator j = cur.begin();j != cur.end();j++)
                toPush(origTable, *j);
        }
    }
    {
        for (std::map<QString, std::list<QString> >::iterator i = NewDescription.begin();i != NewDescription.end();i++)
        {
            std::list<QString> &cur = (*i).second;
            for (std::list<QString>::iterator j = cur.begin();j != cur.end();j++)
                toPush(migrateTable, *j);
        }
    }

    origTable.sort();
    migrateTable.sort();

    return Extractor.migrate(origTable, migrateTable);
}

void toBrowserIndex::displaySQL()
{
    toMemoEditor memo(this, sql(), -1, -1, true, true);
    memo.exec();
}

void toBrowserIndex::addIndex()
{
    if (Table.isEmpty())
        return ;
    bool ok = false;
    QString name = QInputDialog::getText(this,
                                         tr("Enter new index name"),
                                         tr("Enter name of new index."),
                                         QLineEdit::Normal,
                                         QString::null,
                                         &ok);
    if (ok)
    {
        Name->addItem(name);
        Name->setCurrentIndex(Name->count() - 1);
        IndexType[name] = "Normal Index";
        changeIndex();
    }
}

void toBrowserIndex::delIndex()
{
    if (Name->count() > 0)
    {
        IndexType.erase(Current);
        IndexCols.erase(Current);
        Current = QString::null;
        Name->removeItem(Name->currentIndex());
        changeIndex();
    }
}

void toBrowserIndex::changeIndex()
{
    if (!Current.isEmpty())
    {
        QString type = Type->currentText().toUpper();
        if (type == "NORMAL INDEX")
            type = "INDEX";
        IndexType[Current] = type;
        IndexCols[Current] = Cols->text();
    }
    Current = Name->currentText();
    if (Current.isEmpty())
    {
        Cols->setEnabled(false);
        Type->setEnabled(false);
    }
    else
    {
        Cols->setEnabled(true);
        Type->setEnabled(true);
        Cols->setText(IndexCols[Current]);
        int i;
        QString type = IndexType[Current];
        if (type == "INDEX")
            type = "NORMAL INDEX";
        for (i = 0;i < Type->count();i++)
        {
            if (Type->itemText(i).toUpper() == type)
            {
                Type->setCurrentIndex(i);
                break;
            }
        }
        if (i == Type->count())
        {
            Type->addItem(IndexType[Current]);
            Type->setCurrentIndex(Type->count() - 1);
        }
    }
}

void toBrowserIndex::changeTable(const QString &table)
{
    saveChanges();
    describeTable(table);
}
