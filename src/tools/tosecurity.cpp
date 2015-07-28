
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

#include "tools/tosecurity.h"
#include "ui_tosecurityuserui.h"
#include "ui_tosecurityroleui.h"

#include "core/utils.h"
#include "core/tochangeconnection.h"
#include "core/tomemoeditor.h"
#include "core/toconnectionsub.h"
#include "core/toglobalevent.h"

#include <QSplitter>
#include <QToolBar>

#include "icons/addrole.xpm"
#include "icons/adduser.xpm"
#include "icons/commit.xpm"
#include "icons/copyuser.xpm"
#include "icons/refresh.xpm"
#include "icons/sql.xpm"
#include "icons/tosecurity.xpm"
#include "icons/trash.xpm"

static toSQL SQLUserInfo("toSecurity:UserInfo",
                         "SELECT Account_Status,\n"
                         "       Password,\n"
                         "       External_Name,\n"
                         "       Profile,\n"
                         "       Default_Tablespace,\n"
                         "       Temporary_Tablespace\n"
                         "  FROM sys.DBA_Users\n"
                         " WHERE UserName = :f1<char[100]>",
                         "Get information about a user, must have same columns and same binds.");

static toSQL SQLUserInfo7("toSecurity:UserInfo",
                          "SELECT 'OPEN',\n"
                          "       Password,\n"
                          "       NULL,\n"
                          "       Profile,\n"
                          "       Default_Tablespace,\n"
                          "       Temporary_Tablespace\n"
                          "  FROM sys.DBA_Users\n"
                          " WHERE UserName = :f1<char[100]>",
                          "",
                          "0703");

static toSQL SQLRoleInfo("toSecurity:RoleInfo",
                         "SELECT Role,Password_required FROM sys.DBA_Roles WHERE Role = :f1<char[101]>",
                         "Get information about a role, must have same columns and same binds.");

static toSQL SQLProfiles("toSecurity:Profiles",
                         "SELECT DISTINCT Profile FROM sys.DBA_Profiles ORDER BY Profile",
                         "Get profiles available.");

static toSQL SQLTablespace("toSecurity:Tablespaces",
                           "SELECT DISTINCT Tablespace_Name FROM sys.DBA_Tablespaces\n"
                           " WHERE contents = :f1<char[30]>\n"
                           " ORDER BY Tablespace_Name",
                           "Get tablespaces available.");

static toSQL SQLRoles("toSecurity:Roles",
                      "SELECT Role FROM sys.Dba_Roles ORDER BY Role",
                      "Get roles available in DB, should return one entry");

static toSQL SQLListSystem("toSecurity:ListSystemPrivs",
                           "SELECT a.name\n"
                           "  FROM system_privilege_map a,\n"
                           "       v$enabledprivs b\n"
                           " WHERE b.priv_number = a.privilege\n"
                           " ORDER BY a.name",
                           "Get name of available system privileges");

static toSQL SQLQuota("toSecurity:Quota",
                      "SELECT Tablespace_name,\n"
                      "       Bytes,\n"
                      "       Max_bytes\n"
                      "  FROM sys.DBA_TS_Quotas\n"
                      " WHERE Username = :f1<char[200]>\n"
                      " ORDER BY Tablespace_name",
                      "Get information about what quotas the user has, "
                      "must have same columns and same binds.");

static toSQL SQLSystemGrant("toSecurity:SystemGrant",
                            "SELECT privilege, NVL(admin_option,'NO') FROM sys.dba_sys_privs WHERE grantee = :f1<char[100]>",
                            "Get information about the system privileges a user has, should have same bindings and columns");

static toSQL SQLRoleGrant("toSecurity:RoleGrant",
                          "SELECT granted_role,\n"
                          "       admin_option,\n"
                          "       default_role\n"
                          "  FROM sys.dba_role_privs\n"
                          " WHERE grantee = :f1<char[100]>",
                          "Get the roles granted to a user or role, "
                          "must have same columns and binds");

class toSecurityTool : public toTool
{
    protected:
        virtual const char **pictureXPM(void)
        {
            return const_cast<const char**>(tosecurity_xpm);
        }
    public:
        toSecurityTool()
            : toTool(40, "Security Manager")
        { }
        virtual const char *menuItem()
        {
            return "Security Manager";
        }
        virtual toToolWidget* toolWindow(QWidget *parent, toConnection &connection)
        {
            return new toSecurity(parent, connection);
        }
        virtual void closeWindow(toConnection &connection) {};
};

static toSecurityTool SecurityTool;


void toSecurityQuota::changeSize(void)
{
    if (CurrentItem)
    {
        if (Value->isChecked())
        {
            QString siz;
            siz.sprintf("%.0f KB", double(Size->value()));
            CurrentItem->setText(1, siz);
        }
        else if (None->isChecked())
        {
            CurrentItem->setText(1, qApp->translate("toSecurityQuota", "None"));
        }
        else if (Unlimited->isChecked())
        {
            CurrentItem->setText(1, qApp->translate("toSecurityQuota", "Unlimited"));
        }
    }
    else
        SizeGroup->setEnabled(false);
}

toSecurityQuota::toSecurityQuota(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    CurrentItem = NULL;
    update();

    QButtonGroup *group = new QButtonGroup(SizeGroup);
    group->addButton(Value, 1);
    group->addButton(None, 2);
    group->addButton(Unlimited, 3);

    connect(Tablespaces, SIGNAL(selectionChanged()),
            this, SLOT(changeTablespace()));
    connect(group, SIGNAL(buttonClicked(int)),
            this, SLOT(changeSize()));
    connect(Size, SIGNAL(valueChanged()),
            this, SLOT(changeSize()));
    connect(Value, SIGNAL(toggled(bool)),
            Size, SLOT(setEnabled(bool)));
}

void toSecurityQuota::update(void)
{
    Tablespaces->clear();
    try
    {
        toConnectionSubLoan conn(toConnection::currentConnection(this));
        toQuery tablespaces(conn, SQLTablespace, toQueryParams() << QString::fromLatin1("PERMANENT"));
        toTreeWidgetItem *item = NULL;
        while (!tablespaces.eof())
        {
            item = new toResultViewItem(Tablespaces, item, (QString)tablespaces.readValue());
            item->setText(1, qApp->translate("toSecurityQuota", "None"));
            item->setText(3, qApp->translate("toSecurityQuota", "None"));
        }
    }
    TOCATCH
}

void toSecurityQuota::clearItem(toTreeWidgetItem *item)
{
    item->setText(1, qApp->translate("toSecurityQuota", "None"));
    item->setText(2, QString::null);
    item->setText(3, qApp->translate("toSecurityQuota", "None"));
}

void toSecurityQuota::clear(void)
{
    for (toTreeWidgetItem *item = Tablespaces->firstChild(); item; item = item->nextSibling())
        item->setText(3, qApp->translate("toSecurityQuota", "None"));
}

void toSecurityQuota::changeUser(const QString &user)
{
    Tablespaces->show();
    SizeGroup->show();
    Disabled->hide(); // Do we really have to bother about this?

    Tablespaces->clearSelection();
    toTreeWidgetItem *item = Tablespaces->firstChild();
    if (!user.isEmpty())
    {
        try
        {
            toConnectionSubLoan conn(toConnection::currentConnection(this));
            toQuery quota(conn, SQLQuota, toQueryParams() << user);
            while (!quota.eof())
            {
                double maxQuota;
                double usedQuota;
                QString tbl(quota.readValue());
                while (item && item->text(0) != tbl)
                {
                    clearItem(item);
                    item = item->nextSibling();
                }
                usedQuota = quota.readValue().toDouble();
                maxQuota = quota.readValue().toDouble();
                if (item)
                {
                    QString usedStr;
                    QString maxStr;
                    usedStr.sprintf("%.0f KB", usedQuota / 1024);
                    if (maxQuota < 0)
                        maxStr = qApp->translate("toSecurityQuota", "Unlimited");
                    else if (maxQuota == 0)
                        maxStr = qApp->translate("toSecurityQuota", "None");
                    else
                    {
                        maxStr.sprintf("%.0f KB", maxQuota / 1024);
                    }
                    item->setText(1, maxStr);
                    item->setText(2, usedStr);
                    item->setText(3, maxStr);
                    item = item->nextSibling();
                }
            }
        }
        TOCATCH
    }
    while (item)
    {
        clearItem(item);
        item = item->nextSibling();
    }
    SizeGroup->setEnabled(false);
    CurrentItem = NULL;
}

void toSecurityQuota::changeTablespace(void)
{
    CurrentItem = Tablespaces->selectedItem();
    if (CurrentItem)
    {
        QString siz = CurrentItem->text(1);
        if (siz == qApp->translate("toSecurityQuota", "None"))
            None->setChecked(true);
        else if (siz == qApp->translate("toSecurityQuota", "Unlimited"))
            Unlimited->setChecked(true);
        else
        {
            Value->setChecked(true);
            Size->setValue(siz.toInt());
        }
    }
    SizeGroup->setEnabled(true);
}

QString toSecurityQuota::sql(void)
{
    QString ret;
    for (toTreeWidgetItem *item = Tablespaces->firstChild(); item; item = item->nextSibling())
    {
        if (item->text(1) != item->text(3))
        {
            QString siz = item->text(1);
            if (siz.right(2) == QString::fromLatin1("KB"))
                siz.truncate(siz.length() - 1);
            else if (siz == qApp->translate("toSecurityQuota", "None"))
                siz = QString::fromLatin1("0 K");
            else if (siz == qApp->translate("toSecurityQuota", "Unlimited"))
                siz = QString::fromLatin1("UNLIMITED");
            ret += QString::fromLatin1(" QUOTA ");
            ret += siz;
            ret += QString::fromLatin1(" ON ");
            ret += item->text(0);
        }
    }
    return ret;
}

class toSecurityUpper : public QValidator
{
    public:
        toSecurityUpper(QWidget *parent)
            : QValidator(parent)
        { }
        virtual State validate(QString &str, int &) const
        {
            str = str.toUpper();
            return Acceptable;
        }
};

class toSecurityUser : public QWidget, public Ui::toSecurityUserUI
{
        toConnection &Connection;

        toSecurityQuota *Quota;
        enum
        {
            password,
            global,
            external
        } AuthType;
        QString OrgProfile;
        QString OrgDefault;
        QString OrgTemp;
        QString OrgGlobal;
        QString OrgPassword;

        bool OrgLocked;
        bool OrgExpired;
    public:
        toSecurityUser(toSecurityQuota *quota, toConnection &conn, QWidget *parent);
        void clear(bool all = true);
        void update();
        void changeUser(const QString &);
        QString name(void)
        {
            return Name->text();
        }
        QString sql(void);
};

QString toSecurityUser::sql(void)
{
    QString extra;
    if (Authentication->currentWidget() == PasswordTab)
    {
        if (Password->text() != Password2->text())
        {
            switch (TOMessageBox::warning(this,
                                          qApp->translate("toSecurityUser", "Passwords don't match"),
                                          qApp->translate("toSecurityUser", "The two versions of the password doesn't match"),
                                          qApp->translate("toSecurityUser", "Don't save"),
                                          qApp->translate("toSecurityUser", "Cancel")))
            {
                case 0:
                    return QString::null;
                case 1:
                    throw qApp->translate("toSecurityUser", "Passwords don't match");
            }
        }
        if (Password->text() != OrgPassword)
        {
            extra = QString::fromLatin1(" IDENTIFIED BY \"");
            extra += Password->text();
            extra += QString::fromLatin1("\"");
        }
        if (OrgExpired != ExpirePassword->isChecked())
        {
            if (ExpirePassword->isChecked())
                extra += QString::fromLatin1(" PASSWORD EXPIRE");
        }
    }
    else if (Authentication->currentWidget() == GlobalTab)
    {
        if (OrgGlobal != GlobalName->text())
        {
            extra = QString::fromLatin1(" IDENTIFIED GLOBALLY AS '");
            extra += GlobalName->text();
            extra += QString::fromLatin1("'");
        }
    }
    else if ((AuthType != external) && (Authentication->currentWidget() == ExternalTab))
        extra = QString::fromLatin1(" IDENTIFIED EXTERNALLY");

    if (OrgProfile != Profile->currentText())
    {
        extra += QString::fromLatin1(" PROFILE \"");
        extra += Profile->currentText();
        extra += QString::fromLatin1("\"");
    }
    if (OrgDefault != DefaultSpace->currentText())
    {
        extra += QString::fromLatin1(" DEFAULT TABLESPACE \"");
        extra += DefaultSpace->currentText();
        extra += QString::fromLatin1("\"");
    }
    if (OrgTemp != TempSpace->currentText())
    {
        extra += QString::fromLatin1(" TEMPORARY TABLESPACE \"");
        extra += TempSpace->currentText();
        extra += QString::fromLatin1("\"");
    }
    if (OrgLocked != Locked->isChecked())
    {
        extra += QString::fromLatin1(" ACCOUNT ");
        if (Locked->isChecked())
            extra += QString::fromLatin1("LOCK");
        else
            extra += QString::fromLatin1("UNLOCK");
    }
    extra += Quota->sql();

    QString sql;
    if (Name->isEnabled())
    {
        if (Name->text().isEmpty())
            return QString::null;
        sql = QString::fromLatin1("CREATE ");
    }
    else
    {
        if (extra.isEmpty())
            return QString::null;
        sql = QString::fromLatin1("ALTER ");
    }
    sql += QString::fromLatin1("USER \"");
    sql += Name->text();
    sql += QString::fromLatin1("\"");
    sql += extra;
    return sql;
}

toSecurityUser::toSecurityUser(toSecurityQuota *quota, toConnection &conn, QWidget *parent)
    : QWidget(parent), Connection(conn), Quota(quota)
{
    setupUi(this);
    Name->setValidator(new toSecurityUpper(Name));
    setFocusProxy(Name);
    update();
}

void toSecurityUser::update()
{
    try
    {
        toConnectionSubLoan conn(Connection);
        toQuery profiles(conn, SQLProfiles, toQueryParams());
        while (!profiles.eof())
            Profile->addItem((QString)profiles.readValue());

        QString buf;
        toQuery tablespaces(conn, SQLTablespace, toQueryParams() << QString::fromLatin1("PERMANENT"));
        DefaultSpace->clear();
        while (!tablespaces.eof())
        {
            buf = (QString)tablespaces.readValue();
            DefaultSpace->addItem(buf);
        }

        toQuery temp(conn, SQLTablespace, toQueryParams() << QString::fromLatin1("TEMPORARY"));
        TempSpace->clear();
        while (!temp.eof())
        {
            buf = (QString)temp.readValue();
            TempSpace->addItem(buf);
        }
    }
    TOCATCH
}

void toSecurityUser::clear(bool all)
{
    Name->setText(QString::null);
    Password->setText(QString::null);
    Password2->setText(QString::null);
    GlobalName->setText(QString::null);
    if (all)
    {
        Profile->setCurrentIndex(0);
        Authentication->setCurrentIndex(Authentication->indexOf(PasswordTab));
        ExpirePassword->setChecked(false);
        ExpirePassword->setEnabled(true);
        TempSpace->setCurrentIndex(0);
        DefaultSpace->setCurrentIndex(0);
        Locked->setChecked(false);
    }

    OrgProfile = OrgDefault = OrgTemp = OrgGlobal = QString::null;
    AuthType = password;
    Name->setEnabled(true);
    OrgLocked = OrgExpired = false;
}

void toSecurityUser::changeUser(const QString &user)
{
    clear();
    try
    {
        toConnectionSubLoan conn(Connection);
        toQuery query(conn, SQLUserInfo, toQueryParams() << user);
        if (!query.eof())
        {
            Name->setEnabled(false);
            Name->setText(user);

            QString str(query.readValue());
            if (str.contains(QString::fromLatin1("EXPIRED")))
            {
                ExpirePassword->setChecked(true);
                ExpirePassword->setEnabled(false);
                OrgExpired = true;
            }
            if (str.contains(QString::fromLatin1("LOCKED")))
            {
                Locked->setChecked(true);
                OrgLocked = true;
            }

            OrgPassword = (QString)query.readValue();
            QString pass = (QString)query.readValue();
            if (OrgPassword == QString::fromLatin1("GLOBAL"))
            {
                OrgPassword = QString::null;
                Authentication->setCurrentIndex(Authentication->indexOf(GlobalTab));
                OrgGlobal = pass;
                GlobalName->setText(OrgGlobal);
                AuthType = global;
            }
            else if (OrgPassword == QString::fromLatin1("EXTERNAL"))
            {
                OrgPassword = QString::null;
                Authentication->setCurrentIndex(Authentication->indexOf(ExternalTab));
                AuthType = external;
            }
            else
            {
                Password->setText(OrgPassword);
                Password2->setText(OrgPassword);
                AuthType = password;
            }

            {
                str = (QString)query.readValue();
                for (int i = 0; i < Profile->count(); i++)
                {
                    if (Profile->itemText(i) == str)
                    {
                        Profile->setCurrentIndex(i);
                        OrgProfile = str;
                        break;
                    }
                }
            }

            {
                str = (QString)query.readValue();
                for (int i = 0; i < DefaultSpace->count(); i++)
                {
                    if (DefaultSpace->itemText(i) == str)
                    {
                        DefaultSpace->setCurrentIndex(i);
                        OrgDefault = str;
                        break;
                    }
                }
            }

            {
                str = (QString)query.readValue();
                for (int i = 0; i < TempSpace->count(); i++)
                {
                    if (TempSpace->itemText(i) == str)
                    {
                        TempSpace->setCurrentIndex(i);
                        OrgTemp = str;
                        break;
                    }
                }
            }
        }
    }
    TOCATCH
}

class toSecurityRole : public QWidget, public Ui::toSecurityRoleUI
{
        toConnection &Connection;
        toSecurityQuota *Quota;
        enum
        {
            password,
            global,
            external,
            none
        } AuthType;
    public:
        toSecurityRole(toSecurityQuota *quota, toConnection &conn, QWidget *parent)
            : QWidget(parent), Connection(conn)
            , Quota(quota)
            , AuthType(password)
        {
            setupUi(this);
            Name->setValidator(new toSecurityUpper(Name));
            setFocusProxy(Name);
        }
        void clear(void);
        void changeRole(const QString &);
        QString sql(void);
        QString name(void)
        {
            return Name->text();
        }
};

QString toSecurityRole::sql(void)
{
    QString extra;
    if (Authentication->currentWidget() == PasswordTab)
    {
        if (Password->text() != Password2->text())
        {
            switch (TOMessageBox::warning(this,
                                          qApp->translate("toSecurityRole", "Passwords don't match"),
                                          qApp->translate("toSecurityRole", "The two versions of the password doesn't match"),
                                          qApp->translate("toSecurityRole", "Don't save"),
                                          qApp->translate("toSecurityRole", "Cancel")))
            {
                case 0:
                    return QString::null;
                case 1:
                    throw qApp->translate("toSecurityRole", "Passwords don't match");
            }
        }
        if (Password->text().length() > 0)
        {
            extra = QString::fromLatin1(" IDENTIFIED BY \"");
            extra += Password->text();
            extra += QString::fromLatin1("\"");
        }
    }
    else if ((AuthType != global) && (Authentication->currentWidget() == GlobalTab))
        extra = QString::fromLatin1(" IDENTIFIED GLOBALLY");
    else if ((AuthType != external) && (Authentication->currentWidget() == ExternalTab))
        extra = QString::fromLatin1(" IDENTIFIED EXTERNALLY");
    else if ((AuthType != none) && (Authentication->currentWidget() == NoneTab))
        extra = QString::fromLatin1(" NOT IDENTIFIED");
    extra += Quota->sql();
    QString sql;
    if (Name->isEnabled())
    {
        if (Name->text().isEmpty())
            return QString::null;
        sql = QString::fromLatin1("CREATE ");
    }
    else
    {
        if (extra.isEmpty())
            return QString::null;
        sql = QString::fromLatin1("ALTER ");
    }
    sql += QString::fromLatin1("ROLE \"");
    sql += Name->text();
    sql += QString::fromLatin1("\"");
    sql += extra;
    return sql;
}

void toSecurityRole::clear(void)
{
    Name->setText(QString::null);
    Name->setEnabled(true);
}

void toSecurityRole::changeRole(const QString &role)
{
    try
    {
        toConnectionSubLoan conn(Connection);
        toQuery query(conn, SQLRoleInfo, toQueryParams() << role);
        Password->setText(QString::null);
        Password2->setText(QString::null);
        if (!query.eof())
        {
            Name->setText(role);
            Name->setEnabled(false);

            QString str(query.readValue());
            if (str == QString::fromLatin1("YES"))
            {
                AuthType = password;
                Authentication->setCurrentIndex(Authentication->indexOf(PasswordTab));
            }
            else if (str == QString::fromLatin1("GLOBAL"))
            {
                AuthType = global;
                Authentication->setCurrentIndex(Authentication->indexOf(GlobalTab));
            }
            else if (str == QString::fromLatin1("EXTERNAL"))
            {
                AuthType = external;
                Authentication->setCurrentIndex(Authentication->indexOf(ExternalTab));
            }
            else
            {
                AuthType = none;
                Authentication->setCurrentIndex(Authentication->indexOf(NoneTab));
            }
        }
        else
        {
            Name->setText(QString::null);
            Name->setEnabled(true);
            AuthType = none;
            Authentication->setCurrentIndex(Authentication->indexOf(NoneTab));
        }
    }
    TOCATCH
}

class toSecurityPage : public QWidget
{
        toSecurityRole *Role;
        toSecurityUser *User;

    public:
        toSecurityPage(toSecurityQuota *quota, toConnection &conn, QWidget *parent)
            : QWidget(parent)
        {
            QVBoxLayout *vbox = new QVBoxLayout;
            vbox->setSpacing(0);
            vbox->setContentsMargins(0, 0, 0, 0);
            setLayout(vbox);

            Role = new toSecurityRole(quota, conn, this);
            vbox->addWidget(Role);
            Role->hide();
            User = new toSecurityUser(quota, conn, this);
            vbox->addWidget(User);
            setFocusProxy(User);
        }
        void changePage(const QString &nam, bool user)
        {
            if (user)
            {
                Role->hide();
                User->show();
                User->changeUser(nam);
                setFocusProxy(User);
            }
            else
            {
                User->hide();
                Role->show();
                Role->changeRole(nam);
                setFocusProxy(Role);
            }
        }
        QString name(void)
        {
            if (User->isHidden())
                return Role->name();
            else
                return User->name();
        }
        void clear(void)
        {
            if (User->isHidden())
                Role->clear();
            else
                User->clear(false);
        }
        bool user(void)
        {
            if (User->isHidden())
                return false;
            return true;
        }
        QString sql(void)
        {
            if (User->isHidden())
                return Role->sql();
            else
                return User->sql();
        }
        void update()
        {
            User->update();
            Role->update();
        }
};

toSecurityObject::toSecurityObject(QWidget *parent)
    : QTreeView(parent)
{
    setObjectName("toSecurityObject");

    m_model = new toSecurityTreeModel(this);
    setModel(m_model);
//     update();
}


void toSecurityObject::update(void)
{
    m_model->setupModelData(toConnection::currentConnection(this).user());
}

void toSecurityObject::eraseUser(bool all)
{
    m_model->setupModelData(toConnection::currentConnection(this).user());
}

void toSecurityObject::changeUser(const QString &user)
{
    m_model->setupModelData(user);
}

void toSecurityObject::sql(const QString &user, std::list<QString> &sqlLst)
{
    m_model->sql(user, sqlLst);
}

toSecuritySystem::toSecuritySystem(QWidget *parent)
    : toListView(parent)
{
    addColumn(tr("Privilege name"));
    setRootIsDecorated(true);
    update();
    setSorting(0);
    connect(this, SIGNAL(clicked(toTreeWidgetItem *)), this, SLOT(changed(toTreeWidgetItem *)));
}

void toSecuritySystem::update(void)
{
    clear();
    try
    {
        toConnectionSubLoan conn(toConnection::currentConnection(this));
        toQuery priv(conn, SQLListSystem, toQueryParams());
        while (!priv.eof())
        {
            toResultViewCheck *item = new toResultViewCheck(this, (QString)priv.readValue(),
                    toTreeWidgetCheck::CheckBox);
            new toResultViewCheck(item, tr("Admin"), toTreeWidgetCheck::CheckBox);
        }
    }
    TOCATCH
}

void toSecuritySystem::sql(const QString &user, std::list<QString> &sqlLst)
{
    for (toTreeWidgetItem *item = firstChild(); item; item = item->nextSibling())
    {
        QString sql;
        toResultViewCheck *check = dynamic_cast<toResultViewCheck *>(item);
        toResultViewCheck *chld = dynamic_cast<toResultViewCheck *>(item->firstChild());
        if (chld && chld->isOn() && chld->text(1).isEmpty())
        {
            sql = QString::fromLatin1("GRANT ");
            sql += item->text(0);
            sql += QString::fromLatin1(" TO \"");
            sql += user;
            sql += QString::fromLatin1("\" WITH ADMIN OPTION");
            sqlLst.insert(sqlLst.end(), sql);
        }
        else if (check->isOn() && !item->text(1).isEmpty())
        {
            if (chld && !chld->isOn() && !chld->text(1).isEmpty())
            {
                sql = QString::fromLatin1("REVOKE ");
                sql += item->text(0);
                sql += QString::fromLatin1(" FROM \"");
                sql += user;
                sql += QString::fromLatin1("\"");
                sqlLst.insert(sqlLst.end(), sql);

                sql = QString::fromLatin1("GRANT ");
                sql += item->text(0);
                sql += QString::fromLatin1(" TO \"");
                sql += user;
                sql += QString::fromLatin1("\"");
                sqlLst.insert(sqlLst.end(), sql);
            }
        }
        else if (check->isOn() && item->text(1).isEmpty())
        {
            sql = QString::fromLatin1("GRANT ");
            sql += item->text(0);
            sql += QString::fromLatin1(" TO \"");
            sql += user;
            sql += QString::fromLatin1("\"");
            sqlLst.insert(sqlLst.end(), sql);
        }
        else if (!check->isOn() && !item->text(1).isEmpty())
        {
            sql = QString::fromLatin1("REVOKE ");
            sql += item->text(0);
            sql += QString::fromLatin1(" FROM \"");
            sql += user;
            sql += QString::fromLatin1("\"");
            sqlLst.insert(sqlLst.end(), sql);
        }
    }
}

void toSecuritySystem::changed(toTreeWidgetItem *org)
{
    toResultViewCheck *item = dynamic_cast<toResultViewCheck *>(org);
    if (item)
    {
        if (item->isOn())
        {
            item = dynamic_cast<toResultViewCheck *>(item->parent());
            if (item)
                item->setOn(true);
        }
        else
        {
            item = dynamic_cast<toResultViewCheck *>(item->firstChild());
            if (item)
                item->setOn(false);
        }
    }
}

void toSecuritySystem::eraseUser(bool all)
{
    for (toTreeWidgetItem *item = firstChild(); item; item = item->nextSibling())
    {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(item);
        if (chk && all)
            chk->setOn(false);
        item->setText(1, QString::null);
        for (toTreeWidgetItem *chld = item->firstChild(); chld; chld = chld->nextSibling())
        {
            chld->setText(1, QString::null);
            toResultViewCheck *chk = dynamic_cast<toResultViewCheck *>(chld);
            if (chk && all)
                chk->setOn(false);
        }
    }
}

void toSecuritySystem::changeUser(const QString &user)
{
    eraseUser();
    try
    {
        toConnectionSubLoan conn(toConnection::currentConnection(this));
        toQuery query(conn, SQLSystemGrant, toQueryParams() << user);
        while (!query.eof())
        {
            QString str = (QString)query.readValue();
            QString admin = (QString)query.readValue();
            for (toTreeWidgetItem *item = firstChild(); item; item = item->nextSibling())
            {
                if (item->text(0) == str)
                {
                    toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(item);
                    if (chk)
                        chk->setOn(true);
                    item->setText(1, tr("ON"));
                    if (admin != tr("NO") && item->firstChild())
                    {
                        chk = dynamic_cast<toResultViewCheck *>(item->firstChild());
                        if (chk)
                            chk->setOn(true);
                        if (chk->parent())
                            chk->parent()->setOpen(true);
                        item->firstChild()->setText(1, tr("ON"));
                    }
                    break;
                }
            }
        }
    }
    TOCATCH
}

toSecurityRoleGrant::toSecurityRoleGrant(QWidget *parent)
    : toListView(parent)
{
    addColumn(tr("Role name"));
    setRootIsDecorated(true);
    update();
    setSorting(0);
    connect(this, SIGNAL(clicked(toTreeWidgetItem *)), this, SLOT(changed(toTreeWidgetItem *)));
}

void toSecurityRoleGrant::update(void)
{
    clear();
    try
    {
        toConnectionSubLoan conn(toConnection::currentConnection(this));
        toQuery priv(conn, SQLRoles, toQueryParams());
        while (!priv.eof())
        {
            toResultViewCheck *item = new toResultViewCheck(this, (QString)priv.readValue(), toTreeWidgetCheck::CheckBox);
            new toResultViewCheck(item, tr("Admin"), toTreeWidgetCheck::CheckBox);
            new toResultViewCheck(item, tr("Default"), toTreeWidgetCheck::CheckBox);
        }
    }
    TOCATCH
}

toTreeWidgetCheck *toSecurityRoleGrant::findChild(toTreeWidgetItem *parent, const QString &name)
{
    for (toTreeWidgetItem *item = parent->firstChild(); item; item = item->nextSibling())
    {
        if (item->text(0) == name)
        {
            toResultViewCheck * ret = dynamic_cast<toResultViewCheck *>(item);
            if (ret->isEnabled())
                return ret;
            else
                return NULL;
        }
    }
    return NULL;
}

void toSecurityRoleGrant::sql(const QString &user, std::list<QString> &sqlLst)
{
    bool any = false;
    bool chg = false;
    QString except;
    QString sql;
    for (toTreeWidgetItem *item = firstChild(); item; item = item->nextSibling())
    {
        toResultViewCheck * check = dynamic_cast<toResultViewCheck *>(item);
        toTreeWidgetCheck *chld = findChild(item, tr("Admin"));
        toTreeWidgetCheck *def = findChild(item, tr("Default"));
        if (def && check)
        {
            if (!def->isOn() && check->isOn())
            {
                if (!except.isEmpty())
                    except += QString::fromLatin1(",\"");
                else
                    except += QString::fromLatin1(" EXCEPT \"");
                except += item->text(0);
                except += QString::fromLatin1("\"");
            }
            else if (check->isOn() && def->isOn())
                any = true;
            if (def->isOn() == def->text(1).isEmpty())
                chg = true;
        }
        if (chld && chld->isOn() && chld->text(1).isEmpty())
        {
            if (check->isOn() && !item->text(1).isEmpty())
            {
                sql = QString::fromLatin1("REVOKE \"");
                sql += item->text(0);
                sql += QString::fromLatin1("\" FROM \"");
                sql += user;
                sql += QString::fromLatin1("\"");
                sqlLst.insert(sqlLst.end(), sql);
            }
            sql = QString::fromLatin1("GRANT \"");
            sql += item->text(0);
            sql += QString::fromLatin1("\" TO \"");
            sql += user;
            sql += QString::fromLatin1("\" WITH ADMIN OPTION");
            sqlLst.insert(sqlLst.end(), sql);
            chg = true;
        }
        else if (check->isOn() && !item->text(1).isEmpty())
        {
            if (chld && !chld->isOn() && !chld->text(1).isEmpty())
            {
                sql = QString::fromLatin1("REVOKE \"");
                sql += item->text(0);
                sql += QString::fromLatin1("\" FROM \"");
                sql += user;
                sql += QString::fromLatin1("\"");
                sqlLst.insert(sqlLst.end(), sql);

                sql = QString::fromLatin1("GRANT \"");
                sql += item->text(0);
                sql += QString::fromLatin1("\" TO \"");
                sql += user;
                sql += QString::fromLatin1("\"");
                sqlLst.insert(sqlLst.end(), sql);
                chg = true;
            }
        }
        else if (check->isOn() && item->text(1).isEmpty())
        {
            sql = QString::fromLatin1("GRANT \"");
            sql += item->text(0);
            sql += QString::fromLatin1("\" TO \"");
            sql += user;
            sql += QString::fromLatin1("\"");
            sqlLst.insert(sqlLst.end(), sql);
            chg = true;
        }
        else if (!check->isOn() && !item->text(1).isEmpty())
        {
            sql = QString::fromLatin1("REVOKE \"");
            sql += item->text(0);
            sql += QString::fromLatin1("\" FROM \"");
            sql += user;
            sql += QString::fromLatin1("\"");
            sqlLst.insert(sqlLst.end(), sql);
            chg = true;
        }
    }
    if (chg)
    {
        sql = QString::fromLatin1("ALTER USER \"");
        sql += user;
        sql += QString::fromLatin1("\" DEFAULT ROLE ");
        if (any)
        {
            sql += QString::fromLatin1("ALL");
            sql += except;
        }
        else
            sql += QString::fromLatin1("NONE");
        sqlLst.insert(sqlLst.end(), sql);
    }
}

void toSecurityRoleGrant::changed(toTreeWidgetItem *org)
{
    toResultViewCheck *item = dynamic_cast<toResultViewCheck *>(org);
    if (item)
    {
        if (item->isOn())
        {
            toTreeWidgetCheck *chld = findChild(item, tr("Default"));
            if (chld)
                chld->setOn(true);
            item = dynamic_cast<toResultViewCheck *>(item->parent());
            if (item)
                item->setOn(true);
        }
        else
        {
            for (toTreeWidgetItem *item = firstChild(); item; item = item->nextSibling())
            {
                toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(item->firstChild());
                if (chk)
                    chk->setOn(false);
            }
        }
    }
}

void toSecurityRoleGrant::eraseUser(bool user, bool all)
{
    for (toTreeWidgetItem *item = firstChild(); item; item = item->nextSibling())
    {
        toResultViewCheck * chk = dynamic_cast<toResultViewCheck *>(item);
        if (chk && all)
            chk->setOn(false);
        item->setText(1, QString::null);
        for (toTreeWidgetItem *chld = item->firstChild(); chld; chld = chld->nextSibling())
        {
            chld->setText(1, QString::null);
            toResultViewCheck *chk = dynamic_cast<toResultViewCheck *>(chld);
            if (chk)
            {
                if (all)
                {
                    chk->setOn(false);
                    if (chk->text(0) == tr("Default"))
                        chk->setEnabled(user);
                }
            }
        }
    }
}

void toSecurityRoleGrant::changeUser(bool user, const QString &username)
{
    eraseUser(user);
    try
    {
        toConnectionSubLoan conn(toConnection::currentConnection(this));
        toQuery query(conn, SQLRoleGrant, toQueryParams() << username);
        while (!query.eof())
        {
            QString str = (QString)query.readValue();
            QString admin = (QString)query.readValue();
            QString def = (QString)query.readValue();
            for (toTreeWidgetItem *item = firstChild(); item; item = item->nextSibling())
            {
                if (item->text(0) == str)
                {
                    toTreeWidgetCheck * chk = dynamic_cast<toResultViewCheck *>(item);
                    if (chk)
                        chk->setOn(true);
                    item->setText(1, tr("ON"));
                    chk = findChild(item, tr("Admin"));
                    if (admin == tr("YES") && chk)
                    {
                        chk->setOn(true);
                        chk->setText(1, tr("ON"));
                        if (chk->parent())
                            chk->parent()->setOpen(true);
                    }
                    chk = findChild(item, tr("Default"));
                    if (def == tr("YES") && chk)
                    {
                        chk->setOn(true);
                        chk->setText(1, tr("ON"));
                        if (chk->parent())
                            chk->parent()->setOpen(true);
                    }
                    break;
                }
            }
        }
    }
    TOCATCH
}

toSecurity::toSecurity(QWidget *main, toConnection &connection)
    : toToolWidget(SecurityTool, "security.html", main, connection, "toSecurity")
{
    Utils::toBusy busy;

    QToolBar *toolbar = Utils::toAllocBar(this, tr("Security manager"));
    toolbar->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    layout()->addWidget(toolbar);

    UpdateListAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                                tr("Update user and role list"), this);
    connect(UpdateListAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    UpdateListAct->setShortcut(QKeySequence::Refresh);
    toolbar->addAction(UpdateListAct);

    toolbar->addSeparator();

    SaveAct = new QAction(QPixmap(const_cast<const char**>(commit_xpm)),
                          tr("Save changes"), this);
    connect(SaveAct, SIGNAL(triggered()), this, SLOT(saveChanges(void)));
    SaveAct->setShortcut(Qt::CTRL | Qt::Key_Return);
    toolbar->addAction(SaveAct);

    DropAct = new QAction(QPixmap(const_cast<const char**>(trash_xpm)),
                          tr("Remove user/role"), this);
    connect(DropAct, SIGNAL(triggered()), this, SLOT(drop(void)));
    toolbar->addAction(DropAct);
    DropAct->setEnabled(false);

    toolbar->addSeparator();

    AddUserAct = new QAction(QPixmap(const_cast<const char**>(adduser_xpm)),
                             tr("Add new user"), this);
    connect(AddUserAct, SIGNAL(triggered()), this, SLOT(addUser(void)));
    AddUserAct->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_U);
    toolbar->addAction(AddUserAct);

    AddRoleAct = new QAction(QPixmap(const_cast<const char**>(addrole_xpm)),
                             tr("Add new role"), this);
    connect(AddRoleAct, SIGNAL(triggered()), this, SLOT(addRole(void)));
    AddRoleAct->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_R);
    toolbar->addAction(AddRoleAct);

    CopyAct = new QAction(QPixmap(const_cast<const char**>(copyuser_xpm)),
                          tr("Copy current user or role"), this);
    connect(CopyAct, SIGNAL(triggered()), this, SLOT(copy(void)));
    CopyAct->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_O);
    toolbar->addAction(CopyAct);
    CopyAct->setEnabled(false);

    toolbar->addSeparator();

    DisplaySQLAct = new QAction(QPixmap(const_cast<const char**>(sql_xpm)),
                                tr("Display SQL needed to make current changes"), this);
    connect(DisplaySQLAct, SIGNAL(triggered()), this, SLOT(displaySQL(void)));
    DisplaySQLAct->setShortcut(Qt::Key_F4);
    toolbar->addAction(DisplaySQLAct);

    toolbar->addWidget(new Utils::toSpacer());

    new toChangeConnection(toolbar);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    layout()->addWidget(splitter);
    UserList = new toListView(splitter);
    UserList->addColumn(tr("Users/Roles"));
    UserList->setSQLName(QString::fromLatin1("toSecurity:Users/Roles"));
    UserList->setRootIsDecorated(true);
    UserList->setSelectionMode(toTreeWidget::Single);
    Tabs = new QTabWidget(splitter);
    Quota = new toSecurityQuota(Tabs);
    General = new toSecurityPage(Quota, connection, Tabs);
    Tabs->addTab(General, tr("&General"));
    RoleGrant = new toSecurityRoleGrant(Tabs);
    Tabs->addTab(RoleGrant, tr("&Roles"));
    SystemGrant = new toSecuritySystem(Tabs);
    Tabs->addTab(SystemGrant, tr("&System Privileges"));
    ObjectGrant = new toSecurityObject(Tabs);
    Tabs->addTab(ObjectGrant, tr("&Object Privileges"));
    Tabs->addTab(Quota, tr("&Quota"));
    UserList->setSelectionMode(toTreeWidget::Single);
    connect(UserList, SIGNAL(selectionChanged(toTreeWidgetItem *)),
            this, SLOT(changeUser(toTreeWidgetItem *)));
    ToolMenu = NULL;
    // connect(toMainWidget()->workspace(), SIGNAL(subWindowActivated(QMdiSubWindow *)),
    //         this, SLOT(windowActivated(QMdiSubWindow *)));
    refresh();
    connect(this, SIGNAL(connectionChange()),
            this, SLOT(refresh()));
    setFocusProxy(Tabs);
}

void toSecurity::slotWindowActivated(toToolWidget *widget)
{
    if (!widget)
        return;
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Security"), this);

            ToolMenu->addAction(UpdateListAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(SaveAct);
            ToolMenu->addAction(DropAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(AddUserAct);
            ToolMenu->addAction(AddRoleAct);
            ToolMenu->addAction(CopyAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(DisplaySQLAct);

            toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

void toSecurity::displaySQL(void)
{
    std::list<QString> lines = sql();
    QString res;
    for (std::list<QString>::iterator i = lines.begin(); i != lines.end(); i++)
    {
        res += *i;
        res += QString::fromLatin1(";\n");
    }
    if (res.length() > 0)
        new toMemoEditor(this, res, -1, -1, true);
    else
        Utils::toStatusMessage(tr("No changes made"));
}

std::list<QString> toSecurity::sql(void)
{
    std::list<QString> ret;
    try
    {
        QString tmp = General->sql();
        if (!tmp.isEmpty())
            ret.insert(ret.end(), tmp);
        QString name = General->name();
        if (!name.isEmpty())
        {
            SystemGrant->sql(name, ret);
            ObjectGrant->sql(name, ret);
            RoleGrant->sql(name, ret);
        }
    }
    catch (const QString &str)
    {
        Utils::toStatusMessage(str);
        std::list<QString> empty;
        return empty;
    }

    return ret;
}

void toSecurity::changeUser(bool ask)
{
    if (ask)
    {
        try
        {
            std::list<QString> sqlList = sql();
            if (sqlList.size() != 0)
            {
                switch (TOMessageBox::warning(this,
                                              tr("Save changes?"),
                                              tr("Save the changes made to this user?"),
                                              tr("Save"), tr("Discard"), tr("Cancel")))
                {
                    case 0:
                        saveChanges();
                        return ;
                    case 1:
                        break;
                    case 2:
                        return ;
                }
            }
        }
        catch (const QString &str)
        {
            Utils::toStatusMessage(str);
            return ;
        }
    }

    try
    {
        QString sel;
        toTreeWidgetItem *item = UserList->selectedItem();
        if (item)
        {
            Utils::toBusy busy;
            UserID = item->text(1);
            DropAct->setEnabled(item->parent());
            CopyAct->setEnabled(item->parent());

            if (UserID[4].toLatin1() != ':')
                throw tr("Invalid security ID");
            bool user = false;
            if (UserID.startsWith(QString::fromLatin1("USER")))
                user = true;
            QString username = UserID.right(UserID.length() - 5);
            General->changePage(username, user);
            Quota->changeUser(username);
            Tabs->setTabEnabled(Tabs->indexOf(Quota), user);
            RoleGrant->changeUser(user, username);
            SystemGrant->changeUser(username);
            ObjectGrant->changeUser(username);
        }
    }
    TOCATCH
}

void toSecurity::refresh(void)
{
    Utils::toBusy busy;
    disconnect(UserList, SIGNAL(selectionChanged(toTreeWidgetItem *)),
               this, SLOT(changeUser(toTreeWidgetItem *)));
    SystemGrant->update();
    RoleGrant->update();
    ObjectGrant->update();
    Quota->update();
    General->update();

    UserList->clear();
    try
    {
        toTreeWidgetItem *parent = new toResultViewItem(UserList, NULL, QString::fromLatin1("Users"));
        parent->setText(1, QString::fromLatin1("USER:"));
        parent->setOpen(true);
        toConnectionSubLoan conn(connection());
        toQuery user(conn, toSQL::string(toSQL::TOSQL_USERLIST, connection()), toQueryParams());
        toTreeWidgetItem *item = NULL;
        while (!user.eof())
        {
            QString tmp = (QString)user.readValue();
            QString id = QString::fromLatin1("USER:");
            id += tmp;
            item = new toResultViewItem(parent, item, tmp);
            item->setText(1, id);
            if (id == UserID)
                UserList->setSelected(item, true);
        }
        parent = new toResultViewItem(UserList, parent, tr("Roles"));
        parent->setText(1, QString::fromLatin1("ROLE:"));
        parent->setOpen(true);
        toQuery roles(conn, SQLRoles, toQueryParams());
        item = NULL;
        while (!roles.eof())
        {
            QString tmp = (QString)roles.readValue();
            QString id = QString::fromLatin1("ROLE:");
            id += tmp;
            item = new toResultViewItem(parent, item, tmp);
            item->setText(1, id);
            if (id == UserID)
                UserList->setSelected(item, true);
        }
    }
    TOCATCH
    connect(UserList, SIGNAL(selectionChanged(toTreeWidgetItem *)),
            this, SLOT(changeUser(toTreeWidgetItem *)));
}

void toSecurity::saveChanges()
{
    std::list<QString> sqlList = sql();
    toConnectionSubLoan conn(connection());
    for (std::list<QString>::iterator i = sqlList.begin(); i != sqlList.end(); i++)
    {
        try
        {
            conn->execute(*i);
        }
        TOCATCH
    }
    if (General->user())
        UserID = QString::fromLatin1("USER:");
    else
        UserID = QString::fromLatin1("ROLE:");
    UserID += General->name();
    refresh();
    changeUser(false);
}

void toSecurity::drop()
{
    if (UserID.length() > 5)
    {
        QString str = QString::fromLatin1("DROP ");
        if (General->user())
            str += QString::fromLatin1("USER");
        else
            str += QString::fromLatin1("ROLE");
        str += QString::fromLatin1(" \"");
        str += UserID.right(UserID.length() - 5);
        str += QString::fromLatin1("\"");
        try
        {
            toConnectionSubLoan conn(connection());
            conn->execute(str);
            refresh();
            changeUser(false);
        }
        catch (...)
        {
            switch (TOMessageBox::warning(this,
                                          tr("Are you sure?"),
                                          tr("The user still owns objects, add the cascade option?"),
                                          tr("Yes"), tr("No")))
            {
                case 0:
                    str += QString::fromLatin1(" CASCADE");
                    try
                    {
                        toConnectionSubLoan conn(connection());
                        conn->execute(str);
                        refresh();
                        changeUser(false);
                    }
                    TOCATCH
                    return ;
                case 1:
                    break;
            }
        }
    }
}

void toSecurity::addUser(void)
{
    for (toTreeWidgetItem *item = UserList->firstChild(); item; item = item->nextSibling())
        if (item->text(1) == QString::fromLatin1("USER:"))
        {
            UserList->clearSelection();
            UserList->setCurrentItem(item);
            Tabs->setCurrentIndex(Tabs->indexOf(General));
            General->setFocus();
            break;
        }
}

void toSecurity::addRole(void)
{
    for (toTreeWidgetItem *item = UserList->firstChild(); item; item = item->nextSibling())
        if (item->text(1) == QString::fromLatin1("ROLE:"))
        {
            UserList->clearSelection();
            UserList->setCurrentItem(item);
            Tabs->setCurrentIndex(Tabs->indexOf(General));
            General->setFocus();
            break;
        }
}

void toSecurity::copy(void)
{
    General->clear();
    SystemGrant->eraseUser(false);
    RoleGrant->eraseUser(General->user(), false);
    ObjectGrant->eraseUser(false);
    Quota->clear();
    if (General->user())
        UserID = QString::fromLatin1("USER:");
    else
        UserID = QString::fromLatin1("ROLE:");
    for (toTreeWidgetItem *item = UserList->firstChild(); item; item = item->nextSibling())
        if (item->text(1) == UserID)
        {
            disconnect(UserList, SIGNAL(selectionChanged(toTreeWidgetItem *)),
                       this, SLOT(changeUser(toTreeWidgetItem *)));
            UserList->clearSelection();
            UserList->setCurrentItem(item);
            connect(UserList, SIGNAL(selectionChanged(toTreeWidgetItem *)),
                    this, SLOT(changeUser(toTreeWidgetItem *)));
            break;
        }
}
