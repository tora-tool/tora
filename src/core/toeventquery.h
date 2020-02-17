
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

#ifndef TOEVENTQUERY_H
#define TOEVENTQUERY_H

#include "core/toquery.h"
#include "core/toconnection.h"
#include "core/toconnectionsub.h"
#include "core/toconnectionsubloan.h"
//#include "widgets/toresultstats.h"
#include "core/toqvalue.h"

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QSharedPointer>
#include <QtCore/QWaitCondition>

class toResultStats;
class toEventQueryWorker;
class BGThread;

/**
 * Run a query in the background without blocking. This class should
 * always be in the main thread, it uses toEventQueryWorker to actually
 * run the sql.
 */
class toEventQuery : public QObject
{
        Q_OBJECT;

        friend class toEventQueryWorker;
    public:
        enum FETCH_MODE
        {
            READ_FIRST,
            READ_ALL
        };

        class Client
        {
            protected:
                virtual ~Client() {};

                /**
                 * Emitted when header descriptions are available
                 */
                virtual void eqDescriptionAvailable(toEventQuery*) {};

                /**
                 * Emitted when data has been read.
                 * @param rows Number of rows to be read
                 */
                virtual void eqDataAvailable(toEventQuery*) = 0;

                /**
                 * Emitted with error string
                 */
                virtual void eqError(toEventQuery*, const toConnection::exception &) = 0;

                /**
                 * Emitted when done
                 */
                virtual void eqDone(toEventQuery*) = 0;
        };

        class WaitConditionWithMutex
        {
            public:
                QMutex Mutex;
                QWaitCondition WaitCondition;
        };

        /**
         * Create a new query.
         *
         * @param conn Connection to run on.
         * @param sql SQL to execute.
         * @param param Parameters to pass to query.
         * @param statistics Optional statistics widget to update with values from query.
         */
        toEventQuery(QObject *parent
                     , toConnection &conn
                     , QString const &sql
                     , toQueryParams const &param
                     , FETCH_MODE
                     , QList<QString> const &init = QList<QString>()
                             //, toResultStats *statistics = NULL
                    );

        /**
         * Create a new query.
         *
         * @param conn already leased Connection to run on.
         * @param sql SQL to execute.
         * @param param Parameters to pass to query.
         * @param statistics Optional statistics widget to update with values from query.
         */
        toEventQuery(QObject *parent
                     , QSharedPointer<toConnectionSubLoan> &conn
                     , QString const &sql
                     , toQueryParams const &param
                     , FETCH_MODE
                     , QList<QString> const &init = QList<QString>()
                             //, toResultStats *statistics = NULL
                    );

        virtual ~toEventQuery();

        /**
         * Start the query.
         * First connect all the slots and then call start
         * Must be called prior to any other function,
         */
        void start(void);

        void setFetchMode(FETCH_MODE);

        /**
         * Get description of columns.
         * @return Description of columns list.
         */
        toQColumnDescriptionList const& describe(void) const;

        /**
         * Get column count
         */
        unsigned columnCount(void) const;

        /**
         * Get the number of rows processed.
         * @return Number of rows processed.
         */
        unsigned long rowsProcessed(void) const;

        /**
         * number of rows fetched in one bulk operation
         */
        unsigned int rowsAvaiable() const;

        /**
         * Read the next value from the query.
         * @return The next available value.
         */
        toQValue readValue(void);

        /**
         * Check if at end of query.
         * @return True if query is done.
         */
        bool eof(void) const;

        /**
         * return query's sql command
         */
        QString const& sql(void) const;

        /**
         * Returns true if more data is available for readValue()
         */
        bool hasMore(void) const;

    public slots:
        /**
         * Stop reading query
         */
        void stop(void);

    signals:
        /**
         * Emitted when header descriptions are available
         */
        void descriptionAvailable(toEventQuery*);

        /**
         * Emitted when data has been read.
         * @param rows Number of rows to be read
         */
        void dataAvailable(toEventQuery*);

        /**
         * Emitted with error string
         */
        void error(toEventQuery*, const toConnection::exception &);

        /**
         * Emitted when done
         */
        void done(toEventQuery*, unsigned long);

        /**
         * Signals to be sent to Worker
         */
        void consumed(void);
        void dataRequested(void);
        void stopRequested(void);

    private slots:
        /**
         * Messages received from Worker thread
         */
        void slotStarted();

        // handle worker's data() signal. emits dataAvailable()
        void slotData(const ValuesList &values);

        // handle worker's headers() signal emits descriptionAvailable()
        void slotDesc(toQColumnDescriptionList &desc, int columns);

        // handle worker's error() signal
        void slotError(const toConnection::exception &msg);

        // handle worker's finish
        void slotFinished(void);

        // sets Processed. signal is sent if > 0
        void slotRowsProcessed(unsigned long rows);

        // emitted immediately before the Thread is destroyed
        void slotThreadEnd();

    private:
        /** Undefined copy contructor.Don't clone me. */
        toEventQuery(toEventQuery const& other);

        ValuesList Values;

        // SQL to execute.
        QString SQL;

        // Bind parameters
        toQueryParams Param;

        // Number of columns in Description
        unsigned ColumnCount;

        // Number of rows processed.
        unsigned long Processed;

        // Description of result
        toQColumnDescriptionList Description;

        // reference to a BG producer thread, these are deleted from event loop
        BGThread *Thread;
        toEventQueryWorker *Worker;

        bool Started;
        bool WorkDone;

        // connection for this query
        QSharedPointer<toConnectionSubLoan> Connection;

        QSharedPointer<WaitConditionWithMutex> CancelCondition;

        FETCH_MODE Mode;
};

#endif
