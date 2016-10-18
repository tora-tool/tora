
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

#ifndef TOTABWIDGET_H
#define TOTABWIDGET_H

#include <QTabWidget>
#include <QtCore/QMap>
#include <QtCore/QPointer>
#include <QtCore/QString>


/** An extension of the regular tab widget with possibility to hide
 * tabs.
 */
class toTabWidget: public QTabWidget
{
        Q_OBJECT;

        struct page
        {
            QString label;
            int     index;
        };

        typedef QMap<QWidget *, struct page> WidgetMap;
        WidgetMap tabs;

    public:
        /** Constructor.
         */
        toTabWidget(QWidget *parent = 0);

        /** Specify if a tab is shown.
         * @param w Widget to change visibility.
         * @param shown Show tab if true, hide if false.
         */
        virtual void setTabShown(QWidget *w, bool value);

        /** Specify that a tab is shown.
         * @param w Tab to show.
         */
        virtual void showTab(QWidget *w)
        {
            setTabShown(w, true);
        }
        /** Specify that a tab is hidden.
         * @param w Tab to hide.
         */
        virtual void hideTab(QWidget *w)
        {
            setTabShown(w, false);
        }

        // ---------------------------------------- overrides QTabWidget

        virtual int addTab(QWidget *widget, const QString &);
        virtual int addTab(QWidget *widget,
                           const QIcon& icon,
                           const QString &label);

    private:
        /* this should not be used  for simplicity */
        int insertTab(int index, QWidget *widget, const QString &);
        int insertTab(int index,
                      QWidget *widget,
                      const QIcon& icon,
                      const QString &label);

        void removeTab(int index);

    private slots:
        void tabChanged(int index);

    signals:
        void currentTabChanged(QWidget *);
};

#endif
