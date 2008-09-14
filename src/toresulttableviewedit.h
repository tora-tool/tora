
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

#ifndef TORESULTTABLEVIEWEDIT_H
#define TORESULTTABLEVIEWEDIT_H

#include "config.h"
#include "toresulttableview.h"
#include "toqvalue.h"
#include "tosql.h"
#include "toresult.h"
#include "toconnection.h"
#include "toresultmodel.h"

#include <QObject>
#include <QAbstractTableModel>
#include <QTableView>
#include <QModelIndex>
#include <QList>
#include <QHeaderView>
#include <QMenu>


class toResultTableViewEdit : public toResultTableView
{
    Q_OBJECT;

    enum ChangeKind
    {
        Add,
        Delete,
        Update
    };

    struct ChangeSet
    {
        ChangeKind         kind;         /* sql change mode */
        QString            columnName;   /* column name */
        int                column;       /* the real column number
                                          * after adjusting for
                                          * numbercolumn */
        toQValue           newValue;     /* data after the change */
        toResultModel::Row row;          /* data before the change */
    };

    // keep a history of changes to commit.
    // this is a fifo -- don't sort or insert. just append.
    QList<struct ChangeSet> Changes;

    QString Owner;
    QString Table;

    void commitDelete(ChangeSet &change, toConnection &conn);
    void commitAdd(ChangeSet &change, toConnection &conn);
    void commitUpdate(ChangeSet &change, toConnection &conn);

public:
    /**
     * Creates a new tableview for editing data
     *
     * @param readable use readable headers
     * @param numberColumn display vertical header
     * @param parent qobject parent
     * @param name sets objectName property
     */
    toResultTableViewEdit(bool readable,
                          bool numberColumn,
                          QWidget *parent,
                          const char *name = 0);
    virtual ~toResultTableViewEdit(void);


    /**
     * Reimplemented to create query and new model.
     *
     */
    virtual void query(const QString &, const toQList &params);


    /**
     * Reimplemented to create query and new model.
     *
     */
    void query(const QString &sql)
    {
        toQList p;
        query(sql, p);
    }


    /**
     * True if data has been modified.
     *
     */
    bool changed(void)
    {
        return Changes.size() > 0;
    }


signals:
    /**
     * Parameter is true after changes, false after save or load.
     *
     */
    void changed(bool edit);


public slots:
    /**
     * Append change to Changes
     *
     */
    void recordChange(const QModelIndex &,
                      const toQValue &,
                      const toResultModel::Row &);


    /**
     * Append a new row to Changes
     *
     */
    void recordAdd(const toResultModel::Row &);


    /**
     * Record a deletion in Changes
     *
     */
    void recordDelete(const toResultModel::Row &);


    /**
     * Calls Model to add new record.
     *
     */
    void addRecord(void);


    /**
     * Calls Model to add new record.
     *
     */
    void duplicateRecord(void);


    /**
     * Calls Model to delete current record
     *
     */
    void deleteRecord(void);


    /**
     * Writes cached changes to database.
     *
     * @param status Should display a status message.
     * @return success
     */
    bool commitChanges(bool status = true);


    /**
     * Handle connection toolbar's commit and rollback.
     *
     * @param conn Connection that is commited.
     * @param cmt true for commit, false for rollback
     */
    void commitChanges(toConnection &conn, bool cmt);


    /**
     * Clears Changes
     *
     */
    void revertChanges(void);


    /**
     * Handle signal from model when rows are added
     *
     */
    void handleNewRows(const QModelIndex &parent, int start, int end);
};


#endif
