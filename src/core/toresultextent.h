
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

#ifndef __TORESULT_EXTENT_H__
#define __TORESULT_EXTENT_H__

#include "core/toresult.h"

#include <QSplitter>

class toResultTableView;
class toStorageExtent;
class toResultTableView;

class toResultExtent : public QSplitter, public toResult
{
	Q_OBJECT;

	toStorageExtent   *Graph;
	toResultTableView *List;
public:
	toResultExtent(QWidget *parent, const char *name = NULL);

	/** Support Oracle
	 */
	virtual bool canHandle(const toConnection &conn);
public slots:
	/** Perform a query.
	 * @param sql Execute an SQL statement.
	 * @param params Parameters needed as input to execute statement.
	 */
	virtual void query(const QString &sql, toQueryParams const& params);

	/** Clear result widget */
	virtual void clearData();
};

class toStorageExtent : public QWidget
{
	Q_OBJECT;

public:
	struct extentName
	{
		QString Owner;
		QString Table;
		QString Partition;
		int Size;

		extentName(void)
		{
			Size = 0;
		}
		extentName(const QString &owner, const QString &table, const QString &partition, int size);
		bool operator == (const extentName &) const;
	};

	struct extentTotal : public extentName
	{
		int Extents;
		int LastBlock;
		extentTotal(const QString &owner, const QString &table, const QString &partition, int block, int size)
			: extentName(owner, table, partition, size)
		{
			Extents = 1;
			LastBlock = block;
		}
		bool operator < (const extentTotal &) const;
	};

	struct extent : public extentName
	{
		int File;
		int Block;

		extent(void)
		{
			File = Block = 0;
		}
		extent(const QString &owner, const QString &table, const QString &partition,
		       int file, int block, int size);
		bool operator < (const extent &) const;
		bool operator == (const extent &) const;
	};

private:
	std::list<extent> Extents;
	extentName Highlight;
	QString Tablespace;

	std::map<int, int> FileOffset;
	int Total;
	static bool fileView;
public:
	toStorageExtent(QWidget *parent, const char *name = NULL);
	void highlight(const QString &owner, const QString &table, const QString &partition);

	void setTablespace(const QString &tablespace);
	void setFile(const QString &tablespace, int file);

	std::list<extentTotal> objects(void);
protected:
	virtual void paintEvent(QPaintEvent *);
}; // toStorageExtent

#endif
