#ifdef HAVE_DB2

#include "db2connectionsub.h"
#include "todb2connection.h"

#include <sqlcli1.h>
#include <sqlutil.h>
#include <sqlenv.h>


DB2ConnectionSub::DB2ConnectionSub(toConnection & conn)
        : toConnectionSub()
{
    SQLRETURN rc;
    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv );
    rc = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc );
    rc = SQLSetConnectAttr(hdbc,
                           SQL_ATTR_AUTOCOMMIT,
                           // TODO/FIXME: from prefs
                           SQL_AUTOCOMMIT_OFF, //(SQLPOINTER)(autoCommit ? SQL_AUTOCOMMIT_ON:SQL_AUTOCOMMIT_OFF),
                           SQL_NTS);
    // TODO/FIXME: check retvals
    rc = SQLConnect(hdbc,
                    qstringToSql(conn.database()), SQL_NTS,
                    qstringToSql(conn.user()), SQL_NTS,
                    qstringToSql(conn.password()), SQL_NTS);

}

DB2ConnectionSub::~DB2ConnectionSub()
{
    if (hdbc)
    {
        SQLRETURN rc;
        rc = SQLDisconnect(hdbc);
        if (rc != SQL_SUCCESS)
            DB2Connection::handleError("cannot disconnect connection", hdbc, SQL_HANDLE_DBC);
        rc = SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
        if (rc != SQL_SUCCESS)
            DB2Connection::handleError("cannot free connection resource", hdbc, SQL_HANDLE_DBC);
        hdbc = 0;
    }
    // TODO/FIXME: henv
}

void DB2ConnectionSub::cancel()
{
    query()->cancel();
}

void DB2ConnectionSub::commit()
{
    SQLRETURN rc;
    rc = SQLEndTran(SQL_HANDLE_DBC, henv, SQL_COMMIT);
    // TODO/FIXME: chect retval
}

void DB2ConnectionSub::rollback()
{
    SQLRETURN rc;
    rc = SQLEndTran(SQL_HANDLE_DBC, henv, SQL_ROLLBACK);
    // TODO/FIXME: chect retval
}

QString DB2ConnectionSub::version()
{
    SQLSMALLINT len;
    SQLRETURN rc;
    char dbmsVer[255];
    rc = SQLGetInfo(hdbc, SQL_DBMS_VER, dbmsVer, 255, &len);
    // TODO/FIXME: check retval
    return dbmsVer;
}

SQLHANDLE DB2ConnectionSub::getHdbc()
{
    return hdbc;
}

SQLCHAR* DB2ConnectionSub::qstringToSql(const QString & s)
{
    return (SQLCHAR*) s.toUtf8().constData();
}


#endif
