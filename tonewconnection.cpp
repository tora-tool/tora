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

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qfile.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtooltip.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>

#ifdef WIN32
#  include "windows/cregistry.h"
#endif

#ifdef TO_KDE
#  if KDE_VERSION >= 0x30400
#    define TO_KDE_KACCELMANAGER
#  endif
#endif
 
#ifdef TO_KDE_KACCELMANAGER
#  include <kaccelmanager.h>
#endif

#include "tonewconnection.moc"
#include "tonewconnectionui.moc"

toNewConnection::toNewConnection(QWidget* parent, const char* name, bool modal, WFlags fl)
        : toNewConnectionUI(parent, name, modal, fl),
        toHelpContext(QString::fromLatin1("newconnection.html"))
{
    toHelp::connectDialog(this);

    OptionGroup->setColumnLayout(0, Qt::Vertical );
    OptionGroup->layout()->setSpacing( 6 );
    OptionGroup->layout()->setMargin( 11 );
    OptionGroupLayout = new QVBoxLayout( OptionGroup->layout() );
    OptionGroupLayout->setAlignment( Qt::AlignTop );
#ifdef TO_KDE_KACCELMANAGER
    KAcceleratorManager::setNoAccel( OptionGroup );
#endif

    QPopupMenu *menu = new QPopupMenu(Previous);
    Database->insertItem(toTool::globalConfig(CONF_DATABASE, DEFAULT_DATABASE));
    Previous->addColumn(tr("Provider"));
    Previous->addColumn(tr("Host"));
    Previous->addColumn(tr("Database"));
    Previous->addColumn(tr("Username"));
    Previous->setSelectionMode(QListView::Single);
    menu->insertItem("Delete", TONEWCONNECTION_DELETE);
    connect(menu, SIGNAL(activated(int)), this, SLOT(menuCallback(int)));
    Previous->setDisplayMenu(menu);
    connect(Previous, SIGNAL(selectionChanged()), this, SLOT(historySelection()));
    connect(Previous, SIGNAL(doubleClicked(QListViewItem *)), this, SLOT(historyConnect()));
    std::list<QCString> lst = toConnectionProvider::providers();
    int sel = 0, cur = 0;
    QCString provider = toTool::globalConfig(CONF_PROVIDER, DEFAULT_PROVIDER).latin1();
    for (std::list<QCString>::iterator i = lst.begin();i != lst.end();i++)
    {
        Provider->insertItem(QString::fromLatin1(*i));
        if (*i == provider)
            sel = cur;
        cur++;
    }
    if (cur == 0)
    {
        TOMessageBox::information(this,
                                  tr("No connection provider"),
                                  tr("No available connection provider, plugins probably missing"));
        reject();
        return ;
    }
    Provider->setCurrentItem(sel);
    changeProvider();
    processOptions(toTool::globalConfig(CONF_OPTIONS, DEFAULT_OPTIONS));

    QString host = toTool::globalConfig(CONF_HOST, DEFAULT_HOST);

    int portix = host.find(":");
    if (portix >= 0)
    {
        Host->lineEdit()->setText(host.mid(0, portix));
        Port->setValue(host.mid(portix + 1).toInt());
    }
    else
        Host->lineEdit()->setText(host);

    Username->setText(toTool::globalConfig(CONF_USER, DEFAULT_USER));
    Username->setFocus();

    bool pass = toTool::globalConfig(CONF_SAVE_PWD, DEFAULT_SAVE_PWD).isEmpty();
    if (pass)
        Password->setText(QString::fromLatin1(DEFAULT_PASSWORD));
    else
        Password->setText(toUnobfuscate(toTool::globalConfig(CONF_PASSWORD, DEFAULT_PASSWORD)));

    QString defdb = toTool::globalConfig(CONF_DATABASE, DEFAULT_DATABASE);
    Database->setEditable(true);

    {
        int maxHist = toTool::globalConfig(CONF_CONNECT_CURRENT, 0).toInt();
        Previous->setSorting( -1);
        QListViewItem *last = NULL;
        for (int i = 0;i < maxHist;i++)
        {
            QCString path = CONF_CONNECT_HISTORY;
            path += ":";
            path += QString::number(i).latin1();
            QCString tmp = path;
            tmp += CONF_USER;
            QString user = toTool::globalConfig(tmp, "");

            tmp = path;
            tmp += CONF_PASSWORD;
            QString passstr = (pass ? QString::fromLatin1(DEFAULT_PASSWORD) :
                               (toUnobfuscate(toTool::globalConfig(tmp, DEFAULT_PASSWORD))));

            tmp = path;
            tmp += CONF_HOST;
            QString host = toTool::globalConfig(tmp, DEFAULT_HOST);

            tmp = path;
            tmp += CONF_DATABASE;
            QString database = toTool::globalConfig(tmp, DEFAULT_DATABASE);

            tmp = path;
            tmp += CONF_PROVIDER;
            QString provider = toTool::globalConfig(tmp, DEFAULT_PROVIDER);

            tmp = path;
            tmp += CONF_OPTIONS;
            QString options = toTool::globalConfig(tmp, DEFAULT_OPTIONS);

            last = new QListViewItem(Previous, last, provider, host, database, user, passstr, options);
        }
    }
}

toNewConnection::~toNewConnection()
{}

void toNewConnection::changeProvider(void)
{
    try
    {
        std::list<QString> hosts = toConnectionProvider::hosts(Provider->currentText().latin1());
        QString current = Host->currentText();

        bool sqlNet = false;

        Host->clear();
        DefaultPort = 0;
        for (std::list<QString>::iterator i = hosts.begin();i != hosts.end();i++)
        {
            if ((*i).isEmpty())
                sqlNet = true;
            else if ((*i).startsWith(":"))
            {
                DefaultPort = (*i).mid(1).toInt();
            }
            else
                Host->insertItem(*i);
        }
        Port->setValue(DefaultPort);
        if (sqlNet)
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
        Host->lineEdit()->setText(current);

        for (std::list<QWidget *>::iterator k = OptionWidgets.begin();k != OptionWidgets.end();k++)
        {
            if ((*k)->isA("QCheckBox"))
            {
                QCheckBox *box = (QCheckBox *)(*k);
                Options[box->text()] = box->isChecked();
            }
            delete *k;
        }
        OptionWidgets.clear();

        std::list<QString> options = toConnectionProvider::options(Provider->currentText().latin1());
        for (std::list<QString>::iterator j = options.begin();j != options.end();j++)
        {
            if ((*j) == "-")
            {
                QFrame *frame = new QFrame(OptionGroup);
                frame->setFrameShape( QFrame::HLine );
                frame->setFrameShadow( QFrame::Sunken );
                OptionGroupLayout->addWidget(frame);
                frame->show();
                OptionWidgets.insert(OptionWidgets.end(), frame);
            }
            else
            {
                QString option = *j;
                bool defOn = false;
                if (option.startsWith("*"))
                {
                    defOn = true;
                    option = option.mid(1);
                }

                QCheckBox *ow = new QCheckBox(OptionGroup);
                ow->setText(option);
                if (Options.find(option) != Options.end())
                    ow->setChecked((*(Options.find(option))).second);
                else
                    ow->setChecked(defOn);
                OptionGroupLayout->addWidget(ow);
                ow->show();
                OptionWidgets.insert(OptionWidgets.end(), ow);
            }
        }
        if ( options.empty() )
            OptionGroup->hide();
        else
            OptionGroup->show();

    }
    catch (const QString &str)
    {
        Host->clear();
        toStatusMessage(str);
    }
}

void toNewConnection::changeHost(void)
{
    try
    {
        if (!Host->isHidden())
        {
            QString host;
            host = Host->currentText();
            std::list<QString> databases = toConnectionProvider::databases(Provider->currentText().latin1(),
                                           host,
                                           Username->text(),
                                           Password->text());
            QString current = Database->currentText();

            Database->clear();
            for (std::list<QString>::iterator i = databases.begin();i != databases.end();i++)
                Database->insertItem(*i);
            Database->lineEdit()->setText(current);
        }
    }
    catch (const QString &str)
    {
        Database->clear();
        toStatusMessage(str);
    }
}

void toNewConnection::processOptions(const QString &str)
{
    QStringList options = QStringList::split(",", str);
    std::map<QString, bool> values;
    for (unsigned int i = 0;i < options.count();i++)
    {
        QString val = options[i];
        if (val.startsWith("*"))
            values[val.mid(1)] = true;
        else
            values[val] = false;
    }
    for (std::list<QWidget *>::iterator k = OptionWidgets.begin();k != OptionWidgets.end();k++)
    {
        if ((*k)->isA("QCheckBox"))
        {
            QCheckBox *box = (QCheckBox *)(*k);
            if (values.find(box->text()) != values.end())
                box->setChecked(values[box->text()]);
        }
    }
}

toConnection *toNewConnection::makeConnection(void)
{
    try
    {
        toTool::globalSetConfig(CONF_PROVIDER, Provider->currentText());
        toTool::globalSetConfig(CONF_USER, Username->text());
        QString pass;
        QString host;
        if (!Host->isHidden())
            host = Host->currentText();

        QString optionstring;
        std::set
            <QString> options;

        for (std::list<QWidget *>::iterator k = OptionWidgets.begin();k != OptionWidgets.end();k++)
        {
            if ((*k)->isA("QCheckBox"))
            {
                if (!optionstring.isEmpty())
                    optionstring += ",";
                QCheckBox *box = (QCheckBox *)(*k);
                if (box->isChecked())
                {
                    optionstring += "*";
                    options.insert(box->text());
                }
                optionstring += box->text();
            }
        }
        toTool::globalSetConfig(CONF_OPTIONS, optionstring);

        std::list<QString> con = toMainWidget()->connections();
        for (std::list<QString>::iterator i = con.begin();i != con.end();i++)
        {
            try
            {
                toConnection &conn = toMainWidget()->connection(*i);
                if (conn.user() == Username->text() &&
                        conn.provider() == Provider->currentText().latin1() &&
                        conn.host() == host &&
                        conn.database() == Database->currentText())
                    return &conn;
            }
            catch (...)
            {}
        }
        if (Port->value() != 0 && Port->value() != DefaultPort)
            host += ":" + QString::number(Port->value());

        toTool::globalSetConfig(CONF_HOST, host);

        toConnection *retCon = new toConnection(Provider->currentText().latin1(),
                                                Username->text(),
                                                Password->text(),
                                                host,
                                                Database->currentText(),
                                                options);
        {
            for (QListViewItem *item = Previous->firstChild();item;item = item->nextSibling())
            {
                if (item->text(0) == Provider->currentText() &&
                        ((item->text(1) == host) || (item->text(1).isEmpty() && host.isEmpty())) &&
                        item->text(2) == Database->currentText() &&
                        item->text(3) == Username->text())
                {
                    delete item;
                    break;
                }
            }
        }

        if (!toTool::globalConfig(CONF_SAVE_PWD, DEFAULT_SAVE_PWD).isEmpty())
            pass = retCon->password();
        else
            pass = DEFAULT_PASSWORD;
        toTool::globalSetConfig(CONF_PASSWORD, toObfuscate(pass));
        toTool::globalSetConfig(CONF_DATABASE, Database->currentText());

        new QListViewItem(Previous, NULL,
                          Provider->currentText(),
                          host,
                          Database->currentText(),
                          Username->text(),
                          retCon->password(),
                          optionstring);
        historySave();
        return retCon;
    }
    catch (const QString &exc)
    {
        QString str = tr("Unable to connect to the database.\n");
        str.append(exc);
        TOMessageBox::information(this,
                                  tr("Unable to connect to the database"),
                                  str);
        return NULL;
    }
}

void toNewConnection::historySave(void)
{
    int siz = toTool::globalConfig(CONF_CONNECT_SIZE, DEFAULT_CONNECT_SIZE).toInt();
    int i = 0;
    int j = 0;

    for (QListViewItem *item = Previous->firstChild();i < siz;item = (item ? item = item->nextSibling() : 0))
    {
        QCString path = CONF_CONNECT_HISTORY;
        path += ":";
        path += QString::number(i).latin1();

        QCString tmp = path;
        tmp += CONF_PROVIDER;
        if (i < siz && item)
            toTool::globalSetConfig(tmp, item->text(0));
        else
            toTool::globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_HOST;
        if (i < siz && item)
            toTool::globalSetConfig(tmp, item->text(1));
        else
            toTool::globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_DATABASE;
        if (i < siz && item)
            toTool::globalSetConfig(tmp, item->text(2));
        else
            toTool::globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_USER;
        if (i < siz && item)
            toTool::globalSetConfig(tmp, item->text(3));
        else
            toTool::globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_PASSWORD;
        if (i < siz && item)
            toTool::globalSetConfig(tmp, toObfuscate(item->text(4)));
        else
            toTool::globalEraseConfig(tmp);

        tmp = path;
        tmp += CONF_OPTIONS;
        if (i < siz && item)
            toTool::globalSetConfig(tmp, item->text(5));
        else
            toTool::globalEraseConfig(tmp);

        i++;
        if (i < siz && item)
            j++;
    }
    toTool::globalSetConfig(CONF_CONNECT_CURRENT, QString::number(j));
    toTool::saveConfig();
}

void toNewConnection::historySelection(void)
{
    QListViewItem *item = Previous->selectedItem();
    if (item)
    {
        for (int i = 0;i < Provider->count();i++)
            if (Provider->text(i) == item->text(0))
            {
                Provider->setCurrentItem(i);
                break;
            }
        changeProvider();

        QString host = item->text(1);
        int portix = host.find(":");
        if (portix >= 0)
        {
            Host->lineEdit()->setText(host.mid(0, portix));
            Port->setValue(host.mid(portix + 1).toInt());
        }
        else
        {
            Host->lineEdit()->setText(host);
            Port->setValue(DefaultPort);
        }

        processOptions(item->text(5));

        Database->lineEdit()->setText(item->text(2));
        Username->setText(item->text(3));
        if (item->text(4) != DEFAULT_PASSWORD)
            Password->setText(item->text(4));
    }
}

void toNewConnection::historyConnect(void)
{
    bool ok = true;
    if (toTool::globalConfig(CONF_SAVE_PWD, DEFAULT_SAVE_PWD).isEmpty())
    {
        ok = false;
        QString name = QInputDialog::getText(tr("Enter password"),
                                             tr("Enter password to use for connection."),
                                             QLineEdit::Password, QString::null, &ok, this);
        if (ok)
            Password->setText(name);
    }
    if (ok)
        accept();
}
void toNewConnection::menuCallback(int cmd)
{
    switch (cmd)
    {
    case TONEWCONNECTION_DELETE:
        {
            historyDelete();
            break;
        }
    }
}

void toNewConnection::historyDelete()
{
    QListViewItem *item = Previous->selectedItem();
    if (item)
    {
        Previous->takeItem(item);
        delete item;
        historySave();
    }
}
