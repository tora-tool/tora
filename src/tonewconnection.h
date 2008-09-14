/* -*- Mode: C++ -*- */
/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

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
class toConnectionModel;
class QSortFilterProxyModel;
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

    toConnectionModel * m_connectionModel;
    QSortFilterProxyModel * m_proxyModel;

    // connection created by dialog
    toConnection *NewConnection;

    QString realProvider(void);
    void readSettings(void);
    void writeSettings(bool checkHistory = false);
    int findHistory(const QString &provider,
                    const QString &username,
                    const QString &host,
                    const QString &database,
                    const QString &schema);
    // fills form with data from previous connection at param row
    void loadPrevious(const QModelIndex & current);

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
    void previousCellChanged(const QModelIndex & current);
    void previousMenu(const QPoint &pos);
    void historyDelete(void);
    void changeProvider(int current);
    void changeHost(void);
    void importButton_clicked(void);
    void searchEdit_textEdited(const QString & text);
};


#endif // TONEWCONNECTION_H
