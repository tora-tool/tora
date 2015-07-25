
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

#ifndef TOEVENTQUERYTASK_H
#define TOEVENTQUERYTASK_H


#include "core/toconnection.h"
#include "core/toquery.h"
#include "core/toqvalue.h"
#include "core/tocache.h"
#include "core/toeventquery.h"
#include "core/utils.h"

#include <QtCore/QObject>
#include <QtCore/QThread>
#include <QtCore/QPointer>
#include <QtCore/QMetaType>
#include <QtCore/QEvent>
#include <QtCore/QMutex>
#include <QtCore/QList>

class toResultStats;
class toEventQuery;
class toEventQueryWorker;

/* This class is just a temporary wrapper for QThread */
class BGThread : public QThread
{
        Q_OBJECT;
    public:
        BGThread(QObject* parent) : QThread(parent), Slave(NULL), Parent((toEventQuery*)parent) {};
        ~BGThread() {};
        // These two pointers are for debugging purposes only
        toEventQueryWorker* Slave;
        toEventQuery* Parent;

        static void msleep (unsigned long s)
        {
            QThread::msleep(s);
        }
    protected:
        void run(void)
        {
            Utils::toSetThreadName(*this);
            QThread::run();
        }
};


class toEventQueryWorker : public QObject
{
        friend class toEventQuery;
        Q_OBJECT;
    public:
        toEventQueryWorker(toEventQuery*
                           , QSharedPointer<toConnectionSubLoan> &
                           , QSharedPointer<toEventQuery::WaitConditionWithMutex> &
                           , QString &
                           , toQueryParams&);

        virtual ~toEventQueryWorker();

    public slots:
        void init(void);

        void slotStop();

    signals:
        /** started */
        void started();

        /**
        * A read was requested
        */
        void readRequested();

        /**
        * Error message
        */
        void error(toConnection::exception const &msg);

        /**
        * Result headers
        *
        * @param desc list of header values
        * @param columns number of columns in result
        */
        void headers(toQColumnDescriptionList &desc, int columns);

        // must be careful when defining signals. passing by reference
        // will share memory between threads.
        // also QObject's will have it's affinity set to background thread
        // and should be disposed within the context of the main thread
        /**
        * Data read from query
        */
        void data(const ValuesList &values);

        /**
        * Emitted when sql query is done
        */
        void workDone();
        void finished();

        /**
        * Emitted if query.rowsProcessed() > 0. Number of affected rows.
        */
        void rowsProcessed(unsigned long rows);

    private slots:
        void slotRead();

    private:
        void close(void);

        toEventQuery *Consumer;

        // sql and bind parameters
        QString SQL;
        toQueryParams Params;

        QSharedPointer<toConnectionSubLoan> Connection;
        QSharedPointer<toEventQuery::WaitConditionWithMutex> CancelCondition;

        unsigned ColumnCount;

        bool Stopped, Closed;

        // the real query object
        toQuery Query;
};

#endif
