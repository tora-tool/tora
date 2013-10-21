
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

#ifndef TORESULTSCHEMA_H
#define TORESULTSCHEMA_H


#include "core/toresultcombo.h"

class toConnection;

/**
 * This widget displays a list of schemas
 *
 */
class toResultSchema : public toResultCombo
{
    Q_OBJECT;
public:
    /**
     * Create the widget.
     *
     * @param parent Parent widget.
     * @param name Name of widget.
     *
     * NOTE: this widget does not eneed any reference to toConnection,
     * whenever queriyng it finds toCurrentConnection
     */
    toResultSchema(QWidget *parent, const char *name = NULL);

    virtual ~toResultSchema() {};

    /** Reimplemented abstract method
     */
    virtual void query(const QString &sql, toQueryParams const& param);

public slots:
    virtual void refresh(void);

private slots:
    // stores last schema selected in qsettings
	// also set schema in toConnection
    void updateLastSchema(const QString &schema);

    // refreshes user list from connections cache, no query is executed
    void slotUsersFromCache(void);
private:
    void init(toConnection &conn);

    QString ConnectionKey;
    //QList<toCache::CacheEntry*> userList;
};

#endif
