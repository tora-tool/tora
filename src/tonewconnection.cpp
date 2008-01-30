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

#include "utils.h"

#include "toconf.h"
#include "toconnection.h"
#include "tonewconnection.h"
#include "tomain.h"
#include "toresultview.h"
#include "totool.h"

#include <q3buttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <totreewidget.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <q3whatsthis.h>

#include <QString>
#include <QFrame>
#include <QVBoxLayout>


toNewConnection::toNewConnection(QWidget* parent,
                                 const char* name,
                                 bool modal,
                                 Qt::WFlags fl)
    : QDialog(parent, fl),
      toHelpContext(QString::fromLatin1("newconnection.html"))
{
    setupUi(this);

    if(name)
        setObjectName(name);
    setModal(modal);

    toHelp::connectDialog(this);

    Database->insertItem(toConfigurationSingle::Instance().globalConfig(
                             CONF_DATABASE, DEFAULT_DATABASE));
    Previous->addColumn(tr("Provider"));
    Previous->addColumn(tr("Host"));
    Previous->addColumn(tr("Database"));
    Previous->addColumn(tr("Username"));
    Previous->setSelectionMode(toTreeWidget::Single);
    Previous->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                        QSizePolicy::Expanding));
    Previous->setMinimumWidth(325);
    Previous->setColumnWidth(0, 80);
    Previous->setColumnWidth(1, 80);
    Previous->setColumnWidth(2, 80);
    Previous->setColumnWidth(3, 80);

    QMenu *menu = new QMenu(Previous);
    QAction *deleteAct = menu->addAction("Delete");
    Previous->setDisplayMenu(menu);

    connect(deleteAct, SIGNAL(triggered()), this, SLOT(historyDelete()));
    connect(Previous,
            SIGNAL(selectionChanged()),
            this,
            SLOT(historySelection()));
    connect(Previous,
            SIGNAL(doubleClicked(toTreeWidgetItem *)),
            this,
            SLOT(historyConnect()));

    std::list<QString> lst = toConnectionProvider::providers();
    int sel = 0, cur = 0;
    QString provider = toConfigurationSingle::Instance().globalConfig(
        CONF_PROVIDER, DEFAULT_PROVIDER).latin1();
    for(std::list<QString>::iterator i = lst.begin(); i != lst.end(); i++) {
        Provider->insertItem(QString::fromLatin1(*i));
        if(*i == provider)
            sel = cur;
        cur++;
    }

    if(cur == 0) {
        TOMessageBox::information(
            this,
            tr("No connection provider"),
            tr("No available connection provider, plugins probably missing"));
        reject();
        return ;
    }

    Provider->setCurrentItem(sel);
    changeProvider();
    processOptions(toConfigurationSingle::Instance().globalConfig(
                       CONF_OPTIONS, DEFAULT_OPTIONS));

    QString host = toConfigurationSingle::Instance().globalConfig(
        CONF_HOST, DEFAULT_HOST);

    int portix = host.find(":");
    if(portix >= 0) {
        Host->lineEdit()->setText(host.mid(0, portix));
        Port->setValue(host.mid(portix + 1).toInt());
    }
    else
        Host->lineEdit()->setText(host);

    Username->setText(toConfigurationSingle::Instance().globalConfig(
                          CONF_USER, DEFAULT_USER));
    Username->setFocus();

    bool pass = toConfigurationSingle::Instance().globalConfig(
        CONF_SAVE_PWD, DEFAULT_SAVE_PWD).isEmpty();
    if(pass)
        Password->setText(QString::fromLatin1(DEFAULT_PASSWORD));
    else
        Password->setText(toUnobfuscate(
                              toConfigurationSingle::Instance().globalConfig(
                                  CONF_PASSWORD, DEFAULT_PASSWORD)));

    QString defdb = toConfigurationSingle::Instance().globalConfig(
        CONF_DATABASE, DEFAULT_DATABASE);
    Database->setEditable(true);

    {
        int maxHist = toConfigurationSingle::Instance().globalConfig(
            CONF_CONNECT_CURRENT, 0).toInt();
        Previous->setSorting(-1);
        toTreeWidgetItem *last = NULL;
        for(int i = 0;i < maxHist;i++) {
            QString path = CONF_CONNECT_HISTORY;
            path += ":";
            path += QString::number(i).latin1();
            QString tmp = path;
            tmp += CONF_USER;
            QString user = toConfigurationSingle::Instance().globalConfig(tmp, "");

            tmp = path;
            tmp += CONF_PASSWORD;
            QString passstr = (pass ? QString::fromLatin1(DEFAULT_PASSWORD) :
                               (toUnobfuscate(toConfigurationSingle::Instance().globalConfig(
                                                  tmp, DEFAULT_PASSWORD))));

            tmp = path;
            tmp += CONF_HOST;
            QString host = toConfigurationSingle::Instance().globalConfig(
                tmp, DEFAULT_HOST);

            tmp = path;
            tmp += CONF_DATABASE;
            QString database = toConfigurationSingle::Instance().globalConfig(
                tmp, DEFAULT_DATABASE);

            tmp = path;
            tmp += CONF_PROVIDER;
            QString provider = toConfigurationSingle::Instance().globalConfig(
                tmp, DEFAULT_PROVIDER);

            tmp = path;
            tmp += CONF_OPTIONS;
            QString options = toConfigurationSingle::Instance().globalConfig(
                tmp, DEFAULT_OPTIONS);

            last = new toTreeWidgetItem(Previous,
                                        last,
                                        provider,
                                        host,
                                        database,
                                        user,
                                        passstr,
                                        options);
        }
    }
    adjustSize();
}

toNewConnection::~toNewConnection()
{}

void toNewConnection::changeProvider(void)
{
    try {
        std::list<QString> hosts = toConnectionProvider::hosts(Provider->currentText().latin1());
        QString current = Host->currentText();

        bool sqlNet = false;

        Host->clear();
        DefaultPort = 0;
        for(std::list<QString>::iterator i = hosts.begin();i != hosts.end();i++) {
            if((*i).isEmpty())
                sqlNet = true;
            else if((*i).startsWith(":")) {
                DefaultPort = (*i).mid(1).toInt();
            }
            else
                Host->insertItem(*i);
        }
        Port->setValue(DefaultPort);
        if(sqlNet) {
            HostLabel->hide();
            Host->hide();
            PortLabel->hide();
            Port->hide();
        }
        else {
            HostLabel->show();
            Host->show();
            PortLabel->show();
            Port->show();
        }
        Host->lineEdit()->setText(current);

        for(std::list<QWidget *>::iterator k = OptionWidgets.begin();k != OptionWidgets.end();k++) {
            if((*k)->isA("QCheckBox")) {
                QCheckBox *box = (QCheckBox *)(*k);
                Options[box->text()] = box->isChecked();
            }
            delete *k;
        }
        OptionWidgets.clear();

        std::list<QString> options = toConnectionProvider::options(Provider->currentText().latin1());
        for(std::list<QString>::iterator j = options.begin();j != options.end();j++) {
            if((*j) == "-") {
                QFrame *frame = new QFrame(OptionGroup);
                frame->setFrameShape(QFrame::HLine);
                frame->setFrameShadow(QFrame::Sunken);
                OptionGroup->layout()->addWidget(frame);
                frame->show();
                OptionWidgets.insert(OptionWidgets.end(), frame);
            }
            else {
                QString option = *j;
                bool defOn = false;
                if(option.startsWith("*")) {
                    defOn = true;
                    option = option.mid(1);
                }

                QCheckBox *ow = new QCheckBox(OptionGroup);
                ow->setText(option);
                if(Options.find(option) != Options.end())
                    ow->setChecked((*(Options.find(option))).second);
                else
                    ow->setChecked(defOn);
                OptionGroup->layout()->addWidget(ow);
                ow->show();
                OptionWidgets.insert(OptionWidgets.end(), ow);
            }
        }
        if(options.empty())
            OptionGroup->hide();
        else
            OptionGroup->show();

    }
    catch(const QString &str) {
        Host->clear();
        toStatusMessage(str);
    }
}

void toNewConnection::changeHost(void)
{
    try {
        if(!Host->isHidden()) {
            QString host;
            host = Host->currentText();
            std::list<QString> databases = toConnectionProvider::databases(Provider->currentText().latin1(),
                                                                           host,
                                                                           Username->text(),
                                                                           Password->text());
            QString current = Database->currentText();

            Database->clear();
            for(std::list<QString>::iterator i = databases.begin();i != databases.end();i++)
                Database->insertItem(*i);
            Database->lineEdit()->setText(current);
        }
    }
    catch(const QString &str) {
        Database->clear();
        toStatusMessage(str);
    }
}

void toNewConnection::processOptions(const QString &str)
{
    QStringList options = QStringList::split(",", str);
    std::map<QString, bool> values;
    for(int i = 0;i < options.count();i++) {
        QString val = options[i];
        if(val.startsWith("*"))
            values[val.mid(1)] = true;
        else
            values[val] = false;
    }
    for(std::list<QWidget *>::iterator k = OptionWidgets.begin();k != OptionWidgets.end();k++) {
        if((*k)->isA("QCheckBox")) {
            QCheckBox *box = (QCheckBox *)(*k);
            if(values.find(box->text()) != values.end())
                box->setChecked(values[box->text()]);
        }
    }
}


toConnection *toNewConnection::makeConnection(void) {
    // make sure widgets don't change on us
    disconnect(Previous,
               SIGNAL(selectionChanged()),
               this,
               SLOT(historySelection()));

    try {
        toConfigurationSingle::Instance().globalSetConfig(
            CONF_PROVIDER, Provider->currentText());
        toConfigurationSingle::Instance().globalSetConfig(
            CONF_USER, Username->text());
        QString pass;
        QString host;
        if(!Host->isHidden())
            host = Host->currentText();

        QString optionstring;
        std::set<QString> options;

        for(std::list<QWidget *>::iterator k = OptionWidgets.begin();
            k != OptionWidgets.end();
            k++)
        {
            if((*k)->isA("QCheckBox")) {
                if(!optionstring.isEmpty())
                    optionstring += ",";
                QCheckBox *box = (QCheckBox *)(*k);
                if(box->isChecked()) {
                    optionstring += "*";
                    options.insert(box->text());
                }
                optionstring += box->text();
            }
        }
        toConfigurationSingle::Instance().globalSetConfig(CONF_OPTIONS, optionstring);

        std::list<QString> con = toMainWidget()->connections();
        for(std::list<QString>::iterator i = con.begin();i != con.end();i++) {
            try {
                toConnection &conn = toMainWidget()->connection(*i);
                if(conn.user() == Username->text() &&
                    conn.provider() == Provider->currentText().latin1() &&
                    conn.host() == host &&
                    conn.database() == Database->currentText())
                    return &conn;
            }
            catch(...) {}
        }
        if(Port->value() != 0 && Port->value() != DefaultPort)
            host += ":" + QString::number(Port->value());

        toConfigurationSingle::Instance().globalSetConfig(CONF_HOST, host);

        toConnection *retCon = new toConnection(
            Provider->currentText().latin1(),
            Username->text(),
            Password->text(),
            host,
            Database->currentText(),
            options);
        {
            QTreeWidgetItemIterator it(Previous);
            for(; (*it); it++) {
                if((*it)->text(0) == Provider->currentText() &&
                   (((*it)->text(1) == host) || ((*it)->text(1).isEmpty() &&
                                                 host.isEmpty())) &&
                   (*it)->text(2) == Database->currentText() &&
                   (*it)->text(3) == Username->text()) {
                    delete (*it);
                    break;
                }
            }
        }

        if(!toConfigurationSingle::Instance().globalConfig(
                CONF_SAVE_PWD, DEFAULT_SAVE_PWD).isEmpty())
            pass = retCon->password();
        else
            pass = DEFAULT_PASSWORD;
        toConfigurationSingle::Instance().globalSetConfig(
            CONF_PASSWORD, toObfuscate(pass));
        toConfigurationSingle::Instance().globalSetConfig(
            CONF_DATABASE, Database->currentText());

        new toTreeWidgetItem(Previous,
                             NULL,
                             Provider->currentText(),
                             host,
                             Database->currentText(),
                             Username->text(),
                             retCon->password(),
                             optionstring);
        historySave();
        return retCon;
    }
    catch(const QString &exc) {
        QString str = tr("Unable to connect to the database.\n");
        str.append(exc);
        TOMessageBox::information(this->parentWidget(false),
                                  tr("Unable to connect to the database"),
                                  str);
        return NULL;
    }
}

void toNewConnection::historySave(void)
{
    int siz = toConfigurationSingle::Instance().globalConfig(
        CONF_CONNECT_SIZE,
        DEFAULT_CONNECT_SIZE).toInt();

    int i = 0;
    int j = 0;

    QTreeWidgetItemIterator it(Previous);
    for(; i < siz; it++, i++) {
        QString path = CONF_CONNECT_HISTORY;
        path += ":";
        path += QString::number(i).latin1();

        QString tmp = path;
        tmp += CONF_PROVIDER;
        if(i < siz && (*it))
            toConfigurationSingle::Instance().globalSetConfig(tmp, (*it)->text(0));
        else
            toConfigurationSingle::Instance().globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_HOST;
        if(i < siz && (*it))
            toConfigurationSingle::Instance().globalSetConfig(tmp, (*it)->text(1));
        else
            toConfigurationSingle::Instance().globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_DATABASE;
        if(i < siz && (*it))
            toConfigurationSingle::Instance().globalSetConfig(tmp, (*it)->text(2));
        else
            toConfigurationSingle::Instance().globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_USER;
        if(i < siz && (*it))
            toConfigurationSingle::Instance().globalSetConfig(tmp, (*it)->text(3));
        else
            toConfigurationSingle::Instance().globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_PASSWORD;
        if(i < siz && (*it))
            toConfigurationSingle::Instance().globalSetConfig(tmp, toObfuscate((*it)->text(4)));
        else
            toConfigurationSingle::Instance().globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_OPTIONS;
        if(i < siz && (*it))
            toConfigurationSingle::Instance().globalSetConfig(tmp, (*it)->text(5));
        else
            toConfigurationSingle::Instance().globalEraseConfig(tmp);

        if(i < siz && (*it))
            j++;
    }
    toConfigurationSingle::Instance().globalSetConfig(
        CONF_CONNECT_CURRENT, QString::number(j));
    toConfigurationSingle::Instance().saveConfig();
}

void toNewConnection::historySelection(void)
{
    toTreeWidgetItem *item = Previous->selectedItem();
    if(item) {
        for(int i = 0;i < Provider->count();i++)
            if(Provider->text(i) == item->text(0)) {
                Provider->setCurrentItem(i);
                break;
            }
        changeProvider();

        QString host = item->text(1);
        int portix = host.find(":");
        if(portix >= 0) {
            Host->lineEdit()->setText(host.mid(0, portix));
            Port->setValue(host.mid(portix + 1).toInt());
        }
        else {
            Host->lineEdit()->setText(host);
            Port->setValue(DefaultPort);
        }

        processOptions(item->text(5));

        Database->lineEdit()->setText(item->text(2));
        Username->setText(item->text(3));
        if(item->text(4) != DEFAULT_PASSWORD)
            Password->setText(item->text(4));
    }
}

void toNewConnection::historyConnect(void)
{
    bool ok = true;
    if(toConfigurationSingle::Instance().globalConfig(CONF_SAVE_PWD, DEFAULT_SAVE_PWD).isEmpty()) {
        ok = false;
        QString name = QInputDialog::getText(tr("Enter password"),
                                             tr("Enter password to use for connection."),
                                             QLineEdit::Password, QString::null, &ok, this);
        if(ok)
            Password->setText(name);
    }
    if(ok)
        accept();
}

void toNewConnection::historyDelete()
{
    toTreeWidgetItem *item = Previous->selectedItem();
    if(item) {
        delete item;
        historySave();
    }
}
