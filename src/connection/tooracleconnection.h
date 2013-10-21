
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

#ifndef __ORACLE_CONNECTION__
#define __ORACLE_CONNECTION__

#undef  TORA_MAKE_DLL
#ifndef TORA_DLL
#define TORA_DLL
#endif
#ifndef TROTL_DLL
#define TROTL_DLL
#endif

#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/utils.h"

class toOracleProvider;

namespace trotl
{
struct OciEnv;
struct OciConnection;
struct OciLogin;
struct OciException;
class SqlStatement;
};

/** re-throw an exception while converting it from @ref trotl::OciException
* into @ref toConnection::exception
*/
_Noreturn void ReThrowException(const ::trotl::OciException &exc);

class toOracleConnectionImpl: public toConnection::connectionImpl
{
    friend class toOracleProvider;
    typedef toConnection::connectionImpl super;
protected:
    toOracleConnectionImpl(toConnection &conn, ::trotl::OciEnv &env);
public:
    /** Create a new connection to the database. */
    virtual toConnectionSub *createConnection(void);

    /** Close a connection to the database. */
    virtual void closeConnection(toConnectionSub *);

    ::trotl::OciEnv &_env;
};

class toOracleConnectionSub: public toConnectionSub
{
    friend class oracleQuery;
    typedef toConnectionSub super;
public:
    toOracleConnectionSub(::trotl::OciConnection *, ::trotl::OciLogin *);

    virtual ~toOracleConnectionSub();
    virtual void cancel();
    virtual void close();
    virtual void commit();
    virtual void rollback();
    virtual QString version();
    virtual QString sessionId();
    virtual bool hasTransaction();
    virtual queryImpl* createQuery(toQuery *query);

    virtual toQAdditionalDescriptions* decribe(toCache::ObjectRef const&);

private:
    enum TransactionFlagStateEnum { // three state boolean NO/YES/DUNNO
    	NO_TRANSACTION,
    	HAS_TRANSACTION,
    	DIRTY_FLAG
    };
    ::trotl::OciConnection *_conn;
    ::trotl::OciLogin *_login;
    ::trotl::SqlStatement *_hasTransactionStat;
    mutable TransactionFlagStateEnum _hasTransaction; // cache calls to hasTransaction()
};


#endif
