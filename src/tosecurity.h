/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TOSECURITY_H
#define TOSECURITY_H

#include "config.h"
#include "totool.h"
// due the toListView
#include "toresultview.h"
#include "ui_tosecurityquotaui.h"

#include <list>

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

class toSecurityObject : public toListView
{
    Q_OBJECT

public:
    toSecurityObject(QWidget *parent);
    void changeUser(const QString &);
    void sql(const QString &user, std::list<QString> &sql);
    void eraseUser(bool all = true);
    void update(void);
public slots:
    virtual void changed(toTreeWidgetItem *item);
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
    virtual void windowActivated(QMdiSubWindow *widget);
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
