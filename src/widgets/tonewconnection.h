/* -*- Mode: C++ -*- */
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

#include "core/tohelpcontext.h"
#include "core/utils.h"
#include "ui_tonewconnectionui.h"

#include <QtCore/QMap>
#include <QtCore/QVector>
#include <QtCore/QSettings>

class toConnection;
class toConnectionModel;
class QSortFilterProxyModel;
class QMenu;

class toNewConnection : public QDialog
    , public Ui::toNewConnectionUI
    , public toHelpContext
{
        Q_OBJECT;

    private:
        QSettings Settings;
        // provider's default port
        int DefaultPort;

        QMenu *PreviousContext;

        // connection created by dialog
        toConnection *NewConnection;

        QString getCurrentProvider(void);

        void readSettings(void);
        void writeSettings(bool checkHistory = false);
        int findHistory(const QString &provider,
                        const QString &username,
                        const QString &host,
                        const QString &database,
                        const QString &schema,
                        quint16 port);
        // fills form with data from previous connection at param row
        void loadPrevious(const QModelIndex & current);

        toConnection* makeConnection(bool savePrefs, bool test);

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    public slots:
        virtual void done(int r);

    public:
        toNewConnection(QWidget* parent = 0,
                        toWFlags fl = 0);

        toConnection* connection()
        {
            return NewConnection;
        }

        static toConnectionModel* connectionModel(void);
        static QSortFilterProxyModel* proxyModel(void);

    private slots:

        /**
         * Handles selection changes in Previous. On selecting a new row,
         * will load details from history.
         *
         */
        void previousCellChanged(const QModelIndex & current);
        void previousMenu(const QPoint &pos);
        void historyDelete(void);
        void changeProvider(int current);
        void changeHost(void);
        void importButton_clicked(void);
        void searchEdit_textEdited(const QString & text);
        void testConnectionButton_clicked();
        void saveConnectionButton_clicked();
};
