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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/


#include <time.h>

#include <qlabel.h>
#include <qlistview.h>
#include <qmultilineedit.h>
#include <qnamespace.h>
#include <qpixmap.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
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

#include <stdio.h>

#include "totool.h"
#include "toresultplan.h"
#include "toworksheet.h"
#include "toresultview.h"
#include "toresultresources.h"
#include "tohighlightedtext.h"

#include "toworksheet.moc"

#include "icons/toworksheet.xpm"
#include "icons/refresh.xpm"
#include "icons/execute.xpm"
#include "icons/executestep.xpm"
#include "icons/executeall.xpm"
#include "icons/commit.xpm"
#include "icons/rollback.xpm"
#include "icons/eraselog.xpm"

#define CONF_AUTO_COMMIT "AutoCommit"
#define CONF_AUTO_SAVE   "AutoSave"
#define CONF_CHECK_SAVE  "CheckSave"
#define CONF_AUTO_LOAD   "AutoLoad"
#define CONF_LOG_AT_END  "LogAtEnd"
#define CONF_LOG_MULTI   "LogMulti"

static struct {
  int Pos;
  char *Start;
  bool WantEnd;
  bool WantSemi;
  bool CloseBlock;
} Blocks[] = { { 0,"begin",	true ,false,false },
	       { 0,"if",	true ,false,false },
	       { 0,"loop",	true ,false,false },
	       { 0,"while",	true ,false,false },
	       { 0,"declare",	false,false,false },
	       { 0,"create",	false,false,false },
	       { 0,"package",	true ,false,false },
	       { 0,"end",	false,true ,true  },
	       { 0,NULL,	false,false }
};

toWorksheetPrefs::toWorksheetPrefs(toTool *tool,QWidget* parent = 0,const char* name = 0)
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
  QToolTip::add(  AutoSave, tr( "Auto save file when closing worksheet." ) );
  
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
  LogMulti->setText( tr( "&Multiple lines in log."  ) );
  QToolTip::add(  LogMulti, tr( "Display multiple lines in logging SQL column.") );
  
  FileChoose = new QPushButton( GroupBox1, "FileChoose" );
  FileChoose->setGeometry( QRect( 280, 224, 80, 32 ) ); 
  FileChoose->setText( tr( "&Browse"  ) );
  
  DefaultFile = new QLineEdit( GroupBox1, "DefaultFile" );
  DefaultFile->setGeometry( QRect( 100, 230, 170, 23 ) ); 
  
  TextLabel2 = new QLabel( GroupBox1, "TextLabel2" );
  TextLabel2->setGeometry( QRect( 20, 230, 80, 20 ) ); 
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
  Tool->setConfig(CONF_AUTO_LOAD,DefaultFile->text());
}

void toWorksheetPrefs::chooseFile(void)
{
  QString str=QFileDialog::getOpenFileName(DefaultFile->text(),"*.sql\n*.txt",this);
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
  virtual QWidget *toolWindow(toMain *main,toConnection &connection)
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

void toWorksheet::viewResources(void)
{
  try {
    QString address=toSQLToAddress(Connection,QueryString);

    Resources->changeParams(address);
  } catch (...) {
    toStatusMessage("Couldn't find SQL statement in SGA");
  }
}

toWorksheet::toWorksheet(toMain *main,toConnection &connection)
  : QVBox(main->workspace(),NULL,WDestructiveClose),Connection(connection)
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

  QToolBar *toolbar=new QToolBar("SQL Worksheet",main,this);
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
  toolbar->addSeparator();
  new QToolButton(*toEraseLogPixmap,
		  "Clear execution log",
		  "Clear execution log",
		  this,SLOT(eraseLogButton(void)),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel("",toolbar));
  
  QSplitter *splitter=new QSplitter(Vertical,this);

  Editor=new toHighlightedText(splitter);
  ResultTab=new QTabWidget(splitter);
  Result=new toResultView(connection,ResultTab);
  ResultTab->addTab(Result,"Data");
  Plan=new toResultPlan(connection,ResultTab);
  ResultTab->addTab(Plan,"Execution plan");
  Resources=new toResultResources(connection,ResultTab);
  ResultTab->addTab(Resources,"Information");

  Logging=new toResultView(true,false,connection,ResultTab);
  ResultTab->addTab(Logging,"Logging");
  Logging->addColumn("SQL");
  Logging->addColumn("Result");
  Logging->addColumn("Timestamp");
  LastLogItem=NULL;

  Connection.addWidget(this);

  connect(Editor,SIGNAL(execute(void)),this,SLOT(execute(void)));
  connect(ResultTab,SIGNAL(currentChanged(QWidget *)),
	  this,SLOT(changeResult(QWidget *)));

  Editor->setFilename(WorksheetTool.config(CONF_AUTO_LOAD,""));
  if (!Editor->filename().isEmpty()) {
    QFile file(Editor->filename());
    if (!file.open(IO_ReadOnly)) {
      return;
    }
	    
    int size=file.size();
	    
    char buf[size+1];
    if (file.readBlock(buf,size)==-1)
      return;

    buf[size]=0;
    Editor->setText(buf);
    Editor->setEdited(false);
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
	  int ret=QMessageBox::information(this,
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
	Editor->setFilename(QFileDialog::getSaveFileName(Editor->filename(),
							 "*.sql\n*.txt",this));
      if (Editor->filename().isEmpty())
	return false;	
    }
    QFile file(Editor->filename());
    if (!file.open(IO_WriteOnly)) {
      QMessageBox::warning(this,"File error","Couldn't open file for writing");
      return false;
    }
    QString data=Editor->text();
    if (file.writeBlock(data,data.length())==-1) {
      QMessageBox::warning(this,"File error","Couldn't save data to file");
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
  }
}

void toWorksheet::refresh(void)
{
  if (!QueryString.isEmpty()) {
    Result->query(QueryString);
    if (CurrentTab==Plan)
      Plan->query(QueryString);
    else if (CurrentTab==Resources)
      viewResources();
  }
}

void toWorksheet::query(const QString &str)
{
  QString res=Result->query(str);

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
  item->setText(0,str);

  LastLogItem=item;
  item->setText(1,res);
  {
    item->setText(2,toNow(Connection));
  }
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

#define isident(c) (isalnum(c)||(c)=='_')

void toWorksheet::execute(bool all,bool step)
{
  bool code=true; // Don't strip from done selection
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
    state=beginning;
    NewStatement();
    int BlockCount=0;
    code=false;
    char lastChar;
    char c=' ';
    char nc;

    for (int line=0;line<Editor->numLines()&&state!=done;line++) {
      QString data=Editor->textLine(line);
      const char *str=data;
      for (int i=0;i<(int)data.length()&&state!=done;i++) {
	lastChar=c;
	c=str[i];
	if (i+1<int(data.length()))
	  nc=str[i+1];
	else
	  nc=' ';
	if (state==comment) {
	  state=beginning;
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
	    for (int j=0;Blocks[j].Start;j++) {
	      int &pos=Blocks[j].Pos;
	      if (tolower(c)==Blocks[j].Start[pos]) {
		if (pos>0||!isident(lastChar)) {
		  pos++;
		  if (!Blocks[j].Start[pos]) {
		    if (!isident(nc)) {
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
	    break;
	  case beginning:
	    if (c=='-'&&nc=='-') {
	      state=comment;
	      break;
	    } else if (!isspace(c)) {
	      if (((line==cline&&i>cpos)||(line>cline))&&!all&&!step) {
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
	    if (!code) {
	      bool br=false;
	      for (int j=0;Blocks[j].Start&&!br;j++) {
		int &pos=Blocks[j].Pos;
		if (tolower(c)==Blocks[j].Start[pos]) {
		  if (pos>0||(!isident(lastChar))) {
		    pos++;
		    if (!Blocks[j].Start[pos]) {
		      if (!isident(nc)) {
			if (Blocks[j].CloseBlock) {
			  toStatusMessage("Ending unstarted block");
			  return;
			}
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
	    if (c==':') {
	      toStatusMessage("Can't execute SQL with bind variables in SQL Worksheet");
	      return;
	    } else if (c==';') {
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
		  query(QueryString);
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
    if (!code&&((const char *)QueryString)[QueryString.length()-1]==';')
      QueryString.remove(QueryString.length()-1,1);
    query(QueryString);
    if (CurrentTab==Plan)
      Plan->query(QueryString);
    else if (CurrentTab==Resources)
      viewResources();
  }
}
