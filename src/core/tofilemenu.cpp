
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

#include "core/tofilemenu.h"
#include "core/toconnectionregistry.h"
#include "core/toconfiguration.h"
#include "core/tomainwindow.h"
#include "core/toglobalconfiguration.h"
#include "core/toglobalevent.h"
#include "ts_log/ts_log_utils.h"

#include "icons/connect.xpm"
#include "icons/disconnect.xpm"
#include "icons/fileopen.xpm"
#include "icons/filesave.xpm"
#include "icons/print.xpm"
#include "icons/tora.xpm"
#include "icons/up.xpm"
#include "icons/commit.xpm"
#include "icons/rollback.xpm"
#include "icons/stop.xpm"
#include "icons/refresh.xpm"

#include <QApplication>
#include <QtGui/QClipboard>
#include <QtCore/QMimeData>

toFileMenu::toFileMenu()
    : QMenu(tr("&File"), NULL)
{
    connect(this, SIGNAL(aboutToShow()), this, SLOT(slotAboutToShow()));

    newConnAct = new QAction(QPixmap(const_cast<const char**>(connect_xpm)), tr("&New Connection..."), this);
    newConnAct->setShortcut(Qt::CTRL + Qt::Key_G);
    newConnAct->setToolTip(tr("Create a new connection"));

    closeConnAct = new QAction(QPixmap(const_cast<const char**>(disconnect_xpm)), tr("&Close Connection"), this);
    closeConnAct->setToolTip(tr("Disconnect"));

    commitAct = new QAction(QPixmap(const_cast<const char**>(commit_xpm)), tr("&Commit Connection"), this);
    commitAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_C);
    commitAct->setToolTip(tr("Commit transaction"));
    commitAct->setDisabled(true);

    rollbackAct = new QAction(QPixmap(const_cast<const char**>(rollback_xpm)), tr("&Rollback Connection"), this);
    rollbackAct->setShortcut(Qt::CTRL + Qt::Key_Less);
    rollbackAct->setToolTip(tr("Rollback transaction"));
    rollbackAct->setDisabled(true);

    currentAct = new QAction(tr("&Current Connection"), this);
    currentAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_U);

    stopAct = new QAction(QPixmap(const_cast<const char**>(stop_xpm)), tr("Stop All Queries"), this);
    stopAct->setShortcut(Qt::CTRL + Qt::Key_J);

    refreshAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)), tr("Reread Object Cache"), this);

    openAct = new QAction(QPixmap(const_cast<const char**>(fileopen_xpm)), tr("&Open File..."), this);
    openAct->setShortcut(QKeySequence::Open);

    saveAct = new QAction(QPixmap(const_cast<const char**>(filesave_xpm)), tr("&Save File..."), this);
    saveAct->setShortcut(QKeySequence::Save);

    saveAsAct = new QAction(tr("Save &As..."), this);
    saveAsAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_W);

#ifdef TORA3_SESSION
    openSessionAct = new QAction(QPixmap(const_cast<const char**>(fileopen_xpm)), tr("Open Session..."), this);
    saveSessionAct = new QAction(QPixmap(const_cast<const char**>(filesave_xpm)), tr("Save Session..."), this);
    restoreSessionAct = new QAction(tr("Restore Last Session"), this);
    closeSessionAct = new QAction(tr("Close Session"), this);
#endif

    quitAct = new QAction(tr("&Quit"), this);

    addAction(newConnAct);
    addAction(closeConnAct);
    addSeparator();

    addAction(commitAct);
    addAction(rollbackAct);
    addAction(currentAct);
    addAction(stopAct);
    addAction(refreshAct);
    addSeparator();

    addAction(openAct);
    // add recentMenu after, setup later
    recentMenu = addMenu(tr("R&ecent Files"));

    addAction(saveAct);
    addAction(saveAsAct);
    addSeparator();

#ifdef TORA3_SESSION
    addAction(openSessionAct);
    addAction(saveSessionAct);
    addAction(restoreSessionAct);
    addAction(closeSessionAct);
    addSeparator();
#endif

    addAction(quitAct);

    updateRecent();

    connect(&toGlobalEventSingle::Instance(), SIGNAL(s_addRecentFile(QString const&)), this, SLOT(addRecentFile(QString const&)));
}

toFileMenu::~toFileMenu()
{
    //should be never called due to: Loki::NoDestroy
}

void toFileMenu::menuAboutToShow()
{
    bool hasconnection = !toConnectionRegistrySing::Instance().isEmpty();
    newConnAct->setEnabled(true);
    closeConnAct->setEnabled(hasconnection);
    //
    commitAct  ->setEnabled(hasconnection);
    rollbackAct->setEnabled(hasconnection);
    currentAct ->setEnabled(hasconnection);
    stopAct->setDisabled(true);
    refreshAct ->setEnabled(hasconnection && toConfigurationNewSingle::Instance().option(ToConfiguration::Global::CacheDiskBool).toBool());
    //
    openAct->setDisabled(true);
    updateRecent();
    saveAct->setDisabled(true);
    saveAsAct->setDisabled(true);
    //
#ifdef TORA3_SESSION
    openSessionAct->setDisabled(true);
    saveSessionAct->setDisabled(true);
    restoreSessionAct->setDisabled(true);
    closeSessionAct->setDisabled(true);
#endif
    //
    quitAct->setEnabled(true);
}

void toFileMenu::slotAboutToShow()
{
    menuAboutToShow();
}

void toFileMenu::updateRecent()
{
    QStringList files(toConfigurationNewSingle::Instance().option(ToConfiguration::Main::RecentFiles).toStringList());
    recentMenu->clear();

    int index = 1;
    QMutableListIterator<QString> i(files);
    i.toBack();
    QString f;
    while (i.hasPrevious())
    {
        f = i.previous();

        QFileInfo fi(f);
        if (!fi.exists())
        {
            i.remove();
            continue;
        }

        // store file name in tooltip. this is used later to
        // open the file, and is handy to know what file tora
        // is opening.
        QString caption = fi.fileName();
        if (index < 10)
            caption = "&" + QString::number(index++) + "  " + caption;

        QAction *r = new QAction(caption, this);
        r->setToolTip(f);
        recentMenu->addAction(r);
    }

    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Main::RecentFiles, QVariant(files));
}


void toFileMenu::addRecentFile(const QString &file)
{
    QStringList files(toConfigurationNewSingle::Instance().option(ToConfiguration::Main::RecentFiles).toStringList());
    int maxnum = toConfigurationNewSingle::Instance().option(ToConfiguration::Main::RecentMax).toInt();

    files.removeAll(file);
    if (files.count() >= maxnum)
        files.removeAt(0);
    files.append(file);
    toConfigurationNewSingle::Instance().setOption(ToConfiguration::Main::RecentFiles, QVariant(files));
}
