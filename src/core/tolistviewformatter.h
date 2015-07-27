
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

#ifndef TOLISTVIEWFORMATTER
#define TOLISTVIEWFORMATTER

#include "core/toconfenum.h"

#include <QtCore/QString>
#include <QtCore/QModelIndexList>
#include <QtCore/QVector>


class toListView;
class toResultModel;

namespace ToConfiguration
{
    class Exporter : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Exporter() : ConfigContext("Exporter", ENUM_REF(Exporter,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                CsvDelimiter = 13000
                               , CsvSeparator
            };
            virtual QVariant defaultValue(int) const;
    };
};

/**
 * Class for storing settings for export.
 *
 */
class toExportSettings
{
    public:

        enum RowExport
        {
            RowsAll = 0,
            RowsDisplayed,
            RowsSelected
        };

        enum ColumnExport
        {
            ColumnsAll = 0,
            ColumnsSelected
        };

        RowExport    rowsExport;
        ColumnExport columnsExport;
        bool rowsHeader;
        bool columnsHeader;
        int  type;
        QString separator;
        QString delimiter;
        QString extension;
        QString owner; // owner (schema) of the object
        QString objectName; // name of object data of which is being exported

        QModelIndexList selected;

        toExportSettings(RowExport _rowsExport,
                         ColumnExport _columnsExport,
                         int _type,
                         bool _rowsHeader,
                         bool _columnsHeader,
                         const QString &_sep,
                         const QString &_del)
        {
            rowsExport = _rowsExport;
            columnsExport = _columnsExport;
            type = _type;
            rowsHeader = _rowsHeader;
            columnsHeader = _columnsHeader;
            separator = _sep;
            delimiter = _del;

            switch (type)
            {
                default:
                    extension = "*.txt";
                    break;
                case 2:
                    extension = "*.csv";
                    break;
                case 3:
                    extension = "*.html";
                    break;
                case 4:
                    extension = "*.sql";
                    break;
            };
        }

        bool requireSelection()
        {
            return (rowsExport == RowsSelected) || (columnsExport == ColumnsSelected);
        }
    private:
        static ToConfiguration::Exporter s_Exporter;
};


class toListViewFormatter
{
    public:
        toListViewFormatter();
        virtual ~toListViewFormatter();
        virtual QString getFormattedString(toExportSettings &settings,
                                           const QAbstractItemModel * model) = 0;

    protected:
        virtual void endLine(QString &output);
        // build a vector of selected rows for easy searching
        virtual QVector<int> selectedRows(const QModelIndexList &selected);
        virtual QVector<int> selectedColumns(const QModelIndexList &selected);
};

#endif
