
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

#ifndef TORESULTTABLEVIEWEDIT_H
#define TORESULTTABLEVIEWEDIT_H


#include "core/toresulttableview.h"
#include "core/toqvalue.h"
#include "core/tosql.h"
#include "core/toresult.h"
#include "core/toconnection.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractTableModel>
#include <QtGui/QTableView>
#include <QtCore/QModelIndex>
#include <QtCore/QList>
#include <QtGui/QHeaderView>
#include <QtGui/QMenu>

class toResultModel;
class toResultModelEdit;

/*! \brief Displays query result as a table and provides functionality to edit it.
  Used in Schema browser tool.
*/
class toResultTableViewEdit : public toResultTableView
{
    Q_OBJECT;
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
    virtual void query(const QString &sql, toQueryParams const& param)
    {
        query(sql, param, PriKeys);
    }

    virtual void query(const QString &, toQueryParams const& params, const QList<QString> priKeys);

    /**
     * Writes cached changes to database.
     *
     * @param status Should display a status message.
     * @return success
     */
    bool commitChanges(bool status = true);

    toResultModelEdit* editModel();
protected slots:
	// reimplemented
	virtual void slotHandleDoubleClick(const QModelIndex &);

private slots:
    /**
     * Handle connection toolbar's commit and rollback.
     *
     * @param conn Connection that is committed.
     * @param cmt true for commit, false for rollback
     */
    void commitChanges(toConnection &conn);
    void rollbackChanges(toConnection &conn);

    /**
     * Calls Model to add new record.
     */
    void addRecord(void);

    /**
     * Calls Model to add new record.
     */
    void duplicateRecord(void);

    /**
     * Calls Model to delete current record
     */
    void deleteRecord(void);
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
protected:
    /*
     * Helper function - allocate new instance of model
     * toResultTableView uses toResultModel
     * while
     * toResultTableViewEdit uses toResultModelEdit
     */
    virtual toResultModel* allocModel(toEventQuery *query);

    QString Owner, Table;

    QList<QString> PriKeys;
};


#endif
