
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

#ifndef TORESULTDEPEND_H
#define TORESULTDEPEND_H

#include "toresultview.h"

class toEventQuery;

/** This widget displays information about the dependencies of an object
 * specified by the first and second parameter in the query. The sql is not
 * used in the query. It will also recurs through all dependencies of the
 * objects depended on.
 */
class toResultDepend : public toResultView
{
        Q_OBJECT
    public:
        /** Create the widget.
         * @param parent Parent widget.
         * @param name Name of widget.
         */
        toResultDepend(QWidget *parent, const char *name = NULL);

        /** Object destructor.
         */
        ~toResultDepend();

        /** Reimplemented for internal reasons.
         */
        virtual void query(const QString &sql, toQueryParams const& param);

        /** Support Oracle
         */
        virtual bool canHandle(const toConnection &conn);

        /** Clear result widget */
        virtual void clearData();

    public slots:
        void receiveData(toEventQuery*);
        void slotQueryDone(void);

    private:
        /** Check if an object already exists.
         * @param owner Owner of object.
         * @param name Name of object.
         * @return True if object exists.
         */
        bool exists(const QString &owner, const QString &name);

        toEventQuery *Query;
        toTreeWidgetItem *Current;
};

#endif
