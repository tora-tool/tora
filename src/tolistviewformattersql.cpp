//
// C++ Implementation: toListViewFormatterSQL
//
// Copyright: See COPYING file that comes with this distribution
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

    QString sql("INSERT INTO tablename %1 VALUES (%2);");
    QString columnNames;
    QString output;

    if (settings.columnsHeader)
    {
        columnNames += "(";
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
        columnNames = columnNames.left(columnNames.length()-2) + ")";
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
                    values += "TO_DATE(" + currVal + ",\'" + toConfigurationSingle::Instance().dateFormat() + "\')";
                }
            }
            else if (h.contains("CHAR"))
                values += (currVal.isEmpty()) ? "NULL" : "\'" + currVal + "\'";
            else
                values += (currVal.isEmpty()) ? "NULL" : currVal;
            
            values += ", ";
        }
        values = values.left(values.length()-2);
        QString line(sql.arg(columnNames).arg(values));
        endLine(line);
        output += line;
    }

    return output;
}
