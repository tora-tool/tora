
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
#include <qmessagebox.h>
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
static toSQL SQLDefaultTablespace("toBrowserTable:DefaultTablespaces",
                                  "select default_tablespace from dba_users where username = user",
                                  "List the default tablespace for current user.");



void toBrowserTable::editTable(toConnection &conn, const QString &owner, const QString &table, QWidget *parent)
{
    toBrowserTable dialog(conn, owner, table, parent);
    if (dialog.exec())
    {
/*
  TS 2009-10-31 Moved this code to done() as this should be performed before actually
                closing dialog so that in case of error there would be a chance to fix it.
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
        TOCATCH;*/
    }
}

static QString getDefaultTablespace(toConnection &conn)
{
    toQList result;
    result = toQuery::readQuery(conn, SQLDefaultTablespace);
    return toShift(result);
} // getDefaultTablespace

// handle acceptance of table dialog (f.e. pressing OK button)
// all actions should be done here so that in case of error user would have
// a chance to fix parameters and retry creation/modification of table
void toBrowserTable::done(int r)
{
    if (!r)
    {
        QDialog::done(r);
        return;
    } else {

        std::list<toSQLParse::statement> statements = toSQLParse::parse(sql(), *cnct);
        try
        {
            QProgressDialog prog(tr("Performing table changes"),
                                 tr("Stop"),
                                 0,
                                 statements.size(),
                                 this);
            prog.setWindowTitle(tr("Performing table changes"));
            for (std::list<toSQLParse::statement>::iterator i = statements.begin();i != statements.end();i++)
            {
                QString sql = toSQLParse::indentStatement(*i, *cnct);
                int l = sql.length() - 1;
                while (l >= 0 && (sql.at(l) == ';' || sql.at(l).isSpace()))
                    l--;
                if (l >= 0)
                    cnct->execute(sql.mid(0, l + 1));
                qApp->processEvents();
                if (prog.wasCanceled())
                    throw tr("Canceled ongoing table modification, table might be corrupt");
            }
            QDialog::done(r);
        }
// Cannot use standard TOCATCH. Table creation dialog always pop'us
// above standard error message window.
// TODO: play around with modal/nonmodal properties to fix this
//        TOCATCH;
	catch (const QString &str) {                \
		QMessageBox::warning(0,
				qApp->translate("toStatusMessage", "TOra Message"),
				str
				);
	}
    }
} // toBrowserTable::done

toBrowserTable::toBrowserTable(toConnection &conn,
                               const QString &owner,
                               const QString &table,
                               QWidget *parent,
                               const char *name)
        : QDialog(parent), toConnectionWidget(conn, this), Extractor(conn, NULL)
{
    setupUi(this);

    cnct = &conn;

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

    connect(ButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

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
            // Editing existing table. Extract current description of table.

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
                        // TODO: currently toOracleExtract::describe is NOT returning name of
                        //       tablespace so this one will never be called. Should be fixed.
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
        else {
            // create new column, add first column and select default tablespace
            addColumn();
            tablespace = getDefaultTablespace(connection()); // TODO: NEW connection??!!
        }
        if (toIsMySQL(connection()))
        {
            ParallelLabel->hide();
            ParallelDeclarations->hide();
            SchemaLabel->setText(tr("&Database"));
        }
        // set-up tablespace combo box
        try
        {
            // get a list of tablespaces
            toQuery query(connection(), SQLListTablespaces); // TODO: does this create NEW connection??!!
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
    toPush(ctx, Owner.toLower());
    toPush(ctx, QString("TABLE"));
    if (Table.isEmpty())
        toPush(ctx, Name->text());
    else
        toPush(ctx, Table.toLower());
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

            // add modify instruction if datatype has changed
            // remove any spaces in datatype (for exampe NUMBER  (2) -> NUMBER(2))
            if ((*datatype)->type().remove(" ") != (*column).Definition)
                toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "MODIFY", (*datatype)->type());

            // add this anyways to match later with column type row in OriginalDescription
            toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, (*column).Definition);

            // add rename instruction if column name has changed
            if ((*name)->text() != (*column).Name)
                toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "RENAME", (*name)->text());
            column++;
        }
        else {
            cname = (*name)->text();
            toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, (*datatype)->type());
        }

        toExtract::addDescription(migrateTable, ctx, "COLUMN", cname);
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
    // when creating new table tablespace must be specified
    if (OriginalDescription.empty()) {
        addParameters(migrateTable, ctx, "TABLESPACE", Tablespace->currentText());
    }

    migrateTable.sort();

    return Extractor.migrate(OriginalDescription, migrateTable);
}

// Generates and displays sql statements required to perform chosen operation
// (creation of new or modifying existing table).
// If no changes are to be done - warning is displayed.
void toBrowserTable::displaySQL()
{
    QString statements = sql();

    if (!statements.isNull()) {
        toMemoEditor memo(this, statements, -1, -1, true, true);
        memo.exec();
    } else {
        QMessageBox::warning(0, qApp->translate("toStatusMessage", "TOra Message"), "No changes.");
    }
} // displaySQL

void toBrowserTable::addColumn()
{
    ColumnNumber++;
    QLineEdit *tl;
    toDatatype *td;

    tl = new QLineEdit;
    ColumnGridLayout->addWidget(tl, ColumnNumber, 0);
    tl->setObjectName(QString::number(ColumnNumber)); // name will be used when deleting
    tl->show();
    ColumnNames.insert(ColumnNames.end(), tl);

    td = new toDatatype(connection(), ColumnGrid);
    ColumnGridLayout->addWidget(td, ColumnNumber, 1);
    td->setObjectName(QString::number(ColumnNumber));
    td->show();
    Datatypes.insert(Datatypes.end(), td);

    tl = new QLineEdit;
    ColumnGridLayout->addWidget(tl, ColumnNumber, 2);
    tl->setObjectName(QString::number(ColumnNumber));
    tl->show();
    Extra.insert(Extra.end(), tl);
}

// removes currently selected column
void toBrowserTable::removeColumn()
{
    QString toBeRemoved = qApp->focusWidget()->objectName();

    std::list<QLineEdit *>::const_iterator name = ColumnNames.begin();
    std::list<toDatatype *>::const_iterator datatype = Datatypes.begin();
    std::list<QLineEdit *>::const_iterator extra = Extra.begin();
    std::list<toExtract::columnInfo>::const_iterator column = Columns.begin();
    while (name != ColumnNames.end() &&
           datatype != Datatypes.end() &&
           extra != Extra.end() &&
           column != Columns.end()
          )
    {
       if ((*name)->objectName() == toBeRemoved) {
           ColumnGridLayout->removeWidget(*name);
           ColumnNames.remove(*name);
           delete *name;
           ColumnGridLayout->removeWidget(*datatype);
           Datatypes.remove(*datatype);
           delete *datatype;
           ColumnGridLayout->removeWidget(*extra);
           Extra.remove(*extra);
           delete *extra;
           Columns.remove(*column);
           return;
       }
       name++;
       datatype++;
       extra++;
       column++;
    } // while
printf("Could not find column to delete.\n");
} // removeColumn

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
