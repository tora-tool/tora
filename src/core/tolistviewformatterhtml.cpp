
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

#include "core/tolistviewformatterhtml.h"
#include "core/tolistviewformatterfactory.h"
#include "core/tolistviewformatteridentifier.h"
#include "core/toresultview.h"
#include "core/toresultmodel.h"

#include <QtGui/QTextDocument>

#include <iostream>

namespace
{
toListViewFormatter* createHTML()
{
    return new toListViewFormatterHTML();
}
const bool registered = toListViewFormatterFactory::Instance().Register(
                            toListViewFormatterIdentifier::HTML, createHTML);
}


toListViewFormatterHTML::toListViewFormatterHTML() : toListViewFormatter()
{
}

toListViewFormatterHTML::~toListViewFormatterHTML()
{
}
QString toListViewFormatterHTML::getFormattedString(toExportSettings &settings,
        //const toResultModel *model);
        const QAbstractItemModel * model)
{
    int     columns   = model->columnCount();
    int     rows      = model->rowCount();

    QString output;
    QString indent;

    QVector<int> rlist = selectedRows(settings.selected);
    QVector<int> clist = selectedColumns(settings.selected);

    output = QString("<HTML><HEAD><TITLE>Export</TITLE></HEAD><BODY><TABLE>");
    endLine(output);

    QString bgcolor;
    if (settings.columnsHeader)
    {
        output += QString("<TR>");
        endLine(output);

        for (int j = 0; j < columns; j++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(j))
                continue;
            output += QString("\t<TH>");
            endLine(output);

            QString text(Qt::escape(model->headerData(j,
                                    Qt::Horizontal,
                                    Qt::DisplayRole).toString()));

            output += "\t\t" + text;

            endLine(output);
            output += QString("\t</TH>");
            endLine(output);
        }

        output += "</TR>";
        endLine(output);
    }

    QModelIndex mi;
    for (int row = 0; row < rows; row++)
    {
        if (settings.rowsExport == toExportSettings::RowsSelected && !rlist.contains(row))
            continue;

        output += "<TR>";
        endLine(output);

        for (int i = 0; i < columns; i++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(i))
                continue;
            if (!settings.rowsHeader && i == 0)
                continue;
            output += QString("\t<TD>");
            endLine(output);

            mi = model->index(row, i);
            QString text(Qt::escape(model->data(mi, Qt::EditRole).toString()));

            output += "\t\t" + text;
            endLine(output);

            output += QString("\t</TD>");
            endLine(output);
        }

        output += "</TR>";
        endLine(output);
    }

    output += "</TABLE></BODY></HTML>";
    return output;
}
