
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

#include <QSettings>

#include "tounittest.h"
#include "utils.h"
#include "tohighlightedtext.h"
#include "toworksheetwidget.h"
#include "toeventquery.h"
#include "toresultcombo.h"
#include "toresultschema.h"
#include "tochangeconnection.h"
#include "tocodemodel.h"

#include "icons/unittest.xpm"
#include "icons/refresh.xpm"

// helper definitons for SQLPackageParams and SQLUnitParams columns
#define ARGUMENT_NAME 1
#define DATA_TYPE 2
#define DATA_LENGTH 3
#define IN_OUT 4
#define DATA_LEVEL 5
#define POSITION 6
//#define DEFAULT_VALUE 7


static toSQL SQLPackageParams("toUnitTest:PackageParams",
                           "select distinct argument_name, data_type, data_length,\n"
                           "        in_out, data_level, position --, default_value\n"
                            "   from all_arguments\n"
                            "   where owner = upper(:f1<char[101]>)\n"
                            "   and object_name = upper(:f2<char[101]>)\n"
                            "   and package_name = upper(:f3<char[101]>)\n"
                            "   and data_level = 0\n"
                            "   order by position\n",
                           "List PL/SQL package unit parameters.",
                           "0800");

static toSQL SQLUnitParams("toUnitTest:UnitParams",
                                "select distinct argument_name, data_type, data_length,\n"
                                "        in_out, data_level, position --, default_value\n"
                                "   from all_arguments\n"
                                "   where owner = upper(:f1<char[101]>)\n"
                                "   and object_name = upper(:f2<char[101]>)\n"
                                "   and data_level = 0\n"
                                "   order by position\n",
                            "List PL/SQL function or procedure unit parameters.",
                            "0800");

static toSQL SQLListPackage("toUnitTest:ListPackageMethods",
                           "select distinct object_name as \"Package Members\"\n"
                                   "   from all_arguments\n"
                                   "   where owner = upper(:f1<char[101]>)\n"
                                   "   and package_name = upper(:f2<char[101]>)"
                                   "order by 1",
                                   "List package procedures and functions",
                            "0800");


class toUnitTestTool : public toTool
{
    protected:
        virtual const char **pictureXPM(void)
        {
            return const_cast<const char**>(unittest_xpm);
        }
    public:
        toUnitTestTool()
        : toTool(120, "PL/SQL Unit Tester") { }
        virtual const char *menuItem()
        {
            return "PL/SQL Unit Tester";
        }
        virtual QWidget *toolWindow(QWidget *parent, toConnection &connection)
        {
            return new toUnitTest(parent, connection);
        }
//         virtual QWidget *configurationTab(QWidget *parent)
//         {
//             return new toSGATracePrefs(this, parent);
//         }
        virtual bool canHandle(toConnection &conn)
        {
            return toIsOracle(conn);
        }
        virtual void closeWindow(toConnection &connection) {};
};


static toUnitTestTool UnitTestTool;


toUnitTest::toUnitTest(QWidget * parent, toConnection &connection)
    : toToolWidget(UnitTestTool, "unittest.html", parent, connection, "toUnitTest"),
    m_model(0)
{
    setObjectName("unitTest");

    m_owner = connection.user();

    QToolBar *toolbar = toAllocBar(this, tr("UTbrowser"));
    layout()->addWidget(toolbar);

    QAction *refreshAct =
            toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(refresh_xpm))),
                               tr("Refresh list"),
                               this, SLOT(refreshCodeList()));
    refreshAct->setShortcut(QKeySequence::Refresh);

    toolbar->addWidget(new toSpacer());

    QLabel * labSchema = new QLabel(tr("Schema") + " ", toolbar);
    toolbar->addWidget(labSchema);
    Schema = new toResultSchema(connection, toolbar,
                                "UTresultSchema");
    try
    {
        Schema->refresh();
    }
    catch (...) {}
    connect(Schema, SIGNAL(activated(const QString &)),
            this, SLOT(changeSchema(const QString &)));
    toolbar->addWidget(Schema);

    new toChangeConnection(toolbar, "UTchangeConnection");

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);

    codeSplitter = new QSplitter(this);
    codeSplitter->setOrientation(Qt::Horizontal);

    codeList = new QTreeView(splitter);
    codeModel = new toCodeModel(codeList);
    codeList->setModel(codeModel);
    connect(codeList, SIGNAL(doubleClicked(const QModelIndex &)),
             this, SLOT(changePackage(const QModelIndex &)));

    packageList = new toResultTableView(true, false,
                                        codeSplitter, "packageList");
    packageList->setSQL(SQLListPackage);
    packageList->setVisible(false);

    codeSplitter->addWidget(codeList);
    codeSplitter->addWidget(packageList);

    worksheet = new toWorksheetWidget(splitter,
                                      "UTworksheet", connection);

    splitter->addWidget(codeSplitter);
    splitter->addWidget(worksheet);

    layout()->addWidget(splitter);

    splitter->setChildrenCollapsible(false);
    codeSplitter->setChildrenCollapsible(false);
    codeList->setMinimumWidth(200);
    packageList->setMinimumWidth(200);

    QSettings s;
    s.beginGroup("toUnitTest");
    splitter->restoreState(s.value("splitter", QByteArray()).toByteArray());
    codeSplitter->restoreState(s.value("codeSplitter", QByteArray()).toByteArray());
    s.endGroup();

    connect(packageList, SIGNAL(selectionChanged()),
            this, SLOT(packageList_selectionChanged()));

    refreshCodeList();
}

toUnitTest::~toUnitTest()
{
    QSettings s;
    s.beginGroup("toUnitTest");
    s.setValue("splitter", splitter->saveState());
    s.setValue("codeSplitter", codeSplitter->saveState());
    s.endGroup();
}

bool toUnitTest::canHandle(toConnection &conn)
{
    try
    {
        return toIsOracle(conn);
    }
    TOCATCH
    return false;
}

void toUnitTest::refreshCodeList()
{
    if (!Schema->currentText().isEmpty())
        m_owner = Schema->currentText();
    else
        m_owner = connection().user().toUpper();
    codeModel->refresh(connection(), m_owner);
}

void toUnitTest::changeSchema(const QString & name)
{
    m_owner = name;
    refreshCodeList();
}

void toUnitTest::changePackage(const QModelIndex &current)
{
    toBusy busy;

    worksheet->editor()->setText("-- select PL/SQL unit, please.");

    toCodeModelItem *item = static_cast<toCodeModelItem*>(current.internalPointer());
    if (item && item->parent())
    {
        m_type = item->parent()->display();
        if(m_type.isEmpty() || m_type == "Code")
            return;
        m_type = m_type.toUpper();
        m_name = item->display();
    }

    if (m_type == "PACKAGE")
    {
        packageList->setVisible(true);
        packageList->clearParams();
        packageList->changeParams(m_owner, m_name);
        packageList->setReadAll(true);
    }
    else
    {
        packageList->setVisible(false);
        packageList_selectionChanged();
    }
}

void toUnitTest::packageList_selectionChanged()
{
    toQList p;
    if (m_type == "PACKAGE")
    {
        p.push_back(m_owner);
        p.push_back(packageList->selectedIndex(1).data(Qt::EditRole).toString());
        p.push_back(m_name);
    }
    else
    {
        p.push_back(m_owner);
        p.push_back(m_name);
    }
    worksheet->editor()->setText("-- getting the script...");
    try
    {
        toQuery q(worksheet->toToolWidget::connection(),
                    (m_type == "PACKAGE" ? SQLPackageParams : SQLUnitParams),
                    p
                    );
        toEventQuery * query = new toEventQuery(worksheet->toToolWidget::connection(), q.sql(), p);
        if (m_model)
        {
            delete m_model;
            m_model = 0;
        }
        m_model = new toResultModel(query, this);
        m_model->readAll();
        connect(m_model, SIGNAL(done()), this, SLOT(handleDone()));

    }
    TOCATCH
}

void toUnitTest::handleDone()
{
    QString returnClause;
    QStringList res;

    m_model->readAll();

    res.append("DECLARE\n");

    // params declarations
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
//         res.append("\n");
//         res.append(QString("-- %1").arg(i) + QString(" arg ") + m_model->data(i, ARGUMENT_NAME).toString());
//         res.append(QString("-- %1").arg(i) + QString(" lev ") + m_model->data(i, DATA_LEVEL).toString());
//         res.append(QString("-- %1").arg(i) + QString(" pos ") + m_model->data(i, POSITION).toString());
//         res.append(QString("-- %1").arg(i) + QString(" typ ") + m_model->data(i, DATA_TYPE).toString());
//         res.append(QString("-- %1").arg(i) + QString(" len ") + m_model->data(i, DATA_LENGTH).toString());
//         res.append(QString("-- %1").arg(i) + QString(" i/o ") + m_model->data(i, IN_OUT).toString());

        QString t("\t%1 %2(%3); -- %4");
        if (m_model->data(i, ARGUMENT_NAME).isNull()
                  && m_model->data(i, DATA_LEVEL).toInt() == 0
                  && m_model->data(i, POSITION).toInt() == 0)
        {
            returnClause = t.arg("ret ")
                    .arg(m_model->data(i, DATA_TYPE).toString().replace("PL/SQL", ""))
                    .arg(m_model->data(i, DATA_LENGTH).toString())
                    .arg("function return value").replace("()", "(22)");
            continue;
        }
        if (m_model->data(i, ARGUMENT_NAME).isNull()
                  && m_model->data(i, DATA_LEVEL).toInt() == 0
                  && m_model->data(i, POSITION).toInt() != 0)
        {
//             res.append("-- DEBUG " + m_model->data(i, POSITION).toString() + " skipped");
            continue;
        }
        QString item(t.arg(m_model->data(i, ARGUMENT_NAME).toString())
                      .arg(m_model->data(i, DATA_TYPE).toString().replace("PL/SQL", ""))
                      .arg(m_model->data(i, DATA_LENGTH).toString())
                      .arg(m_model->data(i, IN_OUT).toString()));
        // 1) fixed-length chars should be handled from all_arguments
        // 2) all other "numeric" and/or pl/sql types should use ()
        //    or length from all_arguments
        if (m_model->data(i, DATA_TYPE) == "VARCHAR2"
            || m_model->data(i, DATA_TYPE) == "VARCHAR"
            || m_model->data(i, DATA_TYPE) == "NVARCHAR2")
        {
            res.append(item.replace("()", "(3000)") + "; 3000 is the dummy size for testing");
        }
        else
            res.append(item.replace("()", ""));
    }

    if (!returnClause.isNull())
        res.append(returnClause);

    if (res.count() == 1)
        res.clear();

    res.append("\nBEGIN\n");

    // inputs
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QString t("\t%1 := %2; -- %3");
        QString def("NULL");
        if (m_model->data(i, ARGUMENT_NAME).isNull())
            continue;
/*        if (!m_model->data(i, DEFAULT_VALUE).isNull())
            def = m_model->data(i, DEFAULT_VALUE).toString();*/
        // skip OUT params
        if (m_model->data(i, IN_OUT).toString().startsWith("IN"))
        {
            res.append(t.arg(m_model->data(i, ARGUMENT_NAME).toString())
                        .arg(def)
                        .arg(m_model->data(i, DATA_TYPE).toString()));
        }
    }

    if (!returnClause.isNull())
        res.append("\n\tret :=");
    else
        res.append("\n");

    if (m_type == "PACKAGE")
        res.append("\t" + m_owner + "." + m_name
                + "." + packageList->currentIndex().data(Qt::EditRole).toString());
    else
        res.append("\t" + m_owner + "." + m_name);
    res.append("\t\t(");

    // params
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QString t("\t\t%1 => %2%3");
        if (m_model->data(i, ARGUMENT_NAME).isNull())
            continue;
        res.append(t.arg(m_model->data(i, ARGUMENT_NAME).toString())
                .arg(m_model->data(i, ARGUMENT_NAME).toString())
                .arg((i+1)==m_model->rowCount()? "" : ","));
    }
    res.append("\t\t);\n");

    // outputs
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QString t("\tsys.dbms_output.put_line('%1 => ' || %2);");
        if (m_model->data(i, ARGUMENT_NAME).isNull())
            continue;
        res.append(t.arg(m_model->data(i, ARGUMENT_NAME).toString())
                .arg(m_model->data(i, ARGUMENT_NAME).toString()));
    }
    if (!returnClause.isNull())
    {
        res.append("\tsys.dbms_output.put_line('ret => ' || ret);");
    }

    res.append("\nEND;\n");

    worksheet->editor()->setText(res.join("\n").replace("\t", "    "));
}
