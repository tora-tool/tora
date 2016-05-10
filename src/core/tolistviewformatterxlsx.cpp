
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

#include "core/tolistviewformatterxlsx.h"
#include "core/tolistviewformatterfactory.h"
#include "core/tolistviewformatteridentifier.h"
#include "widgets/toresultmodel.h"

#include <QtCore/QVector>

#include <iostream>
#include "tools/toresultview.h"

namespace
{
    toListViewFormatter* createXLSX()
    {
        return new toListViewFormatterXLSX();
    }
    const bool registered = toListViewFormatterFactory::Instance().Register(toListViewFormatterIdentifier::XLSX, createXLSX);
}

toListViewFormatterXLSX::toListViewFormatterXLSX() : toListViewFormatter()
{
}

QString toListViewFormatterXLSX::getFormattedString(toExportSettings &settings, const QAbstractItemModel * model)
{
    // Thx to ClipView tool
    static QString const DOC_START(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<?mso-application progid=\"Excel.Sheet\"?>\r\n"
    "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"\r\n"
    " xmlns:o=\"urn:schemas-microsoft-com:office:office\"\r\n"
    " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"\r\n"
    " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"\r\n"
    " xmlns:html=\"http://www.w3.org/TR/REC-html40\">\r\n"
    " <Worksheet ss:Name=\"Sheet1\">\r\n"
    "  <Table ss:ExpandedColumnCount=\"%1\" ss:ExpandedRowCount=\"%2\">\r\n"
    );
    static QString const ROW_START("   <Row>\r\n");
    static QString const ROW_LINE ("    <Cell><Data ss:Type=\"%1\">%2</Data></Cell>\r\n");
    static QString const ROW_END  ("   </Row>\r\n");
    static QString const DOC_END  (
    "  </Table>\r\n"
    " </Worksheet>\r\n"
    "</Workbook>\r\n"
    );

    int columns = model->columnCount();
    int rows    = model->rowCount();

    QString output;

    QVector<int> rlist = selectedRows(settings.selected);
    QVector<int> clist = selectedColumns(settings.selected);

    // -1 for XLSX does not support row number
    int columnCount = settings.columnsExport == toExportSettings::ColumnsSelected ? clist.size()-1 : columns;
    int rowCount    = settings.rowsExport    == toExportSettings::RowsSelected    ? rlist.size()   : rows;

    output += DOC_START.arg(columnCount).arg(rowCount);
    // write data
    for (int row = 0; row < rows; row++)
    {
        if (settings.rowsExport == toExportSettings::RowsSelected && !rlist.contains(row))
            continue;

        output += ROW_START;
        for (int column = 1; column < columns; column++)
        {
            if (settings.columnsExport == toExportSettings::ColumnsSelected && !clist.contains(column))
                continue;

            QVariant data = model->data(model->index(row, column), Qt::EditRole);
            QVariant type = model->headerData(column, Qt::Horizontal, Qt::UserRole);

            QString value;
            if (data.isNull())
                value = "{null}";
            else
                value = TO_ESCAPE(data.toString().toHtmlEscaped());

            output += ROW_LINE.arg("String").arg(value);
        }
        output += ROW_END;
    }
    output += DOC_END;

    return output;
}
