
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

#include "core/tolistviewformatter.h"
#include "ts_log/ts_log_utils.h"

QVariant ToConfiguration::Exporter::defaultValue(int option) const
{
	switch(option)
	{
	case CsvSeparator: 	return QVariant(QString(";"));
	case CsvDelimiter: 	return QVariant(QString("\""));
	default:
		Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Exporter un-registered enum value: %1").arg(option)));
		return QVariant();
	}
}

ToConfiguration::Exporter toExportSettings::s_Exporter;

toListViewFormatter::toListViewFormatter()
{
}

toListViewFormatter::~toListViewFormatter()
{
}

void toListViewFormatter::endLine(QString &output)
{
#ifdef Q_OS_WIN32
    output += "\r\n";
#else
    output += "\n";
#endif
}

QVector<int> toListViewFormatter::selectedRows(
    const QModelIndexList &selected)
{

    QVector<int> ret;
    for (QList<QModelIndex>::const_iterator it = selected.begin();
            it != selected.end();
            it++)
    {
        int r = (*it).row();
        if (!ret.contains(r))
            ret.append(r);
    }

    return ret;
}

QVector<int> toListViewFormatter::selectedColumns(
    const QModelIndexList &selected)
{
    QVector<int> ret;
    ret.append(0); // for later check for row headers
    for (QList<QModelIndex>::const_iterator it = selected.begin();
            it != selected.end();
            it++)
    {
        int c = (*it).column();
        if (!ret.contains(c))
            ret.append(c);
    }

    return ret;
}
