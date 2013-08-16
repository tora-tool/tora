
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
 *      these libraries.
 *
 *      You may link this product with any GPL'd Qt library.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "tests/test5window.h"

#include "core/toabout.h"
#include "core/toconf.h"

#include "core/toconnection.h"
#include "core/todocklet.h"

#include "core/toconfiguration.h"
#include "core/toraversion.h"

#include "core/toworkspace.h"
//#include "tools/toworksheet.h"
#include "tools/toplsqleditor.h"
#include "tools/toplsqltext.h"

#include <QtGui/QStatusBar>
#include <QtGui/QScrollArea>
#include <QtGui/QSplitter>

Test5Window::Test5Window(QString user, QString password, QString connect, QSet<QString>& options)
	: toMainWindow()
	, Workspace(toWorkSpaceSingle::Instance())
{
	Workspace.setParent(this);

	toConnection *oraCon = new toConnection(
		QString("Oracle"),
		user,
		password,
		"",
		connect,
		"",
		"",
		options);
	TLOG(0,toDecorator,__HERE__) << "Version: " 
				     << oraCon->version().toUtf8().constData() 
				     << std::endl;	
	addConnection(oraCon);
	{
		QSplitter *splitter;
		
		splitter = new QSplitter(this);
		splitter->setOrientation(Qt::Horizontal);

		QMdiArea *Workspace = new QMdiArea(splitter);
		Workspace->setActivationOrder(QMdiArea::CreationOrder);
		Workspace->setViewMode(QMdiArea::TabbedView); 
	
		//toWorksheet *w1 = new toWorksheet(Workspace, *oraCon);	
		//toWorksheet *w2 = new toWorksheet(Workspace, *oraCon);
		//MdiChild *mc1 = new MdiChild;
		//MdiChild *mc2 = new MdiChild;
		//Workspace->addSubWindow(mc1);
		//Workspace->addSubWindow(mc2);
		//Workspace->addSubWindow(w1);
		//Workspace->addSubWindow(w2);
		
		splitter->addWidget(&this->Workspace);
		
		m_describeAction = new QAction("&Describe", this);
		
		///createDocklets(); // Must be called after Wokspace is initialized

		//this->setCentralWidget(splitter);

		//statusbar = new QStatusBar(this);
		//this->setStatusBar(statusbar);

		createActions();
		createMenus();
		createToolBars();
		//createStatusBar();		
	}

	show();
	
	///createDockbars();   
	
	// for (ToolsRegistrySing::ObjectType::iterator i = ToolsRegistrySing::Instance().begin();
	//      i != ToolsRegistrySing::Instance().end();
	//      ++i)
	// {
	// 	if(i.value()->name() == "SQL Editor")
	// 		//if(i.value()->name() == "Security Manager")
	// 		//if(i.value()->name() == "Schema Browser")
	// 	{
	// 		i.value()->customSetup();
	// 		i.value()->createWindow();
	// 	}
	// }
}

void Test5Window::createDocklets()
{
	foreach(toDocklet *let, toDocklet::docklets())
		addDockWidget(Qt::LeftDockWidgetArea, let);
}
void Test5Window::moveDocklet(toDocklet *let, Qt::DockWidgetArea area)
{
	if(area == Qt::RightDockWidgetArea)
	{
		leftDockbar->removeDocklet(let);
		rightDockbar->addDocklet(let);
	}

	if(area == Qt::LeftDockWidgetArea)
	{
		rightDockbar->removeDocklet(let);
		leftDockbar->addDocklet(let);
	}
}
toDockbar* Test5Window::dockbar(toDocklet *let)
{
	if(rightDockbar->contains(let))
		return rightDockbar;
	return leftDockbar;
}
void Test5Window::createDockbars()
{
	leftDockbar = new toDockbar(Qt::LeftToolBarArea,
				    tr("Left Dockbar"),
				    this);
	addToolBar(Qt::LeftToolBarArea, leftDockbar);
	leftDockbar->hide();

	rightDockbar = new toDockbar(Qt::RightToolBarArea,
				     tr("Right Dockbar"),
				     this);
	addToolBar(Qt::RightToolBarArea, rightDockbar);
	rightDockbar->hide();

	// toDockbar keeps it's own settings, but just in case something
	// goes wrong, or a new setup, add any visible docklets to the
	// dockbar.

	foreach(toDocklet *let, toDocklet::docklets())
	{
		if(let->isVisible())
			moveDocklet(let, dockWidgetArea(let));

		connect(let,
			SIGNAL(dockletLocationChanged(toDocklet *, Qt::DockWidgetArea)),
			this,
			SLOT(moveDocklet(toDocklet *, Qt::DockWidgetArea)));
	}

	leftDockbar->restoreState(toConfigurationSingle::Instance().leftDockbarState());
	rightDockbar->restoreState(toConfigurationSingle::Instance().rightDockbarState());
}

void Test5Window::createActions()
{
	newAct = new QAction(QIcon(":/images/new.png"), tr("&New"), this);
	newAct->setShortcuts(QKeySequence::New);
	newAct->setStatusTip(tr("Create a new file"));
	connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));

	openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
	openAct->setShortcuts(QKeySequence::Open);
	openAct->setStatusTip(tr("Open an existing file"));
	// connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

	saveAct = new QAction(QIcon(":/images/save.png"), tr("&Save"), this);
	saveAct->setShortcuts(QKeySequence::Save);
	saveAct->setStatusTip(tr("Save the document to disk"));
	// connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

	saveAsAct = new QAction(tr("Save &As..."), this);
	saveAsAct->setShortcuts(QKeySequence::SaveAs);
	saveAsAct->setStatusTip(tr("Save the document under a new name"));
	// connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

//! [0]
	exitAct = new QAction(tr("E&xit"), this);
	exitAct->setShortcuts(QKeySequence::Quit);
	exitAct->setStatusTip(tr("Exit the application"));
	connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
//! [0]

	cutAct = new QAction(QIcon(":/images/cut.png"), tr("Cu&t"), this);
	cutAct->setShortcuts(QKeySequence::Cut);
	cutAct->setStatusTip(tr("Cut the current selection's contents to the "
				"clipboard"));
	// connect(cutAct, SIGNAL(triggered()), this, SLOT(cut()));

	copyAct = new QAction(QIcon(":/images/copy.png"), tr("&Copy"), this);
	copyAct->setShortcuts(QKeySequence::Copy);
	copyAct->setStatusTip(tr("Copy the current selection's contents to the "
				 "clipboard"));
	// connect(copyAct, SIGNAL(triggered()), this, SLOT(copy()));

	pasteAct = new QAction(QIcon(":/images/paste.png"), tr("&Paste"), this);
	pasteAct->setShortcuts(QKeySequence::Paste);
	pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
				  "selection"));
	// connect(pasteAct, SIGNAL(triggered()), this, SLOT(paste()));

	closeAct = new QAction(tr("Cl&ose"), this);
	closeAct->setStatusTip(tr("Close the active window"));
	connect(closeAct, SIGNAL(triggered()),
		&Workspace, SLOT(closeActiveSubWindow()));

	closeAllAct = new QAction(tr("Close &All"), this);
	closeAllAct->setStatusTip(tr("Close all the windows"));
	connect(closeAllAct, SIGNAL(triggered()),
		&Workspace, SLOT(closeAllSubWindows()));

	tileAct = new QAction(tr("&Tile"), this);
	tileAct->setStatusTip(tr("Tile the windows"));
	connect(tileAct, SIGNAL(triggered()),
			&Workspace, SLOT(tileSubWindows()));

	cascadeAct = new QAction(tr("&Cascade"), this);
	cascadeAct->setStatusTip(tr("Cascade the windows"));
	connect(cascadeAct, SIGNAL(triggered()),
			&Workspace, SLOT(cascadeSubWindows()));

	nextAct = new QAction(tr("Ne&xt"), this);
	nextAct->setShortcuts(QKeySequence::NextChild);
	nextAct->setStatusTip(tr("Move the focus to the next window"));
	connect(nextAct, SIGNAL(triggered()),
		&Workspace, SLOT(activateNextSubWindow()));

	previousAct = new QAction(tr("Pre&vious"), this);
	previousAct->setShortcuts(QKeySequence::PreviousChild);
	previousAct->setStatusTip(tr("Move the focus to the previous "
				     "window"));
	connect(previousAct, SIGNAL(triggered()),
		&Workspace, SLOT(activatePreviousSubWindow()));

	separatorAct = new QAction(this);
	separatorAct->setSeparator(true);

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void Test5Window::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(newAct);
	fileMenu->addAction(openAct);
	fileMenu->addAction(saveAct);
	fileMenu->addAction(saveAsAct);
	fileMenu->addSeparator();
	QAction *action = fileMenu->addAction(tr("Switch layout direction"));
	connect(action, SIGNAL(triggered()), this, SLOT(switchLayoutDirection()));
	fileMenu->addAction(exitAct);
}

void Test5Window::createToolBars()
{
	fileToolBar = addToolBar(tr("File"));
	fileToolBar->addAction(newAct);
	// fileToolBar->addAction(openAct);
	// fileToolBar->addAction(saveAct);

	// editToolBar = addToolBar(tr("Edit"));
	// editToolBar->addAction(cutAct);
	// editToolBar->addAction(copyAct);
	// editToolBar->addAction(pasteAct);

	// toolsToolBar = addToolBar(tr("Tools"));
	// ToolsRegistrySing::Instance().toolsToolbar(toolsToolBar);
}

void Test5Window::createStatusBar()
{
	statusBar()->showMessage(tr("Ready"));
}

void Test5Window::newFile()
{
	//QTimer::singleShot(0, this, SLOT(addTool()));
	addTool();
}

void Test5Window::addTool()
{
	//toWorksheet *w1 = new toWorksheet(&Workspace, *Connections.front());
	toPLSQL *w1 = new toPLSQL(&Workspace, *Connections.front());
	w1->showMaximized();
	//newsub->setWidget(w1);
	Workspace.addToolWidget(w1);

	//toWorksheet *w1 = new toWorksheet(Workspace, *Connections.front()); 
	//Workspace->addSubWindow(w1);
	//newsub->show();
	w1->show();
}
