
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

#include "tobrowserconstraint.h"
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
#include <qradiobutton.h>
#include <qtimer.h>

#include <QStackedWidget>
#include <QProgressDialog>


void toBrowserConstraint::modifyConstraint(toConnection &conn, const QString &owner, const QString &table, QWidget *parent)
{
    toBrowserConstraint dialog(conn, owner, table, parent);
    if (dialog.exec())
    {
        dialog.changeConstraint();
        std::list<toSQLParse::statement> statements = toSQLParse::parse(dialog.sql(), conn);
        try
        {
            QProgressDialog prog(tr("Performing constraint changes"),
                                 tr("Stop"),
                                 0,
                                 statements.size(),
                                 &dialog);
            prog.setWindowTitle(tr("Performing constraint changes"));
            for (std::list<toSQLParse::statement>::iterator j = statements.begin();j != statements.end();j++)
            {
                QString sql = toSQLParse::indentStatement(*j, conn);
                int i = sql.length() - 1;
                while (i >= 0 && (sql.at(i) == ';' || sql.at(i).isSpace()))
                    i--;
                if (i >= 0)
                    conn.execute(sql.mid(0, i + 1));
                qApp->processEvents();
                if (prog.wasCanceled())
                    throw tr("Canceled ongoing constraint modification, constraints might be corrupt");
            }
        }
        TOCATCH
    }
}

toBrowserConstraint::toBrowserConstraint(toConnection &conn, const QString &owner, const QString &table, QWidget *parent, const char *name)
        : QDialog(parent), toConnectionWidget(conn, this), Extractor(conn, NULL)
{
    setObjectName(name);
    setModal(true);

    setupUi(this);
    Extractor.setIndexes(false);
    Extractor.setConstraints(true);
    Extractor.setPrompt(false);
    Extractor.setHeading(false);

    if (toIsMySQL(connection()))
        UniqueType->hide();

    SourceColList->displayHeader(false);
    UniqueColList->displayHeader(false);
    CheckColumn->displayHeader(false);
    ReferColList->displayHeader(false);

    if (!owner.isEmpty() && !table.isEmpty())
    {
        QString fulltable = conn.quote(owner) + "." + conn.quote(table);
        TableSelect->setTable(fulltable);
        describeTable(fulltable);
    }
    else if (!owner.isEmpty())
        TableSelect->setTable(conn.quote(owner));

    connect(TableSelect, SIGNAL(selectTable(const QString &)), this, SLOT(changeTable(const QString &)));
}

void toBrowserConstraint::describeTable(const QString &table)
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

            for (std::list<QString>::iterator i = origDescription.begin();i != origDescription.end();)
            {
                std::list<QString> ctx = toExtract::splitDescribe(*i);
                toShift(ctx);
                toShift(ctx);
                toShift(ctx);
                if (toShift(ctx) == "CONSTRAINT")
                {
                    QString name = toShift(ctx);
                    if (toShift(ctx) == "DEFINITION")
                        Constraints[name] = toShift(ctx);
                    i++;
                }
                else
                    i = origDescription.erase(i);  // Remove everything that isn't constraints
            }
            if ( OriginalDescription[table].empty() )
                OriginalDescription[table] = NewDescription[table];
        }
        Name->clear();
        for (std::map<QString, QString>::iterator i = Constraints.begin();i != Constraints.end();i++)
            Name->addItem((*i).first);

        SourceColList->changeParams(Owner, Table);
        UniqueColList->changeParams(Owner, Table);
        CheckColumn->changeParams(Owner, Table);

        ReferTable->clear();
        std::list<toConnection::objectName> objects = connection().objects(false);
        {
            for (std::list<toConnection::objectName>::iterator i = objects.begin();i != objects.end();i++)
            {
                if ((*i).Type == "TABLE" && (*i).Owner == Owner)
                    ReferTable->addItem((*i).Name);
            }
        }

        changeConstraint();
    }
    catch (const QString &exc)
    {
        toStatusMessage(exc);
        reject();
    }
}

QString toBrowserConstraint::sql()
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

QString toBrowserConstraint::table(void)
{
    if (Owner.isEmpty() && Table.isEmpty())
        return QString::null;
    return connection().quote(Owner) + "." + connection().quote(Table);
}

void toBrowserConstraint::saveChanges(void)
{
    if (table().isEmpty())
        return ;

    changeConstraint();
    std::list<QString> migrateTable;

    std::list<QString> ctx;
    toPush(ctx, Owner);
    toPush(ctx, QString("TABLE"));
    toPush(ctx, Table);
    toPush(ctx, QString("CONSTRAINT"));

    for (std::map<QString, QString>::iterator i = Constraints.begin();i != Constraints.end();i++)
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
        ReferColList->changeParams(parts[0], parts[1]);
    else
        ReferColList->changeParams(ReferTable->currentText());
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
