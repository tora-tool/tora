
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

//
// C++ Implementation: toTool, toToolWidget, toConnectionWidget
//

#include "core/totool.h"
#include "core/utils.h"
#include "core/tologger.h"

#include "core/toconnection.h"
#include "core/toconnectionregistry.h"
#include "widgets/toworkspace.h"
#include "core/tomainwindow.h"
#include "core/toconfiguration.h"
#include "core/toglobalevent.h"
#include "core/toglobalconfiguration.h"

#include <QToolBar>
#include <QMenu>
#include <QBoxLayout>
#include <QAction>
#include <QComboBox>
#include <QtCore/QtDebug>
#include <QSpinBox>
#include <QLineEdit>
#include <QCheckBox>

// A little magic to get lrefresh to work and get a check on qApp
#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

QString const& toTool::name() const
{
    return Name;
}

QString const& toTool::key() const
{
    return Key;
}

const int toTool::priority() const
{
    return Priority;
}

const char **toTool::pictureXPM(void)
{
    return (const char **) NULL;  // compiler warning
}

toTool::toTool(int priority, const char *name)
    : Name(name),
      Priority(priority),
      ButtonPicture(0),
      toolAction(0)
{
    Key = QString::asprintf("%05d%s", priority, name);
    ToolsRegistrySing::Instance().insert(Key, this);
}

toTool::~toTool()
{
    ToolsRegistrySing::Instance().erase(ToolsRegistrySing::Instance().find(Key));
    delete ButtonPicture;
}

const QPixmap *toTool::toolbarImage()
{
    if (!ButtonPicture)
    {
        const char **picture = pictureXPM();
        if (picture)
            ButtonPicture = new QPixmap(picture);
    }
    return ButtonPicture;
}

toToolWidget* toTool::createWindow()
{
    toToolWidget *newWin = NULL;
    try
    {
        toConnection &conn = toConnectionRegistrySing::Instance().currentConnection();

        if (!canHandle(conn))
            throw QString("The tool %1 doesn't support the current database").arg(QString(name()));

        newWin = toolWindow(NULL, conn);
        // Some tools like toOutput return NULL if there is one window for session opened
        if (newWin)
            toWorkSpaceSingle::Instance().addToolWidget(newWin);
    }
    TOCATCH;

    return newWin;
}


bool toTool::canHandle(const toConnection &conn)
{
#ifdef TORA3_CONNECTION
    return (conn.providerIs("Oracle"));
#endif
    return true;
}

QAction* toTool::getAction(void)
{
    if (toolAction)
        return toolAction;

    toolAction = new QAction(toMainWindow::lookup());
    if (toolbarImage())
        toolAction->setIcon(QIcon(QPixmap(pictureXPM())));
    if (menuItem())
        toolAction->setText(menuItem());
    if (toolbarTip())
        toolAction->setToolTip(toolbarTip());

    connect(toolAction, SIGNAL(triggered()), this, SLOT(createWindow()));

    return toolAction;
}

void toTool::enableAction(bool en)
{
    getAction()->setEnabled(en);
}

void toTool::enableAction(toConnection const& conn)
{
    enableAction(canHandle(conn));
}

void toTool::setActionVisible(toConnection &conn)
{
    getAction()->setVisible(canHandle(conn));
}

void toTool::setActionVisible(bool en)
{
    getAction()->setVisible(en);
}

QToolBar* toToolsRegistry::toolsToolbar(QToolBar *toolbar) const
{
    if (!toolbar)
        return toolbar;

    try
    {
        int lastPriorityPix = 0;
        QMap<QString, QVariant> cfgTools = toConfigurationNewSingle::Instance().option(ToConfiguration::Main::ToolsMap).toMap();

        for (super::const_iterator i = super::begin(); i != super::end(); ++i)
        {
            toTool *pTool = i.value();
            QAction *toolAct = pTool->getAction();
            const QPixmap *pixmap = pTool->toolbarImage();

            // set the tools for the first run
            if (!cfgTools.contains(i.key()))
                cfgTools[i.key()] = QVariant((bool)true);
            // only enabled tools are set
            if (cfgTools[i.key()].toBool() == false)
                continue;

            int priority = pTool->priority();
            if (priority / 100 != lastPriorityPix / 100 && pixmap)
            {
                toolbar->addSeparator();
                lastPriorityPix = priority;
            }

            if (pixmap)
                toolbar->addAction(toolAct);

        } // for tools
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Main::ToolsMap, cfgTools);
    }
    TOCATCH;
    return toolbar;
}

QMenu* toToolsRegistry::toolsMenu(QMenu *menu) const
{
    if (!menu)
        return menu;

    try
    {
        int lastPriorityMenu = 0;

        QMap<QString, QVariant> cfgTools = toConfigurationNewSingle::Instance().option(ToConfiguration::Main::ToolsMap).toMap();

        for (super::const_iterator i = super::begin(); i != super::end(); ++i)
        {
            toTool *pTool = i.value();
            QAction *toolAct = pTool->getAction();
            const QPixmap *pixmap = pTool->toolbarImage();
            const char *menuName = pTool->menuItem();

            // set the tools for the first run
            if (!cfgTools.contains(i.key()))
                cfgTools[i.key()] = QVariant((bool)true);
            // only enabled tools are set
            if (cfgTools[i.key()].toBool() == false)
                continue;

            int priority = pTool->priority();
            if (priority / 100 != lastPriorityMenu / 100 && menuName)
            {
                menu->addSeparator();
                lastPriorityMenu = priority;
            }

            if (menuName)
                menu->addAction(toolAct);
        } // for tools
        toConfigurationNewSingle::Instance().setOption(ToConfiguration::Main::ToolsMap, cfgTools);
    }
    TOCATCH;
    return menu;
}
