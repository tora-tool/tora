
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

#include "views/tobindsview.h"
#include <QXmlSimpleReader>
#include <QXmlStreamAttribute>
#include <QXmlStreamAttributes>

static toSQL toMonitoredBind("toBindVariablesView:ListMonitoredSQLs",
"select USERNAME, SERVICE_NAME, SQL_EXEC_START, SQL_PLAN_HASH_VALUE, ERROR_MESSAGE, ELAPSED_TIME, BINDS_XML as \" BINDS_XML\" \n"
" from v$sql_monitor                            \n"
" where sql_id = :sql_id<char[16],in>           \n"
,"List locks in a session"
,"1200"
,"Oracle"
);

QVariant toBindModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        goto INVALID;
        return QVariant();
    }

    if (index.row() >= Rows.size()) //
    {
        goto INVALID;
        return QVariant();
    }

    int r = index.row();
    int c = index.column();
    if (index.column() >= Rows.at(index.row()).size())
    {
        return QVariant();
    }

    return super::data(index, role);

INVALID:
    throw QString("Invalid index row: %1").arg(index.row());
    return QVariant();
}

Qt::ItemFlags toBindModel::flags(QModelIndex const& index) const
{
    Qt::ItemFlags fl = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;

    if (index.column() == 0)
        return fl;              // row number column

    if (!index.isValid() || index.row() >= Rows.size())
    {
        return fl;
    }

    if (index.column() >= Rows.at(index.row()).size())
    {
        return fl;
    }

    return super::flags(index);
}

QVariant toBindModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return super::headerData(section, orientation, role);
}

int toBindModel::columnCount(const QModelIndex &parent) const
{
    return super::columnCount(parent);
}

void toBindModel::appendRows(toQueryAbstr::RowList &rows)
{
    int idx = headers().indexOf(" BINDS_XML");

    int oldRowCount = rowCount();

    for(auto&r : rows)
    {
        //std::cout << r << std::endl;
        auto lob = r[idx];
    }

    beginInsertRows(QModelIndex(), oldRowCount, oldRowCount + rows.size() - 1);
    Rows << rows;
    endInsertRows();

    if (oldRowCount == 0)
        emit firstResultReceived();
}

void toBindModel::appendRow(toQueryAbstr::Row &r)
{
    int oldRowCount = rowCount();

    int idx = headers().indexOf(" BINDS_XML");
    int columns = columnCount();
    bool bindColumnsMissing = (columns <= (idx+1));

    auto lob = r[idx];
    if (lob.isComplexType())
    {
        QString xmlString = lob.userData();
        QXmlStreamReader xml(xmlString);

        if (xml.readNextStartElement() && xml.name() == "binds")
        {
            auto n = xml.name().toString();
            while (xml.readNextStartElement()) {
                auto n = xml.name().toString();

                // <bind name=":3" pos="3" dty="2" dtystr="NUMBER" maxlen="22" len="5">13564652</bind>
                // <bind name=":4" pos="4" dty="1" dtystr="VARCHAR2(2000)" maxlen="2000" csid="873" len="23">%attrezzeria noris sas%</bind>
                QString name;
                QString pos;
                QString dtystr;
                QString value;

                if (xml.name() == "bind")
                {
                    if (xml.isStartElement())
                    {
                        QXmlStreamAttributes a = xml.attributes();
                        if (a.hasAttribute("name"))
                            name = a.value("name").toString();
                        if (a.hasAttribute("pos"))
                            pos = a.value("pos").toString();
                        if (a.hasAttribute("dtystr"))
                            dtystr = a.value("dtystr").toString();
                        xml.readNext();
                    }
                    if (xml.isCharacters() && !xml.isWhitespace()) {
                        value = xml.text().toString();
                        r.append(toQValue(value));
                        xml.readNext();
                        if (bindColumnsMissing)
                        {
                            beginInsertColumns(QModelIndex(), idx+1, idx+1);
                            setHeaderData(++idx, Qt::Horizontal, QVariant(name), Qt::DisplayRole);
                            endInsertColumns();
                        }
                    }
                } else {
                    xml.skipCurrentElement();
                }
            }
        }
    }
    super::appendRow(r);
}


toBindVariablesView::toBindVariablesView(QWidget *parent)
    : super(parent)
{
    setSQLName(QString::fromLatin1("toBindVariablesView:ListMonitoredSQLs"));
}
