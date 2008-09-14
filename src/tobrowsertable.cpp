
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#include "tobrowsertable.h"
#include "toconnection.h"
#include "todatatype.h"
#include "toextract.h"
#include "tohighlightedtext.h"
#include "tomemoeditor.h"
#include "toresultcombo.h"
#include "tosql.h"

#include <qapplication.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtimer.h>

#include <QResizeEvent>
#include <QProgressDialog>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>


static toSQL SQLListTablespaces("toBrowserTable:ListTablespaces",
                                "SELECT Tablespace_Name FROM sys.DBA_TABLESPACES\n"
                                " ORDER BY Tablespace_Name",
                                "List the available tablespaces in a database.");

void toBrowserTable::editTable(toConnection &conn, const QString &owner, const QString &table, QWidget *parent)
{
    toBrowserTable dialog(conn, owner, table, parent);
    if (dialog.exec())
    {
        std::list<toSQLParse::statement> statements = toSQLParse::parse(dialog.sql(), conn);
        try
        {
            QProgressDialog prog(tr("Performing table changes"),
                                 tr("Stop"),
                                 0,
                                 statements.size(),
                                 &dialog);
            prog.setWindowTitle(tr("Performing table changes"));
            for (std::list<toSQLParse::statement>::iterator i = statements.begin();i != statements.end();i++)
            {
                QString sql = toSQLParse::indentStatement(*i, conn);
                int l = sql.length() - 1;
                while (l >= 0 && (sql.at(l) == ';' || sql.at(l).isSpace()))
                    l--;
                if (l >= 0)
                    conn.execute(sql.mid(0, l + 1));
                qApp->processEvents();
                if (prog.wasCanceled())
                    throw tr("Canceled ongoing table modification, table might be corrupt");
            }
        }
        TOCATCH;
    }
}

toBrowserTable::toBrowserTable(toConnection &conn,
                               const QString &owner,
                               const QString &table,
                               QWidget *parent,
                               const char *name)
        : QDialog(parent), toConnectionWidget(conn, this), Extractor(conn, NULL)
{
    setupUi(this);

    // the central widget to the scrollarea is a widget with a vbox
    // layout so the qgrid doesn't take all the vertical space.

    QWidget *box = new QWidget(ColumnList);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    box->setLayout(vbox);

    ColumnList->setWidget(box);
    ColumnList->setWidgetResizable(true);

    ColumnGrid = new QWidget(box);
    vbox->addWidget(ColumnGrid);
    vbox->addItem(new QSpacerItem(
                      0,
                      0,
                      QSizePolicy::Minimum,
                      QSizePolicy::MinimumExpanding));

    ColumnGridLayout = new QGridLayout;
    ColumnGridLayout->setSpacing(0);
    ColumnGridLayout->setContentsMargins(0, 0, 0, 0);
    ColumnGrid->setLayout(ColumnGridLayout);

    ColumnNumber = 0;
    ColumnGridLayout->addWidget(new QLabel(tr("Name")), 0, 0);
    ColumnGridLayout->addWidget(new QLabel(tr("Datatype")), 0, 1);
    ColumnGridLayout->addWidget(new QLabel(tr("Extra parameters")), 0, 2);

    ColumnGridLayout->setColumnStretch(0, 1);
    ColumnGridLayout->setColumnStretch(1, 1);
    ColumnGridLayout->setColumnStretch(2, 1);

    Extractor.setIndexes(false);
    Extractor.setConstraints(false);
    Extractor.setPrompt(false);
    Extractor.setHeading(false);

    UglyFlag = false; // Indicates wether the correct size has been
    // retreived at least once.

    try
    {
        Owner = owner;
        Table = table;

        QString tablespace;
        Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
        Schema->setSelected(Owner);

        if (!Table.isEmpty())
        {
            std::list<QString> Objects;
            Objects.insert(Objects.end(), "TABLE:" + Owner + "." + Table);

            OriginalDescription = Extractor.describe(Objects);
            Schema->setEnabled(false);

            bool invalid = false;

            Name->setText(table);

            QString declarations;
            QString storage;
            QString parallel;

            for (std::list<QString>::iterator i = OriginalDescription.begin();i != OriginalDescription.end();i++)
            {
                std::list<QString> row = toExtract::splitDescribe(*i);
                if (toShift(row) != connection().quote(owner))
                {
                    invalid = true;
                    break;
                }
                if (toShift(row) != "TABLE")
                    continue;
                if (toShift(row) != connection().quote(table))
                {
                    invalid = true;
                    break;
                }
                QString type = toShift(row);
                if (type == "COLUMN")
                {
                    // Nop, handled by the parseColumnDescription call below
                }
                else if (type == "COMMENT")
                    Comment->setText(toShift(row));
                else if (type == "STORAGE")
                    storage += toShift(row) + " ";
                else if (type == "PARALLEL")
                    parallel += toShift(row) + " ";
                else if (type == "PARAMETERS")
                {
                    QString t = toShift(row);
                    if (t.startsWith("TABLESPACE"))
                    {
                        tablespace = connection().unQuote(t.mid(10).trimmed());
                    }
                    else
                        declarations += t + " ";
                }
                else if (!type.isEmpty())
                    invalid = true;
            }

            Columns = toExtract::parseColumnDescription(OriginalDescription);

            ExtraDeclarations->setText(declarations.trimmed());
            StorageDeclarations->setText(storage.trimmed());
            ParallelDeclarations->setText(parallel.trimmed());
            {
                for (unsigned int i = 0;i < Columns.size();i++)
                    addColumn();
            }
            std::list<QLineEdit *>::iterator name = ColumnNames.begin();
            std::list<toDatatype *>::iterator datatype = Datatypes.begin();
            std::list<QLineEdit *>::iterator extra = Extra.begin();
            for (std::list<toExtract::columnInfo>::iterator column = Columns.begin();
                    name != ColumnNames.end() &&
                    datatype != Datatypes.end() &&
                    extra != Extra.end() &&
                    column != Columns.end();
                    name++, datatype++, extra++, column++)
            {

                if ((*column).Order == 0)
                    invalid = true;
                (*name)->setText((*column).Name);
                (*datatype)->setType((*column).Definition);
                (*extra)->setText((*column).Data["EXTRA"]);
            }
            if (invalid)
            {
                reject();
                toStatusMessage("Invalid output from extraction layer prevents "
                                "this dialog from being properly filled in.");
                return ;
            }
        }
        else
            addColumn();
        if (toIsMySQL(connection()))
        {
            ParallelLabel->hide();
            ParallelDeclarations->hide();
            SchemaLabel->setText(tr("&Database"));
        }
        try
        {
            toQuery query(connection(), SQLListTablespaces);
            while (!query.eof())
            {
                QString t = query.readValueNull();
                Tablespace->addItem(t);
                if (t == tablespace)
                    Tablespace->setCurrentIndex(Tablespace->count() - 1);
            }
        }
        catch (...)
        {
            TablespaceLabel->hide();
            Tablespace->hide();
        }
    }
    catch (const QString &exc)
    {
        toStatusMessage(exc);
        reject();
    }
    uglyWorkaround();
}

void toBrowserTable::addParameters(std::list<QString> &migrateTable,
                                   const std::list<QString> &ctx,
                                   const QString &type,
                                   const QString &data)
{
    toSQLParse::statement statement = toSQLParse::parseStatement(data, connection());
    std::list<toSQLParse::statement>::iterator beg = statement.subTokens().begin();
    std::list<toSQLParse::statement>::iterator end = beg;
    while (end != statement.subTokens().end())
    {
        if (beg != end)
        {
            if ((*end).String != "=")
            {
                if ((*end).String == "(")
                {
                    do
                    {
                        end++;
                    }
                    while (end != statement.subTokens().end() && (*end).String != ")");
                }
                end++;
                toExtract::addDescription(migrateTable, ctx, type, Extractor.createFromParse(beg, end));
                beg = end;
            }
            else
                end++;
        }
        else
            end++;
    }
    if (beg != end)
        toExtract::addDescription(migrateTable, ctx, type, Extractor.createFromParse(beg, end).trimmed());
}

QString toBrowserTable::sql()
{
    std::list<QString> migrateTable;

    std::list<QString> ctx;
    toPush(ctx, Owner);
    toPush(ctx, QString("TABLE"));
    if (Table.isEmpty())
        toPush(ctx, Name->text());
    else
        toPush(ctx, Table);
    toExtract::addDescription(migrateTable, ctx);
    if (Name->text() != Table && !Table.isEmpty())
        toExtract::addDescription(migrateTable, ctx, "RENAME", Name->text());

    std::list<toExtract::columnInfo>::const_iterator column = Columns.begin();
    std::list<QLineEdit *>::const_iterator name = ColumnNames.begin();
    std::list<toDatatype *>::const_iterator datatype = Datatypes.begin();
    std::list<QLineEdit *>::const_iterator extra = Extra.begin();
    int num = 1;
    while (name != ColumnNames.end() && datatype != Datatypes.end() && extra != Extra.end())
    {
        QString cname;
        if (column != Columns.end())
        {
            cname = (*column).Name;
            if ((*name)->text() != (*column).Name)
                toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "RENAME", (*name)->text());
            column++;
        }
        else
            cname = (*name)->text();

        toExtract::addDescription(migrateTable, ctx, "COLUMN", cname);
        toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, (*datatype)->type());
        if (!(*extra)->text().isEmpty())
            toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "EXTRA", (*extra)->text());
        toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "ORDER", QString::number(num++));
        name++;
        datatype++;
        extra++;
    }

    if (!Comment->text().isEmpty())
        toExtract::addDescription(migrateTable, ctx, "COMMENT", Comment->text());
    addParameters(migrateTable, ctx, "STORAGE", StorageDeclarations->text());
    addParameters(migrateTable, ctx, "PARALLEL", ParallelDeclarations->text());
    addParameters(migrateTable, ctx, "PARAMETERS", ExtraDeclarations->text());

    migrateTable.sort();

    return Extractor.migrate(OriginalDescription, migrateTable);
}

void toBrowserTable::displaySQL()
{
    toMemoEditor memo(this, sql(), -1, -1, true, true);
    memo.exec();
}

void toBrowserTable::addColumn()
{
    ColumnNumber++;
    QLineEdit *tl;
    toDatatype *td;

    tl = new QLineEdit;
    ColumnGridLayout->addWidget(tl, ColumnNumber, 0);
    tl->setObjectName(QString::number(ColumnNumber));
    tl->show();
    ColumnNames.insert(ColumnNames.end(), tl);

    td = new toDatatype(connection(), ColumnGrid);
    ColumnGridLayout->addWidget(td, ColumnNumber, 1);
    td->show();
    Datatypes.insert(Datatypes.end(), td);

    tl = new QLineEdit;
    ColumnGridLayout->addWidget(tl, ColumnNumber, 2);
    tl->show();
    Extra.insert(Extra.end(), tl);
}

void toBrowserTable::removeColumn()
{
    // Not implemented yet
}

void toBrowserTable::toggleCustom(bool val)
{
    for (std::list<toDatatype *>::iterator i = Datatypes.begin();i != Datatypes.end();i++)
        (*i)->setCustom(val);
}

void toBrowserTable::uglyWorkaround()
{
    // Somehome the size doesn't get updated until way later so just
    // keep calling until it gets set.
    if (ColumnList->width() > 220 || UglyFlag)
    {
        ColumnGrid->setFixedWidth(ColumnList->width() - 30);
        UglyFlag = true;
    }
    else
        QTimer::singleShot(100, this, SLOT(uglyWorkaround()));
}

void toBrowserTable::resizeEvent(QResizeEvent *e)
{
    QDialog::resizeEvent(e);
    uglyWorkaround();
}
