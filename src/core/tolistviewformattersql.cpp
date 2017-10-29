
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

//
// C++ Implementation: toListViewFormatterSQL
//

#include "core/tolistviewformattersql.h"
#include "core/toconnectionregistry.h"
#include "core/toconnectiontraits.h"
#include "core/toconfiguration.h"
#include "core/toconfiguration.h"
#include "core/tolistviewformatterfactory.h"
#include "core/tolistviewformatteridentifier.h"
#include "core/toeditorconfiguration.h"
#include "core/utils.h"
#include "widgets/toresultmodel.h"

namespace
{
    toListViewFormatter* createSQL()
    {
        return new toListViewFormatterSQL();
    }
    const bool registered = toListViewFormatterFactory::Instance().Register( toListViewFormatterIdentifier::SQL, createSQL );
}


toListViewFormatterSQL::toListViewFormatterSQL() : toListViewFormatter()
{}

toListViewFormatterSQL::~toListViewFormatterSQL()
{}


QString toListViewFormatterSQL::getFormattedString(toExportSettings &settings,
        //const toResultModel *model);
        const QAbstractItemModel * model)
{
    using namespace ToConfiguration;

    toConnection &conn = toConnectionRegistrySing::Instance().currentConnection();

    int     columns   = model->columnCount();
    int     rows      = model->rowCount();

    QVector<int> rlist = selectedRows(settings.selected);
    QVector<int> clist = selectedColumns(settings.selected);

    QString sql;
    QString objectName;
    QString columnNames;
    QString output;

    if (toConfigurationNewSingle::Instance().option(Editor::KeywordUpperBool).toBool())
        sql = "INSERT INTO %1%2 VALUES (%3);";
    else
        sql = "insert into %1%2 values (%3);";

    if (!settings.objectName.isEmpty())
    {
        if (!settings.owner.isEmpty())
            objectName = settings.owner + "." + settings.objectName;
        else
            objectName = settings.objectName;
    }
    else
    {
        // If name of object is unknown (which shouldn't be the case)
        // fall back to "general" name for object.
        objectName = "tablename";
    }

    if (settings.columnsHeader)
    {
        columnNames += " (";
        for (int j = 1; j < columns; j++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(j))
                continue;
            columnNames += model->headerData(
                               j,
                               Qt::Horizontal,
                               Qt::DisplayRole).toString();
            columnNames += ", ";
        }
        columnNames = columnNames.left(columnNames.length() - 2) + ")";
    }

    const toResultModel *resultModel = reinterpret_cast<const toResultModel*>(model);
    if (!model)
        return "-- cannot access data result. Maybe it's not a SQL export";

    QModelIndex mi;
    toResultModel::HeaderList hdr = resultModel->headers();
    for (int row = 0; row < rows; row++)
    {
        if (settings.rowsExport == toExportSettings::RowsSelected && !rlist.contains(row))
            continue;

        QString values;
        for (int i = 1; i < columns; i++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(i))
                continue;
//             if (!settings.rowsHeader && i == 0)
//                 continue;

            mi = model->index(row, i);
            QVariant currVal(model->data(mi, Qt::EditRole));

            QString h(hdr.at(i).datatype.toUpper());
            if (h.contains("DATE"))
            {
                if (currVal.toString().isEmpty())
                    values += "NULL";
                else
                {
                    values += conn.getTraits().formatDate(currVal);
                }
            }
            else if (h.contains("CHAR"))
            	values += currVal.toString().isEmpty() ? "NULL" : conn.getTraits().quoteVarchar(currVal.toString());
            else
                values += currVal.toString().isEmpty() ? "NULL" : currVal.toString();

            values += ", ";
        }
        values = values.left(values.length() - 2);
        QString line(sql.arg(objectName).arg(columnNames).arg(values));
        endLine(line);
        output += line;
    }

    return output;
}
