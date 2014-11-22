
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

#ifndef TORESULTTABLEDATA_H
#define TORESULTTABLEDATA_H

#include "core/toresultmodeledit.h"
#include "ui_toresulttabledataui.h"

#include <QWidget>
#include <QtCore/QMap>

class QAction;
class QCloseEvent;
class toResultDataSingle;
class toResultTableViewEdit;
//class toResultModel;
//class toResultModelEdit;

class toResultTableData
	: public QWidget
	, public Ui::toResultTableDataUI
	, public toResult
{
    Q_OBJECT;

public:
    toResultTableData(QWidget *parent = 0, const char *name = 0, toWFlags f = 0);

    /**
     * Reimplemented from toResult. First parameter is ignored.
     */
    virtual void query(const QString &, toQueryParams const& params);

    /** Clear result widget */
    virtual void clearData();

    /**
     * Reimplemented from toResult. Can handle all connections.
     */
    virtual bool canHandle(const toConnection &)
    {
        return true;
    }

    /**
     * Prompt to save if modified.
     */
    bool maybeSave(void);

signals:
    /**
     * Emitted when changes are saved. (used to call FLUSH PRIVILEGES - MySQL only?)
     */
    void changesSaved(void);

protected:
    virtual void closeEvent(QCloseEvent *);

protected slots:
    void navigate(QAction *);
    void changeFilter(bool);
    void removeFilter(bool);
    void setModel(toResultModel*);

public slots:
    void singleRecordForm(bool display);
    void updateForm(void);
    void save(void);
    void refreshWarn(void);

private:
    // generate filtername
    QString filterName();

    bool commitChanges();

    void commitUpdate(toConnectionSubLoan &conn, const toQuery::Row &row, unsigned int &updated);

    unsigned commitUpdate(toConnectionSubLoan &conn, toResultModelEdit::ChangeSet &change);
    unsigned commitAdd(toConnectionSubLoan &conn, toResultModelEdit::ChangeSet &change);
    unsigned commitDelete(toConnectionSubLoan &conn, toResultModelEdit::ChangeSet &change);

    toResultModelEdit* Model;

    // toolbar actions
    QAction *filterAct;
    QAction *removeAct;
    QAction *refreshAct;
    QAction *addAct;
    QAction *saveAct;
    QAction *discardAct;
    QAction *deleteAct;
    QAction *duplicateAct;
    QAction *firstAct;
    QAction *previousAct;
    QAction *nextAct;
    QAction *lastAct;
    QAction *singleAct;

    // Owner of table.
    QString Owner;

    // Table name
    QString Table;

    // SQL select statement
    QString SQL;

    // ---------------------------------------- filter settings

    // Use filter for all tables.
    bool AllFilter;

    // Filter selection criteria
    QMap<QString, QString> Criteria;

    // Filter retrieve order
    QMap<QString, QString> Order;

    // Current filter name in map
    QString FilterName;
};


#endif
