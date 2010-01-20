
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

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"
#include "tohighlightedtext.h"
#include "toinvalid.h"
#include "tomain.h"
#include "toresultextract.h"
#include "toresulttableview.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#include <qsplitter.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <QMdiArea>

#include <QPixmap>
#include <QProgressDialog>
#include <QList>

#include "icons/refresh.xpm"
#include "icons/toinvalid.xpm"
#include "icons/compile.xpm"

static toSQL SQLListInvalid("toInvalid:ListInvalid",
                            "SELECT owner \"Owner\",object_name \"Object\",object_type \"Type\",status \"Status\"\n"
                            "  FROM sys.all_objects\n"
                            " WHERE status <> 'VALID'",
                            "Get invalid objects, must have same first three columns.");

static toSQL SQLListSource("toInvalid:ListSource",
                           "SELECT Text FROM SYS.ALL_SOURCE\n"
                           " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]> AND type = :f3<char[101]>",
                           "List source of an object.");

static toSQL SQLReadErrors("toInvalid:ReadErrors",
                           "SELECT Line-1,Text FROM sys.All_Errors\n"
                           " WHERE OWNER = :f1<char[101]>\n"
                           "   AND NAME = :f2<char[101]>\n"
                           "   AND TYPE = :f3<char[101]>",
                           " ORDER BY Type,Line",
                           "Get lines with errors in object (Observe first line 0)");


class toInvalidTool : public toTool
{
    virtual const char **pictureXPM(void)
    {
        return const_cast<const char**>(toinvalid_xpm);
    }
public:
    toInvalidTool()
            : toTool(130, "Invalid Objects") { }
    virtual const char *menuItem()
    {
        return "Invalid Objects";
    }
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
    {
        return new toInvalid(parent, connection);
    }
    virtual void closeWindow(toConnection &connection) {};
};


static toInvalidTool InvalidTool;


toInvalid::toInvalid(QWidget *main, toConnection &connection)
        : toToolWidget(InvalidTool, "invalid.html", main, connection, "toInvalid")
{

    QToolBar *toolbar = toAllocBar(this, tr("Invalid Objects"));
    layout()->addWidget(toolbar);

    QAction *refreshAct =
        toolbar->addAction(
            QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
            tr("Refresh list"),
            this,
            SLOT(refresh()));
    refreshAct->setShortcut(QKeySequence::Refresh);

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(compile_xpm))),
                       tr("Recompile all invalid"),
                       this,
                       SLOT(recompileSelected()));

    toolbar->addWidget(new toSpacer());

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(splitter);

    Objects = new toResultTableView(false, false, splitter);
    Objects->setSQL(SQLListInvalid);

    connect(Objects, SIGNAL(selectionChanged()), this, SLOT(changeSelection()));

    Source = new toResultExtract(false, splitter);
    Source->setSQL(SQLListSource);

    connect(Source, SIGNAL(executed()), this, SLOT(refresh()));

    refresh();
    setFocusProxy(Objects);
}
#include <QDebug>
void toInvalid::recompileSelected(void)
{
    QProgressDialog progress(tr("Recompiling all invalid"),
                             tr("Cancel"),
                             0,
                             Objects->model()->rowCount(),
                             this);
    progress.setWindowTitle("Recompiling");
    progress.show();

    for (toResultTableView::iterator it(Objects); (*it).isValid(); it++)
    {
        toConnection &conn = connection();
        progress.setLabelText("Recompiling " +
                              Objects->model()->data((*it).row(), 2).toString() +
                              "." +
                              Objects->model()->data((*it).row(), 3).toString());
        progress.setValue(progress.value() + 1);

        qApp->processEvents();

        if (progress.wasCanceled())
            break;

        QString type = Objects->model()->data((*it).row(), 3).toString();
        QString name = Objects->model()->data((*it).row(), 2).toString();
        QString sql;
        if (type == "INDEX")
            sql = "ALTER " + type + " " +
                  conn.quote(Objects->model()->data((*it).row(), 1).toString()) + "." +
                  conn.quote(name) + " REBUILD";
        else if (type == "PACKAGE BODY")
            sql = "ALTER PACKAGE " + conn.quote(Objects->model()->data((*it).row(), 1).toString()) + "." +
                  conn.quote(Objects->model()->data((*it).row(), 2).toString()) + " COMPILE BODY";
        else if ((type == "SYNONYM") && (Objects->model()->data((*it).row(), 1).toString() == "PUBLIC"))
        {
            // only SYS user is allowed to do ALTER PUBLIC SYNONYM ...
            // other users can only do CREATE OR REPLACE PUBLIC SYNONYM ...
            std::list<QString> objects;
            toExtract extract(conn, NULL);
            extract.setCode(true);
            extract.setHeading(false);
            extract.setPrompt(false);
            extract.setReplace(true); // get create OR REPLACE statement
            objects.insert(objects.end(), type + QString::fromLatin1(":") + "PUBLIC" + QString::fromLatin1(".") + name);
            sql = extract.create(objects);
        }
        else
            sql = "ALTER " + Objects->model()->data((*it).row(), 3).toString() + " " +
                  conn.quote(Objects->model()->data((*it).row(), 1).toString()) + "." +
                  conn.quote(Objects->model()->data((*it).row(), 2).toString()) + " COMPILE";

        try
        {
            // remove trailing newlines, spaces, tabs and semicolons from execution
            // as this could cause "execution" of empty statement (doing nothing)
            int l = sql.length() - 1;
            while (l >= 0 && (sql.at(l) == ';' || sql.at(l).isSpace()))
                l--;
#ifdef DEBUG
            qDebug() << "statement=" << sql.mid(0, l + 1);
#endif
            if (l >= 0)
                conn.execute(sql.mid(0, l + 1));
        }
        catch (...)
            {}
    }

    if (progress.isVisible())
        progress.close();

    qApp->processEvents();
    this->refresh();
}

void toInvalid::refresh(void)
{
    QModelIndex item = Objects->selectedIndex();

    QString owner;
    QString object;
    QString type;
    if (item.isValid())
    {
        owner = Objects->model()->data(item.row(), 1).toString();
        object = Objects->model()->data(item.row(), 2).toString();
        type = Objects->model()->data(item.row(), 3).toString();
    }

    Objects->refresh();

    if (item.isValid())
    {
        for (toResultTableView::iterator it(Objects); (*it).isValid(); it++)
        {
            if (Objects->model()->data(item.row(), 1).toString() == owner &&
                    Objects->model()->data(item.row(), 2).toString() == object &&
                    Objects->model()->data(item.row(), 3).toString() == type)
            {

                Objects->selectionModel()->select(QItemSelection((*it), (*it)),
                                                  QItemSelectionModel::ClearAndSelect);
                Objects->setCurrentIndex((*it));
                break;
            }
        }

        changeSelection();
    }
}

void toInvalid::changeSelection(void)
{
    try
    {
        QModelIndex item = Objects->selectedIndex();
        if (item.isValid())
        {
            Source->changeParams(Objects->model()->data(item.row(), 1).toString(),
                                 Objects->model()->data(item.row(), 2).toString(),
                                 Objects->model()->data(item.row(), 3).toString());
            QMap<int, QString> Errors;

            toQuery errors(connection(),
                           SQLReadErrors,
                           Objects->model()->data(item.row(), 1).toString(),
                           Objects->model()->data(item.row(), 2).toString(),
                           Objects->model()->data(item.row(), 3).toString());

            while (!errors.eof())
            {
                int line = errors.readValue().toInt();
                Errors[line] += QString::fromLatin1(" ");
                Errors[line] += errors.readValue();
            }

            Source->editor()->setErrors(Errors);
        }
    }
    TOCATCH;
}
