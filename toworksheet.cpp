//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifdef TO_KDE
#include <kfiledialog.h>
#include <kmenubar.h>
#endif

#include <qcombobox.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qmenubar.h>
#include <qworkspace.h>
#include <qgrid.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qtooltip.h>
#include <qlineedit.h>
#include <qfiledialog.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qheader.h>
#include <qfileinfo.h>
#include <qinputdialog.h>

#include "totool.h"
#include "toresultplan.h"
#include "toworksheet.h"
#include "toresultview.h"
#include "toresultresources.h"
#include "tohighlightedtext.h"
#include "toparamget.h"
#include "toresultlong.h"
#include "toresultstats.h"
#include "toresultcols.h"
#include "toconf.h"
#include "tochangeconnection.h"
#include "toworksheetsetupui.h"
#include "tosession.h"
#include "toresultbar.h"
#include "tovisualize.h"
#include "toworksheetstatistic.h"

#include "toworksheet.moc"
#include "toworksheetsetupui.moc"

#include "icons/toworksheet.xpm"
#include "icons/refresh.xpm"
#include "icons/execute.xpm"
#include "icons/executestep.xpm"
#include "icons/executeall.xpm"
#include "icons/eraselog.xpm"
#include "icons/stop.xpm"
#include "icons/clock.xpm"
#include "icons/describe.xpm"
#include "icons/compile.xpm"
#include "icons/previous.xpm"
#include "icons/filesave.xpm"

#define TO_ID_STATISTICS		(toMain::TO_TOOL_MENU_ID+ 0)
#define TO_ID_STOP			(toMain::TO_TOOL_MENU_ID+ 1)

#define CONF_AUTO_SAVE   "AutoSave"
#define CONF_CHECK_SAVE  "CheckSave"
#define CONF_AUTO_LOAD   "AutoLoad"
#define CONF_LOG_AT_END  "LogAtEnd"
#define CONF_LOG_MULTI   "LogMulti"
#define CONF_PLSQL_PARSE "PLSQLParse"
#define CONF_STATISTICS	 "Statistics"
#define CONF_TIMED_STATS "TimedStats"
#define CONF_NUMBER	 "Number"
#define CONF_MOVE_TO_ERR "MoveToError"
#define CONF_HISTORY	 "History"

static struct {
  int Pos;
  char *Start;
  bool WantEnd;
  bool WantSemi;
  bool CloseBlock;
  bool Comment;
  bool BeforeCode;
  bool StartCode;
  bool NoBinds;
} Blocks[] = { { 0,"begin",	true ,false,false,false,true ,true ,false},
	       { 0,"if",	true ,false,false,false,false,false,false},
	       { 0,"loop",	true ,false,false,false,false,false,false},
	       { 0,"while",	true ,false,false,false,false,false,false},
	       { 0,"declare",	false,false,false,false,true ,true ,false},
	       { 0,"create",	false,false,false,false,true ,false,false},
	       { 0,"package",	true ,false,false,false,false,true ,false},
	       { 0,"procedure",	false,false,false,false,false,true ,false},
	       { 0,"function",	false,false,false,false,false,true ,false},
	       { 0,"trigger",   false,false,false,false,false,true ,false},
	       { 0,"end",	false,true ,true ,false,false,false,false},
	       { 0,"rem",	false,false,false,true ,false,false,false},
	       { 0,"store",	false,false,false,true ,false,false,false},
	       { 0,"spool",	false,false,false,true ,false,false,false},
	       { 0,"prompt",	false,false,false,true ,false,false,false},
	       { 0,NULL,	false,false,false,false,false,false,false}
};

class toWorksheetSetup : public toWorksheetSetupUI, public toSettingTab
{ 
  toTool *Tool;

public:
  toWorksheetSetup(toTool *tool,QWidget* parent = 0,const char* name = 0)
    : toWorksheetSetupUI(parent,name),toSettingTab("worksheet.html#preferences"),Tool(tool)
  {
    if (!tool->config(CONF_AUTO_SAVE,"").isEmpty())
      AutoSave->setChecked(true);
    if (!tool->config(CONF_CHECK_SAVE,"Yes").isEmpty())
      CheckSave->setChecked(true);
    if (!tool->config(CONF_LOG_AT_END,"Yes").isEmpty())
      LogAtEnd->setChecked(true);
    if (!tool->config(CONF_LOG_MULTI,"Yes").isEmpty())
      LogMulti->setChecked(true);
    if (!tool->config(CONF_PLSQL_PARSE,"Yes").isEmpty())
      PLSQLParse->setChecked(true);
    MoveToError->setChecked(!tool->config(CONF_MOVE_TO_ERR,"Yes").isEmpty());
    if (!tool->config(CONF_STATISTICS,"").isEmpty())
      Statistics->setChecked(true);
    TimedStatistics->setChecked(!tool->config(CONF_TIMED_STATS,"Yes").isEmpty());
    History->setChecked(!tool->config(CONF_HISTORY,"").isEmpty());
    if (!tool->config(CONF_NUMBER,"Yes").isEmpty())
      DisplayNumber->setChecked(true);
    DefaultFile->setText(tool->config(CONF_AUTO_LOAD,""));
  }
  virtual void saveSetting(void)
  {
    if (AutoSave->isChecked())
      Tool->setConfig(CONF_AUTO_SAVE,"Yes");
    else
      Tool->setConfig(CONF_AUTO_SAVE,"");
    if (CheckSave->isChecked())
      Tool->setConfig(CONF_CHECK_SAVE,"Yes");
    else
      Tool->setConfig(CONF_CHECK_SAVE,"");
    if (LogAtEnd->isChecked())
      Tool->setConfig(CONF_LOG_AT_END,"Yes");
    else
      Tool->setConfig(CONF_LOG_AT_END,"");
    if (LogMulti->isChecked())
      Tool->setConfig(CONF_LOG_MULTI,"Yes");
    else
      Tool->setConfig(CONF_LOG_MULTI,"");
    if (PLSQLParse->isChecked())
      Tool->setConfig(CONF_PLSQL_PARSE,"Yes");
    else
      Tool->setConfig(CONF_PLSQL_PARSE,"");
    Tool->setConfig(CONF_MOVE_TO_ERR,MoveToError->isChecked()?"Yes":"");
    Tool->setConfig(CONF_STATISTICS,Statistics->isChecked()?"Yes":"");
    Tool->setConfig(CONF_HISTORY,History->isChecked()?"Yes":"");
    Tool->setConfig(CONF_TIMED_STATS,TimedStatistics->isChecked()?"Yes":"");
    Tool->setConfig(CONF_NUMBER,DisplayNumber->isChecked()?"Yes":"");
    Tool->setConfig(CONF_AUTO_LOAD,DefaultFile->text());
  }
public slots:
  void chooseFile(void)
  {
    QString str=toOpenFilename(DefaultFile->text(),"*.sql\n*.txt",this);
    if (!str.isEmpty())
      DefaultFile->setText(str);
  }
};

class toWorksheetTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return toworksheet_xpm; }
public:
  toWorksheetTool()
    : toTool(1,"SQL Worksheet")
  { }
  virtual const char *menuItem()
  { return "SQL Worksheet"; }
  virtual QWidget *toolWindow(QWidget *main,toConnection &connection)
  {
    return new toWorksheet(main,connection);
  }
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toWorksheetSetup(this,parent);
  }
  virtual bool canHandle(toConnection &conn)
  { return true; }
};

static toWorksheetTool WorksheetTool;

class toWorksheetText : public toHighlightedText {
  toWorksheet *Worksheet;
public:
  toWorksheetText(toWorksheet *worksheet,QWidget *parent,const char *name=NULL)
    : toHighlightedText(parent,name),Worksheet(worksheet)
  { }
  /** Reimplemented for internal reasons.
   */
  virtual void keyPressEvent(QKeyEvent *e)
  {
    if (e->state()==ControlButton&&
	e->key()==Key_Return) {
      Worksheet->execute();
      e->accept();
    } else if (e->state()==0&&
	       e->key()==Key_F8) {
      Worksheet->executeAll();
      e->accept();
    } else if (e->state()==0&&
	       e->key()==Key_F9) {
      Worksheet->executeStep();
      e->accept();
    } else if (e->state()==ShiftButton&&
	       e->key()==Key_F9) {
      Worksheet->executeNewline();
      e->accept();
    } else if (e->state()==0&&
	       e->key()==Key_F7) {
      Worksheet->executeSaved();
      e->accept();
    } else if (e->state()==0&&
	       e->key()==Key_F4) {
      Worksheet->describe();
      e->accept();
    } else if (e->state()==AltButton&&
	       e->key()==Key_Up) {
      Worksheet->executePreviousLog();
      e->accept();
    } else if (e->state()==AltButton&&
	       e->key()==Key_Down) {
      Worksheet->executeNextLog();
      e->accept();
    } else {
      toHighlightedText::keyPressEvent(e);
    }
  }
  virtual void editOpen(void)
  {
    toHighlightedText::editOpen();
    QFileInfo file(filename());
    toToolCaption(Worksheet,WorksheetTool.name()+" "+file.fileName());
  }
};

void toWorksheet::viewResources(void)
{
  try {
    QString address=toSQLToAddress(connection(),QueryString);

    Resources->changeParams(address);
  } TOCATCH
}

#define TOWORKSHEET "toWorksheet:"

void toWorksheet::setup(bool autoLoad)
{
  toConnection &connection=toWorksheet::connection();
  QToolBar *toolbar=toAllocBar(this,"SQL worksheet",connection.description());

  new QToolButton(QPixmap((const char **)execute_xpm),
		  "Execute current statement",
		  "Execute current statement",
		  this,SLOT(execute(void)),
		  toolbar);
  new QToolButton(QPixmap((const char **)executeall_xpm),
		  "Execute all statements",
		  "Execute all statements",
		  this,SLOT(executeAll(void)),
		  toolbar);
  new QToolButton(QPixmap((const char **)executestep_xpm),
		  "Step through statements",
		  "Step through statements",
		  this,SLOT(executeStep(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Reexecute Last Statement",
		  "Reexecute Last Statement",
		  this,SLOT(refresh(void)),
		  toolbar);

  LastLine=LastOffset=-1;
  LastID=0;

  if (Light) {
    Editor=new toWorksheetText(this,this);
    Current=Result=new toResultLong(this);
    Result->hide();
    connect(Result,SIGNAL(done(void)),this,SLOT(queryDone(void)));
    connect(Result,SIGNAL(firstResult(const QString &,const toConnection::exception &)),
	    this,SLOT(addLog(const QString &,const toConnection::exception &)));
    ResultTab=NULL;
    Plan=NULL;
    CurrentTab=NULL;
    Resources=NULL;
    Statistics=NULL;
    Logging=NULL;
    LastLogItem=NULL;
    StatisticButton=NULL;
    StatTab=NULL;
    Columns=NULL;
    Refresh=NULL;
    ToolMenu=NULL;
    Visualize=NULL;
    WaitChart=IOChart=NULL;
    toolbar->addSeparator();
    new QToolButton(QPixmap((const char **)describe_xpm),
		    "Describe under cursor",
		    "Describe under cursor",
		    this,SLOT(describe(void)),
		    toolbar);
    StopButton=new QToolButton(QPixmap((const char **)stop_xpm),
			       "Stop execution",
			       "Stop execution",
			       Result,SLOT(stop(void)),
			       toolbar);
    StopButton->setEnabled(false);
    toolbar->setStretchableWidget(Started=new QLabel(toolbar));
    Started->setAlignment(AlignRight|AlignVCenter|ExpandTabs);
  } else {
    QSplitter *splitter=new QSplitter(Vertical,this);

    Editor=new toWorksheetText(this,splitter);
    ResultTab=new QTabWidget(splitter);
    QVBox *box=new QVBox(ResultTab);
    ResultTab->addTab(box,"&Result");

    Current=Result=new toResultLong(box);
    connect(Result,SIGNAL(done(void)),this,SLOT(queryDone(void)));
    connect(Result,SIGNAL(firstResult(const QString &,const toConnection::exception &)),
	    this,SLOT(addLog(const QString &,const toConnection::exception &)));

    Columns=new toResultCols(box);
    Columns->hide();

    ResultTab->setTabEnabled(Columns,false);
    Visualize=new toVisualize(Result,ResultTab);
    ResultTab->addTab(Visualize,"&Visualize");
    Plan=new toResultPlan(ResultTab);
    ResultTab->addTab(Plan,"E&xecution plan");
    Resources=new toResultResources(ResultTab);
    ResultTab->addTab(Resources,"&Information");
    StatTab=new QVBox(ResultTab);
    {
      QToolBar *stattool=toAllocBar(StatTab,"Worksheet Statistics",connection.description());
      new QToolButton(QPixmap((const char **)filesave_xpm),
		      "Save statistics for later analysis",
		      "Save statistics for later analysis",
		      this,SLOT(saveStatistics(void)),
		      stattool);
      stattool->setStretchableWidget(new QLabel(stattool));
    }
    splitter=new QSplitter(Horizontal,StatTab);
    Statistics=new toResultStats(true,splitter);
    Statistics->setTabWidget(ResultTab);
    WaitChart=new toResultBar(splitter);
    try {
      toSQL sql=toSQL::sql(TO_SESSION_WAIT);
      WaitChart->setSQL(sql);
    } catch(...) {
    }
    WaitChart->setTitle("Wait states");
    WaitChart->setYPostfix("ms/s");
    WaitChart->setSamples(-1);
    connect(Statistics,SIGNAL(sessionChanged(const QString &)),
	    WaitChart,SLOT(changeParams(const QString &)));
    IOChart=new toResultBar(splitter);
    try {
      toSQL sql=toSQL::sql(TO_SESSION_IO);
      IOChart->setSQL(sql);
    } catch(...) {
    }
    IOChart->setTitle("I/O");
    IOChart->setYPostfix("blocks/s");
    IOChart->setSamples(-1);
    connect(Statistics,SIGNAL(sessionChanged(const QString &)),
	    IOChart,SLOT(changeParams(const QString &)));
    ResultTab->addTab(StatTab,"&Statistics");
    ResultTab->setTabEnabled(StatTab,false);

    Logging=new toListView(ResultTab);
    ResultTab->addTab(Logging,"&Logging");
    Logging->addColumn("SQL");
    Logging->addColumn("Result");
    Logging->addColumn("Timestamp");
    Logging->addColumn("Duration");
    Logging->setColumnAlignment(3,AlignRight);
    LastLogItem=NULL;

    toolbar->addSeparator();
    new QToolButton(QPixmap((const char **)describe_xpm),
		    "Describe under cursor",
		    "Describe under cursor",
		    this,SLOT(describe(void)),
		    toolbar);
    StopButton=new QToolButton(QPixmap((const char **)stop_xpm),
			       "Stop execution",
			       "Stop execution",
			       Result,SLOT(stop(void)),
			       toolbar);
    StopButton->setEnabled(false);
    toolbar->addSeparator();
    new QToolButton(QPixmap((const char **)eraselog_xpm),
		    "Clear execution log",
		    "Clear execution log",
		    this,SLOT(eraseLogButton(void)),
		    toolbar);

    toolbar->addSeparator();
    StatisticButton=new QToolButton(toolbar);
    StatisticButton->setToggleButton(true);
    StatisticButton->setIconSet(QIconSet(QPixmap((const char **)clock_xpm)));
    connect(StatisticButton,SIGNAL(toggled(bool)),this,SLOT(enableStatistic(bool)));
    QToolTip::add(StatisticButton,"Gather session statistic of execution");
    new QLabel("Refresh ",toolbar);
    Refresh=toRefreshCreate(toolbar);
    connect(Refresh,SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));
    connect(StatisticButton,SIGNAL(toggled(bool)),Refresh,SLOT(setEnabled(bool)));
    Refresh->setEnabled(false);
    Refresh->setFocusPolicy(NoFocus);

    toolbar->addSeparator();

    SavedButton=new QToolButton(QPixmap((const char **)compile_xpm),
				"Run current saved SQL",
				"Run current saved SQL",
				this,SLOT(executeSaved(void)),
				toolbar);
    SavedMenu=new QPopupMenu(SavedButton);
    SavedButton->setPopup(SavedMenu);
    SavedButton->setPopupDelay(0);
    connect(SavedMenu,SIGNAL(aboutToShow()),this,SLOT(showSaved()));
    connect(SavedMenu,SIGNAL(activated(int)),this,SLOT(executeSaved(int)));
    new QToolButton(QPixmap((const char **)previous_xpm),
		    "Save last SQL",
		    "Save last SQL",
		    this,SLOT(saveLast(void)),
		    toolbar);

    toolbar->setStretchableWidget(Started=new QLabel(toolbar));
    Started->setAlignment(AlignRight|AlignVCenter|ExpandTabs);
    new toChangeConnection(toolbar);

    connect(ResultTab,SIGNAL(currentChanged(QWidget *)),
	    this,SLOT(changeResult(QWidget *)));

    if (autoLoad) {
      Editor->setFilename(WorksheetTool.config(CONF_AUTO_LOAD,""));
      if (!Editor->filename().isEmpty()) {
	try {
	  QCString data=toReadFile(Editor->filename());
	  Editor->setText(QString::fromLocal8Bit(data));
	  Editor->setEdited(false);
	} TOCATCH
      }
    }

    ToolMenu=NULL;
    connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	    this,SLOT(windowActivated(QWidget *)));

    if (connection.provider()=="Oracle") {
      if (!WorksheetTool.config(CONF_STATISTICS,"").isEmpty()) {
	show();
	StatisticButton->setOn(true);
      }
    } else {
      StatisticButton->setEnabled(false);
    }

#if 0
    setTabOrder(Editor,ResultTab);
    setTabOrder(ResultTab,Result);
    setTabOrder(Result,Columns);
    setTabOrder(Columns,Refresh);
#endif

    connect(this,SIGNAL(connectionChange()),this,SLOT(connectionChanged()));
  }
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
  setFocusProxy(Editor);
}

toWorksheet::toWorksheet(QWidget *main,toConnection &connection,bool autoLoad)
  : toToolWidget(WorksheetTool,"worksheet.html",main,connection),Light(false)
{
  setup(autoLoad);
}

toWorksheet::toWorksheet(QWidget *main,const char *name,toConnection &connection)
  : toToolWidget(WorksheetTool,"worksheetlight.html",main,connection,name),Light(true)
{
  setup(false);
}

void toWorksheet::changeRefresh(const QString &str)
{
  if (!Light&&StopButton->isEnabled()&&StatisticButton->isOn())
    toRefreshParse(timer(),str);
}

void toWorksheet::windowActivated(QWidget *widget)
{
  if (Light)
    return;

  QWidget *w=this;
  while(w&&w!=widget) {
    w=w->parentWidget();
  }

  if (widget==w) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)execute_xpm),
			   "&Execute Current",this,SLOT(execute(void)),
			   CTRL+Key_Return);
      ToolMenu->insertItem(QPixmap((const char **)executeall_xpm),
			   "Execute &All",this,SLOT(executeAll(void)),
			   Key_F8);
      ToolMenu->insertItem(QPixmap((const char **)executestep_xpm),
			   "Execute &Next",this,SLOT(executeStep(void)),
			   Key_F9);
      ToolMenu->insertItem("Execute &Newline Separated",this,
			   SLOT(executeNewline(void)),SHIFT+Key_F9);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),
			   "&Reexecute Last Statement",this,SLOT(refresh(void)),
			   Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)describe_xpm),
			   "&Describe Under Cursor",this,SLOT(describe(void)),
			   Key_F4);
      ToolMenu->insertItem("&Enable Statistics",this,SLOT(toggleStatistic(void)),
			   0,TO_ID_STATISTICS);
      ToolMenu->insertItem(QPixmap((const char **)stop_xpm),
			   "&Stop Execution",Result,SLOT(stop(void)),
			   0,TO_ID_STOP);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem("Execute Saved SQL",
			   this,SLOT(executeSaved()),
			   Key_F7);
      ToolMenu->insertItem("Select Saved SQL",
			   this,SLOT(selectSaved()),
			   CTRL+SHIFT+Key_S);
      ToolMenu->insertItem(QPixmap((const char **)previous_xpm),
			   "Save last SQL",
			   this,SLOT(saveLast()));
      ToolMenu->insertItem("Edit Saved SQL",
			   this,SLOT(editSaved()));
      ToolMenu->insertSeparator();
      ToolMenu->insertItem("Previous Log Entry",this,SLOT(executePreviousLog()),
			   ALT+Key_Up);
      ToolMenu->insertItem("Next Log Entry",this,SLOT(executeNextLog()),
			   ALT+Key_Down);
      ToolMenu->insertItem(QPixmap((const char **)eraselog_xpm),
			   "Erase &Log",this,SLOT(eraseLogButton(void)));

      toMainWidget()->menuBar()->insertItem("W&orksheet",ToolMenu,-1,toToolMenuIndex());
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,StopButton->isEnabled());
      toMainWidget()->menuBar()->setItemChecked(TO_ID_STATISTICS,
						StatisticButton->isOn());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toWorksheet::connectionChanged(void)
{
  if (connection().provider()=="Oracle") {
    StatisticButton->setEnabled(true);
  } else {
    StatisticButton->setEnabled(false);
  }
}

bool toWorksheet::checkSave(bool input)
{
  if (Light)
    return true;
  if (Editor->edited()) {
    if(WorksheetTool.config(CONF_AUTO_SAVE,"").isEmpty()||
       Editor->filename().isEmpty()) {
      if (!WorksheetTool.config(CONF_CHECK_SAVE,"Yes").isEmpty()) {
	if (input) {
	  QString str("Save changes to worksheet for ");
	  str.append(connection().description());
	  int ret=TOMessageBox::information(this,
					    "Save file",
					    str,
					    "&Yes","&No","&Cancel",0,2);
	  if (ret==1)
	    return true;
	  else if (ret==2)
	    return false;
	} else
	  return true;
      } else
	return true;
      if (Editor->filename().isEmpty()&&input)
	Editor->setFilename(toSaveFilename(Editor->filename(),
					   "*.sql\n*.txt",this));
      if (Editor->filename().isEmpty())
	return false;	
    }
    if (!toWriteFile(Editor->filename(),Editor->text()))
      return false;
    Editor->setEdited(false);
  }
  return true;
}

bool toWorksheet::close(bool del)
{
  if (checkSave(true)) {
    Result->stop();
    return QVBox::close(del);
  }
  return false;
}

toWorksheet::~toWorksheet()
{
  checkSave(false);
  eraseLogButton();
}

#define LARGE_BUFFER 4096

void toWorksheet::changeResult(QWidget *widget)
{
  CurrentTab=widget;
  if (QueryString.length()) {
    if (CurrentTab==Plan)
      Plan->query(QueryString);
    else if (CurrentTab==Resources)
      viewResources();
    else if (CurrentTab==Statistics&&Result->running())
      Statistics->refreshStats(false);
  }
}

void toWorksheet::refresh(void)
{
  if (!QueryString.isEmpty()) {
    query(QueryString,false);
    StopButton->setEnabled(true);
    Poll.start(1000);
    toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,true);
    if (Light)
      return;
    if (CurrentTab==Visualize)
      Visualize->display();
    else if (CurrentTab==Plan)
      Plan->query(QueryString);
    else if (CurrentTab==Resources)
      viewResources();
  }
}

static QString unQuote(const QString &str)
{
  if (str.at(0)=='\"'&&str.at(str.length()-1)=='\"')
    return str.left(str.length()-1).right(str.length()-2);
  return str.upper();
}

bool toWorksheet::describe(const QString &query)
{
  QRegExp white("[ \r\n\t.]+");
  QStringList part=QStringList::split(white,query);
  if (part[0].upper()=="DESC"||
      part[0].upper()=="DESCRIBE") {
    if (Light)
      return true;
    if (toIsOracle(connection())) {
      if (part.count()==2) {
	Columns->changeParams(unQuote(part[1]));
      } else if (part.count()==3) {
	Columns->changeParams(unQuote(part[1]),unQuote(part[2]));
      } else
	throw QString("Wrong number of parameters for describe");
    } else if (connection().provider()=="MySQL") {
      if (part.count()==2) {
	Columns->changeParams(part[1]);
      } else
	throw QString("Wrong number of parameters for describe");
    }
    Current->hide();
    Columns->show();
    Current=Columns;
    return true;
  } else {
    if (Light)
      return false;
    QWidget *curr=ResultTab->currentPage();
    Current->hide();
    Result->show();
    Current=Result;
    if (curr==Columns)
      ResultTab->showPage(Result);
    return false;
  }
}

void toWorksheet::query(const QString &str,bool direct)
{
  Result->stop();

  QRegExp strq("'[^']*'");
  QString chk=str.lower();
  chk.replace(strq," ");
  bool code=false;
  int pos=chk.find("end",0);
  while (pos>0) {  // Ignore position 0, since that isn't a block anyway
    QChar c=chk[pos-1];
    QChar ec=chk[pos+3];
    if (!toIsIdent(c)&&!toIsIdent(ec)) {
      code=true;
      break;
    }
    pos=chk.find("end",pos+1);
  }
  QueryString=str;
  if (!code&&QueryString.length()>0&&QueryString.at(QueryString.length()-1)==';')
    QueryString.truncate(QueryString.length()-1);
  
  bool nobinds=false;
  chk=str.lower();
  chk.replace(strq," ");
  chk=chk.simplifyWhiteSpace();
  chk.replace(QRegExp(" or replace ")," ");
  if(chk.startsWith("create trigger "))
    nobinds=true;
  
  if (!describe(QueryString)) {
    toQList param;
    if (!nobinds)
      try {
	param=toParamGet::getParam(this,QueryString);
      } catch (const QString &str) {
	toStatusMessage(str);
	return;
      }
    toStatusMessage("Processing query",true);
    if (direct) {
      try {
	First=false;
	Timer.start();
	toQuery query(connection(),toQuery::Long,QueryString,param);

	char buffer[100];
	if (query.rowsProcessed()>0)
	  sprintf(buffer,"%d rows processed",(int)query.rowsProcessed());
	else
	  sprintf(buffer,"Query executed");
	addLog(QueryString,toConnection::exception(QString(buffer)));
      } catch (const QString &exc) {
	addLog(QueryString,exc);
      }
    } else {
      Result->stop();
      First=false;
      Timer.start();
      StopButton->setEnabled(true);
      Poll.start(1000);
      QToolTip::add(Started,"Duration while query has been running\n\n"+QueryString);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,true);
      Result->setNumberColumn(!WorksheetTool.config(CONF_NUMBER,"Yes").isEmpty());
      try {
	saveHistory();
	Result->setSQL(QString::null);
	Result->query(QueryString,param);
      } catch (const toConnection::exception &exc) {
	addLog(QueryString,exc);
      } catch (const QString &exc) {
	addLog(QueryString,exc);
      }
      if (!Light) {
	if (StatisticButton->isOn())
	  toRefreshParse(timer(),Refresh->currentText());
      }
      Result->setSQLName(QueryString.simplifyWhiteSpace().left(40));
    }
  }
}

void toWorksheet::saveHistory(void)
{
  if (WorksheetTool.config(CONF_HISTORY,"").isEmpty())
    return;
  if (Result->firstChild()&&Current==Result&&!Light) {
    History[LastID]=Result;
    Result->hide();
    Result->stop();
    disconnect(Result,SIGNAL(done(void)),this,SLOT(queryDone(void)));
    disconnect(Result,SIGNAL(firstResult(const QString &,const toConnection::exception &)),
	       this,SLOT(addLog(const QString &,const toConnection::exception &)));
    disconnect(StopButton,SIGNAL(clicked(void)),Result,SLOT(stop(void)));

    Result=new toResultLong(Result->parentWidget());
    if (StatisticButton->isOn())
      enableStatistic(true);
    Result->show();
    Current=Result;
    connect(StopButton,SIGNAL(clicked(void)),Result,SLOT(stop(void)));
    connect(Result,SIGNAL(done(void)),this,SLOT(queryDone(void)));
    connect(Result,SIGNAL(firstResult(const QString &,const toConnection::exception &)),
	    this,SLOT(addLog(const QString &,const toConnection::exception &)));
  }
}

QString toWorksheet::duration(int dur,bool hundreds)
{
  char buf[100];
  if (dur>=3600000) {
    if (hundreds)
      sprintf(buf,"%d:%02d:%02d.%02d",dur/3600000,(dur/60000)%60,(dur/1000)%60,(dur/10)%100);
    else
      sprintf(buf,"%d:%02d:%02d",dur/3600000,(dur/60000)%60,(dur/1000)%60);
  } else {
    if (hundreds)
      sprintf(buf,"%d:%02d.%02d",dur/60000,(dur/1000)%60,(dur/10)%100);
    else
      sprintf(buf,"%d:%02d",dur/60000,(dur/1000)%60);
  }
  return buf;
}

void toWorksheet::addLog(const QString &sql,const toConnection::exception &result)
{
  QString now=toNow(connection());
  toResultViewItem *item;

  LastID++;

  int dur=0;
  if (!Timer.isNull())
    dur=Timer.elapsed();
  First=true;

  if (!Light) {
    if (WorksheetTool.config(CONF_LOG_MULTI,"Yes").isEmpty()) {
      if (WorksheetTool.config(CONF_LOG_AT_END,"Yes").isEmpty())
	item=new toResultViewItem(Logging,NULL);
      else
	item=new toResultViewItem(Logging,LastLogItem);
    } else if (WorksheetTool.config(CONF_LOG_AT_END,"Yes").isEmpty())
      item=new toResultViewMLine(Logging,NULL);
    else
      item=new toResultViewMLine(Logging,LastLogItem);
    item->setText(0,sql);
  
    LastLogItem=item;
    item->setText(1,result);
    item->setText(2,now);
    if (!WorksheetTool.config(CONF_HISTORY,"").isEmpty())
      item->setText(4,QString::number(LastID));
  }

  if (result.offset()>=0&&LastLine>=0&&LastOffset>=0&&
      !WorksheetTool.config(CONF_MOVE_TO_ERR,"Yes").isEmpty()) {
    QChar cmp='\n';
    int lastnl=0;
    int lines=0;
    for (int i=0;i<result.offset();i++) {
      if (sql.at(i)==cmp) {
	LastOffset=0;
	lastnl=i+1;
	lines++;
      }
    }
    Editor->setCursorPosition(LastLine+lines,LastOffset+result.offset()-lastnl,false);
    LastLine=LastOffset=-1;
  }

  QString buf=duration(dur);

  if (!Light) {
    item->setText(3,buf);

    QListViewItem *last=Logging->currentItem();
    toResultViewItem *citem=NULL;
    if (last)
      citem=dynamic_cast<toResultViewItem *>(last);
    if (!citem||citem->allText(0)!=sql) {
      Logging->setCurrentItem(item);
      Logging->ensureItemVisible(item);
    }
  }

  {
    QString str=result;
    str+="\n(Duration ";
    str+=buf;
    str+=")";
    if (str.contains("ORA-"))
      toStatusMessage(str);
    else
      toStatusMessage(str,false,false);
  }
  if (!Light)
    changeResult(CurrentTab);

  static QRegExp re("^[1-9]\\d* rows processed$");
  if (result.contains(re)) {
    if (!toTool::globalConfig(CONF_AUTO_COMMIT,"").isEmpty())
      connection().commit();
    else
      toMainWidget()->setNeedCommit(connection());
  }
  saveDefaults();
}

static void NewStatement(void)
{
  for (int i=0;Blocks[i].Start;i++)
    Blocks[i].Pos=0;
}

void toWorksheet::execute(bool all,bool step)
{
  bool sqlparse;
  bool code=true;
  bool beforeCode=false;
  if(connection().provider()=="Oracle")
    sqlparse=!WorksheetTool.config(CONF_PLSQL_PARSE,"Yes").isEmpty();
  else
    sqlparse=false;
  TryStrip=true;
  if (!Editor->hasMarkedText()||all||step) {
    int cpos,cline,cbpos,cbline;
    if (!Editor->getMarkedRegion(&cbline,&cbpos,&cline,&cpos)) {
      Editor->getCursorPosition(&cline,&cpos);
      step=false;
    }
    enum {
      beginning,
      comment,
      inStatement,
      inString,
      inCode,
      endCode,
      done
    } state,lastState;

    int startLine=-1,startPos=-1;
    int endLine=-1,endPos=-1;
    lastState=state=beginning;
    NewStatement();
    int BlockCount=0;
    beforeCode=code=TryStrip=false;
    QChar lastChar;
    QChar c=' ';
    QChar nc;

    for (int line=0;line<Editor->numLines()&&state!=done;line++) {
      QString data=Editor->textLine(line);
      c='\n'; // Set correct previous character
      for (int i=0;i<(int)data.length()&&state!=done;i++) {
	lastChar=c;
	c=data[i];
	if (i+1<int(data.length()))
	  nc=data[i+1];
	else
	  nc=' ';
	if (state==comment) {
	  state=lastState;
	  break;
	} else if (state!=inString&&c=='\'') {
	  lastState=state;
	  state=inString;
	} else {
	  switch(state) {
	  case comment:
	    throw QString("Internal error, comment shouldn't have gotten here.");
	  case endCode:
	    if (c==';')
	      state=inCode;
	    break;
	  case inCode:
	    if (c=='-'&&nc=='-') {
	      lastState=state;
	      state=comment;
	    } else {
	      for (int j=0;Blocks[j].Start;j++) {
		int &pos=Blocks[j].Pos;
		if (c.lower()==Blocks[j].Start[pos]&&!Blocks[j].Comment) {
		  if (pos>0||!toIsIdent(lastChar)) {
		    pos++;
		    if (!Blocks[j].Start[pos]) {
		      if (!toIsIdent(nc)) {
			if (Blocks[j].CloseBlock) {
			  BlockCount--;
			  if (BlockCount<=0)
			    state=inStatement;
			} else if (Blocks[j].WantEnd)
			  BlockCount++;
			NewStatement();
			if (state==inCode) {
			  if (Blocks[j].WantSemi)
			    state=endCode;
			  else
			    state=inCode;
			}
			break;
		      } else
			pos=0;
		    }
		  } else
		    pos=0;
		} else
		  pos=0;
	      }
	    }
	    break;
	  case beginning:
	    {
	      QString rest=data.right(data.length()-i).lower();
	      for (int j=0;Blocks[j].Start;j++) {
		if (Blocks[j].Comment) {
		  unsigned int len=strlen(Blocks[j].Start);
		  if (rest.lower().startsWith(Blocks[j].Start)&&(rest.length()<=len||!toIsIdent(rest.at(len)))) {
		    lastState=state;
		    state=comment;
		    break;
		  }
		}
	      }
	      if (state==comment)
		break;
	    }
	    if (c=='-'&&nc=='-') {
	      lastState=state;
	      state=comment;
	      break;
	    } else if (!c.isSpace()&&(c!='/'||data.length()!=1)) {
	      if (((line==cline&&i>cpos)||(line>cline))&&!all&&!step&&startLine>=0&&startPos>=0) {
		state=done;
		break;
	      } else
		beforeCode=code=false;
	      startLine=line;
	      startPos=i;
	      endLine=-1;
	      endPos=-1;
	      state=inStatement;
	    } else
	      break;
	  case inStatement:
	    if (!code&&sqlparse) {
	      bool br=false;
	      for (int j=0;Blocks[j].Start&&!br;j++) {
		int &pos=Blocks[j].Pos;
		if (c.lower()==Blocks[j].Start[pos]&&!Blocks[j].Comment) {
		  if (pos>0||(!toIsIdent(lastChar))) {
		    pos++;
		    if (!Blocks[j].Start[pos]) {
		      if (!toIsIdent(nc)) {
			if (Blocks[j].BeforeCode) {
			  beforeCode=true;
			  pos=0;
			  br=true;
			}
			if (beforeCode) {
			  if (Blocks[j].CloseBlock) {
			    toStatusMessage("Ending unstarted block");
			    return;
			  } else if (Blocks[j].StartCode) {
			    if (Blocks[j].WantEnd)
			      BlockCount++;

			    code=true;
			    if (Blocks[j].WantSemi)
			      state=endCode;
			    else
			      state=inCode;
			    NewStatement();
			    br=true;
			  }
			}
		      } else
			pos=0;
		    } 
		  } else
		    pos=0;
		} else
		  pos=0;
	      }
	      if (br)
		break;
	    }
	    if (c==';') {
	      endLine=line;
	      endPos=i+1;
	      state=beginning;
	      if (all) {
		Editor->setCursorPosition(startLine,startPos,false);
		Editor->setCursorPosition(endLine,endPos,true);
		LastLine=LastOffset=-1;
		if (Editor->hasMarkedText()) {
		  query(Editor->markedText(),true);
		  qApp->processEvents();
		  NewStatement();
		  beforeCode=code=false;
		}
	      } else if (step&&
			 ((line==cline&&i>cpos)||(line>cline))) {
		state=done;
		break;
	      }
	    }
	    break;
	  case inString:
	    if (c=='\'') {
	      state=lastState;
	    }
	    break;
	  case done:
	    break;
	  }
	}
      }
    }
    if (endLine==-1) {
      endLine=Editor->numLines()-1;
      endPos=Editor->textLine(endLine).length();
      if (all&&endLine) {
	LastLine=startLine;
	LastOffset=startPos;
	Editor->setCursorPosition(startLine,startPos,false);
	Editor->setCursorPosition(endLine,endPos,true);
	if (Editor->hasMarkedText()) {
	  query(Editor->markedText(),false);
	}
      }
    }
    if (all) {
      Editor->setCursorPosition(0,0,false);
      Editor->setCursorPosition(endLine,endPos,true);
    } else {
      LastLine=startLine;
      LastOffset=startPos;
      Editor->setCursorPosition(startLine,startPos,false);
      Editor->setCursorPosition(endLine,endPos,true);
    }
  }
  if (Editor->hasMarkedText()&&!all) {
    query(Editor->markedText(),false);
    if (Light)
      return;
    else if (CurrentTab==Visualize)
      Visualize->display();
    else if (CurrentTab==Plan)
      Plan->query(QueryString);
    else if (CurrentTab==Resources)
      viewResources();
  }
}

void toWorksheet::eraseLogButton()
{
  if (Light)
    return;
  Logging->clear();
  LastLogItem=NULL;
  for(std::map<int,toResultLong *>::iterator i=History.begin();i!=History.end();i++)
    delete (*i).second;
  History.clear();
}

void toWorksheet::queryDone(void)
{
  if (!First&&!QueryString.isEmpty())
    addLog(QueryString,toConnection::exception("Aborted"));
  else
    emit executed();
  timer()->stop();
  StopButton->setEnabled(false);
  Poll.stop();
  QToolTip::remove(Started);
  Started->setText(QString::null);
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,false);
  saveDefaults();
}

void toWorksheet::saveDefaults(void)
{
  QListViewItem *item=Result->firstChild();
  if (item) {
    QHeader *head=Result->header();
    for (int i=0;i<Result->columns();i++) {
      toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
      QString str;
      if (resItem)
	str=resItem->allText(i);
      else if (item)
	str=item->text(i);

      if (str=="{null}")
	str=QString::null;
      toParamGet::setDefault(head->label(i).lower(),str);
    }
  }
}

#define ENABLETIMED "ALTER SESSION SET TIMED_STATISTICS = TRUE"

void toWorksheet::enableStatistic(bool ena)
{
  if (ena) {
    Result->setStatistics(Statistics);
    ResultTab->setTabEnabled(StatTab,true);
    toMainWidget()->menuBar()->setItemChecked(TO_ID_STATISTICS,true);
    Statistics->clear();
    if (!WorksheetTool.config(CONF_TIMED_STATS,"Yes").isEmpty()) {
      try {
	connection().allExecute(ENABLETIMED);
	connection().addInit(ENABLETIMED);
      } TOCATCH
    }
  } else {
    connection().delInit(ENABLETIMED);
    Result->setStatistics(NULL);
    ResultTab->setTabEnabled(StatTab,false);
    toMainWidget()->menuBar()->setItemChecked(TO_ID_STATISTICS,false);
  }
}

void toWorksheet::executeNewline(void)
{
  int cline,epos;

  Editor->getCursorPosition(&cline,&epos);

  if (cline>0)
    cline--;
  while(cline>0) {
    QString data=Editor->textLine(cline).simplifyWhiteSpace();
    if (data.length()==0||data==" ") {
      cline++;
      break;
    }
    cline--;
  }

  while(cline<Editor->numLines()) {
    QString data=Editor->textLine(cline).simplifyWhiteSpace();
    if (data.length()!=0&&data!=" ")
      break;
    cline++;
  }

  int eline=cline;

  while(eline<Editor->numLines()) {
    QString data=Editor->textLine(eline).simplifyWhiteSpace();
    if (data.length()==0||data==" ") {
      eline--;
      break;
    }
    epos=Editor->textLine(eline).length();
    eline++;
  }
  Editor->setCursorPosition(cline,0,false);
  Editor->setCursorPosition(eline,epos,true);
  LastLine=cline;
  LastOffset=0;
  if (Editor->hasMarkedText())
    query(Editor->markedText(),false);
}

void toWorksheet::describe(void)
{
  if (Light)
    return;

  QString owner,table;
  Editor->tableAtCursor(owner,table);

  if (owner.isNull())
    Columns->changeParams(table);
  else
    Columns->changeParams(owner,table);
  Current->hide();
  Columns->show();
  Current=Columns;
}

void toWorksheet::executeSaved(void)
{
  if (Light)
    return;

  LastLine=LastOffset=-1;

  if (SavedLast.length()>0) {
    try {
      query(toSQL::string(SavedLast,connection()),false);
    } TOCATCH
  }
}

void toWorksheet::executeSaved(int id)
{
  std::list<QString> def=toSQL::range(TOWORKSHEET);
  for(std::list<QString>::iterator i=def.begin();i!=def.end();i++) {
    id--;
    if (id==0) {
      SavedLast=(*i);
      executeSaved();
      break;
    }
  }
}

void toWorksheet::showSaved(void)
{
  static QRegExp colon(":");
  std::list<QString> def=toSQL::range(TOWORKSHEET);
  SavedMenu->clear();
  std::map<QString,QPopupMenu *> menues;
  int id=0;
  for(std::list<QString>::iterator sql=def.begin();sql!=def.end();sql++) {

    id++;

    QStringList spl=QStringList::split(colon,*sql);
    spl.remove(spl.begin());

    if (spl.count()>0) {
      QString name=spl.last();
      spl.remove(spl.fromLast());

      QPopupMenu *menu;
      if (spl.count()==0)
	menu=SavedMenu;
      else {
	QStringList exs=spl;
	while (exs.count()>0&&menues.find(exs.join(":"))==menues.end())
	  exs.remove(exs.fromLast());
	if (exs.count()==0)
	  menu=SavedMenu;
	else
	  menu=menues[exs.join(":")];
	QString subname=exs.join(":");
	for (unsigned int i=exs.count();i<spl.count();i++) {
	  QPopupMenu *next=new QPopupMenu(this);
	  if (i!=0)
	    subname+=":";
	  subname+=spl[i];
	  menu->insertItem(spl[i],next);
	  menu=next;
	  menues[subname]=menu;
	}
      }
      menu->insertItem(name,id);
    }
  }
}

void toWorksheet::editSaved(void)
{
  QString sql=TOWORKSHEET;
  sql+="Untitled";
  toMainWidget()->editSQL(sql);
}

void toWorksheet::selectSaved()
{
  SavedMenu->popup(SavedButton->mapToGlobal(QPoint(0,SavedButton->height())));
}

void toWorksheet::executePreviousLog(void)
{
  if (Light)
    return;

  LastLine=LastOffset=-1;
  saveHistory();

  QListViewItem *item=Logging->currentItem();
  if (item) {
    QListViewItem *prev=Logging->firstChild();
    while(prev&&prev->nextSibling()!=item)
      prev=prev->nextSibling();
    toResultViewItem *item=dynamic_cast<toResultViewItem *>(prev);
    if (item) {
      Logging->setCurrentItem(item);
      if (item->text(4).isEmpty())
	query(item->allText(0),false);
      else {
	std::map<int,toResultLong *>::iterator i=History.find(item->text(4).toInt());
	if (i!=History.end()&&(*i).second) {
	  Current->hide();
	  Current=(*i).second;
	  Current->show();
	}
      }
    }
  }
}

void toWorksheet::executeNextLog(void)
{
  if (Light)
    return;

  LastLine=LastOffset=-1;
  saveHistory();

  QListViewItem *item=Logging->currentItem();
  if (item&&item->nextSibling()) {
    toResultViewItem *next=dynamic_cast<toResultViewItem *>(item->nextSibling());
    if (next) {
      Logging->setCurrentItem(next);

      if (next->text(4).isEmpty())
	query(next->allText(0),false);
      else {
	std::map<int,toResultLong *>::iterator i=History.find(next->text(4).toInt());
	if (i!=History.end()&&(*i).second) {
	  Current->hide();
	  Current=(*i).second;
	  Current->show();
	}
      }
    }
  }
}

void toWorksheet::poll(void)
{
  Started->setText(duration(Timer.elapsed(),false));
}

void toWorksheet::saveLast(void)
{
  if (QueryString.isEmpty()) {
    TOMessageBox::warning(this,"No SQL to save",
			  "You haven't executed any SQL yet",
			  "&Ok");
    return;
  }
  bool ok=false;
  QString name=QInputDialog::getText("Enter title",
				     "Enter the title in the menu of the saved SQL,\n"
				     "submenues are separated by a ':' character.",
				     QLineEdit::Normal,QString::null,&ok,this);
  if (ok&&!name.isEmpty()) {
    toSQL::updateSQL(TOWORKSHEET+name,
		     QueryString,
		     "Undescribed",
		     "Any",
		     connection().provider());
    toSQL::saveSQL(toTool::globalConfig(CONF_SQL_FILE,DEFAULT_SQL_FILE));
  }
}

void toWorksheet::saveStatistics(void)
{
  std::map<QString,QString> stat;

  Statistics->exportData(stat,"Stat");
  IOChart->exportData(stat,"IO");
  WaitChart->exportData(stat,"Wait");
  stat["Description"]=QueryString;
  
  toWorksheetStatistic::saveStatistics(stat);
}

void toWorksheet::exportData(std::map<QString,QString> &data,const QString &prefix)
{
  Editor->exportData(data,prefix+":Edit");
  if (StatisticButton->isOn())
    data[prefix+":Stats"]=Refresh->currentText();
  toToolWidget::exportData(data,prefix);
}

void toWorksheet::importData(std::map<QString,QString> &data,const QString &prefix)
{
  Editor->importData(data,prefix+":Edit");
  QString stat=data[prefix+":Stats"];
  if (stat) {
    for (int i=0;i<Refresh->count();i++) {
      if (Refresh->text(i)==stat) {
	Refresh->setCurrentItem(i);
	break;
      }
    }
    StatisticButton->setOn(true);
  } else
    StatisticButton->setOn(false);

  toToolWidget::importData(data,prefix);
}

