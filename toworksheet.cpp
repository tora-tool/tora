/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include <time.h>

#ifdef TO_KDE
#include <kfiledialog.h>
#endif

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

#include "totool.h"
#include "toresultplan.h"
#include "toworksheet.h"
#include "toresultview.h"
#include "toresultresources.h"
#include "tohighlightedtext.h"
#include "toparamget.h"
#include "toresultlong.h"
#include "toresultstats.h"
#include "toconf.h"

#include "toworksheet.moc"

#include "icons/toworksheet.xpm"
#include "icons/refresh.xpm"
#include "icons/execute.xpm"
#include "icons/executestep.xpm"
#include "icons/executeall.xpm"
#include "icons/commit.xpm"
#include "icons/rollback.xpm"
#include "icons/eraselog.xpm"
#include "icons/stop.xpm"
#include "icons/clock.xpm"

#define TO_ID_STATISTICS		(TO_TOOL_MENU_ID+ 0)
#define TO_ID_STOP			(TO_TOOL_MENU_ID+ 1)

#define CONF_AUTO_COMMIT "AutoCommit"
#define CONF_AUTO_SAVE   "AutoSave"
#define CONF_CHECK_SAVE  "CheckSave"
#define CONF_AUTO_LOAD   "AutoLoad"
#define CONF_LOG_AT_END  "LogAtEnd"
#define CONF_LOG_MULTI   "LogMulti"
#define CONF_PLSQL_PARSE "PLSQLParse"
#define CONF_STATISTICS	 "Statistics"

static struct {
  int Pos;
  char *Start;
  bool WantEnd;
  bool WantSemi;
  bool CloseBlock;
  bool Comment;
} Blocks[] = { { 0,"begin",	true ,false,false,false },
	       { 0,"if",	true ,false,false,false },
	       { 0,"loop",	true ,false,false,false },
	       { 0,"while",	true ,false,false,false },
	       { 0,"declare",	false,false,false,false },
	       { 0,"package",	true ,false,false,false },
	       { 0,"procedure",	false,false,false,false },
	       { 0,"function",	false,false,false,false },
	       { 0,"end",	false,true ,true ,false },
	       { 0,"rem",	false,false,false,true  },
	       { 0,"prompt",	false,false,false,true  },
	       { 0,"set",	false,false,false,true  },
	       { 0,NULL,	false,false,false,false }
};

toWorksheetPrefs::toWorksheetPrefs(toTool *tool,QWidget* parent,const char* name)
  : QFrame(parent,name),Tool(tool)
{
  GroupBox1 = new QGroupBox( this, "GroupBox1" );
  GroupBox1->setGeometry( QRect( 10, 10, 380, 380 ) ); 
  GroupBox1->setTitle( tr( "SQL Worksheet"  ) );
  
  AutoCommit = new QCheckBox( GroupBox1, "AutoCommit" );
  AutoCommit->setGeometry( QRect( 20, 30, 340, 20 ) ); 
  AutoCommit->setText( tr( "&Auto commit"  ) );
  QToolTip::add(  AutoCommit, tr( "Auto commit each statement." ) );
  
  AutoSave = new QCheckBox( GroupBox1, "AutoSave" );
  AutoSave->setGeometry( QRect( 20, 70, 340, 20 ) ); 
  AutoSave->setText( tr( "Auto &Save"  ) );
  QToolTip::add(  AutoSave, tr( "Auto save file when closing worksheet\n"
				"(Without asking questions)." ) );
  
  CheckSave = new QCheckBox( GroupBox1, "CheckSave" );
  CheckSave->setGeometry( QRect( 20, 110, 340, 20 ) ); 
  CheckSave->setText( tr( "Ask about saving &changes"  ) );
  QToolTip::add(  CheckSave, tr( "Ask about saving changes when closing worksheet." ) );
  
  LogAtEnd = new QCheckBox( GroupBox1, "LogAtEnd" );
  LogAtEnd->setGeometry( QRect( 20, 150, 340, 20 ) ); 
  LogAtEnd->setText( tr( "&Log entries at end"  ) );
  QToolTip::add(  LogAtEnd, tr( "Add newly executed queries at end of log instead of top.\n"
				"Changing this will mess up the log in the windows that are opened.") );
  
  LogMulti = new QCheckBox( GroupBox1, "LogMulti" );
  LogMulti->setGeometry( QRect( 20, 190, 340, 20 ) ); 
  LogMulti->setText( tr( "&Multiple lines in log"  ) );
  QToolTip::add(  LogMulti, tr( "Display multiple lines in logging SQL column.") );
  
  PLSQLParse = new QCheckBox( GroupBox1, "PLSQLParse" );
  PLSQLParse->setGeometry( QRect( 20, 230, 340, 20 ) ); 
  PLSQLParse->setText( tr( "&Parse PL/SQL blocks"  ) );
  QToolTip::add(PLSQLParse, tr( "Parse PL/SQL blocks in worksheet. You usually need this\n"
				"but it if you forget one end the rest of the editor will\n"
				"be one block.") );

  Statistics = new QCheckBox( GroupBox1, "Statistics" );
  Statistics->setGeometry( QRect( 20, 270, 340, 20 ) ); 
  Statistics->setText( tr( "&Enable Statistics"  ) );
  QToolTip::add(Statistics, tr( "Enable statistic collection per default.") );

  FileChoose = new QPushButton( GroupBox1, "FileChoose" );
  FileChoose->setGeometry( QRect( 280, 304, 80, 32 ) ); 
  FileChoose->setText( tr( "&Browse"  ) );
  
  DefaultFile = new QLineEdit( GroupBox1, "DefaultFile" );
  DefaultFile->setGeometry( QRect( 100, 310, 170, 23 ) ); 
  
  TextLabel2 = new QLabel( GroupBox1, "TextLabel2" );
  TextLabel2->setGeometry( QRect( 20, 310, 80, 20 ) ); 
  TextLabel2->setText( tr( "&Default file"  ) );
  QToolTip::add(  TextLabel2, tr( "File to automatically load when opening a worksheet." ) );
  
  TextLabel2->setBuddy( DefaultFile );
  
  if (!tool->config(CONF_AUTO_COMMIT,"").isEmpty())
    AutoCommit->setChecked(true);
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
  if (!tool->config(CONF_STATISTICS,"").isEmpty())
    Statistics->setChecked(true);
  DefaultFile->setText(tool->config(CONF_AUTO_LOAD,""));

  connect(FileChoose,SIGNAL(clicked()),this,SLOT(chooseFile()));
}

void toWorksheetPrefs::saveSetting(void)
{
  if (AutoCommit->isChecked())
    Tool->setConfig(CONF_AUTO_COMMIT,"Yes");
  else
    Tool->setConfig(CONF_AUTO_COMMIT,"");
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
  Tool->setConfig(CONF_STATISTICS,Statistics->isChecked()?"Yes":"");
  Tool->setConfig(CONF_AUTO_LOAD,DefaultFile->text());
}

void toWorksheetPrefs::chooseFile(void)
{
  QString str=TOFileDialog::getOpenFileName(DefaultFile->text(),"*.sql\n*.txt",this);
  if (!str.isEmpty())
    DefaultFile->setText(str);
}

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
    QWidget *window=new toWorksheet(main,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toWorksheetPrefs(this,parent);
  }
};

static toWorksheetTool WorksheetTool;

static QPixmap *toRefreshPixmap;
static QPixmap *toExecutePixmap;
static QPixmap *toExecuteAllPixmap;
static QPixmap *toExecuteStepPixmap;
static QPixmap *toCommitPixmap;
static QPixmap *toRollbackPixmap;
static QPixmap *toEraseLogPixmap;
static QPixmap *toStopPixmap;
static QPixmap *toStatisticPixmap;

void toWorksheet::viewResources(void)
{
  try {
    QString address=toSQLToAddress(Connection,QueryString);

    Resources->changeParams(address);
  } TOCATCH
}

toWorksheet::toWorksheet(QWidget *main,toConnection &connection,bool autoLoad)
  : QVBox(main,NULL,WDestructiveClose),Connection(connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toExecutePixmap)
    toExecutePixmap=new QPixmap((const char **)execute_xpm);
  if (!toExecuteAllPixmap)
    toExecuteAllPixmap=new QPixmap((const char **)executeall_xpm);
  if (!toExecuteStepPixmap)
    toExecuteStepPixmap=new QPixmap((const char **)executestep_xpm);
  if (!toCommitPixmap)
    toCommitPixmap=new QPixmap((const char **)commit_xpm);
  if (!toRollbackPixmap)
    toRollbackPixmap=new QPixmap((const char **)rollback_xpm);
  if (!toEraseLogPixmap)
    toEraseLogPixmap=new QPixmap((const char **)eraselog_xpm);
  if (!toStopPixmap)
    toStopPixmap=new QPixmap((const char **)stop_xpm);
  if (!toStatisticPixmap)
    toStatisticPixmap=new QPixmap((const char **)clock_xpm);

  QToolBar *toolbar=toAllocBar(this,"SQL worksheet",connection.connectString());

  new QToolButton(*toExecutePixmap,
		  "Execute current statement",
		  "Execute current statement",
		  this,SLOT(execute(void)),
		  toolbar);
  new QToolButton(*toExecuteAllPixmap,
		  "Execute all statements",
		  "Execute all statements",
		  this,SLOT(executeAll(void)),
		  toolbar);
  new QToolButton(*toExecuteStepPixmap,
		  "Step through statements",
		  "Step through statements",
		  this,SLOT(executeStep(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(*toRefreshPixmap,
		  "Refresh",
		  "Update last executed statement",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(*toCommitPixmap,
		  "Commit work",
		  "Commit work",
		  this,SLOT(commitButton(void)),
		  toolbar);
  new QToolButton(*toRollbackPixmap,
		  "Rollback work",
		  "Rollback work",
		  this,SLOT(rollbackButton(void)),
		  toolbar);

  QSplitter *splitter=new QSplitter(Vertical,this);

  Editor=new toHighlightedText(splitter);
  ResultTab=new QTabWidget(splitter);
  Result=new toResultLong(connection,ResultTab);
  connect(Result,SIGNAL(done(void)),this,SLOT(queryDone(void)));
  connect(Result,SIGNAL(firstResult(const QString &,const QString &)),
	  this,SLOT(addLog(const QString &,const QString &)));
  ResultTab->addTab(Result,"Data");
  Plan=new toResultPlan(connection,ResultTab);
  ResultTab->addTab(Plan,"Execution plan");
  Resources=new toResultResources(connection,ResultTab);
  ResultTab->addTab(Resources,"Information");
  Statistics=new toResultStats(true,connection,ResultTab);
  ResultTab->addTab(Statistics,"Statistics");
  ResultTab->setTabEnabled(Statistics,false);

  Logging=new toResultView(true,false,connection,ResultTab);
  ResultTab->addTab(Logging,"Logging");
  Logging->addColumn("SQL");
  Logging->addColumn("Result");
  Logging->addColumn("Timestamp");
  LastLogItem=NULL;

  toolbar->addSeparator();
  StatisticButton=new QToolButton(toolbar);
  StatisticButton->setToggleButton(true);
  StatisticButton->setIconSet(QIconSet(*toStatisticPixmap));
  connect(StatisticButton,SIGNAL(toggled(bool)),this,SLOT(enableStatistic(bool)));
  QToolTip::add(StatisticButton,"Gather session statistic of execution");

  StopButton=new QToolButton(*toStopPixmap,
			     "Stop execution",
			     "Stop execution",
			     Result,SLOT(stop(void)),
			     toolbar);
  StopButton->setEnabled(false);
  toolbar->addSeparator();
  new QToolButton(*toEraseLogPixmap,
		  "Clear execution log",
		  "Clear execution log",
		  this,SLOT(eraseLogButton(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  Connection.addWidget(this);

  connect(ResultTab,SIGNAL(currentChanged(QWidget *)),
	  this,SLOT(changeResult(QWidget *)));

  if (autoLoad) {
    Editor->setFilename(WorksheetTool.config(CONF_AUTO_LOAD,""));
    if (!Editor->filename().isEmpty()) {
      QFile file(Editor->filename());
      if (file.open(IO_ReadOnly)) {
	int size=file.size();
	    
	char *buf=new char[size+1];
	if (file.readBlock(buf,size)!=-1) {
	  buf[size]=0;
	  Editor->setText(QString::fromLocal8Bit(buf));
	  Editor->setEdited(false);
	}
	delete buf;
      }
    }
  }

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  if (!WorksheetTool.config(CONF_STATISTICS,"").isEmpty()) {
    show();
    StatisticButton->setOn(true);
  }
}

void toWorksheet::windowActivated(QWidget *widget)
{
  QWidget *w=this;
  while(w&&w!=widget)
    w=dynamic_cast<QWidget *>(w->parent());

  if (widget==w) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(*toExecutePixmap,"&Execute Current",this,SLOT(execute(void)),
			   CTRL+Key_Return);
      ToolMenu->insertItem(*toExecuteAllPixmap,"Execute &All",this,SLOT(executeAll(void)),
			   Key_F8);
      ToolMenu->insertItem(*toExecuteStepPixmap,"Execute &Next",this,SLOT(executeStep(void)),
			   Key_F9);
      ToolMenu->insertItem(*toRefreshPixmap,"&Refresh",this,SLOT(refresh(void)),
			   Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(*toCommitPixmap,"&Commit",this,SLOT(commitButton(void)),
			   Key_F2);
      ToolMenu->insertItem(*toRollbackPixmap,"&Rollback",this,SLOT(rollbackButton(void)),
			   CTRL+Key_Backspace);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem("&Enable statistics",this,SLOT(toggleStatistic(void)),
			   0,TO_ID_STATISTICS);
      ToolMenu->insertItem(*toStopPixmap,"&Stop execution",Result,SLOT(stop(void)),
			   0,TO_ID_STOP);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(*toEraseLogPixmap,"Erase &Log",this,SLOT(eraseLogButton(void)));

      toMainWidget()->menuBar()->insertItem("W&orksheet",ToolMenu,-1,TO_TOOL_MENU_INDEX);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,StopButton->isEnabled());
      toMainWidget()->menuBar()->setItemChecked(TO_ID_STATISTICS,
						StatisticButton->isOn());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

bool toWorksheet::checkSave(bool input)
{
  if (Editor->edited()) {
    if(WorksheetTool.config(CONF_AUTO_SAVE,"").isEmpty()||
       Editor->filename().isEmpty()) {
      if (!WorksheetTool.config(CONF_CHECK_SAVE,"Yes").isEmpty()) {
	if (input) {
	  QString str("Save changes to worksheet for ");
	  str.append(Connection.connectString());
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
	Editor->setFilename(TOFileDialog::getSaveFileName(Editor->filename(),
							  "*.sql\n*.txt",this));
      if (Editor->filename().isEmpty())
	return false;	
    }
    QFile file(Editor->filename());
    if (!file.open(IO_WriteOnly)) {
      TOMessageBox::warning(this,"File error","Couldn't open file for writing");
      return false;
    }
    QString data=Editor->text();
    if (file.writeBlock(data,data.length())==-1) {
      TOMessageBox::warning(this,"File error","Couldn't save data to file");
      return false;
    }
    Editor->setEdited(false);
  }
  return true;
}

bool toWorksheet::close(bool del)
{
  if (checkSave(true))
    return QVBox::close(del);
  return false;
}

toWorksheet::~toWorksheet()
{
  checkSave(false);
  Connection.delWidget(this);
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
    query(QueryString);
    StopButton->setEnabled(true);
    toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,true);
    if (CurrentTab==Plan)
      Plan->query(QueryString);
    else if (CurrentTab==Resources)
      viewResources();
  }
}

void toWorksheet::query(const QString &str)
{

  try {
    QString execSql=str;
    list<QString> param=toParamGet::getParam(this,execSql);
    Result->query(execSql,param);
    Result->setSQLName(execSql.simplifyWhiteSpace().left(40));
    StopButton->setEnabled(true);
    toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,true);
  } catch (const QString &str) {
    toStatusMessage(str);
  }
}

void toWorksheet::addLog(const QString &sql,const QString &result)
{
  QString now=toNow(Connection);
  toResultViewItem *item;

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
  Logging->setCurrentItem(item);
  Logging->ensureItemVisible(item);
  if (!WorksheetTool.config(CONF_AUTO_COMMIT,"").isEmpty())
    Connection.connection().commit();
  else
    Connection.setNeedCommit();
}

void NewStatement(void)
{
  for (int i=0;Blocks[i].Start;i++)
    Blocks[i].Pos=0;
}

void toWorksheet::execute(bool all,bool step)
{
  bool sqlparse=!WorksheetTool.config(CONF_PLSQL_PARSE,"Yes").isEmpty();
  bool code=true; // Don't strip from done selection
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
    code=TryStrip=false;
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
		unsigned int len=strlen(Blocks[j].Start);
		if (rest.lower().startsWith(Blocks[j].Start)&&(rest.length()<=len||!toIsIdent(rest.at(len)))) {
		  lastState=state;
		  state=comment;
		  break;
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
		code=false;
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
			if (Blocks[j].CloseBlock) {
			  toStatusMessage("Ending unstarted block");
			  return;
			} else if (Blocks[j].WantEnd)
			  BlockCount++;

			code=true;
			if (Blocks[j].WantSemi)
			  state=endCode;
			else
			  state=inCode;
			NewStatement();
			br=true;
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
		if (!code)
		  Editor->setCursorPosition(endLine,endPos-1,true);
		else
		  Editor->setCursorPosition(endLine,endPos,true);
		if (Editor->hasMarkedText()) {
		  QueryString=Editor->markedText();
		  try {
		    otl_stream str(1,
				   QueryString.utf8(),
				   Connection.connection());
		    char buffer[100];
		    if (str.get_rpc()>0)
		      sprintf(buffer,"%d rows processed",(int)str.get_rpc());
		    else
		      sprintf(buffer,"Query executed");
		    addLog(QueryString,buffer);
		  } catch (const otl_exception &exc) {
		    addLog(QueryString,QString::fromUtf8((const char *)exc.msg));
		  } 
		  qApp->processEvents();
		  NewStatement();
		  code=false;
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
	Editor->setCursorPosition(startLine,startPos,false);
	Editor->setCursorPosition(endLine,endPos,true);
	if (Editor->hasMarkedText()) {
	  QueryString=Editor->markedText();
	  query(QueryString);
	}
      }
    }
    if (all) {
      Editor->setCursorPosition(0,0,false);
      Editor->setCursorPosition(endLine,endPos,true);
    } else {
      Editor->setCursorPosition(startLine,startPos,false);
      Editor->setCursorPosition(endLine,endPos,true);
    }
  }
  if (Editor->hasMarkedText()&&!all) {
    QueryString=Editor->markedText();
    if (!code&&QueryString.at(QueryString.length()-1)==';')
      QueryString.remove(QueryString.length()-1,1);
    query(QueryString);
    if (CurrentTab==Plan)
      Plan->query(QueryString);
    else if (CurrentTab==Resources)
      viewResources();
  }
}

void toWorksheet::eraseLogButton()
{
  Logging->clear();
  LastLogItem=NULL;
}

void toWorksheet::queryDone(void)
{
  StopButton->setEnabled(false);
  toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,false);
  QListViewItem *item=Result->firstChild();
  if (item) {
    QHeader *head=Result->header();
    for (int i=0;i<Result->columns();i++) {
      toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
      toResultViewCheck *chkItem=dynamic_cast<toResultViewCheck *>(item);
      QString str;
      if (resItem)
	str=resItem->allText(i);
      else if (chkItem)
	str=chkItem->allText(i);
      else if (item)
	str=item->text(i);

      if (str=="{null}")
	str=QString::null;
      toParamGet::setDefault(head->label(i).lower(),str,false);
    }
  }
}

void toWorksheet::enableStatistic(bool ena)
{
  if (ena) {
    if (toTool::globalConfig(CONF_LONG_SESSION,"").isEmpty())
      TOMessageBox::warning(this,
			    "Enable statistics",
			    "Enabling statistics without enabling the long sessions option\n"
			    "is unreliable at bests.",
			    "Ok");
    Result->setStatistics(Statistics);
    ResultTab->setTabEnabled(Statistics,true);
    toMainWidget()->menuBar()->setItemChecked(TO_ID_STATISTICS,true);
    Statistics->clear();
  } else {
    Result->setStatistics(NULL);
    ResultTab->setTabEnabled(Statistics,false);
    toMainWidget()->menuBar()->setItemChecked(TO_ID_STATISTICS,false);
  }
}
