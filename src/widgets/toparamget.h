
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

#ifndef TOPARAMGET_H
#define TOPARAMGET_H

#include "core/tohelpcontext.h"
#include "core/toquery.h"
#include "tools/tohelp.h"

#include <QDialog>
#include <QPushButton>
#include <QtGui/QResizeEvent>
#include <QScrollArea>
#include <QGridLayout>

#include <list>
#include <map>

class toConnection;

class QComboBox;

class toParamGetButton : public QPushButton
{
        Q_OBJECT
        int Row;

    public:
        toParamGetButton(int row, QWidget *parent = 0, const char *name = 0)
            : QPushButton(name, parent), Row(row)
        {

            connect(this, SIGNAL(clicked()), this, SLOT(internalClicked()));
        }

    private slots:
        void internalClicked(void)
        {
            emit clicked(Row);
        }

    signals:
        void clicked(int);
};


/** A help class to parse SQL for input/output bindings and if
 * available also pop up a dialog and ask for values. Also maintains
 * an internal cache of old values.
 */
class toParamGet : public QDialog, public toHelpContext
{
        Q_OBJECT;

        /** Default values cache
         */
        static QHash<QString, QStringList> DefaultCache;

        /** Specified values cache
         */
        static QHash<QString, QStringList> Cache;

        QScrollArea *View;
        QGridLayout *Container;

        QList<QComboBox *> Value;
        toParamGet(QWidget *parent = 0, const char *name = 0);

    public:
        /** Get parameters for specified SQL string.
         * @param conn Connection to get binds for.
         * @param parent Parent widget if dialog is needed.
         * @param str SQL to parse for bindings. Observe that this string can be changed to further
         *            declare the binds.
         * @param interactive If not interactive simply rewrite the query and pass on the defaults.
         * @return Returns a list of values to feed into the query.
         */
        static toQueryParams getParam(toConnection &conn,
                                      QWidget *parent,
                                      QString &str,
                                      bool interactive = true);

        /** Specify a default value for the cache. This can not overwrite a manually
         * specified value by the user.
         * @param conn Connection to get binds for.
         * @param name Name of the bind variable.
         * @param val Value of the bind variable.
         */
        static void setDefault(toConnection &conn,
                               const QString &name,
                               const QString &val);

    private slots:
        virtual void showMemo(int row);
};

#endif
