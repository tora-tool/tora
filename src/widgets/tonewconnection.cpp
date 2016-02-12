
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

#include "widgets/tonewconnection.h"
#include "core/utils.h"
#include "tools/tohelp.h"
#include "core/toconnectionprovider.h"
#include "core/toconnectionregistry.h"
#include "core/toconnectionmodel.h"
#include "widgets/toconnectionimport.h"
#include "core/toconnection.h"
#include "core/tooracleconst.h"
#include "core/toconfiguration.h"
#include "core/toglobalconfiguration.h"

#include "icons/trash.xpm"

#include <QMessageBox>
#include <QHeaderView>
#include <QtCore/QStringList>
#include <QMenu>
#include <QtGui/QIcon>
#include <QSortFilterProxyModel>

// TODO turn these into enum (QMetaEnum)
#define CONF_PROVIDER_LIST_SORT_OFFSET "ProvListSort" //Sort by database/connection name, asc
#define DEFAULT_PROVIDER_LIST_SORT_OFFSET 4

static toConnectionModel     *m_connectionModel = 0;
static QSortFilterProxyModel *m_proxyModel      = 0;

toNewConnection::toNewConnection(QWidget* parent, toWFlags fl)
    : QDialog(parent, fl)
    , toHelpContext(QString::fromLatin1("newconnection.html"))
{
    setupUi(this);

    colorComboBox->addItem("None", "");
    const QMap<QString, QVariant> ColorsMap(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::ColorizedConnectionsMap).toMap());
    QMapIterator<QString, QVariant> it(ColorsMap);
    while (it.hasNext())
    {
        it.next();
        colorComboBox->addItem(Utils::connectionColorPixmap(it.key()), it.value().toString(), it.key());
    }

    toHelp::connectDialog(this);

    Previous->setModel(proxyModel());
    proxyModel()->setFilterWildcard("*");

    QList<QString> lst = toConnectionProviderRegistrySing::Instance().providers();
    foreach(QString p, lst)
    {
        try
        {
            if (p == "Oracle")
            {
                Provider->addItem(ORACLE_INSTANTCLIENT, QVariant(QString::fromLatin1(ORACLE_PROVIDER)));
                Provider->addItem(ORACLE_TNSCLIENT, QVariant(QString::fromLatin1(ORACLE_PROVIDER)));
            }
            else
            {
                toConnectionProvider &provider = toConnectionProviderRegistrySing::Instance().get(p);
                Provider->addItem(provider.displayName(), QVariant(provider.name()));
            }
        }
        TOCATCH
    }

    if (Provider->count() < 1)
    {
        TOMessageBox::information(
            NULL,
            tr("No connection provider"),
            tr("No available connection provider"));

        reject();
        return;
    }

    NewConnection = 0;

    Previous->setContextMenuPolicy(Qt::CustomContextMenu);

    PreviousContext = new QMenu(this);
    QAction *delact = PreviousContext->addAction(
                          QIcon(QPixmap(trash_xpm)),
                          tr("&Delete"));
    connect(delact,
            SIGNAL(triggered()),
            this,
            SLOT(historyDelete()));

    connect(Previous,
            SIGNAL(customContextMenuRequested(const QPoint &)),
            this,
            SLOT(previousMenu(const QPoint &)));

    Settings.beginGroup("connections");
    readSettings();

    Previous->horizontalHeader()->setStretchLastSection(true);
    Previous->horizontalHeader()->setHighlightSections(false);
    Previous->verticalHeader()->setVisible(false);

    connect(Previous->selectionModel(),
            SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
            this,
            SLOT(previousCellChanged(const QModelIndex &)));

    connect(Provider,
            SIGNAL(currentIndexChanged(int)),
            this,
            SLOT(changeProvider(int)));

    connect(Previous,
            SIGNAL(doubleClicked(const QModelIndex &)),
            this,
            SLOT(accept()));

    connect(Host,
            SIGNAL(editTextChanged(QString)),
            this,
            SLOT(changeHost()));

    connect(ImportButton, SIGNAL(clicked()),
            this, SLOT(importButton_clicked()));

    connect(searchEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(searchEdit_textEdited(const QString &)));

    connect(testConnectionButton, SIGNAL(clicked()),
            this, SLOT(testConnectionButton_clicked()));

    connect(saveConnectionButton, SIGNAL(clicked()),
            this, SLOT(saveConnectionButton_clicked()));

    // must make sure this gets called manually.
    changeProvider(Provider->currentIndex());

    int r = connectionModel()->rowCount() - 1;
    QModelIndex last = connectionModel()->index(r, 1);
    Previous->setCurrentIndex(last);
    Previous->selectionModel()->clear();
    searchEdit->setFocus(Qt::OtherFocusReason);
}

QString toNewConnection::getCurrentProvider()
{
    QVariant d = Provider->itemData (Provider->currentIndex(), Qt::UserRole );
    return d.toString();
}

void toNewConnection::readSettings()
{
    restoreGeometry(Settings.value("geometry").toByteArray());

    connectionModel()->readConfig();

    Previous->setSortingEnabled(false);

    Previous->resizeColumnsToContents();
    Previous->setSortingEnabled(true);

    int sortList = Settings.value(CONF_PROVIDER_LIST_SORT_OFFSET, DEFAULT_PROVIDER_LIST_SORT_OFFSET).toInt();
    Previous->sortByColumn(abs(sortList), (sortList >= 0 ? Qt::AscendingOrder : Qt::DescendingOrder));
    checkBoxRememberPasswords->setChecked(toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SavePasswordBool).toBool());

    Previous->hideColumn(0);
}


void toNewConnection::writeSettings(bool checkHistory)
{
    int r = 0;
    Settings.setValue("geometry", saveGeometry());
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Global::SavePasswordBool,
            checkBoxRememberPasswords->isChecked());

    Settings.setValue(CONF_PROVIDER_LIST_SORT_OFFSET,
                      (Previous->horizontalHeader()->sortIndicatorOrder() == Qt:: AscendingOrder ? 1 : -1)*
                      Previous->horizontalHeader()->sortIndicatorSection());

    Settings.remove("history");

    if (!Provider->currentText().isEmpty() && checkHistory)
    {
        Settings.beginGroup("history/0");
        Settings.setValue("provider", Provider->currentText());
        Settings.setValue("username", Username->text());
        if (toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SavePasswordBool).toBool())
        {
            Settings.setValue("password", Utils::toObfuscate(Password->text()));
        }
        Settings.setValue("host", Host->currentText());
        Settings.setValue("port", Port->value());
        Settings.setValue("database", Database->currentText());
        Settings.setValue("schema", Schema->text());
        Settings.setValue("color", colorComboBox->itemData(colorComboBox->currentIndex()));

        Settings.beginGroup("options");
        QList<QCheckBox *> widgets = OptionGroup->findChildren<QCheckBox *>();
        Q_FOREACH(QCheckBox * box, widgets)
        Settings.setValue(box->text(), box->isChecked());
        Settings.endGroup(); // options
        Settings.endGroup(); // history/0
        ++r;
    }

    // find history item with same options. will skip later.
    int skip = -1;
    if (checkHistory)
    {
        skip = findHistory(Provider->currentText(),
                           Username->text(),
                           Host->currentText(),
                           Database->currentText(),
                           Schema->text(),
                           Port->value()
                          );
    }

    QMap<int, toConnectionOptions> c = connectionModel()->availableConnections();
    foreach(int row, c.keys())
    {
        if (row == skip)// && ++skipped)
            continue;

        toConnectionOptions &opt = c[row];

        Settings.beginGroup("history/" + QString::number(r/*row*/));// + 1 - skipped));
        Settings.setValue("provider", opt.provider);
        Settings.setValue("username", opt.username);
        Settings.setValue("host", opt.host);
        Settings.setValue("database", opt.database);
        Settings.setValue("schema", opt.schema);
        Settings.setValue("color", opt.color);
        Settings.setValue("port", opt.port);
        if (toConfigurationNewSingle::Instance().option(ToConfiguration::Global::SavePasswordBool).toBool())
        {
            Settings.setValue("password", Utils::toObfuscate(opt.password));
        }
        Settings.beginGroup("options");
        Q_FOREACH(QString s, opt.options)
        Settings.setValue(s, true);
        Settings.endGroup();

        Settings.endGroup();
        ++r;
    }

    connectionModel()->readConfig();
}


int toNewConnection::findHistory(const QString &provider,
                                 const QString &username,
                                 const QString &host,
                                 const QString &database,
                                 const QString &schema,
                                 quint16 port)
{
    QMapIterator<int, toConnectionOptions> i(connectionModel()->availableConnections());
    while (i.hasNext())
    {
        i.next();
        toConnectionOptions opt = i.value();
        if (provider == opt.provider &&
                username == opt.username &&
                host == opt.host &&
                database == opt.database &&
                schema == opt.schema &&
                port == opt.port)
            return i.key();
    }
    return -1;
}

void toNewConnection::loadPrevious(const QModelIndex & current)
{
    if (!current.isValid())
        return;

    QModelIndex baseIndex = proxyModel()->index(current.row(), 0);
    int index = proxyModel()->data(baseIndex, Qt::DisplayRole).toInt();
    toConnectionOptions opt = connectionModel()->availableConnection(index);

    // Connection provider for selected/current history connection entry was not found => throw exception
    QVariant p = QVariant::fromValue(opt.provider);
    int idx = Provider->findData(p, Qt::DisplayRole);
    if (idx == -1)
        throw QString("Connection provider not loaded: %1").arg(opt.provider);

    QString RealProviderName = Provider->itemData(idx, Qt::UserRole).toString();
    toConnectionProvider &prov = toConnectionProviderRegistrySing::Instance().get(RealProviderName);

    Provider->setCurrentIndex(Provider->findText(opt.provider));
    Host->lineEdit()->setText(opt.host);
    Database->lineEdit()->setText(opt.database);
    Username->setText(opt.username);
    Password->setText(opt.password);
    Port->setValue(opt.port);
    Schema->setText(opt.schema);
    int ix = colorComboBox->findData(opt.color);
    // safe fallback routine for undefined colors...
    if (ix == -1)
    {
        colorComboBox->addItem(Utils::connectionColorPixmap(opt.color), opt.color, opt.color);
        ix = colorComboBox->count() - 1;
    }
    colorComboBox->setCurrentIndex(ix);

    QList<QCheckBox *> widgets = OptionGroup->findChildren<QCheckBox *>();
    Q_FOREACH(QCheckBox * box, widgets)
    box->setChecked(opt.options.find(box->text()) != opt.options.end());
}

void toNewConnection::done(int r)
{
    if (r == QDialog::Rejected)
    {
        QDialog::done(r);
        return;
    }

    NewConnection = makeConnection(/*savePrefs*/true, /*test connection*/false);
    if (!NewConnection)
        return;

    writeSettings();
    QDialog::done(r);
}


void toNewConnection::previousCellChanged(const QModelIndex & current)
{
    loadPrevious(current);
}


void toNewConnection::changeProvider(int current)
{
    try
    {
        QString provider = getCurrentProvider();
        if (provider.isNull() || provider.isEmpty())
            return;

        bool oldStateH = Host->blockSignals(true);
        Host->clear();
        QList<QString> hosts = toConnectionProviderRegistrySing::Instance().get(provider).hosts();
        DefaultPort = 0;
        foreach(QString const & host, hosts)
        {
            if (host.isEmpty())
                continue;
            else if (host.startsWith(":"))
                DefaultPort = host.mid(1).toInt();
            else
                Host->addItem(host); // This might also call changeHost(), unless blockSignals == true
        }
        Host->blockSignals(oldStateH);

        Database->clear();
        changeHost(); // will populate Databases combobox

        // seems i broke this for oracle
        if (!DefaultPort)
        {
            if (provider.startsWith("Oracle"))
                DefaultPort = 1521;
        }

        if (Provider->currentText().startsWith("Oracle")) // TODO add provider property
        {
            SchemaLabel->show();
            Schema->show();
        }
        else
        {
            SchemaLabel->hide();
            Schema->hide();
        }

        Port->setValue(DefaultPort);

        if (Provider->currentText().startsWith(ORACLE_TNSCLIENT) || getCurrentProvider() == "QODBC")
        {
            HostLabel->hide();
            Host->hide();
            PortLabel->hide();
            Port->hide();
        }
        else
        {
            HostLabel->show();
            Host->show();
            PortLabel->show();
            Port->show();
        }

        QList<QWidget *> widgets = OptionGroup->findChildren<QWidget *>();
        foreach(QWidget * w, widgets)
        delete w;

        QList<QString> options = toConnectionProviderRegistrySing::Instance().get(provider).options();
        foreach(QString option, options)
        {

            if (option == "-")
                continue;

            bool defOn = false;
            if (option.startsWith("*"))
            {
                defOn = true;
                option = option.mid(1);
            }

            QCheckBox *ow = new QCheckBox(OptionGroup);
            ow->setText(option);
            ow->setChecked(defOn);
            OptionGroup->layout()->addWidget(ow);
            ow->show();
        }

        if (options.empty())
            OptionGroup->hide();
        else
            OptionGroup->show();
    }
    catch (const QString &str)
    {
        Utils::toStatusMessage(str);
    }
}


void toNewConnection::changeHost(void)
{
    QString prov = getCurrentProvider();

    try
    {
        if (Host->isVisible() || prov == "Oracle" || prov == "QODBC")
        {
            QString host = Host->currentText();
            if (Provider->currentText() == ORACLE_TNSCLIENT)
                host = "SQL*Net";
            QList<QString> databases = toConnectionProviderRegistrySing::Instance().get(prov).databases(host, Username->text(), Password->text());
            QString current = Database->currentText();

            Database->clear();
            Q_FOREACH(QString const & s, databases)
            {
                Database->addItem(s);
            }
            Database->lineEdit()->setText(current);
        }
    }
    catch (const QString &str)
    {
        Database->clear();
        Utils::toStatusMessage(str);
    }
}


void toNewConnection::importButton_clicked()
{
    toConnectionImport dia;
    if (!dia.exec())
        return;

    Previous->setSortingEnabled(false);

    // find latest id (max+1)
    QList<int> keys = connectionModel()->availableConnections().keys();
    qSort(keys);
    int max = 0;
    if (keys.count() > 0)
        max = keys.at(keys.count() - 1) + 1;

    foreach (toConnectionOptions opt, dia.availableConnections().values())
    {
        if (findHistory(opt.provider, opt.username, opt.host, opt.database, opt.schema, opt.port) != -1)
            continue;

        connectionModel()->append(max, opt);
        ++max;
    }
    writeSettings();

    Previous->setSortingEnabled(true);
}

void toNewConnection::searchEdit_textEdited(const QString & text)
{
    proxyModel()->setFilterWildcard(QString("*%1*").arg(text));
}

void toNewConnection::testConnectionButton_clicked()
{
    toConnection * c = makeConnection(/*savePrefs*/false, /*test connection*/ true);
    if (c)
    {
        QMessageBox::information(this->parentWidget(),
                                 tr("Connection succeeded"),
                                 tr("Connection succeeded")
                                );
        delete c;
    }
}

void toNewConnection::saveConnectionButton_clicked()
{
    writeSettings(true);
}

toConnection* toNewConnection::makeConnection(bool savePrefs, bool test)
{
    try
    {
        QString pass;
        QString host;
        QString color = colorComboBox->itemData(colorComboBox->currentIndex()).toString();

        if (Host->isVisible())
            host = Host->currentText();

        QString schema;
        if (Schema->isVisible())
            schema = Schema->text();

        QString optionstring;
        QSet<QString> options;

        // This connection is just for testing. Do not run any BG queries.
        if(test)
        	options.insert("TEST");

        QList<QCheckBox *> widgets = OptionGroup->findChildren<QCheckBox *>();
        foreach(QCheckBox * box, widgets)
        {
            if (!optionstring.isEmpty())
                optionstring += ",";

            if (box->isChecked())
            {
                optionstring += "*";

                // ug. this is awesome. i broke it when i added
                // accelerators.
                options.insert(box->text().replace("&", ""));
            }

            optionstring += box->text();
        }

        QString provider = getCurrentProvider();

        if (Port->isVisible() && Port->value() != 0 && Port->value())
            host += ":" + QString::number(Port->value());

        QString database = Database->currentText();
        int port = Port->value();

        if (Provider->currentText().startsWith(ORACLE_INSTANTCLIENT))
        {
            // create the rest of the connect string. this will work
            // without an ORACLE_HOME.
            database = "//" + Host->currentText() +
                       ":" + QString::number(port) +
                       "/" + database;
            host = "";
        }

        toConnectionOptions opts(getCurrentProvider()
                                 , host
                                 , database
                                 , Username->text()
                                 , Password->text()
                                 , schema
                                 , color
                                 , port
                                 , options);

        // checks for existing connection
        foreach(toConnection * conn, toConnectionRegistrySing::Instance().connections())
        {
            if (opts == *conn)
                return conn;
        }

        toConnection *retCon = new toConnection(
            provider,
            Username->text(),
            Password->text(),
            host,
            database,
            schema,
            color,
            options);

        if (savePrefs)
            writeSettings(true);

        return retCon;
    }
    catch (const QString &exc)
    {
        QString str = tr("Unable to connect to the database.\n");
        str.append(exc);
        TOMessageBox::information(this->parentWidget(),
                                  tr("Unable to connect to the database"),
                                  str);
        return NULL;
    }
}


void toNewConnection::previousMenu(const QPoint &pos)
{
    Q_UNUSED(pos);
    PreviousContext->exec(QCursor::pos());
}

void toNewConnection::historyDelete()
{
    QModelIndex index = proxyModel()->mapToSource(Previous->currentIndex());
    connectionModel()->removeRow(index.row(), QModelIndex());

    writeSettings(false);
    readSettings();
}


toConnectionModel* toNewConnection::connectionModel()
{
    if (!m_connectionModel)
        m_connectionModel = new toConnectionModel;
    return m_connectionModel;
}


QSortFilterProxyModel* toNewConnection::proxyModel()
{
    if (!m_proxyModel)
    {
        m_proxyModel = new QSortFilterProxyModel;
        m_proxyModel->setSourceModel(connectionModel());
        m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        m_proxyModel->setFilterKeyColumn(-1);
    }

    return m_proxyModel;
}
