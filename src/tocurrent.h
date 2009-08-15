
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

#ifndef TOCURRENT_H
#define TOCURRENT_H

#include "config.h"
#include "totool.h"
#include "tosql.h"
#include "tobackground.h"

#include "toresultview.h"

class QTabWidget;
class toNoBlockQuery;
class toResultParam;
class toResultStats;
class toResultView;
class toResultTableView;

class toCurrent : public toToolWidget
{
    Q_OBJECT;

    struct update
    {
        bool IsRole;

        toTreeWidgetItem *Parent;
        QString Type;
        QString SQL;
        QString Role;

        update()
        {
            IsRole = false;
            Parent = NULL;
        }

        update(bool isrole,
               toTreeWidgetItem *parent,
               const QString &type,
               const QString &sql,
               const QString &role)
                : IsRole(isrole),
                Parent(parent),
                Type(type),
                SQL(sql),
                Role(role)
        {
        }
    };

    std::list<update> Updates;

    QTabWidget        *Tabs;
    toResultTableView *Version;
    toListView        *Grants;
    toResultTableView *ResourceLimit;
    toResultParam     *Parameters;
    toResultStats     *Statistics;
    toBackground       Poll;

    update CurrentUpdate;
    toNoBlockQuery *Query;

    virtual void addList(bool isrole,
                         toTreeWidgetItem *parent,
                         const QString &typ,
                         const toSQL &sql,
                         const QString &role = QString::null);

protected:
    void closeEvent(QCloseEvent *event);

public:
    toCurrent(QWidget *parent, toConnection &connection);
    virtual ~toCurrent();

public slots:
    void refresh(void);
    void poll(void);
};

#endif
