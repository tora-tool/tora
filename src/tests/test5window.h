
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

#ifndef TROTL_TEST5_WINDOW_H
#define TROTL_TEST5_WINDOW_H

#include "core/tomainwindow.h"
//#include "core/toworksheet.h"
#include "core/todockbar.h"
#include "core/tobackgroundlabel.h"

#include <QtCore/QObject>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMenuBar>
#include <QtCore/QSet>
#include <QTextEdit>

class toWorkSpace;
class toConnection;

class Test5Window : public toMainWindow
{
        Q_OBJECT;

        void createActions();
        void createMenus();
        void createToolBars();
        void createStatusBar();
        QAction *newAct;
        QAction *openAct;
        QAction *saveAct;
        QAction *saveAsAct;
        QAction *exitAct;
        QAction *cutAct;
        QAction *copyAct;
        QAction *pasteAct;
        QAction *closeAct;
        QAction *closeAllAct;
        QAction *tileAct;
        QAction *cascadeAct;
        QAction *nextAct;
        QAction *previousAct;
        QAction *separatorAct;
        QAction *aboutAct;
        QAction *aboutQtAct;
        QMenu *fileMenu;
        QMenu *editMenu;
        QMenu *windowMenu;
        QMenu *helpMenu;
        QToolBar *fileToolBar;
        QToolBar *editToolBar;
        QToolBar *toolsToolBar;

        toWorkSpace &Workspace;

        toDockbar *leftDockbar,*rightDockbar;

        QAction * m_describeAction;
    public:
        Test5Window(QString, QString, QString, QSet<QString>&);

        void createDockbars();
        void createDocklets();
        void moveDocklet(toDocklet *let, Qt::DockWidgetArea area);

        virtual void addCustomMenu(QMenu *) {};
        virtual toDockbar* dockbar(toDocklet *let);

    private slots:
        void newFile();
        void addTool();
};


#endif
