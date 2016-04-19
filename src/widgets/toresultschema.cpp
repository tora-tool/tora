
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

#include "widgets/toresultschema.h"
#include "core/utils.h"
#include "core/toconnection.h"

#include <QtCore/QSettings>


toResultSchema::toResultSchema(QWidget *parent,
                               const char *name)
    : toResultCombo(parent, name)
{
    setSQL(toSQL::sql(toSQL::TOSQL_USERLIST));

    toConnection &conn = toConnection::currentConnection(parent);
    ConnectionKey =
        conn.provider() + "-" +
        conn.user() + "-" +
        conn.host() + "-" +
        conn.database();

    QString sel = conn.defaultSchema();

    if (sel.isEmpty())
    {
        QSettings s;
        sel = s.value("schema/" + ConnectionKey).toString();
    }

    if (sel.isEmpty())
    {
        if (conn.providerIs("QMYSQL"))
            sel = conn.database();
        else
            sel = conn.user();
    }

    // Oracle usernames are always in upper case
    if (conn.providerIs("Oracle") || conn.providerIs("SapDB"))
        sel = sel.toUpper();

    setSelected(sel);
    setParams(toQueryParams()); // sets QueryReady = true;
    if (SelectedFound)
        conn.setDefaultSchema(sel);
    connect(this, SIGNAL(currentIndexChanged(const QString &)),
            this, SLOT(updateLastSchema(const QString &)));

    connect(&(connection().getCache()), SIGNAL(userListRefreshed()),
            this, SLOT(slotUsersFromCache()));
    setFocusPolicy(Qt::NoFocus);
}

void toResultSchema::query(const QString &sql, toQueryParams const& param)
{
    // do not call setSqlAndParams twice in a row. (also called from toResultCombo::query)
    //if (!setSqlAndParams(sql, param))
    //	return ;

	// Mysql way
    if (connection().getCache().userListExists(toCache::DATABASES))
    {
        slotUsersFromCache();
    }
    // Oracle way
    else if (connection().getCache().userListExists(toCache::USERS))
    {
    	slotUsersFromCache();
    }
    else
    {
        toResultCombo::query(sql, param);
    }
}

void toResultSchema::updateLastSchema(const QString &schema)
{
    if (schema.isEmpty())
        return;
    QSettings s;
    s.setValue("schema/" + ConnectionKey, schema);
    if (SelectedFound)
        connection().setDefaultSchema(schema);
}

void toResultSchema::slotUsersFromCache(void)
{
    SelectedFound = false;
    blockSignals(true); // Do not emit currentIndexChanged when 1st field is inserted
    clear();
    addItems(Additional);
    for (int i = 0; i < Additional.count(); i++)
        if (Additional[i] == Selected)
            setCurrentIndex(i);

    QStringList users;

    if (connection().providerIs("Oracle"))
    	users = connection().getCache().userList(toCache::USERS);
    else
    	users = connection().getCache().userList(toCache::DATABASES);

    for (QStringList::iterator i = users.begin(); i != users.end(); i++)
    {
        QString t = (*i);
        addItem(t);
        if (t == Selected)
        {
            setCurrentIndex(count() - 1);
            SelectedFound = true;
        }
    }
    toResultCombo::slotQueryDone(); // Combo list changed => do select the right field
}

void toResultSchema::slotQueryDone(void)
{
    QAbstractItemModel const* m = model();
    QList<toCache::CacheEntry*> entries;
    bool isOracle = connection().providerIs("Oracle");

    for (int i = 0; i < m->rowCount(); i++)
    {
        QModelIndex idx = m->index(i, 0);
        QString s = m->data(idx).toString();
        if (Additional.contains(s))
            continue;
        if (isOracle)
        	entries << new toCacheEntryUser(s);
        else
        	entries << new toCacheEntryDatabase(s);
    }

    if (isOracle)
    	connection().getCache().upsertUserList(entries);
    else
    	connection().getCache().upsertUserList(entries, toCache::DATABASES);
    toResultCombo::slotQueryDone();
}

void toResultSchema::connectionChanged(void)
{
    if ( ! connection().defaultSchema().isEmpty() )
    {
        // No need to upperize the string. Oracle has it uppercased already,
        // mysql nad pgsql require it as lowercase.
        setSelected(connection().defaultSchema());//.toUpper());
    }
    else if (connection().providerIs("QMYSQL"))
        setSelected(connection().database());
    else if (connection().providerIs("Oracle") || connection().providerIs("SapDB"))
        setSelected(connection().user().toUpper());
    else
        setSelected(connection().user());

    toResultCombo::connectionChanged();
}

void toResultSchema::refresh(void)
{
    try
    {
        toResultCombo::refresh();
    }
    catch (...)
    {
        TLOG(1,toDecorator,__HERE__) << "       Ignored exception." << std::endl;
    }
}
