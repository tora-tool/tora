#ifdef HAVE_DB2

#ifndef DB2QUERY_H
#define DB2QUERY_H

#include "toconnection.h"

#include <sqlcli.h>

class DB2ConnectionSub;


class DB2Query : public toQuery::queryImpl
{
    public:
        DB2Query(toQuery * parent, toConnectionSub * sub);
        ~DB2Query();

        void execute(void);
        toQValue readValue(void);
        bool eof(void);
        int rowsProcessed(void);
        std::list<toQuery::queryDescribe> describe(void);
        int columns(void);
        void cancel(void);

    private:
        DB2ConnectionSub * m_sub;
        SQLHANDLE hstmt;
        bool m_canFetchMore;

        int m_row;
        int m_col;
        QList<SQLSMALLINT> m_describe;

};

#endif // DB2QUERY_H

#endif
