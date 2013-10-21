
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

#ifndef TOROLLBACK_H
#define TOROLLBACK_H

#include "core/totool.h"
#include "core/totreewidget.h"
#include "core/toconnection.h"

#include "ui_torollbackdialogui.h"

#include <QtGui/QDialog>

#include <algorithm>

class QComboBox;
class QMenu;
class toResultItem;
class toResultView;
class toSGAStatement;
class toStorageDefinition;
class toResultTableView;


class toRollbackDialog : public QDialog, public Ui::toRollbackDialogUI
{
    Q_OBJECT;

    toStorageDefinition *Storage;

public:
    toRollbackDialog(toConnection &conn, QWidget *parent = 0, const char *name = 0);

    std::list<QString> sql(void);

public slots:
    void valueChanged(const QString &str);
    void displaySQL(void);
};

class toRollback : public toToolWidget
{
    Q_OBJECT;

    toResultTableView *Segments;
    toResultView *Statements;

    QWidget *CurrentTab;

    toSGAStatement *CurrentStatement;
    toResultTableView *TransactionUsers;

    QAction * UpdateSegmentsAct;
    QAction * enableOldAct;
    QAction * OnlineAct;
    QAction * OfflineAct;
    QAction * NewAct;
    QAction * DropAct;

    QMenu *ToolMenu;
    QComboBox *Refresh;

    QString currentSegment(void);

public:
    toRollback(QWidget *parent, toConnection &connection);

private slots:
    void changeStatement(toTreeWidgetItem *item);
    void changeItem();
    void changeRefresh(const QString &str);

    void enableOld(bool);
    void refresh(void);
    void online(void);
    void offline(void);
    void addSegment(void);
    void dropSegment(void);
    virtual void slotWindowActivated(toToolWidget *widget);
    //! Change (UNIT) string to appropriate Bytes/MB... value
    void updateHeaders(const QString &sql, const toConnection::exception &res, bool error);
};

#endif
