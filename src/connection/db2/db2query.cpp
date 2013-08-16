#ifdef HAVE_DB2

#include "db2query.h"
#include "todb2connection.h"

#include <QtDebug>
#include <sqlenv.h>


DB2Query::DB2Query(toQuery * parent, toConnectionSub * sub)
    : toQuery::queryImpl(parent),
      m_canFetchMore(false)
{
    qDebug() << "DB2 query init" << parent;
    m_sub = DB2Connection::toDB2ConnectionSub(sub);
    if (!m_sub)
        throw "DB2 internal error: 07 Cannot convert common sub to DB2";

    if (SQLAllocHandle(SQL_HANDLE_STMT, DB2Connection::toDB2ConnectionSub(sub)->getHdbc(), &hstmt) != SQL_SUCCESS)
    {
        throw "DB2: cannot allocate query handle";
        hstmt = 0;
    }
}

DB2Query::~DB2Query()
{
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
    // TODO/FIXME: err
}

void DB2Query::execute(void)
{
    SQLSMALLINT numParams;
    qDebug() << "DB2Query::execute";
    // close cursor if opened
    SQLFreeStmt(hstmt, SQL_RESET_PARAMS); // TODO/FIXME: err check
    SQLFreeStmt(hstmt, SQL_UNBIND); // TODO/FIXME: err check
    SQLFreeStmt(hstmt, SQL_CLOSE); // TODO/FIXME: err check
    //
    m_row = 0;
    m_col = 0;
    //SQLSetStmtAttr(hstmt, SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_NONSCROLLABLE, 0); // only forward fetch
    // prepare
    if (SQLPrepare(hstmt, (SQLCHAR*)query()->sql().toUtf8().constData(), SQL_NTS) != SQL_SUCCESS)
    {
        DB2Connection::handleError("cannot prepare SQL statement", hstmt, SQL_HANDLE_STMT);
        return;
    }
    if (SQLNumParams(hstmt, &numParams) != SQL_SUCCESS)
    {
        DB2Connection::handleError("cannot get number of params", hstmt, SQL_HANDLE_STMT);
        return;
    }
    // TODO/FIXME: handle params DB2ParamStruct
    // bind
    // execute
    SQLRETURN rc = SQLExecute(hstmt);
    if (rc == SQL_NEED_DATA)
    {
        // TODO/FIXME
        DB2Connection::handleError("SQL_NEEDS_DATA is not implemented yet", hstmt, SQL_HANDLE_STMT);
    }/*
    else if (rc == SQL_NO_DATA_FOUND)
    {
    }*/
    else if (rc != SQL_SUCCESS)
    {
        DB2Connection::handleError("error in SQL execute", hstmt, SQL_HANDLE_STMT);
        return;
    }
    // check result
}

toQValue DB2Query::readValue(void)
{
    qDebug() << "DB2Query::readValue";
}

bool DB2Query::eof(void)
{
    return !m_canFetchMore;
    qDebug() << "DB2Query::eof";
}

int DB2Query::rowsProcessed(void)
{
    qDebug() << "DB2Query::rowsProcessed";
    SQLINTEGER numRows;
    // TODO/FIXME: set SQL_ATTR_ROWCOUNT_PREFETCH, note: lobs!
    if (SQLRowCount(hstmt, &numRows) == SQL_SUCCESS)
        return numRows;
    else
    {
        DB2Connection::handleError("cannot get row count", hstmt, SQL_HANDLE_STMT);
        return 0;
    }
}

std::list<toQuery::queryDescribe> DB2Query::describe(void)
{
    qDebug() << "DB2Query::describe";
}

int DB2Query::columns(void)
{
    qDebug() << "DB2Query::columns";
    SQLSMALLINT numCols;
    if (SQLNumResultCols(hstmt, &numCols) == SQL_SUCCESS)
        return numCols;
    else
    {
        DB2Connection::handleError("cannot get column count", hstmt, SQL_HANDLE_STMT);
        return 0;
    }
}

void DB2Query::cancel(void)
{
    qDebug() << "DB2Query::cancel";
    SQLRETURN rc = SQLCancel(hstmt);
    if (rc != SQL_SUCCESS)
        DB2Connection::handleError("cannot cancel query", hstmt, SQL_HANDLE_STMT);
}


#endif
