
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

#ifndef __QPSQL_CONNECTION__
#define __QPSQL_CONNECTION__

#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "connection/toqsqlconnection.h"

#include <QtCore/QString>
#include <QtSql/QSqlDatabase>

class toQPSqlConnectionImpl: public toQSqlConnectionImpl
{
        friend class toQPSqlProvider;
    protected:
        toQPSqlConnectionImpl(toConnection &conn);
    public:
        /** Create a new connection to the database. */
        virtual toConnectionSub *createConnection(void);

        /** Close a connection to the database. */
        virtual void closeConnection(toConnectionSub *);
    private:
};

class toQPSqlConnectionSub : public toQSqlConnectionSub
{
        friend class toQPSqlConnectionImpl;
        typedef toQSqlConnectionSub super;
    public:
        toQPSqlConnectionSub(toConnection const& parent, QSqlDatabase const& db, QString const& dbname)
            : toQSqlConnectionSub(parent, db, dbname)
        {
        }

        ~toQPSqlConnectionSub()
        {
            LockingPtr<QSqlDatabase> ptr(Connection, Lock);
            ptr->close();
        }

        /** Implemented abstract method inherited from toConnectionSub */

        virtual void cancel(void)
        {
            // NOT SUPPORTED YET
        };

        /** Close connection. */
        virtual void close(void)
        {
            throw QString("Not implemented yet: toQPSqlConnectionSub::close");
        }

        virtual QString version();
        virtual toQueryParams sessionId();

        virtual queryImpl* createQuery(toQuery *query);

        virtual toQAdditionalDescriptions* decribe(toCache::ObjectRef const&)
        {
            throw QString("Not implemented yet: toQPSqlConnectionSub::describe");
        }

    private:
        int nativeVersion();
        int nativeSessionId();
        void nativeCancel();
};

#endif
