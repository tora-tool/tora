
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

#include "core/toresultextent.h"
#include "core/tosql.h"
#include "core/toresulttableview.h"
#include "core/utils.h"

#include <QProgressDialog>
#include <QtCore/QCoreApplication>
#include <QtGui/QPainter>

static toSQL SQLListExtents("toResultStorage:ListExtents",
                            "SELECT * \n"
                            "  FROM SYS.DBA_EXTENTS WHERE OWNER = :f1<char[101]> AND SEGMENT_NAME = :f2<char[101]>\n"
                            " ORDER BY block_id",
                            "List the extents of a table in a schema.",
                            "" ,
                            "Oracle");

static toSQL SQLObjectsFile("toStorageExtent:ObjectsFile",
                            "SELECT owner,\n"
                            "       segment_name,\n"
                            "       partition_name,\n"
                            "       file_id,\n"
                            "       block_id,\n"
                            "       blocks\n"
                            "  FROM sys.dba_extents\n"
                            " WHERE tablespace_name = :tab<char[101]>\n"
                            "   AND file_id = :fil<int>",
                            "Get objects in a datafile, must have same columns and binds",
                            "0800");

static toSQL SQLObjectsFile7("toStorageExtent:ObjectsFile",
                             "SELECT owner,\n"
                             "       segment_name,\n"
                             "       NULL,\n"
                             "       file_id,\n"
                             "       block_id,\n"
                             "       blocks\n"
                             "  FROM sys.dba_extents\n"
                             " WHERE tablespace_name = :tab<char[101]>\n"
                             "   AND file_id = :fil<int>",
                             "",
                             "0703");

static toSQL SQLObjectsTablespace("toStorageExtent:ObjectsTablespace",
                                  "SELECT owner,\n"
                                  "       segment_name,\n"
                                  "       partition_name,\n"
                                  "       file_id,\n"
                                  "       block_id,\n"
                                  "       blocks\n"
                                  "  FROM sys.dba_extents WHERE tablespace_name = :tab<char[101]>",
                                  "Get objects in a tablespace, must have same columns and binds",
                                  "0800");

static toSQL SQLObjectsTablespace7("toStorageExtent:ObjectsTablespace",
                                   "SELECT owner,\n"
                                   "       segment_name,\n"
                                   "       NULL,\n"
                                   "       file_id,\n"
                                   "       block_id,\n"
                                   "       blocks\n"
                                   "  FROM sys.dba_extents WHERE tablespace_name = :tab<char[101]>",
                                   "",
                                   "0703");

static toSQL SQLTableTablespace("toStorageExtent:TableTablespace",
                                "SELECT MAX(tablespace_name)\n"
                                "  FROM (SELECT tablespace_name FROM sys.all_all_tables\n"
                                "         WHERE owner = :own<char[101]> AND table_name = :tab<char[101]>\n"
                                "        UNION\n"
                                "        SELECT i.tablespace_name\n"
                                "          FROM sys.all_indexes i,\n"
                                "               sys.all_constraints c,\n"
                                "               sys.all_all_tables t\n"
                                "         WHERE t.owner = :own<char[101]> AND t.table_name = :tab<char[101]>\n"
                                "           AND i.table_name = t.table_name AND i.owner = t.owner\n"
                                "           AND c.constraint_name = i.index_name AND c.owner = i.owner\n"
                                "           AND c.constraint_type = 'P')",
                                "Get the tablespace of a table");

static toSQL SQLTableTablespace7("toStorageExtent:TableTablespace",
                                 "SELECT tablespace_name FROM sys.all_tables\n"
                                 " WHERE owner = :own<char[101]> AND table_name = :tab<char[101]>",
                                 "",
                                 "0703");

static toSQL SQLFileBlocks("toStorageExtent:FileSize",
                           "SELECT file_id,blocks FROM sys.dba_data_files\n"
                           " WHERE tablespace_name = :tab<char[101]>\n"
                           "   AND file_id = :fil<int>\n"
                           " ORDER BY file_id",
                           "Get blocks for datafiles, must have same columns and binds");

static toSQL SQLTablespaceBlocks("toStorageExtent:TablespaceSize",
                                 "SELECT file_id,blocks FROM sys.dba_data_files\n"
                                 " WHERE tablespace_name = :tab<char[101]>"
                                 " ORDER BY file_id",
                                 "Get blocks for tablespace datafiles, must have same columns and binds");

toResultExtent::toResultExtent(QWidget *parent, const char *name)
	: QSplitter(Qt::Vertical, parent)
{
	setObjectName(name);
	Graph = new toStorageExtent(this);
	List = new toResultTableView(true, false, this);
	List->setSQL(SQLListExtents);

	setChildrenCollapsible(false);
}

bool toResultExtent::canHandle(const toConnection &conn)
{
	return conn.providerIs("Oracle");
}

void toResultExtent::query(const QString &sql, toQueryParams const& params)
{
	try
	{
		if (!handled())
			return ;

		if (!setSqlAndParams(sql, params))
			return ;

		toQueryParams::const_iterator i = params.begin();
		if (i == params.end())
			return ;
		QString owner = (QString)*i;

		i++;
		if (i == params.end())
			return ;
		QString table = (QString)*i;

		List->refreshWithParams(toQueryParams() << owner << table);

		toQList res = toQuery::readQuery(connection(), SQLTableTablespace, toQueryParams() << owner << table);

		Graph->setTablespace((QString)Utils::toShift(res));
		Graph->highlight(owner, table, QString::null);
	}
	TOCATCH
		}

void toResultExtent::clearData()
{
	List->clearData();
	// TODO: graph should also be cleared. Somehow...
}

toStorageExtent::extentName::extentName(const QString &owner, const QString &table,
                                        const QString &partition, int size)
	: Owner(owner), Table(table), Partition(partition)
{
	Size = size;
}

bool toStorageExtent::extentTotal::operator < (const toStorageExtent::extentTotal &ext) const
{
	if (fileView )
	{
		if (LastBlock < ext.LastBlock)
			return true;
		return false;
	}
	else
	{
		if (Owner < ext.Owner)
			return true;
		if (Owner > ext.Owner)
			return false;
		if (Table < ext.Table)
			return true;
		if (Table > ext.Table)
			return false;
		if (Partition < ext.Partition)
			return true;
		return false;
	}
}

bool toStorageExtent::extentName::operator == (const toStorageExtent::extentName &ext) const
{
	return Owner == ext.Owner && Table == ext.Table && (Partition == ext.Partition || ext.Partition.isNull());
}

toStorageExtent::extent::extent(const QString &owner, const QString &table,
                                const QString &partition,
                                int file, int block, int size)
	: extentName(owner, table, partition, size)
{
	File = file;
	Block = block;
}

bool toStorageExtent::extent::operator < (const toStorageExtent::extent &ext) const
{
	if (File < ext.File)
		return true;
	if (File > ext.File)
		return false;
	if (Block < ext.Block)
		return true;
	return false;
}

bool toStorageExtent::extent::operator == (const toStorageExtent::extent &ext) const
{
	return Owner == ext.Owner && Table == ext.Table && Partition == ext.Partition &&
		File == ext.File && Block == ext.Block && Size == ext.Size;
}

toStorageExtent::toStorageExtent(QWidget *parent, const char *name)
	: QWidget(parent)
	, Total(0)
{
	setObjectName(name);
	QPalette pal = palette();
	pal.setColor(backgroundRole(), Qt::white);
	setPalette(pal);
}

void toStorageExtent::highlight(const QString &owner, const QString &table,
                                const QString &partition)
{
	Highlight.Owner = owner;
	Highlight.Table = table;
	Highlight.Partition = partition;
	update();
}

void toStorageExtent::setTablespace(const QString &tablespace)
{
	fileView = false;
	try
	{
		toConnectionSubLoan c(toConnection::currentConnection(this));
		if (Tablespace == tablespace)
			return ;
		Tablespace = tablespace;
		Utils::toBusy busy;
		Extents.clear();
		FileOffset.clear();
		toQuery query(c, SQLObjectsTablespace, toQueryParams() << tablespace);

		// It's used to keep UI "non-freezed".
		// OK, there are better query classes for it but it's enough for now.
		// TODO: rewrite data fetching for this tool
		int progressMax = 1000;
		int progressCurr = 1;
		QProgressDialog progress("Reading Objects...", "Abort", 0, progressMax, this);

		extent cur;
		while (!query.eof())
		{
			if (progressCurr > (progressMax - 1))
				progressCurr = 0;
			if (progressCurr % 200 == 0)
			{
				progress.setValue(progressCurr);
				QCoreApplication::processEvents();
			}
			if (progress.wasCanceled())
			{
				Extents.clear();
				break;
			}
			++progressCurr;

			cur.Owner = (QString)query.readValue();
			cur.Table = (QString)query.readValue();
			cur.Partition = (QString)query.readValue();
			cur.File = query.readValue().toInt();
			cur.Block = query.readValue().toInt();
			cur.Size = query.readValue().toInt();
			Utils::toPush(Extents, cur);
		}
		toQuery blocks(c, SQLTablespaceBlocks, toQueryParams() << tablespace);
		Total = 0;
		while (!blocks.eof())
		{
			int id = blocks.readValue().toInt();
			FileOffset[id] = Total;
			Total += blocks.readValue().toInt();
		}
	}
	TOCATCH
		Extents.sort();
	update();
}

void toStorageExtent::setFile(const QString &tablespace, int file)
{
	fileView = true;
	try
	{
		Utils::toBusy busy;
		Extents.clear();
		FileOffset.clear();
		toConnectionSubLoan c(toConnection::currentConnection(this));
		toQuery query(c, SQLObjectsFile, toQueryParams() << tablespace << QString::number(file));
		extent cur;

		int progressMax = 1000;
		int progressCurr = 1;
		QProgressDialog progress("Reading Objects...", "Abort", 0, progressMax, this);

		while (!query.eof())
		{
			if (progressCurr > (progressMax - 1))
				progressCurr = 0;
			if (progressCurr % 200 == 0)
			{
				progress.setValue(progressCurr);
				QCoreApplication::processEvents();
			}
			if (progress.wasCanceled())
			{
				Extents.clear();
				break;
			}
			++progressCurr;

			cur.Owner = (QString)query.readValue();
			cur.Table = (QString)query.readValue();
			cur.Partition = (QString)query.readValue();
			cur.File = query.readValue().toInt();
			cur.Block = query.readValue().toInt();
			cur.Size = query.readValue().toInt();
			Utils::toPush(Extents, cur);
		}
		toQuery blocks(c, SQLFileBlocks, toQueryParams() << tablespace << QString::number(file));
		Total = 0;
		while (!blocks.eof())
		{
			int id = blocks.readValue().toInt();
			FileOffset[id] = Total;
			Total += blocks.readValue().toInt();
		}
	}
	TOCATCH
		Extents.sort();
	update();
}

void toStorageExtent::paintEvent(QPaintEvent *)
{
	QPainter paint(this);
	if ( FileOffset.empty() )
		return ;
	QFontMetrics fm = paint.fontMetrics();

	int offset = 2 * fm.lineSpacing();
	// prevent the crash when user wants it smaller (by splitter)
	setMinimumHeight(offset + 20);

	double lineblocks = Total / (height() - offset - FileOffset.size() + 1);

	paint.fillRect(0, 0, width(), offset, palette().window());
	paint.drawText(0, 0, width(), offset, Qt::AlignLeft | Qt::AlignTop, tr("Files: %1").arg(FileOffset.size()));
	paint.drawText(0, 0, width(), offset, Qt::AlignRight | Qt::AlignTop, tr("Extents: %1").arg(Extents.size()));
	if (!Tablespace.isNull())
		paint.drawText(0, 0, width(), offset, Qt::AlignCenter | Qt::AlignTop, tr("Tablespace: %1").arg(Tablespace));
	paint.drawText(0, 0, width(), offset, Qt::AlignLeft | Qt::AlignBottom, tr("Blocks: %1").arg(Total));
	paint.drawText(0, 0, width(), offset, Qt::AlignRight | Qt::AlignBottom, tr("Blocks/line: %1").arg(int(lineblocks)));

	for (std::list<extent>::iterator i = Extents.begin(); i != Extents.end(); i++)
	{
		QColor col("#469446"); //= Qt::darkGreen;
		if (extentName(*i) == Highlight)
			col = Qt::red;
		int fileo = 0;
		for (std::map<int, int>::iterator j = FileOffset.begin(); j != FileOffset.end(); j++, fileo++)
			if ((*j).first == (*i).File)
				break;
		int block = FileOffset[(*i).File] + (*i).Block;

		int y1 = int(block / lineblocks);
		int x1 = int((block / lineblocks - y1) * width());
		block += (*i).Size;
		int y2 = int(block / lineblocks);
		int x2 = int((block / lineblocks - y2) * width());
		paint.setPen(col);
		if (y1 != y2)
		{
			paint.drawLine(x1, y1 + offset + fileo, width() - 1, y1 + offset + fileo);
			paint.drawLine(0, y2 + offset + fileo, x2 - 1, y2 + offset + fileo);
			if (y1 + 1 != y2)
				paint.fillRect(0, y1 + 1 + offset + fileo, width(), y2 - y1 - 1, col);
		}
		else
			paint.drawLine(x1, y1 + offset + fileo, x2, y2 + offset + fileo);
	}
	std::map<int, int>::iterator j = FileOffset.begin();
	j++;
	paint.setPen(Qt::black);
	int fileo = offset;
	while (j != FileOffset.end())
	{
		int block = (*j).second;
		int y1 = int(block / lineblocks);
		int x1 = int((block / lineblocks - y1) * width());
		paint.drawLine(x1, y1 + fileo, width() - 1, y1 + fileo);
		if (x1 != 0)
			paint.drawLine(0, y1 + 1 + fileo, x1 - 1, y1 + fileo + 1);
		j++;
		fileo++;
	}
}


std::list<toStorageExtent::extentTotal> toStorageExtent::objects(void)
{
	std::list<extentTotal> ret;

	for (std::list<extent>::iterator i = Extents.begin(); i != Extents.end(); i++)
	{
		bool dup = false;
		for (std::list<extentTotal>::iterator j = ret.begin(); j != ret.end(); j++)
		{
			if ((*j) == (*i))
			{
				(*j).Size += (*i).Size;
				(*j).Extents++;
				(*j).LastBlock = ( ((*j).LastBlock > (*i).Block) ? (*j).LastBlock : (*i).Block );
				dup = true;
				break;
			}
		}
		if (!dup)
			Utils::toPush(ret, extentTotal((*i).Owner, (*i).Table, (*i).Partition, (*i).Block, (*i).Size));
	}

	ret.sort();

	return ret;
}

bool toStorageExtent::fileView;
