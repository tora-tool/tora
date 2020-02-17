#pragma once
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

#include "core/toresult.h"

#include <QScrollArea>
#include <QtGui/QFont>
#include <QGridLayout>

#include <vector>

class toSQL;
class toEventQuery;

/** Display the first row of a query with each column with a separate label.
 * If the label of the item is a single '-' character the data is treated as
 * a horizontal label (Not bold or sunken), and the label is not printed.
 */

class toResultItem : public QScrollArea, public toResult
{
        Q_OBJECT

    public:
        /** Create widget.
         * @param num Number of columns to arrange data in.
         * @param readable Indicate if columns are to be made more readable. This means that the
         * descriptions are capitalised and '_' are converted to ' '.
         * @param parent Parent of list.
         * @param name Name of widget.
         */
        toResultItem(int num, bool readable, QWidget *parent, const char *name = NULL);
        /** Create widget. Readable columns by default.
         * @param num Number of columns to arrange data in.
         * @param parent Parent of list.
         * @param name Name of widget.
         */
        toResultItem(int num, QWidget *parent, const char *name = NULL);
        //! Dummy constructor for qt Designer
        toResultItem(QWidget *parent = 0, const char *name = "toResultItem", int num = 2);
        /** Destroy object
         */
        ~toResultItem(void);

        /** Set if titles are to be shown.
         * @param val If titles are to be shown.
         */
        void showTitle(bool val);

        /** Set if labels are to be aligned right.
         * @param val If labels are to be aligned right.
         */
        void alignRight(bool val);

        /** Set the font to display data with.
         */
        void dataFont(const QFont &val);

        /** Create title widget.
         * @param parent Parent of widget.
         * @return Title widget created.
         */
        QWidget *createTitle(QWidget *parent);

        /** Create value widget.
         * @param parent Parent of widget.
         * @return Title widget created.
         */
        QWidget *createValue(QWidget *parent);

        /** Set value of title widget.
         * @param widget Widget to use for title.
         * @param title The title of the new widget pair.
         * @param value Vaue of new title widget.
         */
        void setTitle(QWidget *widget, const QString &title, const QString &value);

        /** Set value of value widget.
         * @param widget Widget to use for value.
         * @param title The title of the new widget pair.
         * @param value Vaue of new title widget.
         */
        void setValue(QWidget *widget, const QString &title, const QString &value);

        /** Reimplemented for internal reasons.
         */
        virtual void query(const QString &sql, toQueryParams const& param);

        /** Clear result widget */
        virtual void clearData();

        /** Handle any connection
         */
        virtual bool canHandle(const toConnection &);

    protected:
        /** Start new query, hide all widgets.
         */
        void start(void);
        /** Add a new widget.
         * @param title Title of this value.
         * @param value Value.
         */
        void addItem(const QString &title, const QString &value);
        /** Done with adding queries.
         */
        void done(void);

    private slots:
        void receiveData(toEventQuery*);
        void queryDone(toEventQuery*, unsigned long);
        void queryError(toEventQuery*, const toConnection::exception &);

    private:
        /** Setup widget.
         * @param num Number of columns.
         * @param readable Make columns more readable.
         */
        void setup(int num, bool readable);

        /** Result widget.
         */
        QGridLayout *Result;

        /** Number of created widgets.
         */
        int NumWidgets;

        /** Last widget used.
         */
        int WidgetPos;

        /** number of columns
         */
        int Columns;

        /** List of allocated widgets.
         */
        std::vector<QWidget*> Widgets;

        /** If title names are to be made more readable.
         */
        bool ReadableColumns;

        /** If title names are to be displayed.
         */
        bool ShowTitle;

        /** Align widgets to the right.
         */
        bool Right;

        /** Font to display data with.
         */
        QFont DataFont;

        toEventQuery *Query;
};
