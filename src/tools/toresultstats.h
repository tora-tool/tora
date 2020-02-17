
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

#ifndef TORESULTSTATS_H
#define TORESULTSTATS_H

#include "toresultview.h"

class toEventQuery;

#define TO_STAT_BLOCKS 10
#define TO_STAT_MAX 500

/** This widget will displays information about statistics in either a database or a session.
 */

class toResultStats : public toResultView
{
        Q_OBJECT
    public:
        /** Create statistics widget for session statistics.
         * @param OnlyChanged Only display changed items.
         * @param ses Session ID to display info about.
         * @param parent Parent widget.
         * @param name Name of widget.
         */

        toResultStats(bool OnlyChanged, int ses, QWidget *parent, const char *name = NULL);
        /** Create statistics widget for the current session statistics.
         * @param OnlyChanged Only display changed items.
         * @param parent Parent widget.
         * @param name Name of widget.
         */

        toResultStats(bool OnlyChanged, QWidget *parent, const char *name = NULL);
        /** Create statistics widget for the current database statistics.
         * @param parent Parent widget.
         * @param name Name of widget.
         */

        toResultStats(QWidget *parent, const char *name = NULL);

        /** Destroy object.
         */
        virtual ~toResultStats();

        virtual bool canHandle(const toConnection &conn);

        /**
         * close this object
         */
        bool close(void);

        /** Reset statistics. Read in last values without updating widget data.
         */
        void resetStats(void);

    signals:
        /** Emitted when session is changed.
         * @param ses New session ID.
         */
        void sessionChanged(int ses);

        /** Emitted when session is changed.
         * @param ses New session ID as string.
         */
        void sessionChanged(const QString &ses);

    public slots:
        /** Change the session that the current query will run on.
         * @param query Query to check connection for.
         */
        ///void changeSession(toQuery &conn);
        /** Change session to specified id.
         * @param ses Session ID to change to.
         */
        void slotChangeSession(int ses);
        /** Update the statistics.
         * @param reset Set delta to current values. This means that the next time this widget
         *              is updated the delta will be from the new values.
         */
        void slotRefreshStats(bool reset = true);

    private slots:
        void receiveStatisticNamesData(toEventQuery*);
        void receiveStatisticValuesData(toEventQuery*);
        void queryStatisticNamesDone(toEventQuery*, unsigned long);
        void queryStatisticsValuesDone(toEventQuery*, unsigned long);

    private:
        /** Setup widget.
         */
        void setup();

        int sid();

        /** Add value
         */
        void addValue(bool reset, int id, const QString &name, double value);

        /** Session ID to get statistics for.
         */
        int m_sessionID;

        /** Number of rows of statistics
         */
        int Row;
        /** Get information about if only changed items are to be displayed.
         */
        bool OnlyChanged;
        /** Display system statistics.
         */
        bool System;
        /** Last read values, used to calculate delta values.
         */
        double LastValues[TO_STAT_MAX + TO_STAT_BLOCKS];

        bool Reset;
        toEventQuery *Query;
        toEventQuery *SessionIO;
};

#endif
