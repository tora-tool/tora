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

//
// C++ Implementation: toListViewFormatterSQL
//

#include "utils.h"

#include "toconfiguration.h"
#include "tolistviewformattersql.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultmodel.h"


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
    int     columns   = model->columnCount();
    int     rows      = model->rowCount();

    QVector<int> rlist = selectedRows(settings.selected);
    QVector<int> clist = selectedColumns(settings.selected);

    QString sql;
    QString objectName;
    QString columnNames;
    QString output;

    if (toConfigurationSingle::Instance().keywordUpper())
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
            QString currVal(model->data(mi, Qt::EditRole).toString());

            QString h(hdr.at(i).datatype.toUpper());
            if (h.contains("DATE"))
            {
                if (currVal.isEmpty())
                    values += "NULL";
                else
                {
                    values += "TO_DATE(\'" + currVal + "\' ,\'" + toConfigurationSingle::Instance().dateFormat() + "\')";
                }
            }
            else if (h.contains("CHAR"))
                values += (currVal.isEmpty()) ? "NULL" : "\'" + currVal + "\'";
            else
                values += (currVal.isEmpty()) ? "NULL" : currVal;

            values += ", ";
        }
        values = values.left(values.length() - 2);
        QString line(sql.arg(objectName).arg(columnNames).arg(values));
        endLine(line);
        output += line;
    }

    return output;
}
