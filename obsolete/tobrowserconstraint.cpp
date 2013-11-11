
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

#include "tools/tobrowserconstraint.h"
#include "core/utils.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectiontraits.h"
#include "core/toconnectionsubloan.h"
#include "core/toresultcols.h"
//#include "core/toextract.h"
#include "core/tomemoeditor.h"
#include "core/tosql.h"
#include "core/totableselect.h"

#include <QtGui/QInputDialog>
#include <QtGui/QStackedWidget>
#include <QtGui/QProgressDialog>

void toBrowserConstraint::modifyConstraint(toConnection &conn, const QString &owner, const QString &table, QWidget *parent)
{
    //obsolete toBrowserConstraint dialog(conn, owner, table, parent);
    // if (dialog.exec())
    // {
    //     dialog.changeConstraint();
    //     std::list<toSQLParse::statement> statements = toSQLParse::parse(dialog.sql(), conn);
    //     try
    //     {
    //         QProgressDialog prog(tr("Performing constraint changes"),
    //                              tr("Stop"),
    //                              0,
    //                              statements.size(),
    //                              &dialog);
    //         prog.setWindowTitle(tr("Performing constraint changes"));
    //         for (std::list<toSQLParse::statement>::iterator j = statements.begin(); j != statements.end(); j++)
    //         {
    //             QString sql = toSQLParse::indentStatement(*j, conn);
    //             int i = sql.length() - 1;
    //             while (i >= 0 && (sql.at(i) == ';' || sql.at(i).isSpace()))
    //                 i--;
    //             if (i >= 0)
    //             {
    //                 toConnectionSubLoan connSub(conn);
    //                 connSub->execute(sql.mid(0, i + 1));
    //             }
    //             qApp->processEvents();
    //             if (prog.wasCanceled())
    //                 throw tr("Canceled ongoing constraint modification, constraints might be corrupt");
    //         }
    //     }
    //     TOCATCH
    // }
    throw tr("toBrowserConstraint::modifyConstraint not imlemented yet");    
}

toBrowserConstraint::toBrowserConstraint(toConnection &conn, const QString &owner, const QString &table, QWidget *parent, const char *name)
    : QDialog(parent)
    , toConnectionWidget(conn, this)
    //obsolete , Extractor(conn, NULL)
{
    setObjectName(name);
    setModal(true);

    setupUi(this);
    //obsolete Extractor.setIndexes(false);
    // Extractor.setConstraints(true);
    // Extractor.setPrompt(false);
    // Extractor.setHeading(false);

    if (connection().providerIs("MySQL"))
        UniqueType->hide();

    SourceColList->displayHeader(false);
    UniqueColList->displayHeader(false);
    CheckColumn->displayHeader(false);
    ReferColList->displayHeader(false);

    if (!owner.isEmpty() && !table.isEmpty())
    {
        QString fulltable = conn.getTraits().quote(owner) + "." + conn.getTraits().quote(table);
        TableSelect->setTable(fulltable);
        describeTable(fulltable);
    }
    else if (!owner.isEmpty())
        TableSelect->setTable(conn.getTraits().quote(owner));

    connect(TableSelect, SIGNAL(selectTable(const QString &)), this, SLOT(changeTable(const QString &)));
}

void toBrowserConstraint::describeTable(const QString &table)
{
    try
    {
        QStringList parts = table.split(".");
        if (parts.size() > 1)
        {
            Owner = connection().getTraits().unQuote(parts[0]);
            Table = connection().getTraits().unQuote(parts[1]);
        }
        else
        {
            Table = connection().getTraits().unQuote(table);
            if (connection().providerIs("Oracle"))
                Owner = connection().user();
            else
                Owner = connection().database();
        }

        Constraints.clear();
        Current = QString::null;

        if (!Table.isEmpty())
        {
            if ( OriginalDescription[table].empty() )
            {
                std::list<QString> Objects;
                Objects.insert(Objects.end(), "TABLE:" + Owner + "." + Table);
                Objects.insert(Objects.end(), "TABLE REFERENCES:" + Owner + "." + Table);

                NewDescription[table] = Extractor.describe(Objects);
            }
            std::list<QString> &origDescription = NewDescription[table];

            for (std::list<QString>::iterator i = origDescription.begin(); i != origDescription.end();)
            {
                std::list<QString> ctx = toExtract::splitDescribe(*i);
                Utils::toShift(ctx);
                Utils::toShift(ctx);
                Utils::toShift(ctx);
                if (Utils::toShift(ctx) == "CONSTRAINT")
                {
                    QString name = Utils::toShift(ctx);
                    if (Utils::toShift(ctx) == "DEFINITION")
                        Constraints[name] = Utils::toShift(ctx);
                    i++;
                }
                else
                    i = origDescription.erase(i);  // Remove everything that isn't constraints
            }
            if ( OriginalDescription[table].empty() )
                OriginalDescription[table] = NewDescription[table];
        }
        Name->clear();
        for (std::map<QString, QString>::iterator i = Constraints.begin(); i != Constraints.end(); i++)
            Name->addItem((*i).first);

		SourceColList->changeObject(toCache::ObjectRef(Owner, Table));
        UniqueColList->changeObject(toCache::ObjectRef(Owner, Table));
        CheckColumn->changeObject(toCache::ObjectRef(Owner, Table));

        ReferTable->clear();
        //cache QList<toCache::CacheEntry const*> objects = connection().objects(false);
        //{
        //  for (QList<toCache::CacheEntry const*>::iterator i = objects.begin();i != objects.end();i++)
        //  {
        //      if ((*i)->type == toCache::TABLE && (*i)->name.first == Owner)
        //          ReferTable->addItem((*i)->name.second);
        //  }
        //}

        changeConstraint();
    }
    catch (const QString &exc)
    {
        Utils::toStatusMessage(exc);
        reject();
    }
}

QString toBrowserConstraint::sql()
{
    saveChanges();

    std::list<QString> origTable;
    std::list<QString> migrateTable;

    {
        for (std::map<QString, std::list<QString> >::iterator i = OriginalDescription.begin(); i != OriginalDescription.end(); i++)
        {
            std::list<QString> &cur = (*i).second;
            for (std::list<QString>::iterator j = cur.begin(); j != cur.end(); j++)
                Utils::toPush(origTable, *j);
        }
    }
    {
        for (std::map<QString, std::list<QString> >::iterator i = NewDescription.begin(); i != NewDescription.end(); i++)
        {
            std::list<QString> &cur = (*i).second;
            for (std::list<QString>::iterator j = cur.begin(); j != cur.end(); j++)
                Utils::toPush(migrateTable, *j);
        }
    }

    origTable.sort();
    migrateTable.sort();

    return Extractor.migrate(origTable, migrateTable);
}

QString toBrowserConstraint::table(void)
{
    if (Owner.isEmpty() && Table.isEmpty())
        return QString::null;
    return connection().getTraits().quote(Owner) + "." + connection().getTraits().quote(Table);
}

void toBrowserConstraint::saveChanges(void)
{
    if (table().isEmpty())
        return ;

    changeConstraint();
    std::list<QString> migrateTable;

    std::list<QString> ctx;
    Utils::toPush(ctx, Owner);
    Utils::toPush(ctx, QString("TABLE"));
    Utils::toPush(ctx, Table);
    Utils::toPush(ctx, QString("CONSTRAINT"));

    for (std::map<QString, QString>::iterator i = Constraints.begin(); i != Constraints.end(); i++)
        toExtract::addDescription(migrateTable, ctx, (*i).first, "DEFINITION", (*i).second);

    migrateTable.sort();
    NewDescription[table()] = migrateTable;
}

void toBrowserConstraint::displaySQL()
{
    toMemoEditor memo(this, sql(), -1, -1, true, true);
    memo.exec();
}

void toBrowserConstraint::addConstraint()
{
    if (Table.isEmpty())
        return ;
    bool ok = false;
    QString name = QInputDialog::getText(this,
                                         tr("Enter new constraint name"),
                                         tr("Enter name of new constraint."),
                                         QLineEdit::Normal,
                                         QString::null,
                                         &ok);
    if (ok)
    {
        Name->addItem(name);
        Name->setCurrentIndex(Name->count() - 1);
        changeConstraint();
    }
}

void toBrowserConstraint::delConstraint()
{
    if (Name->count() > 0)
    {
        Constraints.erase(Current);
        Current = QString::null;
        Name->removeItem(Name->currentIndex());
        changeConstraint();
    }
}

void toBrowserConstraint::parseConstraint(const QString &definition)
{
    if (definition.isEmpty())
    {
        Type->setEnabled(true);
        SourceColumns->clear();
        ReferedColumns->clear();
        UniqueCols->clear();
        CheckCondition->clear();
        return ;
    }

    toSQLParse::statement statement = toSQLParse::parseStatement(definition, connection());
    std::list<toSQLParse::statement>::iterator i = statement.subTokens().begin();
    if (i == statement.subTokens().end())
        return ;
    Type->setEnabled(false);

    QString type = (*i).String.toUpper();
    i++;
    if (type == "FOREIGN")
    {
        WidgetStack->setCurrentIndex(WidgetStack->indexOf(ReferentialPage));
        Type->setCurrentIndex(0);
        if (i == statement.subTokens().end())
            return ;

        while (i != statement.subTokens().end() && (*i).Type != toSQLParse::statement::List)
            i++;
        if (i != statement.subTokens().end())
            SourceColumns->setText(toSQLParse::indentStatement(*i, connection()));
        else
            return ;

        i++;  // Ending )
        if (i == statement.subTokens().end())
            return ;
        i++;  // REFERENCES
        if (i == statement.subTokens().end())
            return ;
        i++;  // Refered table finally
        if (i == statement.subTokens().end())
            return ;
        ReferTable->setItemText(ReferTable->currentIndex(), (*i).String);
        changeRefered();
        while (i != statement.subTokens().end() && (*i).Type != toSQLParse::statement::List)
            i++;
        if (i != statement.subTokens().end())
            ReferedColumns->setText(toSQLParse::indentStatement(*i, connection()));
    }
    else if (type == "PRIMARY" || type == "UNIQUE")
    {
        WidgetStack->setCurrentIndex(WidgetStack->indexOf(PrimaryPage));
        Type->setCurrentIndex(2);
        Primary->setChecked(type == "PRIMARY");
        Unique->setChecked(type == "UNIQUE");
        while (i != statement.subTokens().end() && (*i).Type != toSQLParse::statement::List)
            i++;
        if (i != statement.subTokens().end())
            UniqueCols->setText(toSQLParse::indentStatement(*i, connection()));
    }
    else if (type == "CHECK")
    {
        WidgetStack->setCurrentIndex(WidgetStack->indexOf(CheckPage));
        Type->setCurrentIndex(1);
        while (i != statement.subTokens().end() && (*i).Type != toSQLParse::statement::List)
            i++;
        if (i != statement.subTokens().end())
            CheckCondition->setText(toSQLParse::indentStatement(*i, connection()));
    }
}

void toBrowserConstraint::changeConstraint()
{
    if (!Current.isEmpty())
    {
        QString newDefinition;
        if (WidgetStack->currentIndex() == WidgetStack->indexOf(ReferentialPage))
        {
            QString table = ReferTable->currentText();
            QString sourceCols = SourceColumns->text();
            QString destinCols = ReferedColumns->text();
            if (!table.isEmpty() && !sourceCols.isEmpty() && !destinCols.isEmpty())
                newDefinition = "FOREIGN KEY (" + sourceCols + ") REFERENCES " + table + " (" + destinCols + ")";
        }
        else if (WidgetStack->currentIndex() == WidgetStack->indexOf(PrimaryPage))
        {
            QString columns = UniqueCols->text();
            if (!columns.isEmpty())
                newDefinition = (Primary->isChecked() ? "PRIMARY KEY (" : "UNIQUE (") + columns + ")";
        }
        else if (WidgetStack->currentIndex() == WidgetStack->indexOf(CheckPage))
        {
            QString checkCondition = CheckCondition->text();
            if (!checkCondition.isEmpty())
                newDefinition = "CHECK (" + checkCondition + ")";
        }
        if (!newDefinition.isEmpty() && !Current.isEmpty())
            Constraints[Current] = newDefinition;
    }
    Current = Name->currentText();
    if (Current.isEmpty())
    {
        parseConstraint(QString::null);
        Type->setEnabled(!Current.isEmpty());
    }
    else
        parseConstraint(Constraints[Current]);
    WidgetStack->setEnabled(!Current.isEmpty());
}

void toBrowserConstraint::changeRefered(void)
{
    QStringList parts = ReferTable->currentText().split(".");
    if (parts.size() > 1)
        ReferColList->changeObject(toCache::ObjectRef(parts[0], parts[1]));
    else
		ReferColList->changeObject(toCache::ObjectRef(toConnection::currentConnection(this).schema() ,ReferTable->currentText()));
}

void toBrowserConstraint::changeType(int type)
{
    switch (type)
    {
    case 0:
        WidgetStack->setCurrentIndex(WidgetStack->indexOf(ReferentialPage));
        break;
    case 1:
        WidgetStack->setCurrentIndex(WidgetStack->indexOf(CheckPage));
        break;
    case 2:
        WidgetStack->setCurrentIndex(WidgetStack->indexOf(PrimaryPage));
        break;
    }
}

void toBrowserConstraint::changeTable(const QString &table)
{
    saveChanges();
    describeTable(table);
}
