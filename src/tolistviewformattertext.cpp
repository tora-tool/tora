
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

#include "tolistviewformattertext.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"
#include "toresultmodel.h"

#include <iostream>
#include <QVector>

namespace
{
toListViewFormatter* createText()
{
    return new toListViewFormatterText();
}
const bool registered = toListViewFormatterFactory::Instance().Register(toListViewFormatterIdentifier::TEXT, createText);
}

toListViewFormatterText::toListViewFormatterText() : toListViewFormatter()
{
}

toListViewFormatterText::~toListViewFormatterText()
{
}

QString toListViewFormatterText::getFormattedString(toExportSettings &settings,
                                       //const toResultModel *model);
                                       const QAbstractItemModel * model)
{
    int     columns   = model->columnCount();
    int     rows      = model->rowCount();

    QString output;
    QVector<int> sizes(columns);

    QVector<int> rlist = selectedRows(settings.selected);
    QVector<int> clist = selectedColumns(settings.selected);

    // must get widest length for each column
    // zero array or (if writing headers, set their size)
    for (int i = 0; i < columns; i++)
    {
        if (settings.columnsHeader)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(i))
                continue;
            sizes[i] = model->headerData(
                           i,
                           Qt::Horizontal,
                           Qt::DisplayRole).toString().length();
        }
        else
            sizes[i] = 0;
    }

    // loop through model and get column widths
    QModelIndex mi;
    for (int row = 0; row < rows; row++)
    {
        for (int column = 0; column < columns; column++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(column))
                continue;
            if (!settings.rowsHeader && column == 0)
                continue;

            mi = model->index(row, column);
            QVariant data = model->data(mi, Qt::EditRole);
            QString v;
            if (data.isNull())
                v = "{null}";
            else
                v = data.toString();

            int len = v.length();
            if (len > sizes[column])
                sizes[column] = len;
        }
    }

    // write header data to fixed widths
    if (settings.columnsHeader)
    {
        for (int column = 0; column < columns; column++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(column))
                continue;
            if (!settings.rowsHeader && column == 0)
                continue;
            QString value = model->headerData(
                                column,
                                Qt::Horizontal,
                                Qt::DisplayRole).toString();

            output += value;
            for (int left = value.length(); left <= sizes[column]; left++)
                output += ' ';
        }

        endLine(output);

        // write ==== border
        for (int column = 0; column < columns; column++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(column))
                continue;
            if (!settings.rowsHeader && column == 0)
                continue;
            for (int left = 0; left < sizes[column]; left++)
                output += '=';
            output += ' ';
        }

        endLine(output);
    }

    // write data
    for (int row = 0; row < rows; row++)
    {
        if (settings.rowsExport == toExportSettings::RowsSelected && !rlist.contains(row))
            continue;

        for (int column = 0; column < columns; column++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(column))
                continue;
            if (!settings.rowsHeader && column == 0)
                continue;
            mi = model->index(row, column);
            QVariant data = model->data(mi, Qt::EditRole);
            QString value;
            if (data.isNull())
                value = "{null}";
            else
                value = data.toString();

            output += value;
            for (int left = value.length(); left <= sizes[column]; left++)
                output += ' ';
        }

        endLine(output);
    }

    return output;
}
