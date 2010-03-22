
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
#include <QDebug>

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
    dialog.exec();
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
    }
    else
    {

        std::list<toSQLParse::statement> statements = toSQLParse::parse(sql(), *cnct);
        try
        {
            QProgressDialog prog(tr("Performing table changes"),
                                 tr("Stop"),
                                 0,
                                 statements.size(),
                                 this);
            prog.setWindowTitle(tr("Performing table changes"));
            for (std::list<toSQLParse::statement>::iterator i = statements.begin(); i != statements.end(); i++)
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
        catch (const QString &str)
        {
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
    ColumnGridLayout->addWidget(new QLabel(tr("Not null")), 0, 2);
    ColumnGridLayout->addWidget(new QLabel(tr("Default")), 0, 3);
    ColumnGridLayout->addWidget(new QLabel(tr("Comments")), 0, 4);
//    ColumnGridLayout->addWidget(new QLabel(tr("Extra parameters")), 0, 5);

    ColumnGridLayout->setColumnStretch(0, 1);
    ColumnGridLayout->setColumnStretch(1, 1);
    ColumnGridLayout->setColumnStretch(2, 0);
    ColumnGridLayout->setColumnStretch(3, 1);
    ColumnGridLayout->setColumnStretch(4, 1);
//    ColumnGridLayout->setColumnStretch(5, 1);

    Extractor.setIndexes(false);
    Extractor.setConstraints(false);
    Extractor.setPrompt(false);
    Extractor.setHeading(false);

//    connect(ButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
//    connect(ButtonBox, SIGNAL(rejected()), this, SLOT(reject()));

    UglyFlag = false; // Indicates wether the correct size has been
    // retreived at least once.

    try
    {
        Owner = owner;
        Table = cnct->quote(table);

        QString tablespace;
        Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));
        Schema->setSelected(Owner);

        if (!Table.isEmpty())
        {
            // Editing existing table. Extract current description of table.

            std::list<QString> Objects;
            Objects.insert(Objects.end(), "TABLE:" + Owner + "." + table);

            OriginalDescription = Extractor.describe(Objects);
            tablespaceSpecified = false;
            Schema->setEnabled(false);

            bool invalid = false;

            Name->setText(Table);

            QString declarations;
            QString storage;
            QString parallel;

            for (std::list<QString>::iterator i = OriginalDescription.begin(); i != OriginalDescription.end(); i++)
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
                        // TODO: currently toOracleExtract::describe is NOT always returning name of
                        //       tablespace so this one will never be called. Should be fixed.
                        tablespace = connection().unQuote(t.mid(10).trimmed());
                        tablespaceSpecified = true;
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
                for (unsigned int i = 0; i < Columns.size(); i++)
                    addColumn();
            }
            std::list<QLineEdit *>::iterator name = ColumnNames.begin();
            std::list<toDatatype *>::iterator datatype = Datatypes.begin();
            std::list<QCheckBox *>::iterator notnull = NotNulls.begin();
            std::list<QLineEdit *>::iterator defaultValue = Defaults.begin();
//            std::list<QLineEdit *>::iterator extra = Extra.begin();
            std::list<QLineEdit *>::iterator comment = Comments.begin();
            for (std::list<toExtract::columnInfo>::iterator column = Columns.begin();
                    name != ColumnNames.end() &&
                    datatype != Datatypes.end() &&
                    notnull != NotNulls.end() &&
                    defaultValue != Defaults.end() &&
//                    extra != Extra.end() &&
                    comment != Comments.end() &&
                    column != Columns.end();
                    name++, datatype++, notnull++, defaultValue++, /*extra++,*/ comment++, column++)
            {
                if ((*column).Order == 0)
                    invalid = true;
                (*name)->setText((*column).Name);
                (*datatype)->setType((*column).Definition);
                if ((*column).bNotNull)
                    (*notnull)->setChecked(true);
                (*defaultValue)->setText((*column).DefaultValue);
//                (*extra)->setText((*column).Data["EXTRA"]);
                (*comment)->setText((*column).Comment);
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
        {
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
        if ((beg->String == "BUFFER_POOL") ||
                (beg->String == "FREELISTS") ||
                (beg->String == "INITIAL") ||
                (beg->String == "MAXEXTENTS") ||
                (beg->String == "MINEXTENTS") ||
                (beg->String == "NEXT") ||
                (beg->String == "PCTINCREASE") ||
                (beg->String == "INITRANS") ||
                (beg->String == "MAXTRANS") ||
                (beg->String == "PCTFREE") ||
                (beg->String == "DEGREE") ||
                (beg->String == "INSTANCES") ||
                (beg->String == "ORGANIZATION") ||
                (beg->String == "TABLESPACE") ||
                (beg->String == "GROUPS"))
        {
            // process parameters with one argument
            end++;
            end++;
            toExtract::addDescription(migrateTable, ctx, type, Extractor.createFromParse(beg, end));
            beg = end;
        }
        else
        {
            if (beg->String == "FREELIST")
            {
                // process parameters with two arguments
                end++;
                end++;
                end++;
                toExtract::addDescription(migrateTable, ctx, type, Extractor.createFromParse(beg, end));
                beg = end;
            }
            else
            {
                // process parameters without any arguments
                end++;
                toExtract::addDescription(migrateTable, ctx, type, Extractor.createFromParse(beg, end));
                beg = end;
            }
        }
        /*        if (beg != end)
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
                    end++;*/
    }
    if (beg != end)
        toExtract::addDescription(migrateTable, ctx, type, Extractor.createFromParse(beg, end).trimmed());
} // addParameters

// Generates and displays sql statements required to perform chosen operation
// (creation of new or modifying existing table).
QString toBrowserTable::sql()
{
    std::list<QString> migrateTable; // list of complex (\01 separated) strings to be sent to extractor

    std::list<QString> ctx; // context to be added to all strings in migrateTable (f.e. owner, TABLE, tablename)
    toPush(ctx, cnct->quote(Owner));
    toPush(ctx, QString("TABLE"));
    if (Table.isEmpty())
        toPush(ctx, Name->text());
    else
        toPush(ctx, Table);
    toExtract::addDescription(migrateTable, ctx);
    if (Name->text() != Table && !Table.isEmpty())
        toExtract::addDescription(migrateTable, ctx, "RENAME", Name->text());

    // iterating through columns as they have been upon loading table data (before any changes)
    std::list<toExtract::columnInfo>::const_iterator column = Columns.begin();
    // iterating through column data as it is (after changes)
    std::list<QLineEdit *>::const_iterator name = ColumnNames.begin();
    std::list<toDatatype *>::const_iterator datatype = Datatypes.begin();
    std::list<QCheckBox *>::iterator notnull = NotNulls.begin();
    std::list<QLineEdit *>::iterator defaultValue = Defaults.begin();
//    std::list<QLineEdit *>::const_iterator extra = Extra.begin();
    std::list<QLineEdit *>::const_iterator comment = Comments.begin();
    int num = 1; // order number, used to generate ORDERn rows
    while (name != ColumnNames.end() &&
            datatype != Datatypes.end() &&
            notnull != NotNulls.end() &&
            defaultValue != Defaults.end() &&
//           extra != Extra.end() &&
            comment != Comments.end())
    {
        QString cname;
        QString cdatatype;
        if (column != Columns.end())
        {
            cname = (*column).Name;

            // add rename instruction if column name has changed
            if ((*name)->text() != cname)
                toExtract::addDescription(migrateTable, ctx,
                                          "COLUMN", cname,
                                          "RENAME", (*name)->text());

            column++;
        }
        else
        {
            cname = (*name)->text();
        }

        // add column name
        toExtract::addDescription(migrateTable, ctx, "COLUMN", cname);

        // add extra info (currently only "not null" property is supported, what else should there be?)
        if ((*notnull)->isChecked())
            toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "EXTRA", "NOT NULL");

        // add comment info
        if (!(*comment)->text().isEmpty())
            toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "COMMENT", (*comment)->text());

        // could this be used for something?
//        if (!(*extra)->text().isEmpty())
//            toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "EXTRA", (*extra)->text());

        // add order info
        toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, "ORDER", QString::number(num++));

        // add datatype row
        cdatatype = (*datatype)->type();
        if (!(*defaultValue)->text().isEmpty())
            cdatatype += " DEFAULT '" + (*defaultValue)->text() + "'"; // space at the end - workaround
        toExtract::addDescription(migrateTable, ctx, "COLUMN", cname, cdatatype);

        name++;
        datatype++;
        notnull++;
        defaultValue++;
//        extra++;
        comment++;
    }

    if (!Comment->text().isEmpty())
        toExtract::addDescription(migrateTable, ctx, "COMMENT", Comment->text());
    addParameters(migrateTable, ctx, "STORAGE", StorageDeclarations->text());
    addParameters(migrateTable, ctx, "PARALLEL", ParallelDeclarations->text());
    addParameters(migrateTable, ctx, "PARAMETERS", ExtraDeclarations->text());
    // when creating new table tablespace must always be specified
    if (OriginalDescription.empty())
    {
        addParameters(migrateTable, ctx, "TABLESPACE", cnct->quote(Tablespace->currentText()));
    }
    else
    {
        // when adjusting table tablespace is specified only
        // if it was specified in original description
        if (tablespaceSpecified)
        {
            // "quote" should be used for tablespace, but for tablespace
            // it will always do toLower anyway
            addParameters(migrateTable, ctx, "PARAMETERS", "TABLESPACE " + cnct->quote(Tablespace->currentText()));
        }
    }

    migrateTable.sort();

    return Extractor.migrate(OriginalDescription, migrateTable);
} // sql

// Calls sql() to generate required sql statement(s) and then displays it(them).
// If no changes are to be done - warning is displayed.
void toBrowserTable::displaySQL()
{
    QString statements = sql();

    if (!statements.isNull())
    {
        toMemoEditor memo(this, statements, -1, -1, true, true);
        memo.exec();
    }
    else
    {
        QMessageBox::warning(0, qApp->translate("toStatusMessage", "TOra Message"), "No changes.");
    }
} // displaySQL

void toBrowserTable::addColumn()
{
    ColumnNumber++;
    QLineEdit *tl;
    QCheckBox *cb;
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

    cb = new QCheckBox;
    ColumnGridLayout->addWidget(cb, ColumnNumber, 2);
    cb->setObjectName(QString::number(ColumnNumber));
    cb->show();
    NotNulls.insert(NotNulls.end(), cb);

    tl = new QLineEdit;
    ColumnGridLayout->addWidget(tl, ColumnNumber, 3);
    tl->setObjectName(QString::number(ColumnNumber));
    tl->show();
    Defaults.insert(Defaults.end(), tl);

    tl = new QLineEdit;
    ColumnGridLayout->addWidget(tl, ColumnNumber, 4);
    tl->setObjectName(QString::number(ColumnNumber));
    tl->show();
    Comments.insert(Comments.end(), tl);

//    tl = new QLineEdit;
//    ColumnGridLayout->addWidget(tl, ColumnNumber, 5);
//    tl->setObjectName(QString::number(ColumnNumber));
//    tl->show();
//    Extra.insert(Extra.end(), tl);
}

// removes currently selected column
void toBrowserTable::removeColumn()
{
    QString toBeRemoved = qApp->focusWidget()->objectName();

    std::list<QLineEdit *>::const_iterator name = ColumnNames.begin();
    std::list<toDatatype *>::const_iterator datatype = Datatypes.begin();
    std::list<QCheckBox *>::iterator notnull = NotNulls.begin();
    std::list<QLineEdit *>::iterator defaultValue = Defaults.begin();
//    std::list<QLineEdit *>::const_iterator extra = Extra.begin();
    std::list<QLineEdit *>::const_iterator comment = Comments.begin();
    std::list<toExtract::columnInfo>::const_iterator column = Columns.begin();
    while (name != ColumnNames.end() &&
            datatype != Datatypes.end() &&
            notnull != NotNulls.end() &&
            defaultValue != Defaults.end() &&
//           extra != Extra.end() &&
            comment != Comments.end() &&
            column != Columns.end()
          )
    {
        if ((*name)->objectName() == toBeRemoved)
        {
            ColumnGridLayout->removeWidget(*name);
            ColumnNames.remove(*name);
            delete *name;

            ColumnGridLayout->removeWidget(*datatype);
            Datatypes.remove(*datatype);
            delete *datatype;

            ColumnGridLayout->removeWidget(*notnull);
            NotNulls.remove(*notnull);
            delete *notnull;

            ColumnGridLayout->removeWidget(*defaultValue);
            Defaults.remove(*defaultValue);
            delete *defaultValue;

//           ColumnGridLayout->removeWidget(*extra);
//           Extra.remove(*extra);
//           delete *extra;

            ColumnGridLayout->removeWidget(*comment);
            Comments.remove(*comment);
            delete *comment;

            Columns.remove(*column);
            return;
        }
        name++;
        datatype++;
        notnull++;
        defaultValue++;
//       extra++;
        comment++;
        column++;
    } // while
#ifdef DEBUG
    qDebug() << "Could not find column to delete.";
#endif
} // removeColumn

void toBrowserTable::toggleCustom(bool val)
{
    for (std::list<toDatatype *>::iterator i = Datatypes.begin(); i != Datatypes.end(); i++)
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
