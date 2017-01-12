
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

#include "core/tolistviewformattertext.h"
#include "core/tolistviewformatterfactory.h"
#include "core/tolistviewformatteridentifier.h"

#include <QtCore/QVector>

#include <iostream>
#include "tools/toresultview.h"

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

QString toListViewFormatterText::getFormattedString(toExportSettings &settings, const QAbstractItemModel * model)
{
    int     columns   = model->columnCount();
    int     rows      = model->rowCount();

    QString output;
    QVector<int> sizes(columns);

    QVector<int> rlist = selectedRows(settings.selected);
    QVector<int> clist = selectedColumns(settings.selected);

    // must get widest length for each column
    // zero array or (if writing headers, set their size)
    for (int i = 0; i < columns - 1; i++)
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
    for (int row = 0; row < rows; row++)
    {
        for (int column = 0; column < columns - 1; column++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(column))
                continue;
            if (!settings.rowsHeader && column == 0)
                continue;

            QVariant data = model->data(model->index(row, column), Qt::EditRole);
            QString v;
            if (data.isNull())
                v = "{null}";
            else
                v = data.toString();
            sizes[column] = (std::max)(sizes[column], v.length());
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

            output += value.leftJustified(sizes[column], ' ');
            output += ' '; // gap between columns
        }

        endLine(output);

        // write ==== border
        for (int column = 0; column < columns; column++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(column))
                continue;
            if (!settings.rowsHeader && column == 0)
                continue;

            output += QString::fromLatin1("=").leftJustified(sizes[column], '=');
            output += ' '; // gap between columns
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
            QVariant data = model->data(model->index(row, column), Qt::EditRole);
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
