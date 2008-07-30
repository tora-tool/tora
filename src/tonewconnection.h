/* -*- Mode: C++ -*- */
/*****
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003-2005 Quest Software, Inc
 * Portions Copyright (C) 2005 Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 *      You may link this product with any GPL'd Qt library such as Qt/Free
 *
 * All trademarks belong to their respective owners.
 *
 *****/

#ifndef TONEWCONNECTION_H
#define TONEWCONNECTION_H

#include "config.h"
#include "ui_tonewconnectionui.h"
#include "tohelp.h"
#include "toconnection.h"

#include <QMap>
#include <QVector>
#include <QSettings>

#include <set>

class toConnection;
class QMenu;


/**
 * Simple class for storing connection options and comparisons.
 *
 */
class toConnectionOptions
{
public:
    QString           provider;
    QString           host;
    QString           database;
    QString           username;
    QString           password;
    int               port;
    QString           schema;
    std::set<QString> options;

    // must have for qmap
    toConnectionOptions()
    {
    }

    toConnectionOptions(const QString &_prov,
                        const QString &_host,
                        const QString &_data,
                        const QString &_user,
                        const QString &_pass,
                        const QString &_schema,
                        int _port,
                        std::set<QString> _options)
    {
        provider = _prov;
        host     = _host;
        database = _data;
        username = _user;
        password = _pass;
        schema   = _schema;
        port     = _port;
        options  = _options;
    }

    bool operator==(const toConnectionOptions &other)
    {
        return provider == other.provider &&
               host == other.host &&
               database == other.database &&
               username == other.username &&
               schema == other.schema;
    }

    bool operator!=(const toConnectionOptions &other)
    {
        return !operator==(other);
    }
};


class toNewConnection : public QDialog,
            public Ui::toNewConnectionUI,
            public toHelpContext
{
    Q_OBJECT;

private:
    QSettings Settings;
    // provider's default port
    int DefaultPort;

    QMenu *PreviousContext;

    // an enum to keep the columns in qtablewidget straight
    enum PreviousColumns
    {
        IndexColumn = 0,
        ProviderColumn,
        HostColumn,
        DatabaseColumn,
        UsernameColumn,
        SchemaColumn,
    };

    // stores connection options by unique id. these are read from the
    // settings class.
    QMap<int, toConnectionOptions> OptionMap;

    // connection created by dialog
    toConnection *NewConnection;

    QString realProvider(void);
    void readSettings(void);
    void writeSettings(void);
    int findHistory(const QString &provider,
                    const QString &username,
                    const QString &host,
                    const QString &database,
                    const QString &schema);
    // fills form with data from previous connection at param row
    void loadPrevious(int row);

    toConnection* makeConnection();

public slots:
    virtual void done(int r);

public:
    toNewConnection(QWidget* parent = 0,
                    Qt::WFlags fl = 0);

    toConnection* connection()
    {
        return NewConnection;
    }

public slots:

    /**
     * Handles selection changes in Previous. On selecting a new row,
     * will load details from history.
     *
     */
    void previousCellChanged(int currentRow,
                             int currentColumn,
                             int previousRow,
                             int previousColumn);
    void previousMenu(const QPoint &pos);
    void historyDelete(void);
    void changeProvider(int current);
    void changeHost(void);
    void importButton_clicked(void);
};


#endif // TONEWCONNECTION_H
