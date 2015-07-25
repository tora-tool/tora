#ifdef HAVE_DB2
#ifndef DB2CONNECTIONSUB_H
#define DB2CONNECTIONSUB_H

#include "toconnection.h"
#include <sqlcli.h>


class DB2ConnectionSub : public toConnectionSub
{
    public:
        DB2ConnectionSub(toConnection & conn);
        ~DB2ConnectionSub();
        void cancel();
        void commit();
        void rollback();
        QString version();

        SQLHANDLE getHdbc();

    private:
        static SQLCHAR* qstringToSql(const QString & s);

        SQLHANDLE henv;     /* environment handle */
        SQLHANDLE hdbc;     /* connection handle */
};

#endif // DB2CONNECTIONSUB_H

#endif
