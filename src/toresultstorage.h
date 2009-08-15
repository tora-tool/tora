
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#ifndef TORESULTSTORAGE_H
#define TORESULTSTORAGE_H

#include "config.h"
#include "tobackground.h"
#include "toresultview.h"

#include <QSplitter>
#include <QPaintEvent>

class toNoBlockQuery;

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
    virtual bool canHandle(toConnection &conn);
public slots:
    /** Perform a query.
     * @param sql Execute an SQL statement.
     * @param params Parameters needed as input to execute statement.
     */
    virtual void query(const QString &sql, const toQList &params);
};

class toResultStorage : public toResultView
{
    Q_OBJECT;

    bool ShowCoalesced;
    QString Unit;
    toBackground Poll;

    toNoBlockQuery *Tablespaces;
    toNoBlockQuery *Files;

    bool OnlyFiles;
    bool AvailableGraph;
    std::list<QString> TablespaceValues;
    std::list<QString> FileValues;

    QString CurrentSpace;
    QString CurrentFile;

    void saveSelected(void);
    void updateList(void);
public:
    toResultStorage(bool availableGraph, QWidget *parent, const char *name = NULL);
    ~toResultStorage();

    void showCoalesced(bool shw)
    {
        ShowCoalesced = shw;
    }

    void setOnlyFiles(bool only);
    bool onlyFiles(void) const
    {
        return OnlyFiles;
    }

    QString currentTablespace(void);
    QString currentFilename(void);
    /** Support Oracle
     */
    virtual bool canHandle(toConnection &conn);
public slots:
    virtual void query(void);
private slots:
    void poll(void);
};

#endif
