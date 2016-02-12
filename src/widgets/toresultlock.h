
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

#ifndef TORESULTLOCK_H
#define TORESULTLOCK_H

#include "widgets/toresultview.h"
#include <map>

class toEventQuery;

/**
 * A result table displaying information about locks in a hierarchy
 */
class toResultLock : public toResultView
{
        Q_OBJECT

        toEventQuery *Query;
        toTreeWidgetItem *LastItem; // used to point to parent when polling children records
        std::map<int, bool> Checked; // list of sessions which have been checked for "children"

    public:
        toResultLock(QWidget *parent, const char *name = NULL);
        ~toResultLock();

        virtual void query(const QString &sql, const toQueryParams &param);

        /** Support Oracle
         */
        virtual bool canHandle(const toConnection &conn);

    private:
        void startQuery(void); // connect signals-slots and start query

    private slots:
        void poll(void); // Poll query results
        void queryDone(void); // Clean up query
};

#endif
