
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

#include <QTextDocument>

#include "tolistviewformatterhtml.h"
#include "tolistviewformatterfactory.h"
#include "tolistviewformatteridentifier.h"
#include "toresultview.h"
#include "toresultmodel.h"

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
