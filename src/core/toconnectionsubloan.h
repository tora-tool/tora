#ifndef TOCONNECTIONSUBLOAN
#define TOCONNECTIONSUBLOAN

#include "core/tologger.h"

class toConnection;
class toConnectionSub;
class toEventQuery;

class toConnectionSubLoan
{
        friend class toEventQuery;
        friend class toQuery;
    public:

        toConnectionSubLoan(toConnection &con);

        toConnectionSubLoan(toConnection &con, QString const& schema);

        /** This special kind of constructor is used by @ref toQuery while testing the connections*/
        toConnectionSubLoan(toConnection &con, int*);

        ~toConnectionSubLoan();

        /** return pointer onto wrapped type @ref toConnectionSub*/
        inline const toConnectionSub* operator -> () const
        {
            check();
            return ConnectionSub;
        }

        inline toConnectionSub* operator -> ()
        {
            check();
            return ConnectionSub;
        }

        inline operator toConnectionSub*()
        {
            check();
            return ConnectionSub;
        }
        inline operator toConnectionSub const*() const
        {
            check();
            return ConnectionSub;
        }

        toConnection const& ParentConnection;
        //InitModeEnum InitMode;
        bool SchemaInitialized;
        QString Schema;
    private:
        inline void check() const
        {
            Q_ASSERT_X(ConnectionSub != NULL, qPrintable(__QHERE__), "Invalid use of toConnectionSubLoan");
        }

        toConnectionSubLoan(toConnectionSubLoan const& other); // do not clone me
    protected:
        toConnectionSub *ConnectionSub;
};

#endif
