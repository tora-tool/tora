
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

#ifndef TOSECURITY_H
#define TOSECURITY_H

#include "widgets/totoolwidget.h"
// due the toListView
#include "ui_tosecurityquotaui.h"
#include "tools/tosecuritytreemodel.h"

#include <list>
#include "toresultview.h"

class QMenu;
class QTabWidget;
class QToolButton;
class toConnection;
// class toListView;
class toSecurityPage;
class toSecurityQuota;



class toSecuritySystem : public toListView
{
        Q_OBJECT

    public:
        toSecuritySystem(QWidget *parent);
        void changeUser(const QString &);
        void eraseUser(bool all = true);
        void sql(const QString &user, std::list<QString> &sql);
        void update(void);
    public slots:
        virtual void changed(toTreeWidgetItem *item);
};

class toSecurityRoleGrant : public toListView
{
        Q_OBJECT

        toTreeWidgetCheck *findChild(toTreeWidgetItem *parent, const QString &name);
    public:
        toSecurityRoleGrant(QWidget *parent);
        void changeUser(bool user, const QString &);
        void sql(const QString &user, std::list<QString> &sql);
        void eraseUser(bool user, bool all = true);
        void update(void);
    public slots:
        virtual void changed(toTreeWidgetItem *item);
};

class toSecurityObject : public QTreeView
{
        Q_OBJECT

    public:
        toSecurityObject(QWidget *parent);
        void changeUser(const QString &);
        void sql(const QString &user, std::list<QString> &sql);
        void eraseUser(bool all = true);
        void update(void);
    private:
        toSecurityTreeModel * m_model;
};

class toSecurity : public toToolWidget
{
        Q_OBJECT

        struct privilege
        {
            QString Owner;
            QString Object;
            QString Access;
            QString Value;
            bool Admin;
            privilege(const QString &owner, const QString &object, const QString &access,
                      const QString &value, bool admin)
                : Owner(owner), Object(object), Access(access), Value(value), Admin(admin)
            { }
        };

        QString UserID;

        QMenu               *ToolMenu;
        toListView          *UserList;
        toSecuritySystem    *SystemGrant;
        toSecurityRoleGrant *RoleGrant;
        toSecurityObject    *ObjectGrant;
        toSecurityPage      *General;
        toSecurityQuota     *Quota;

        QAction *DropAct;
        QAction *CopyAct;
        QAction *UpdateListAct;
        QAction *SaveAct;
        QAction *AddUserAct;
        QAction *AddRoleAct;
        QAction *DisplaySQLAct;

        QTabWidget *Tabs;
        std::list<QString> sql(void);
    public:
        toSecurity(QWidget *parent, toConnection &connection);
        virtual void changeUser(bool);

    public slots:
        virtual void refresh(void);
        virtual void changeUser(toTreeWidgetItem *)
        {
            changeUser(true);
        }
        virtual void saveChanges(void);
        virtual void addUser(void);
        virtual void addRole(void);
        virtual void drop(void);
        virtual void copy(void);
        virtual void displaySQL(void);
        virtual void slotWindowActivated(toToolWidget *widget);
};


class toSecurityQuota : public QWidget, public Ui::toSecurityQuotaUI
{
        Q_OBJECT

    private:
        toTreeWidgetItem *CurrentItem;
        void clearItem(toTreeWidgetItem *item);
    public:
        toSecurityQuota(QWidget *parent);
        void changeUser(const QString &);
        QString sql(void);
        void clear(void);
        void update(void);
    private slots:
        void changeTablespace(void);
        void changeSize(void);
};

#endif
