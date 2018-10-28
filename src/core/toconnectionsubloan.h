#pragma once

#include "core/tologger.h"

class toConnection;
class toConnectionSub;
class toEventQuery;
class toSQL;

class toConnectionSubLoan
{
        friend class toEventQuery;
        friend class toQuery;
		friend class toQueryAbstr;
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

        void execute(QString const& SQL);
        void execute(toSQL const& SQL);

        /**
         * Add a statement to be run uppon making new connections.
         * @param sql Statement to run.
         */
        void setInit(const QString &key, const QString &sql);

        /** Remove a statement that was added using @ref addInit. */
        void delInit(const QString &key);

        /** Get a list of the current init strings. */
        QList<QString> initStrings() const;

        toConnection const& ParentConnection;
        //InitModeEnum InitMode;
        bool SchemaInitialized;
        QString Schema;
    private:
        inline void check() const
        {
            Q_ASSERT_X(ConnectionSub != NULL, qPrintable(__QHERE__), "Invalid use of toConnectionSubLoan");
        }

        QMap<QString, QString> InitStrings; // Key, SQL

        toConnectionSubLoan(toConnectionSubLoan const& other); // do not clone me
    protected:
        toConnectionSub *ConnectionSub;
};
