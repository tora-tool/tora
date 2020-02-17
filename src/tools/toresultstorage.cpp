
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

#include "tools/toresultstorage.h"
#include "core/utils.h"
#include "core/toeventquery.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

#include <QtGui/QPainter>
#include <QItemDelegate>


// columns count
#define FILECOLUMNS 15
#define COLUMNS (FILECOLUMNS-2)
// order of the result columns
#define COL_NAME 0
#define COL_STATUS 1
#define COL_INFO 2
#define COL_CONTENT 3
#define COL_LOGGING 4
#define COL_SIZE 5
#define COL_FREE_UNIT 6
#define COL_FREE_PERCENT 7
#define COL_AUTOEXT 8
#define COL_USED_FREE_AUTO 9
#define COL_COALESCED 10
#define COL_MAX_FREE 11
#define COL_FREE_FRAGMENTS 12


/*! \brief Item for storage data display and sorting.
*/
class toResultStorageItem : public toResultViewItem
{
        bool AvailableGraph;
    public:
        toResultStorageItem(bool available, toTreeWidget *parent, toTreeWidgetItem *after,
                            const QString &buf = QString::null)
            : toResultViewItem(parent, after, buf), AvailableGraph(available)
        { }
        toResultStorageItem(bool available, toTreeWidgetItem *parent, toTreeWidgetItem *after,
                            const QString &buf = QString::null)
            : toResultViewItem(parent, after, buf), AvailableGraph(available)
        { }

        //! Try to guess real type of the value taken from DB
        void setSortValue(int col, const QVariant & v)
        {
            bool ok;
            qulonglong i = v.toULongLong(&ok);
            if (ok)
            {
                m_sortValues[col] = i;
                return;
            }
            double d = v.toDouble(&ok);
            if (ok)
            {
                m_sortValues[col] = d;
                return;
            }
            m_sortValues[col] = v;
        }

        //! See operator<
        QVariant sortValue(int i) const
        {
            return m_sortValues[i];
        }

        /*! Operator used for sorting. Original operator handles
          only string-based sorting. Now it supports int and double
          for numeric columns.
        */
        bool operator<(const QTreeWidgetItem &other) const
        {
            int column = treeWidget()->sortColumn();
            QVariant v(m_sortValues[column]);

            const toResultStorageItem * o = dynamic_cast<const toResultStorageItem*>(&other);
            Q_ASSERT_X(o, "cast", "only toResultStorageItem are supported");

            switch (v.type())
            {
                case QVariant::ULongLong:
                    return v.toULongLong() < o->sortValue(column).toULongLong();
                case QVariant::Int:
                    return v.toInt() < o->sortValue(column).toInt();
                case QVariant::Double:
                    return v.toDouble() < o->sortValue(column).toDouble();
                case QVariant::String:
                    return v.toString() < o->sortValue(column).toString();
                default:
                    Q_ASSERT_X(0, "compare", "more comparation is not supported now");
                    break;
            };
            // never reached
            return false;
        }

    private:
        //! Store the value used for sorting for every column
        QHash<int, QVariant> m_sortValues;
};

class toResultStorageItemDelegate: public QItemDelegate
{
    public:
        toResultStorageItemDelegate()
        {
        }

        void paint(QPainter *painter, const QStyleOptionViewItem &option,
                   const QModelIndex & index ) const
        {
            if (index.column() == COL_USED_FREE_AUTO )
            {
                int left   = option.rect.left();
                int top    = option.rect.top();
                int width  = option.rect.width();
                int height = option.rect.height();

                QString str = index.model()->data(index, Qt::DisplayRole).toString();
                QStringList pct = str.split(QRegExp("/"));

                if (pct.count() == 3)
                {
                    int w_used = (int) (pct.at(0).toDouble() * width / 100);
                    int w_free = (int) (pct.at(1).toDouble() * width / 100);
                    int w_auto = width - w_used - w_free;

                    painter->fillRect(left, top,
                                      w_used, height, QBrush(Qt::darkRed/*Qt::red*/));
                    painter->fillRect(left + w_used, top,
                                      w_free, height, QBrush(Qt::darkGreen/*Qt::green*/));
                    painter->fillRect(left + w_used + w_free, top,
                                      w_auto, height, QBrush(Qt::darkBlue/*Qt::blue*/));
                    painter->setPen(Qt::white);
                    painter->drawText(option.rect, Qt::TextSingleLine, str);
                }
                else
                {
                    QItemDelegate::paint(painter, option, index);
                }
            }
            else
            {
                QItemDelegate::paint(painter, option, index);
            }
        }
};

bool toResultStorage::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle");
}

toResultStorage::toResultStorage(bool available, QWidget *parent, const char *name)
    : toResultView(false, false, parent, name), AvailableGraph(available)
{
    Unit = toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SizeUnit).toString();
    setAllColumnsShowFocus(true);
    setSortingEnabled(false); // enable it after data fetch
    setRootIsDecorated(true);
    addColumn(tr("Name"));
    addColumn(tr("Status"));
    addColumn(tr("Information"));
    addColumn(tr("Contents"));
    addColumn(tr("Logging"));
    addColumn(tr("Size (%1)").arg(Unit));
    addColumn(tr("Free (%1)").arg(Unit));
    addColumn(tr("Free (%)"));
    addColumn(tr("Autoextend (%1)").arg(Unit));
    if (available)
        addColumn(tr("Used/Free/Autoextend"));
    else
        addColumn(tr("Available"));
    addColumn(tr("Coalesced"));
    addColumn(tr("Max free (%1)").arg(Unit));
    addColumn(tr("Free fragments"));
    setSQLName(tr("toResultStorage"));

    setColumnAlignment(COL_SIZE, Qt::AlignRight);
    setColumnAlignment(COL_FREE_UNIT, Qt::AlignRight);
    setColumnAlignment(COL_FREE_PERCENT, Qt::AlignRight);
    setColumnAlignment(COL_AUTOEXT, Qt::AlignCenter);
    setColumnAlignment(COL_USED_FREE_AUTO, Qt::AlignCenter);
    setColumnAlignment(COL_COALESCED, Qt::AlignRight);
    setColumnAlignment(COL_MAX_FREE, Qt::AlignRight);
    setColumnAlignment(COL_FREE_FRAGMENTS, Qt::AlignRight);

    ShowCoalesced = false;
    OnlyFiles = false;

    Tablespaces = Files = NULL;

    setItemDelegate(new toResultStorageItemDelegate());
}

toResultStorage::~toResultStorage()
{
    delete Tablespaces;
    delete Files;
}

static toSQL SQLShowCoalesced("toResultStorage:ShowCoalesced",
                              "SELECT d.tablespace_name,\n"
                              "       d.status,\n"
                              "       d.extent_management,\n"
                              "       d.contents,\n"
                              "       d.logging,\n"
                              "       TO_CHAR(ROUND(NVL(a.tbs_size,0) / b.unit,2)),\n"
                              "       TO_CHAR(ROUND(NVL(f.free_size,0) / b.unit,2)),\n"
                              "       TO_CHAR(ROUND(NVL(f.free_size,0)/NVL(a.tbs_size,1)*100)),\n" // free%
                              "       TO_CHAR(ROUND(NVL(a.autoextend_size,0) / b.unit,2)),\n"
                              "       '-',\n"
                              "       TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
                              "       '-',\n"
                              "       TO_CHAR(f.free_extents)\n"
                              "  FROM (SELECT :unt<int> unit FROM sys.dual) b,\n"
                              "       sys.dba_tablespaces d\n"
                              "  LEFT JOIN (SELECT tablespace_name,\n"
                              "                    SUM(bytes) tbs_size,\n"
                              "       SUM(DECODE(autoextensible,'YES',maxbytes,bytes)) autoextend_size\n"
                              "               FROM sys.dba_data_files\n"
                              "        GROUP BY tablespace_name) a\n"
                              "       ON d.tablespace_name = a.tablespace_name\n"
                              "  LEFT JOIN (SELECT tablespace_name,\n"
                              "                    total_bytes free_size,\n"
                              "                    total_extents free_extents,\n"
                              "                    percent_extents_coalesced\n"
                              "               FROM sys.dba_free_space_coalesced) f\n"
                              "       ON d.tablespace_name = f.tablespace_name\n"
                              " WHERE NOT (d.extent_management like 'LOCAL' AND d.contents like 'TEMPORARY')\n"
                              " UNION ALL\n"
                              "SELECT d.tablespace_name,\n"
                              "       d.status,\n"
                              "       d.extent_management,\n"
                              "       d.contents,\n"
                              "       d.logging,\n"
                              "       TO_CHAR(ROUND(NVL(a.tbs_size,0) / b.unit,2)),\n"
                              "       TO_CHAR(ROUND(NVL(f.free_size,0) / b.unit,2)),\n"
                              "       TO_CHAR(ROUND(NVL(f.free_size,0)/NVL(a.tbs_size,1)*100)),\n" // free%
                              "       TO_CHAR(ROUND(NVL(a.autoextend_size,0) / b.unit,2)),\n"
                              "       '-',\n"
                              "       '-',\n"
                              "       TO_CHAR(ROUND(NVL(f.max_free,0) / b.unit,2)),\n"
                              "       TO_CHAR(f.total_extents)\n"
                              "  FROM (SELECT :unt<int> unit FROM sys.dual) b,\n"
                              "       sys.dba_tablespaces d\n"
                              "  LEFT JOIN (SELECT tablespace_name,\n"
                              "                    SUM(bytes) tbs_size,\n"
                              "                    SUM(DECODE(autoextensible,'YES',maxbytes,bytes)) autoextend_size\n"
                              "               FROM sys.dba_temp_files\n"
                              "              GROUP BY tablespace_name) a\n"
                              "       ON d.tablespace_name = a.tablespace_name\n"
                              "  LEFT JOIN (SELECT tablespace_name,\n"
                              "                    SUM(bytes_cached) free_size,\n"
                              "                    COUNT(1) total_extents,\n"
                              "                    MAX(bytes_cached) max_free\n"
                              "               FROM v$temp_extent_pool\n"
                              "              GROUP BY tablespace_name) f\n"
                              "       ON d.tablespace_name = f.tablespace_name\n"
                              " WHERE d.extent_management = 'LOCAL' AND d.contents = 'TEMPORARY'\n",
                              "Display storage usage of database. This includes the coalesced columns which may make the query sluggish on some DB:s. "
                              "All columns must be present in output (Should be 12)",
                              "0801");

static toSQL SQLShowCoalesced8("toResultStorage:ShowCoalesced",
                               "SELECT d.tablespace_name,\n"
                               "       d.status,\n"
                               "       ' ',\n"
                               "       d.contents,\n"
                               "       d.logging,\n"
                               "       TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
                               "       TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)),\n"
                               "       TO_CHAR(ROUND(NVL(f.bytes,0)/NVL(a.bytes,1)*100)),\n" // free%
                               "       TO_CHAR(ROUND(NVL(a.maxbytes / b.unit, 0),2)),\n"
                               "       '-',\n"
                               "       TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
                               "       '-',\n"
                               "       TO_CHAR(f.total_extents)\n"
                               "  FROM sys.dba_tablespaces d,\n"
                               "       (select tablespace_name, sum(bytes) bytes, sum(maxbytes) maxbytes from sys.dba_data_files group by tablespace_name) a,\n"
                               "       (select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from sys.dba_free_space_coalesced) f,\n"
                               "       (select :unt<int> unit from sys.dual) b\n"
                               " WHERE d.tablespace_name = a.tablespace_name(+)\n"
                               "   AND d.tablespace_name = f.tablespace_name(+)\n"
                               " ORDER BY d.tablespace_name",
                               "",
                               "0800");

static toSQL SQLShowCoalesced7("toResultStorage:ShowCoalesced",
                               "SELECT d.tablespace_name,\n"
                               "       d.status,\n"
                               "       ' ',\n"
                               "       d.contents,\n"
                               "       'N/A',\n"
                               "       TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
                               "       TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)),\n"
                               "       TO_CHAR(ROUND(NVL(f.bytes,0)/NVL(a.bytes,1)*100)),\n" // free%
                               "       '-',\n"
                               "       '-',\n"
                               "       TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
                               "       '-',\n"
                               "       TO_CHAR(f.total_extents)\n"
                               "  FROM sys.dba_tablespaces d,\n"
                               "       (select tablespace_name, sum(bytes) bytes from sys.dba_data_files group by tablespace_name) a,\n"
                               "       (select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from sys.dba_free_space_coalesced) f,\n"
                               "       (select :unt<int> unit from sys.dual) b\n"
                               " WHERE d.tablespace_name = a.tablespace_name(+)\n"
                               "   AND d.tablespace_name = f.tablespace_name(+)\n"
                               " ORDER BY d.tablespace_name",
                               "",
                               "0703");

static toSQL SQLNoShowCoalesced(
    "toResultStorage:NoCoalesced",
    "SELECT d.tablespace_name,\n"           /* 1 */
    "       d.status,\n"                /* 2 */
    "       d.extent_management,\n"         /* 3 */
    "       d.CONTENTS,\n"              /* 4 */
    "       d.LOGGING,\n"               /* 5 */
    "       TO_CHAR ( ROUND ( NVL ( s.tbs_size,\n"
    "                               0 ) / b.unit,\n"
    "                         2 ) ),\n"         /* 6 */
    "       TO_CHAR ( ROUND ( NVL ( f.free_size,\n"
    "                               0 ) / b.unit,\n"
    "                         2 ) ),\n"         /* 7 */
    "       TO_CHAR(ROUND(NVL(f.free_size,0)/NVL(s.tbs_size,1)*100)),\n" // free%
    "       TO_CHAR ( ROUND ( NVL ( s.autoextend_size,\n"
    "                               0 ) / b.unit,\n"
    "                         2 ) ),\n"         /* 8 */
    "       '-',\n"                 /* 9 */
    "       '-',\n"                 /* 10 */
    "       TO_CHAR ( ROUND ( NVL ( f.max_free,\n"
    "                               0 ) / b.unit,\n"
    "                         2 ) ),\n"         /* 11 */
    "       TO_CHAR ( f.free_extents )\n"       /* 12 */
    "  FROM ( SELECT :unt<int> unit\n"
    "           FROM sys.dual ) b,\n"
    "       sys.dba_tablespaces d,\n"
    "       ( SELECT tablespace_name,\n"
    "                SUM ( bytes ) tbs_size,\n"
    "                SUM ( DECODE ( autoextensible,\n"
    "                               'YES',\n"
    "                               maxbytes,\n"
    "                               bytes ) ) autoextend_size\n"
    "           FROM sys.dba_data_files\n"
    "          GROUP BY tablespace_name\n"
    "          UNION ALL SELECT tablespace_name,\n"
    "                SUM ( bytes ) tbs_size,\n"
    "                SUM ( DECODE ( autoextensible,\n"
    "                               'YES',\n"
    "                               maxbytes,\n"
    "                               bytes ) ) autoextend_size\n"
    "           FROM sys.dba_temp_files\n"
    "          GROUP BY tablespace_name ) s,\n"
    "       ( SELECT tablespace_name,\n"
    "                SUM ( bytes ) free_size,\n"
    "                MAX ( bytes ) max_free,\n"
    "                COUNT ( 1 ) free_extents\n"
    "           FROM sys.dba_free_space\n"
    "          GROUP BY tablespace_name\n"
    "          UNION ALL SELECT tablespace_name,\n"
    "                SUM ( bytes_cached ) free_size,\n"
    "                MAX ( bytes_cached ) max_free,\n"
    "                COUNT ( 1 ) free_extents\n"
    "           FROM v$temp_extent_pool\n"
    "          GROUP BY tablespace_name ) f\n"
    " WHERE d.tablespace_name = s.tablespace_name ( + )\n"
    "   AND d.tablespace_name = f.tablespace_name ( + )",
    "Display storage usage of database. This does not include the coalesced "
    "columns which may make the query sluggish on some DB:s. "
    "All columns must be present in output (Should be 12)",
    "0801");

static toSQL SQLNoShowCoalesced8("toResultStorage:NoCoalesced",
                                 "SELECT d.tablespace_name,\n"
                                 "       d.status,\n"
                                 "       ' ',\n"
                                 "       d.contents,\n"
                                 "       d.logging,\n"
                                 "       TO_CHAR(ROUND(NVL(a.bytes,0) / b.unit,2)),\n"
                                 "       TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)),\n"
                                 "       TO_CHAR(ROUND(NVL(f.bytes,0)/NVL(a.bytes,1)*100)),\n" // free%
                                 "       TO_CHAR(ROUND(NVL(a.maxbytes,0) / b.unit,2)),\n"
                                 "       '-',\n"
                                 "       '-',\n"
                                 "       TO_CHAR(ROUND(NVL(f.maxbytes,0) / b.unit,2)),\n"
                                 "       TO_CHAR(f.total_extents)\n"
                                 "  FROM sys.dba_tablespaces d,\n"
                                 "       (select tablespace_name, sum(bytes) bytes, sum(maxbytes) maxbytes from sys.dba_data_files group by tablespace_name) a,\n"
                                 "       (select tablespace_name, NVL(sum(bytes),0) bytes, count(1) total_extents, NVL(max(bytes),0) maxbytes from sys.dba_free_space group by tablespace_name) f,\n"
                                 "       (select :unt<int> unit from sys.dual) b\n"
                                 " WHERE d.tablespace_name = a.tablespace_name(+)\n"
                                 "   AND d.tablespace_name = f.tablespace_name(+)\n"
                                 " ORDER BY d.tablespace_name",
                                 "",
                                 "0800");

static toSQL SQLNoShowCoalesced7("toResultStorage:NoCoalesced",
                                 "SELECT d.tablespace_name,\n"
                                 "       d.status,\n"
                                 "       ' ',\n"
                                 "       d.contents,\n"
                                 "       'N/A',\n"
                                 "       TO_CHAR(ROUND(NVL(a.bytes,0) / b.unit,2)),\n"
                                 "       TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)),\n"
                                 "       TO_CHAR(ROUND(NVL(f.bytes,0)/NVL(a.bytes,1)*100)),\n" // free%
                                 "       '-',\n"
                                 "       '-',\n"
                                 "       '-',\n"
                                 "       TO_CHAR(ROUND(NVL(f.maxbytes,0) / b.unit,2)),\n"
                                 "       TO_CHAR(f.total_extents)\n"
                                 "  FROM sys.dba_tablespaces d,\n"
                                 "       (select tablespace_name, sum(bytes) bytes from sys.dba_data_files group by tablespace_name) a,\n"
                                 "       (select tablespace_name, NVL(sum(bytes),0) bytes, count(1) total_extents, NVL(max(bytes),0) maxbytes from sys.dba_free_space group by tablespace_name) f,\n"
                                 "       (select :unt<int> unit from sys.dual) b\n"
                                 " WHERE d.tablespace_name = a.tablespace_name(+)\n"
                                 "   AND d.tablespace_name = f.tablespace_name(+)\n"
                                 " ORDER BY d.tablespace_name",
                                 "",
                                 "0703");

static toSQL SQLDatafile(
    "toResultStorage:Datafile",
    "SELECT d.tablespace_name,\n"
    "       v.name,\n"
    "       v.status,\n"
    "       v.enabled,\n"
    "       ' ',\n"
    "       ' ',\n"
    "       to_char ( round ( d.bytes / b.unit,\n"
    "                         2 ) ),\n"
    "       to_char ( round ( s.bytes / b.unit,\n"
    "                         2 ) ),\n"
    "       TO_CHAR(ROUND(NVL(s.bytes,0)/NVL(d.bytes,1)*100)),\n" // free%
    "       to_char ( round ( d.maxbytes / b.unit,\n"
    "                         2 ) ),\n"
    "       '-',\n"
    "       ' ',\n"
    "       to_char ( round ( s.maxbytes / b.unit,\n"
    "                         2 ) ),\n"
    "       to_char ( s.num ),\n"
    "       NULL,\n"            // Used to fill in tablespace name
    "       v.file#\n"          // Needed by toStorage to work
    "  FROM ( SELECT :unt<int> unit\n"
    "           FROM sys.dual ) b,\n"
    "       sys.dba_data_files d\n"
    "       JOIN v$datafile v\n"
    "       ON      d.file_id = v.file#\n"
    "       LEFT JOIN\n"
    "       ( SELECT file_id,\n"
    "                NVL ( SUM ( bytes ),\n"
    "                      0 ) bytes,\n"
    "                COUNT ( 1 ) num,\n"
    "                NVL ( MAX ( bytes ),\n"
    "                      0 ) maxbytes\n"
    "           FROM sys.dba_free_space\n"
    "          GROUP BY file_id ) s\n"
    "       ON      s.file_id = d.file_id\n"
    " UNION ALL SELECT d.tablespace_name,\n"
    "       v.name,\n"
    "       v.status,\n"
    "       v.enabled,\n"
    "       ' ',\n"
    "       ' ',\n"
    "       to_char ( round ( d.bytes / b.unit,\n"
    "                         2 ) ),\n"
    "       to_char ( round ( ( d.user_bytes - t.bytes_cached ) / b.unit,\n"
    "                         2 ) ),\n"
    "       TO_CHAR(ROUND(NVL(d.user_bytes - t.bytes_cached,0)/NVL(d.bytes,1)*100)),\n" // free%
    "       to_char ( round ( d.maxbytes / b.unit,\n"
    "                         2 ) ),\n"
    "       '-',\n"
    "       ' ',\n"
    "       ' ',\n"
    "       '1',\n"
    "       NULL,\n"
    "       v.file#\n"
    "  FROM ( SELECT :unt<int> unit\n"
    "           FROM sys.dual ) b,\n"
    "       sys.dba_temp_files d\n"
    "       JOIN v$tempfile v\n"
    "       ON      d.file_id = v.file#\n"
    "       LEFT JOIN v$temp_extent_pool t\n"
    "       ON      t.file_id = d.file_id",
    "Display information about a datafile in a tablespace. "
    "All columns must be present in the output (Should be 14)",
    "0900");

static toSQL SQLDatafile8(
    "toResultStorage:Datafile",
    "SELECT d.tablespace_name,\n"
    "       v.name,\n"
    "       v.status,\n"
    "       v.enabled,\n"
    "       ' ',\n"
    "       ' ',\n"
    "       to_char ( round ( d.bytes / b.unit,\n"
    "                         2 ) ),\n"
    "       to_char ( round ( s.bytes / b.unit,\n"
    "                         2 ) ),\n"
    "       TO_CHAR(ROUND(NVL(s.bytes,0)/NVL(d.bytes,1)*100)),\n" // free%
    "       to_char ( round ( d.maxbytes / b.unit,\n"
    "                         2 ) ),\n"
    "       '-',\n"
    "       ' ',\n"
    "       to_char ( round ( s.maxbytes / b.unit,\n"
    "                         2 ) ),\n"
    "       to_char ( s.num ),\n"
    "       NULL,\n"            // Used to fill in tablespace name
    "       v.file#\n"          // Needed by toStorage to work
    "  FROM ( SELECT :unt<int> unit\n"
    "           FROM sys.dual ) b,\n"
    "       sys.dba_data_files d,\n"
    "       v$datafile v,\n"
    "       ( SELECT file_id,\n"
    "                NVL ( SUM ( bytes ),\n"
    "                      0 ) bytes,\n"
    "                COUNT ( 1 ) num,\n"
    "                NVL ( MAX ( bytes ),\n"
    "                      0 ) maxbytes\n"
    "           FROM sys.dba_free_space\n"
    "          GROUP BY file_id ) s\n"
    " WHERE d.file_id = v.file#\n"
    "   AND d.file_id = s.file_id ( + )\n"
    " UNION ALL SELECT d.tablespace_name,\n"
    "       v.name,\n"
    "       v.status,\n"
    "       v.enabled,\n"
    "       ' ',\n"
    "       ' ',\n"
    "       to_char ( round ( d.bytes / b.unit,\n"
    "                         2 ) ),\n"
    "       to_char ( round ( ( d.user_bytes - t.bytes_cached ) / b.unit,\n"
    "                         2 ) ),\n"
    "       TO_CHAR(ROUND(NVL(d.user_bytes - t.bytes_cached,0)/NVL(d.bytes,1)*100)),\n" // free%
    "       to_char ( round ( d.maxbytes / b.unit,\n"
    "                         2 ) ),\n"
    "       '-',\n"
    "       ' ',\n"
    "       ' ',\n"
    "       '1',\n"
    "       NULL,\n"
    "       v.file#\n"
    "  FROM ( SELECT :unt<int> unit\n"
    "           FROM sys.dual ) b,\n"
    "       sys.dba_temp_files d,\n"
    "       v$tempfile v,\n"
    "       v$temp_extent_pool t\n"
    " WHERE d.file_id = v.file#\n"
    "   AND t.file_id = d.file_id ( + )",
    "",
    "0800");

static toSQL SQLDatafile7("toResultStorage:Datafile",
                          "SELECT  d.tablespace_name,\n"
                          "    v.name,\n"
                          "    v.status,\n"
                          "    v.enabled,\n"
                          "    ' ',\n"
                          "    ' ',\n"
                          "        to_char(round(d.bytes/b.unit,2)),\n"
                          "        to_char(round(s.bytes/b.unit,2)),\n"
                          "       TO_CHAR(ROUND(NVL(s.bytes,0)/NVL(d.bytes,1)*100)),\n" // free%
                          "        '-',\n"
                          "        '-',\n"
                          "    ' ',\n"
                          "        to_char(round(s.maxbytes/b.unit,2)),\n"
                          "    to_char(s.num),\n"
                          "        NULL,\n"
                          "        v.file#\n"
                          "  FROM  sys.dba_data_files d,\n"
                          "    v$datafile v,\n"
                          "    (SELECT file_id, NVL(SUM(bytes),0) bytes, COUNT(1) num,NVL(MAX(bytes),0) maxbytes FROM sys.dba_free_space  GROUP BY file_id) s,\n"
                          "        (select :unt<int> unit from sys.dual) b\n"
                          " WHERE  (s.file_id (+)= d.file_id)\n"
                          "   AND  (d.file_name = v.name)",
                          "",
                          "0703");

void toResultStorage::saveSelected(void)
{
    toTreeWidgetItem *item = selectedItem();
    if (item)
    {
        if (item->parent() || OnlyFiles)
        {
            CurrentSpace = item->text(COLUMNS);
            CurrentFile = item->text(0);
        }
        else
        {
            CurrentSpace = item->text(0);
            CurrentFile = QString::null;
        }
    }
}

void toResultStorage::query(const QString &sql, toQueryParams const& param)
{
    Q_UNUSED(sql);
    Q_UNUSED(param);

    if (!handled() || Tablespaces || Files)
        return ;

    try
    {
        saveSelected();
        clear();

        toConnection &conn = connection();

        toQueryParams args;
        args << toQValue(Utils::toSizeDecode(Unit));

        TablespaceValues.clear();
        FileValues.clear();

        Tablespaces = new toEventQuery(this
                                       , conn
                                       , toSQL::string(ShowCoalesced ? SQLShowCoalesced : SQLNoShowCoalesced, connection())
                                       , args
                                       , toEventQuery::READ_ALL);
        auto c1 = connect(Files, &toEventQuery::dataAvailable, this, &toResultStorage::receiveTablespaceData);
        connect(Tablespaces, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(slotDoneTablespaces()));
        Tablespaces->start();

        Files = new toEventQuery(this
                                 , conn
                                 , toSQL::string(SQLDatafile, connection())
                                 , args
                                 , toEventQuery::READ_ALL);
        auto c2 = connect(Files, &toEventQuery::dataAvailable, this, &toResultStorage::receiveFilesData);
        connect(Files, SIGNAL(done(toEventQuery*, unsigned long)), this, SLOT(slotDoneFiles()));
        Files->start();
    }
    TOCATCH
}

void toResultStorage::updateList(void)
{
    setSortingEnabled(false); // enable it after data fetch
    clear();
    if (!OnlyFiles)
    {
        for (QStringList::iterator j = TablespaceValues.begin(); j != TablespaceValues.end();)
        {
            toResultStorageItem *tablespace = new toResultStorageItem(AvailableGraph, this, NULL);
            for (int i = 0; i < COLUMNS && j != TablespaceValues.end(); i++, j++)
            {
                if (i == COL_USED_FREE_AUTO)
                    continue;
                tablespace->setText(i, *j);
                tablespace->setSortValue(i, *j);
            }

            // To fill Used/Free/Autoextend column
            double total = tablespace->text(COL_FREE_PERCENT).toDouble();
            double user = tablespace->text(COL_SIZE).toDouble();
            double free = tablespace->text(COL_FREE_UNIT).toDouble();
            if (total < user )
                total = user;
            user /= total;
            free /= total;
            QString t;
//             t.sprintf("%05.1f / %05.1f / %05.1f%%", (user-free)*100, free*100, (1 - user)*100);
// spaces seems better than 0-filling...
            t.sprintf("%#5.1f / %#5.1f / %#5.1f%%", (user - free) * 100, free * 100, (1 - user) * 100);
            tablespace->setText(COL_USED_FREE_AUTO, t);
            tablespace->setSortValue(COL_USED_FREE_AUTO, (user - free) * 100);
            // end of Used/Free/Autoextend column

            if (CurrentSpace == tablespace->text(0))
            {
                if (CurrentFile.isEmpty())
                    tablespace->setSelected(true);
            }
        }
    }

    for (QStringList::iterator k = FileValues.begin(); k != FileValues.end();)
    {
        QString name = *k;
        k++;

        toTreeWidgetItem *file;
        toTreeWidgetItem *tablespace = NULL;
        if (OnlyFiles)
        {
            file = new toResultStorageItem(AvailableGraph, this, NULL);
        }
        else
        {
            for (tablespace = firstChild(); tablespace && tablespace->text(0) != name; tablespace = tablespace->nextSibling())
                ;
            if (!tablespace)
            {
                if (Files)
                    break;
            }
            file = new toResultStorageItem(AvailableGraph, tablespace, NULL);
        }
        for (int i = 0; i < FILECOLUMNS && k != FileValues.end(); i++, k++)
        {
            file->setText(i, *k);
            reinterpret_cast<toResultStorageItem*>(file)->setSortValue(i, *k);
        }

        file->setText(COLUMNS, name);
        reinterpret_cast<toResultStorageItem*>(file)->setSortValue(COLUMNS, name);
        if (CurrentSpace == file->text(COLUMNS) &&
                CurrentFile == file->text(0))
        {
            if (tablespace)
                tablespace->setOpen(true);
            setSelected(file, true);
        }
    }
    setSortingEnabled(true);
}

void toResultStorage::receiveTablespaceData(toEventQuery*)
{
    try
    {
        if (!Utils::toCheckModal(this))
            return;

        int cols = Tablespaces->describe().size();
        while (Tablespaces->hasMore())
        {
            for (int i = 0; i < cols && !Tablespaces->eof(); i++)
                TablespaceValues.append(QString(Tablespaces->readValue()));
        }
    }
    catch (const QString &exc)
    {
        delete Tablespaces;
        Tablespaces = NULL;
        Utils::toStatusMessage(exc);
    }
} // pollTablespaces

void toResultStorage::slotDoneTablespaces(void)
{
    delete Tablespaces;
    Tablespaces = NULL;

    if (Tablespaces == NULL && Files == NULL)
        updateList();
} // doneTablespaces

void toResultStorage::receiveFilesData(toEventQuery*)
{
    try
    {
        if (!Utils::toCheckModal(this))
            return;

        int cols = Files->describe().size();
        while (Files->hasMore())
        {
            for (int i = 0; i < cols && !Files->eof(); i++)
                FileValues.append(QString(Files->readValue()));
        }
    }
    catch (const QString &exc)
    {
        delete Files;
        Files = NULL;
        Utils::toStatusMessage(exc);
    }
}

void toResultStorage::slotDoneFiles(void)
{
    delete Files;
    Files = NULL;

    if (Tablespaces == NULL && Files == NULL)
        updateList();
} // pollFiles

QString toResultStorage::currentTablespace(void)
{
    toTreeWidgetItem *item = selectedItem();
    if (!item)
        throw tr("No tablespace selected");
    QString name;
    if (item->parent() || OnlyFiles)
        name = item->text(COLUMNS);
    else
        name = item->text(0);
    if (name.isEmpty())
        throw tr("Weird, empty tablespace name");
    return name;
}

QString toResultStorage::currentFilename(void)
{
    toTreeWidgetItem *item = selectedItem();
    if (!item || (!item->parent() && !OnlyFiles))
        throw tr("No file selected");
    QString name = item->text(0);
    return name;
}

void toResultStorage::setOnlyFiles(bool only)
{
    saveSelected();
    if (only && !OnlyFiles)
    {
        addColumn(tr("Tablespace"));
        setRootIsDecorated(false);
    }
    else if (!only && OnlyFiles)
    {
        // removing columns doesn't work with qtreewidget
//         removeColumn(columns() - 1); todo
        setRootIsDecorated(true);
    }
    OnlyFiles = only;
    updateList();
}
