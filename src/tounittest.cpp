
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

#include <QSettings>

#include "tounittest.h"
#include "toresultmodel.h"
#include "toresulttableview.h"
#include "utils.h"
#include "tohighlightedtext.h"
#include "toworksheetwidget.h"
#include "toeventquery.h"
#include "toresultcombo.h"
#include "toresultschema.h"
#include "toconf.h"
#include "tochangeconnection.h"
#include "tooutput.h"

#include "icons/unittest.xpm"
#include "icons/refresh.xpm"

// helper definitons for SQLPackageParams and SQLUnitParams columns
#define ARGUMENT_NAME 1
#define DATA_TYPE 2
#define DATA_LENGTH 3
#define IN_OUT 4
#define DATA_LEVEL 5
#define POSITION 6
#define DEFAULT_VALUE 7


static toSQL SQLPackageParams("toUnitTest:PackageParams",
                           "select argument_name, data_type, data_length, in_out, data_level, position, default_value\n"
                            "   from all_arguments\n"
                            "   where owner = upper(:f1<char[101]>)\n"
                            "   and object_name = upper(:f2<char[101]>)\n"
                            "   and package_name = upper(:f3<char[101]>)\n"
                            "   order by position\n",
                           "List PL/SQL package unit parameters.",
                           "0800");

static toSQL SQLUnitParams("toUnitTest:UnitParams",
                                "select argument_name, data_type, data_length, in_out, data_level, position, default_value\n"
                                "   from all_arguments\n"
                                "   where owner = upper(:f1<char[101]>)\n"
                                "   and object_name = upper(:f2<char[101]>)\n"
                                "   order by position\n",
                            "List PL/SQL function or procedure unit parameters.",
                            "0800");

static toSQL SQLListPackage("toUnitTest:ListPackageMethods",
                           "select distinct object_name as \"Package Members\"\n"
                                   "   from all_arguments\n"
                                   "   where owner = upper(:f1<char[101]>)\n"
                                   "   and package_name = upper(:f2<char[101]>)",
                                   "List package procedures and functions",
                            "0800");

static toSQL UnitListCode("toUnitTest:ListCode",
                          "SELECT Object_Name,Object_Type FROM SYS.ALL_OBJECTS\n"
                                  " WHERE OWNER = upper(:f1<char[101]>)\n"
                                  "   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
                                  "                       'PROCEDURE')\n"
                                  " ORDER BY Object_Name",
          "List all PL/SQL codes.", "0800");


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
                                TO_TOOLBAR_WIDGET_NAME);
    try
    {
        Schema->refresh();
    }
    catch (...) {}
    connect(Schema, SIGNAL(activated(const QString &)),
            this, SLOT(changeSchema(const QString &)));
    toolbar->addWidget(Schema);

    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);

    codeSplitter = new QSplitter(this);
    codeSplitter->setOrientation(Qt::Horizontal);

    codeList = new toResultTableView(true, false,
                                     codeSplitter, "codeList");
    codeList->setSQL(UnitListCode);
    refreshCodeList();

    packageList = new toResultTableView(true, false,
                                        codeSplitter, "packageList");
    packageList->setSQL(SQLListPackage);
    packageList->setVisible(false);

    codeSplitter->addWidget(codeList);
    codeSplitter->addWidget(packageList);

    editorSplitter = new QSplitter(this);
    editorSplitter->setOrientation(Qt::Vertical);

    worksheet = new toWorksheetWidget(editorSplitter,
                                      "UTworksheet", connection);
    output = new toOutput(editorSplitter, connection, true);

    editorSplitter->addWidget(worksheet);
    editorSplitter->addWidget(output);

    splitter->addWidget(codeSplitter);
    splitter->addWidget(editorSplitter);

    layout()->addWidget(splitter);

    splitter->setChildrenCollapsible(false);
    codeSplitter->setChildrenCollapsible(false);
    editorSplitter->setChildrenCollapsible(false);
    codeList->setMinimumWidth(200);
    packageList->setMinimumWidth(200);
    worksheet->setMinimumHeight(200);
    output->setMinimumHeight(200);

    QSettings s;
    s.beginGroup("toUnitTest");
    splitter->restoreState(s.value("splitter", QByteArray()).toByteArray());
    codeSplitter->restoreState(s.value("codeSplitter", QByteArray()).toByteArray());
    editorSplitter->restoreState(s.value("editorSplitter", QByteArray()).toByteArray());
    s.endGroup();

    connect(packageList, SIGNAL(selectionChanged()),
            this, SLOT(packageList_selectionChanged()));
    connect(codeList, SIGNAL(selectionChanged()),
            this, SLOT(codeList_selectionChanged()));
}

toUnitTest::~toUnitTest()
{
    QSettings s;
    s.beginGroup("toUnitTest");
    s.setValue("splitter", splitter->saveState());
    s.setValue("codeSplitter", codeSplitter->saveState());
    s.setValue("editorSplitter", editorSplitter->saveState());
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
    codeList->clearParams();
    codeList->changeParams(m_owner);
    codeList->setReadAll(true);
    codeList->resizeColumnsToContents();
}

void toUnitTest::changeSchema(const QString & name)
{
    m_name = name;
    refreshCodeList();
}

void toUnitTest::codeList_selectionChanged()
{
    m_name = codeList->selectedIndex(1).data(Qt::EditRole).toString();
    m_type = codeList->selectedIndex(2).data(Qt::EditRole).toString();
    worksheet->editor()->setText("-- select PL/SQL unit, please.");

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
    output->disable(false);
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
        res.append(t.arg(m_model->data(i, ARGUMENT_NAME).toString())
                .arg(m_model->data(i, DATA_TYPE).toString().replace("PL/SQL", ""))
                .arg(m_model->data(i, DATA_LENGTH).toString())
                .arg(m_model->data(i, IN_OUT).toString()).replace("()", "(22)"));
    }

    if (!returnClause.isNull())
        res.append(returnClause);

    if (res.count() == 1)
        res.clear();

    res.append("\nBEGIN\n");

    // inputs
    for (int i = 0; i < m_model->rowCount(); ++i)
    {
        QString t("\t%1 := %2;");
        QString def;
        if (m_model->data(i, ARGUMENT_NAME).isNull())
            continue;
        if (m_model->data(i, DEFAULT_VALUE).isNull())
            def = "NULL";
        else
            def = m_model->data(i, DEFAULT_VALUE).toString();
        res.append(t.arg(m_model->data(i, ARGUMENT_NAME).toString()).arg(def));
    }

    if (!returnClause.isNull())
        res.append("\n\tret :=");

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
