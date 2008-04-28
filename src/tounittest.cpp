/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#include "tounittest.h"
#include "utils.h"
#include "tohighlightedtext.h"
#include "toworksheetwidget.h"
#include "toresultview.h"
#include "toeventquery.h"

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
                           "select distinct object_name\n"
                                   "   from all_arguments\n"
                                   "   where owner = upper(:f1<char[101]>)\n"
                                   "   and package_name = upper(:f2<char[101]>)",
                                   "List package procedures and functions",
                            "0800");

static toSQL SQLListUnits("toUnitTest:ListUnitMethods",
                            "select distinct object_name\n"
                                    "   from all_arguments\n"
                                    "   where owner = upper(:f1<char[101]>)\n"
                                    "   and object_name = upper(:f2<char[101]>)",
                            "List package procedures and functions",
                            "0800");


toUnitTest::toUnitTest(QWidget * parent, const char *name)
    : QWidget(parent),
    m_model(0)
{
    setObjectName(name);

    QGridLayout *gridLayout = new QGridLayout(this);
    QSplitter *splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);

    worksheet = new toWorksheetWidget(splitter, name, toCurrentConnection(parent));
    packageList = new toResultView(false, false, splitter, "packageList");
    packageList->setReadAll(true);

    splitter->addWidget(packageList);
    splitter->addWidget(worksheet);
    gridLayout->addWidget(splitter, 0, 0, 1, 1);

    connect(packageList, SIGNAL(activated(const QModelIndex &)),
            this, SLOT(packageList_activated(const QModelIndex &)));
}

void toUnitTest::query(const QString &sql, const toQList &param)
{
    if (!setSQLParams(sql, param))
        return;

    toQList::iterator i = params().begin();
    m_owner = *i;
    ++i;
    m_name = *i;
    ++i;
    m_type = *i;

    // set temp params for members list
    toQList p;
    p.push_back(m_owner);
    p.push_back(m_name);

    packageList->query((m_type == "PACKAGE" ? SQLListPackage : SQLListUnits), p);

    worksheet->editor()->setText("");
}

bool toUnitTest::canHandle(toConnection &conn)
{
    try
    {
        return toIsOracle(worksheet->toToolWidget::connection());
    }
    TOCATCH
    return false;
}

void toUnitTest::packageList_activated(const QModelIndex &ix)
{
    toQList p;
    if (m_type == "PACKAGE")
    {
        p.push_back(m_owner);
        p.push_back(ix.data(Qt::EditRole).toString());
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
