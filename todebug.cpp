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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include <ctype.h>
#include <stack>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qnamespace.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qsizepolicy.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstring.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qworkspace.h>

#include "toconf.h"
#include "todebug.h"
#include "todebugtext.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "tooutput.h"
#include "toparamget.h"
#include "toresultview.h"
#include "totool.h"
#include "tosql.h"

#include "todebug.moc"
#include "todebugchange.moc"
#include "todebugwatch.moc"

#include "icons/addwatch.xpm"
#include "icons/changewatch.xpm"
#include "icons/compile.xpm"
#include "icons/delwatch.xpm"
#include "icons/enablebreak.xpm"
#include "icons/execute.xpm"
#include "icons/nextbug.xpm"
#include "icons/prevbug.xpm"
#include "icons/refresh.xpm"
#include "icons/returnfrom.xpm"
#include "icons/scansource.xpm"
#include "icons/showbody.xpm"
#include "icons/showdebug.xpm"
#include "icons/showhead.xpm"
#include "icons/stepinto.xpm"
#include "icons/stepover.xpm"
#include "icons/stop.xpm"
#include "icons/todebug.xpm"
#include "icons/togglebreak.xpm"
#include "icons/toworksheet.xpm"

#define TO_ID_NEW_SHEET		(TO_TOOL_MENU_ID+ 0)
#define TO_ID_SCAN_SOURCE	(TO_TOOL_MENU_ID+ 1)
#define TO_ID_COMPILE		(TO_TOOL_MENU_ID+ 2)
#define TO_ID_EXECUTE		(TO_TOOL_MENU_ID+ 3)
#define TO_ID_STOP		(TO_TOOL_MENU_ID+ 4)
#define TO_ID_STEP_INTO		(TO_TOOL_MENU_ID+ 5)
#define TO_ID_STEP_OVER		(TO_TOOL_MENU_ID+ 6)
#define TO_ID_RETURN_FROM	(TO_TOOL_MENU_ID+ 7)
#define TO_ID_HEAD_TOGGLE	(TO_TOOL_MENU_ID+ 8)
#define TO_ID_DEBUG_PANE	(TO_TOOL_MENU_ID+ 9)
#define TO_ID_DEL_WATCH		(TO_TOOL_MENU_ID+10)
#define TO_ID_CHANGE_WATCH	(TO_TOOL_MENU_ID+11)

class toDebugTool : public toTool {
  map<toConnection *,QWidget *> Windows;

  virtual char **pictureXPM(void)
  { return todebug_xpm; }
public:
  toDebugTool()
    : toTool(10,"PL/SQL Debugger")
  { }
  virtual const char *menuItem()
  { return "PL/SQL Debugger"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end()) {
      (*i).second->setFocus();
      return NULL;
    } else {
      QWidget *window=new toDebug(parent,connection);
      Windows[&connection]=window;
      window->setIcon(*toolbarImage());
      return window;
    }
  }
  void closeWindow(toConnection &connection)
  {
    map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end())
      Windows.erase(i);
  }
};

static toDebugTool DebugTool;

QListViewItem *toDebugWatch::findMisc(const QString &str,QListViewItem *item,toDebugText *editor)
{
  if (item) {
    while(item&&item->text(0)!=str) {
      item=item->nextSibling();
    }
    if (item) {
      item=item->firstChild();
      while(item&&item->text(0)!="Misc") {
	item=item->nextSibling();
      }
      if (item)
	item=item->firstChild();
    }
  }
  return item;
}

toDebugWatch::toDebugWatch(toDebug *parent)
  : toDebugWatchUI(parent,"AddWatch",true),Debugger(parent)
{
  QListViewItem *items=Debugger->contents();
  {
    int curline,curcol;
    Debugger->currentEditor()->getCursorPosition (&curline,&curcol);
    Default=Debugger->currentEditor()->textLine(curline);
    while(curcol>0&&toIsIdent(Default[curcol-1]))
      curcol--;
    while(curcol<int(Default.length())&&!toIsIdent(Default[curcol]))
      curcol++;
    Default.replace(0,curcol,"");
    curcol=1;
    while(curcol<int(Default.length())&&toIsIdent(Default[curcol]))
      curcol++;
    Default=Default.left(curcol);
  }
  QString type=Debugger->headEditor()->type();
  if (type.left(7)=="PACKAGE"||type.left(4)=="TYPE")
    HeadItems=findMisc("Head",items,Debugger->headEditor());
  else {
    HeadScope->setEnabled(false);
    HeadItems=NULL;
  }
  type=Debugger->bodyEditor()->type();
  if (type.left(7)=="PACKAGE"||type.left(4)=="TYPE")
    BodyItems=findMisc("Body",items,Debugger->bodyEditor());
  else {
    BodyScope->setEnabled(false);
    BodyItems=NULL;
  }
  
  Object=Debugger->currentEditor()->object();

  connect(Scope,SIGNAL(clicked(int)),this,SLOT(changeScope(int)));
  changeScope(1);
}

void toDebugWatch::changeScope(int num)
{
  switch(num) {
  case 1:
    Name->clear();
    Name->insertItem(Default);
    break;
  case 2:
    Name->clear();
    {
      for (QListViewItem *item=HeadItems;item;item=item->nextSibling())
        Name->insertItem(item->text(0));
    }
    break;
  case 3:
    Name->clear();
    {
      for (QListViewItem *item=BodyItems;item;item=item->nextSibling())
	Name->insertItem(item->text(0));
    }
    break;
  case 4:
    {
      Name->clear();
      QString str=Debugger->currentEditor()->schema();
      str+=".";
      if (!Object.isEmpty()) {
	str+=Object;
	str+=".";
      }
      str+=Default;
      Name->insertItem(str);
    }
    break;
  }
}

QListViewItem *toDebugWatch::createWatch(QListView *watches)
{
  QString str;
  switch(Scope->id(Scope->selected())) {
  case 1:
    {
      toResultViewItem *item=new toResultViewItem(watches,NULL);
      item->setText(0,"");
      item->setText(1,"");
      item->setText(2,Name->currentText());
      item->setText(3,"");
      item->setText(4,"NOCHANGE");
      return item;
    }
  case 2:
    str=Debugger->headEditor()->schema();
    str+=".";
    str+=Debugger->headEditor()->object();
    str+=".";
    str+=Name->currentText();
    break;
  case 3:
    str=Debugger->bodyEditor()->schema();
    str+=".";
    str+=Debugger->bodyEditor()->object();
    str+=".";
    str+=Name->currentText();
    break;
  case 4:
    str=Name->currentText();
    break;
  }
  QString schema;
  QString object;
  QString name;
  int pos=str.find(".");
  if (pos>0) {
    schema=str.left(pos);
    str.remove(0,pos+1);
  } else {
    toStatusMessage("Can't parse location");
    return NULL;
  }
  pos=str.find(".");
  if (pos>0) {
    object=str.left(pos);
    str.remove(0,pos+1);
  }
  toResultViewItem *item=new toResultViewItem(watches,NULL);
  item->setText(0,schema);
  item->setText(1,object);
  item->setText(2,str);
  item->setText(3,"");
  item->setText(4,"NOCHANGE");
  return item;
}

static toSQL SQLDebugOutputPoll("toDebugOutput:Poll",
				"DECLARE\n"
				"   coll sys.dbms_debug_vc2coll;\n"
				"   ret INTEGER;\n"
				"   i INTEGER;\n"
				"   line VARCHAR2(100);\n"
				"   errm VARCHAR2(100);\n"
				"BEGIN\n"
				"  DBMS_DEBUG.EXECUTE('DECLARE '||\n"
				"                     '  pp SYS.dbms_debug_vc2coll := SYS.dbms_debug_vc2coll(); '||\n"
				"                     '  line VARCHAR2(100); '||\n"
				"                     '  ret INTEGER; '||\n"
				"                     'BEGIN '||\n"
				"                     '  DBMS_OUTPUT.GET_LINE(line,ret); '||\n"
				"                     '  pp.EXTEND(2); '||\n"
				"                     '  pp(1):=ret; '||\n"
				"                     '  pp(2):=line; '||\n"
				"                     '  :1 := pp; '||\n"
				"                     'END;',\n"
				"                     -1,\n"
				"                     1,\n"
				"                     coll,\n"
				"                     line);\n"
				"  i:=coll.first;\n"
				"  IF i IS NOT NULL THEN\n"
				"    ret:=coll(i);\n"
				"    IF ret = 0 THEN\n"
				"      i:=coll.next(i);\n"
				"      line:=coll(i);\n"
				"    ELSE\n"
				"      line:='';\n"
				"    END IF;\n"
				"  ELSE\n"
				"    ret:=1;\n"
				"  END IF;\n"
				"  SELECT ret,line INTO :ret<int,out>,:line<char[101,out> FROM DUAL;\n"
				"END;",
				"Poll for output in the debug session, must have same bindings");
static toSQL SQLDebugOutputDisable("toDebugOutput:Disable",
				   "DECLARE\n"
				   "   coll sys.dbms_debug_vc2coll;\n"
				   "   errm VARCHAR2(100);\n"
				   "BEGIN\n"
				   "  DBMS_DEBUG.EXECUTE('BEGIN DBMS_OUTPUT.DISABLE; END;',\n"
				   "                     -1,\n"
				   "                     0,\n"
				   "                     coll,\n"
				   "                     errm);\n"
				   "END;",
				   "Disable output in the debug session");
static toSQL SQLDebugOutputEnable("toDebugOutput:Enable",
				  "DECLARE\n"
				  "   coll sys.dbms_debug_vc2coll;\n"
				  "   errm VARCHAR2(100);\n"
				  "BEGIN\n"
				  "  DBMS_DEBUG.EXECUTE('BEGIN DBMS_OUTPUT.ENABLE; END;',\n"
				  "                     -1,\n"
				  "                     0,\n"
				  "                     coll,\n"
				  "                     errm);\n"
				  "END;",
				  "Enable output in the debug session");

class toDebugOutput : public toOutput {
  toDebug *Debugger;
public:
  toDebugOutput(toDebug *debug,QWidget *parent,toConnection &conn)
    : toOutput(parent,conn),Debugger(debug)
  {
  }
  virtual void refresh(void)
  {
    if (Debugger->isRunning()&&enabled()) {
      try {
	int ret;
	do {
	  otl_stream poll(1,
			  SQLDebugOutputPoll(Connection),
			  Connection.connection());
	  poll>>ret;
	  char buffer[101];
	  poll>>buffer;
	  if (ret==0||strlen(buffer))
	    insertLine(QString::fromUtf8(buffer));
	} while(ret==0);
      } TOCATCH
    }    
  }
  virtual void disable(bool dis)
  {
    if (Debugger->isRunning()) {
      try {
	if (dis)
	  otl_cursor::direct_exec(Connection.connection(),
				  SQLDebugOutputDisable(Connection));
	else
	  otl_cursor::direct_exec(Connection.connection(),
				  SQLDebugOutputEnable(Connection));
      } catch (...) {
	toStatusMessage("Couldn't enable/disable output for session");
      }
    }
  }
};

bool toDebug::isRunning(void)
{
  toLocker lock(Lock);
  return RunningTarget;
}

static toSQL SQLDebugInit("toDebug:Initialize",
			  "DECLARE\n"
			  "  ret VARCHAR2(200);\n"
			  "BEGIN\n"
			  "  \n"
			  "  ret:=DBMS_DEBUG.INITIALIZE;\n"
			  "  DBMS_DEBUG.DEBUG_ON;\n"
			  "  SELECT ret,dbms_session.unique_session_id INTO :f2<char[201],out>,:f3<char[25],out> FROM DUAL;\n"
			  "END;",
			  "Initialize the debug session, must have same bindings");


void toDebug::targetTask::run(void)
{
  {
    toConnection Connection(Parent.Connection);
    otl_stream init(1,
		    SQLDebugInit(Connection),
		    Connection.connection());
    int colSize;
    {
      char buffer[201];
      toLocker lock(Parent.Lock);
      init>>buffer;
      Parent.TargetID=QString::fromUtf8(buffer);
      init>>buffer;
      Parent.TargetSession=QString::fromUtf8(buffer);
      Parent.ChildSemaphore.up();
      Parent.TargetLog+="Debug session connected\n";
    }
    while(1) {
      {
	toLocker lock(Parent.Lock);
	Parent.RunningTarget=false;
	colSize=Parent.ColumnSize;
      }
      Parent.TargetSemaphore.down();

      QString sql;
      list<QString> inParams;
      list<QString> *outParams=NULL;
      {
	toLocker lock(Parent.Lock);
	Parent.RunningTarget=true;
	sql=Parent.TargetSQL;
	Parent.TargetSQL="";
	inParams=Parent.InputData;
	Parent.InputData.clear(); // To make sure data is not shared
	Parent.OutputData.clear();
      }
      if (sql.isEmpty())
	break;

      try {
	Parent.Lock.lock();
	Parent.TargetLog+="Executing SQL\n";
	Parent.Lock.unlock();

	otl_stream q(1,
		     sql.utf8(),
		     Connection.connection());

	otl_null null;
	for(list<QString>::iterator i=inParams.begin();i!=inParams.end();i++) {
	  if ((*i).isNull())
	    q<<null;
	  else
	    q<<(*i).utf8();
	}

	outParams=new list<QString>;

	char *buffer=new char[colSize];
	while(!q.eof()) {
	  try {
	    q>>buffer;
	  } catch (...) {
	    delete buffer;
	    throw;
	  }
	  if (q.is_null())
	    outParams->insert(outParams->end(),"{null}");
	  else
	    outParams->insert(outParams->end(),QString::fromUtf8(buffer));
	}
	delete buffer;
      } catch (const QString &str) {
	Parent.Lock.lock();
	Parent.TargetLog+="Encountered error: ";
	Parent.TargetLog+=str;
	Parent.TargetLog+="\n";
	Parent.Lock.unlock();
      } catch (const otl_exception &exc) {
	Parent.Lock.lock();
	Parent.TargetLog+="SQL Error Encountered\n";
	Parent.TargetLog+=QString::fromUtf8((const char *)exc.msg);
	Parent.Lock.unlock();
      } catch (...) {
	Parent.Lock.lock();
	Parent.TargetLog+="Encountered unknown exception\n";
	Parent.Lock.unlock();
      }

      {
	toLocker lock(Parent.Lock);
	if (outParams) {
	  Parent.OutputData=(*outParams);
	  delete outParams;
	}
	Parent.TargetLog+="Execution ended\n";
      }
      Parent.ChildSemaphore.up();
    }

  }
  toLocker lock(Parent.Lock);
  Parent.TargetLog+="Closing debug session\n";
  Parent.TargetThread=NULL;
  Parent.ChildSemaphore.up();
}

static struct toBlock {
  int Pos;
  char *Start;
  bool WantEnd;
  bool WantSemi;
  bool CloseBlock;
  bool ChangeDecl;
  bool Declaration;
  bool SeparateType;
  char *TypeName;
} Blocks[] = { { 0,"FUNCTION",  false,false,false,true ,true ,true , "Function"  }, // Must be first in list
	       { 0,"BEGIN",	true ,false,false,true ,false,false, NULL        },
	       { 0,"IF",	true ,false,false,false,false,false, NULL        },
	       { 0,"LOOP",	true ,false,false,false,false,false, NULL        },
	       { 0,"WHILE",	true ,false,false,false,false,false, NULL        },
	       { 0,"DECLARE",	true ,false,false,true ,true ,false, NULL        },
	       { 0,"PROCEDURE", false,false,false,true ,true ,true , "Procedure" },
	       { 0,"AS",	true ,false,false,true ,true ,false, NULL        },
	       { 0,"IS",	true ,false,false,true ,true ,false, NULL        },
	       { 0,"PACKAGE",   true ,false,false,true ,true ,true , "Package"   },
	       { 0,"TYPE",      false,false,false,false,false,true , "Type"      },
	       { 0,"CURSOR",    false,false,false,false,false,true , "Cursor"    },
	       { 0,"END",	false,true ,true ,false,false,false, NULL        },
	       { 0,NULL,	false,false,false,false,false,false, NULL        }
};

static void toClearBlocks(void)
{
  for(int i=0;Blocks[i].Start;i++)
    Blocks[i].Pos=0;
}

static QListViewItem *toFindType(QListViewItem *parent,const QString &type)
{
  QString dsc;
  if (type.isEmpty())
    dsc="Misc";
  else
    dsc=type;
    
  {
    for(QListViewItem *item=parent->firstChild();item;item=item->nextSibling())
      if (item->text(0)==dsc)
        return item;
  }

  QListViewItem *lastItem=NULL;
  {
    for(QListViewItem *item=parent->firstChild();item;item=item->nextSibling()) {
      if (dsc<item->text(0))
        break;
      lastItem=item;
    }
  }
  QListViewItem *item=new QListViewItem(parent,lastItem,dsc);
#ifndef AUTOEXPAND
  item->setSelectable(false);
#endif
  return item;
}

static QListViewItem *toLastItem(QListViewItem *parent) {
  QListViewItem *lastItem=NULL;
  for (QListViewItem *item=parent->firstChild();item;item=item->nextSibling())
    lastItem=item;
  return lastItem;
}

class toContentsItem : public QListViewItem
{
public:
  int Line;
  toContentsItem(QListViewItem *parent,const QString &name,int line)
    : QListViewItem(parent,toLastItem(parent),name)
  { Line=line; }
};

void toDebug::reorderContent(QListViewItem *parent,int start,int diff)
{
  QListViewItem *next;
  for (QListViewItem *item=parent->firstChild();item;item=next) {
    toContentsItem *cont=dynamic_cast<toContentsItem *>(item);
    next=item->nextSibling();
    bool del=false;
    if (cont) {
      int line=toHighlightedText::convertLine(cont->Line,start,diff);
      if (line>=0)
	cont->Line=line;
      else {
	delete cont;
	del=true;
      }
    }
    if (!del&&item->firstChild())
      reorderContent(item,start,diff);
  }
}

QString toDebug::currentName(void)
{
  if (BodyEditor->isHidden())
    return "Head";
  else
    return "Body";
}

QString toDebug::currentSchema(void)
{
  return Schema->currentText();
}

QListViewItem *toDebug::contents(void)
{
  return Contents->firstChild();
}

void toDebug::reorderContent(int start,int diff)
{
  QString name=currentName();
  for (QListViewItem *item=Contents->firstChild();item;item=item->nextSibling()) {
    if (item->text(0)==name)
      reorderContent(item,start,diff);
  }
}

static QString toGetToken(toHighlightedText *text,int &curLine,int &pos)
{
  QChar c;
  QChar nc;
  QChar endString;

  enum {
    space,
    any,
    identifier,
    string
  } state=space;

  QString token;

  while(curLine<int(text->numLines())) {
    QString line=text->textLine(curLine);
    while(pos<int(line.length())) {
      c=line[pos];
      if (pos<int(line.length()-1))
	nc=line[pos+1];
      else
	nc='\n';
      pos++;
      if (state==space) {
	if(c=='-'&&nc=='-')
	  break;
	if(!c.isSpace())
	  state=any;
      }
      if (state!=space) {
	token+=c;
	switch(state) {
	case space:
	  break;
	case any:
	  if (toIsIdent(c)) {
	    if (!toIsIdent(nc))
	      return token;
	    state=identifier;
	  }
	  else if (c=='\''||c=='\"') {
	    endString=c;
	    state=string;
	  } else {
	    if (c==':'&&nc=='=') {
	      token+=nc;
	      pos++;
	    }
	    return token;
	  }
	  break;
	case identifier:
	  if (!toIsIdent(nc))
	    return token;
	  break;
	case string:
	  if (c==endString)
	    return token;
	  break;
	}
      }
    }
    pos=0;
    curLine++;
  }
  return "";
}

bool toDebug::hasMembers(const QString &str)
{
  if (str=="PACKAGE"||str=="PACKAGE BODY"||
      str=="TYPE"||str=="TYPE BODY")
    return true;
  else
    return false;
}

void toDebug::execute(void)
{
  if (!checkStop())
    return;

  if (!checkCompile())
    return;

  QString curName=currentName();
  toHighlightedText *current=currentEditor();
  int curline,curcol;
  current->getCursorPosition (&curline,&curcol);

  bool valid=false;
  int line=-1;
  if (hasMembers(currentEditor()->type())) {
    for (QListViewItem *parent=Contents->firstChild();parent;parent=parent->nextSibling()) {
      if (parent->text(0)==curName) {
	for (parent=parent->firstChild();parent;parent=parent->nextSibling()) {
	  for (QListViewItem *item=parent->firstChild();item;item=item->nextSibling()) {
	    toContentsItem *cont=dynamic_cast<toContentsItem *>(item);
	    if (cont) {
	      QString type=cont->parent()->text(0);
	      if (cont->Line>curline)
		break;
	      if (cont->Line>line) {
		line=cont->Line;
		if (type=="Procedure"||type=="Function")
		  valid=true;
		else
		  valid=false;
	      }
	    }
	  }
	}
	break;
      }
    }
  } else if (currentEditor()->type()=="PROCEDURE"||
	     currentEditor()->type()=="FUNCTION") {
    valid=true;
    line=0;
  }

  if (valid) {
    int pos=0;
    QString token;
    int level=0;

    enum {
      beginning,
      waitingEnd,
      returnType,
      parType,
      inOut,
      name,
      done
    } state=beginning;

    CurrentParams.clear();
    list<debugParam>::iterator cp=CurrentParams.begin();
    QString callName;
    QString retType;

    token=toGetToken(current,line,pos);
    if (token.upper()!="FUNCTION"&&token.upper()!="PROCEDURE") {
      toStatusMessage("Expected function or procedure, internal error");
      return;
    }
    do {
      token=toGetToken(current,line,pos);
      if (token.isEmpty()) {
	toStatusMessage("Unexpected end of declaration.");
	return;
      }
      if (state==returnType) {
	if (retType.isEmpty()||retType.at(retType.length()-1)=='.'||token==".")
	  retType+=token;
	else
	  state=done;
      } else if (token.upper()=="RETURN"&&level==0) {
	state=returnType;
      } if (token=="(") {
	level++;
      } else if (token==")")
	level--;
      else if (level==1) {
	switch(state) {
	case name:
	  {
	    debugParam newPar;
	    CurrentParams.insert(CurrentParams.end(),newPar);
	    cp++;
	    (*cp).Name=token;
	    state=inOut;
	    break;
	  }
	case inOut:
	  if (token.upper()=="IN") {
	    (*cp).In=true;
	    break;
	  } else if (token.upper()=="OUT") {
	    (*cp).Out=true;
	    break;
	  } else if (token.upper()=="NOCOPY")
	    break;
	  if (!(*cp).In&&!(*cp).Out)
	    (*cp).In=true;
	  state=parType;
	case parType:
	  if (token==",") {
	    state=name;
	    break;
	  } else if (token.upper()=="DEFAULT"||token==":=") {
	    state=waitingEnd;
	    break;
	  } else {
	    (*cp).Type+=token;
	  }
	case done:
	case returnType:
	case beginning:
	case waitingEnd:
	  break;
	}
      } else if (state==beginning) {
	callName=token;
	state=name;
      }
    } while(state!=done&&token.upper()!="IS"&&token.upper()!="AS"&&token!=";");

    QChar sep='(';
    QString sql;
    if (!retType.isEmpty())
      sql+="DECLARE\n  ret VARCHAR2(4000);\n";
    sql+="BEGIN\n  ";
    if (!retType.isEmpty())
      sql+="ret:=";
    sql+=currentEditor()->schema();
    sql+=".";
    if (hasMembers(currentEditor()->type())) {
      sql+=currentEditor()->object();
      sql+=".";
    }
    sql+=callName;

    Parameters->clear();
    QListViewItem *head=new toResultViewItem(Parameters,NULL,"Input");
    QListViewItem *last=NULL;
    head->setOpen(true);

    for(list<debugParam>::iterator i=CurrentParams.begin();i!=CurrentParams.end();i++) {
      if ((*i).In)
	last=new toResultViewItem(head,last,(*i).Name);
      sql+=sep;
      sql+=":";
      QString nam=(*i).Name;
      nam.replace(QRegExp("[^a-zA-Z0-9]+"),"_");
      sql+=nam;
      sql+="<char[";
      sql+=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE);
      sql+="],";
      if ((*i).In)
	sql+="in";
      if ((*i).Out)
	sql+="out";
      sql+=">";
      sep=',';
    }
    if (sep==',')
      sql+=")";
    if (!retType.isEmpty()) {
      sql+=";\n  SELECT ret INTO :tora_int_return<char[";
      sql+=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE);
      sql+="],out> FROM DUAL";
    }
    sql+=";\nEND;\n";

    try {
      {
	// Can't hold lock since refresh of output will try to lock
	list<QString> input=toParamGet::getParam(this,sql);
	toLocker lock(Lock);
	InputData=input;
	last=head->firstChild();
	if (InputData.begin()!=InputData.end())
	  for(list<QString>::iterator i=InputData.begin();
	      last&&i!=InputData.end();
	      i++,last=last->nextSibling()) {
	    // Is there a smarter way to make a deep copy
	    last->setText(1,toDeepCopy(*i));
	  }
	else
	  delete head;
	ColumnSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();
	TargetSQL=toDeepCopy(sql); // Deep copy of SQL
	TargetSemaphore.up(); // Go go power rangers!
      }
      if (sync()>=0)
	continueExecution(TO_BREAK_ANY_CALL);
    } TOCATCH
  } else
    toStatusMessage("Couldn't find any function or procedure under cursor.");
}

static toSQL SQLSync("toDebug:Sync",
		     "DECLARE\n"
		     "  ret binary_integer;\n"
		     "  runinf DBMS_DEBUG.Runtime_Info;\n"
		     "BEGIN\n"
		     "  ret:=DBMS_DEBUG.SYNCHRONIZE(runinf,:info<int,in>);\n"
		     "  SELECT ret,\n"
		     "         runinf.Reason\n"
		     "    INTO :ret<int,out>,\n"
		     "         :reason<int,out>\n"
		     "    FROM DUAL;\n"
		     "END;",
		     "Sync with the debugging session, must have same binds");

int toDebug::sync(void)
{
  try {
    otl_stream sync(1,
		    SQLSync(Connection),
		    Connection.connection());
    int ret;
    int reason;
    do {
      sync<<0;
      sync>>ret>>reason;
      {
	toLocker lock(Lock);
	TargetLog+="Syncing debug session\n";
	if (!RunningTarget) {
	  return TO_REASON_KNL_EXIT;
	}
      }
#if 0
      qApp->processEvents();
#endif
    } while(reason==TO_REASON_TIMEOUT||ret==TO_ERROR_TIMEOUT);
    setDeferedBreakpoints();
    if (Output->enabled())
      Output->disable(false);
    else
      Output->disable(true);
    return reason;
  } TOCATCH
  return -1;
}

void toDebug::updateContent(bool body)
{
  toHighlightedText *current;
  QString topName;
  if (body) {
    topName="Body";
    current=BodyEditor;
  } else {
    topName="Head";
    current=HeadEditor;
  }
  
  enum {
    normal,
    endOfStatement,
    space,
    firstWord,
    inString,
  } state,beforeState=normal;

  {
    for (QListViewItem *item=Contents->firstChild();item;item=item->nextSibling()) {
      if (item->text(0)==topName) {
        delete item;
        break;
      }
    }
  }
  QListViewItem *parent;
  parent=new QListViewItem(Contents,NULL);
  parent->setText(0,topName);
#ifndef AUTOEXPAND
  parent->setSelectable(false);
#endif
  parent->setOpen(true);
  QListViewItem *item=NULL;

  QChar stringEnd;
  stack<QString> parentType;
  stack<bool> parentDecl;
  stack<QListViewItem *> parentItem;
  QString type="";
  bool declaration=true;
  state=space;
  int startFirst=0;
  bool firstIgnore=true;

  QString is("IS");
  for(int cline=0;cline<current->numLines();cline++) {
    QString line=current->textLine(cline);
    QChar lc='\n';
    toClearBlocks();
    for (unsigned int i=0;i<line.length();i++) {
      QChar c=line.at(i);
      QChar nc;
      if (i==line.length()-1)
	nc='\n';
      else
	nc=line.at(i+1);
      if (state!=inString&&c=='-'&&nc=='-') {
	i=line.length();
      } else {
	switch(state) {
	case space:
	  if (c.isSpace())
	    break;
	  startFirst=i;
	  state=firstWord;
	case firstWord:
	  if (!toIsIdent(c))
	    state=normal;
	case normal:
	  switch(char(c)) {
	  case '\'':
	  case '\"':
	    beforeState=state;
	    state=inString;
	    stringEnd=c;
	    break;
	  case ';':
	    state=space;
	    type="";
	    break;
	  default:
	    for(int j=0;Blocks[j].Start;j++) {
	      toBlock &cb=Blocks[j];
	      if ((cb.Pos>0||!toIsIdent(lc))&&cb.Start[cb.Pos]==c.upper()) {
		cb.Pos++;
		if (!cb.Start[cb.Pos]) {
		  if (!toIsIdent(nc)&&(is!=cb.Start||type=="Procedure"||type=="Function")) { // IS is a special case, only calid after procedure or function
		    if (cb.WantEnd&&(!declaration||cb.Declaration)) { // Ignore begin after declare
		      parentDecl.push(declaration);
		      parentType.push(type);
		      parentItem.push(parent);
		      if (!declaration) {
			QListViewItem *tp=toFindType(parent,"Anonymous");
			item=new toContentsItem(tp,cb.Start,cline);
		      }
		      if (item) {
			parent=item;
			item=NULL;
		      }
		    }

		    if (cb.CloseBlock) {
		      if (parentType.size()>0) {
			type=parentType.top();
			declaration=parentDecl.top();
			parent=parentItem.top();
			parentDecl.pop();
			parentType.pop();
			parentItem.pop();
		      } else {
			type="";
			declaration=true;
		      }
		    }
		    if (cb.ChangeDecl)
		      declaration=cb.Declaration;
		    if (cb.WantSemi) {
		      state=endOfStatement;
		      break;
		    }
		    if (cb.SeparateType)
		      type=cb.TypeName;
		    else
		      type="";
		    state=space;
		  } else
		    cb.Pos=0;
		}
	      } else
		cb.Pos=0;
	    }
	    if(!toIsIdent(nc)&&state==firstWord&&declaration) {
	      if (firstIgnore) // Usually procude/package definition, don't want that one.
		firstIgnore=false;
	      else {
		QString str=line;
		str=str.left(i+1);
		str=str.right(i+1-startFirst);
		if (str.upper()!="PRAGMA") {
		  QListViewItem *tp=toFindType(parent,type);
		  item=new toContentsItem(tp,str,cline);
		}
	      }
	    }
	    break;
	  }
	  break;
	case inString:
	  if (c==stringEnd)
	    state=beforeState;
	  break;
	case endOfStatement:
	  switch(char(c)) {
	  case '\'':
	  case '\"':
	    beforeState=state;
	    state=inString;
	    stringEnd=c;
	    break;
	  case ';':
	    state=space;
	    type="";
	    break;
	  default:
	    break;
	  }
	  break;
	}
      }
      lc=c;
    }
  }
  if (!parent->firstChild())
    delete parent;
}

static QPixmap *toRefreshPixmap;
static QPixmap *toCompilePixmap;
static QPixmap *toExecutePixmap;
static QPixmap *toStopPixmap;
static QPixmap *toStepOverPixmap;
static QPixmap *toStepIntoPixmap;
static QPixmap *toReturnFromPixmap;
static QPixmap *toShowHeadPixmap;
static QPixmap *toShowBodyPixmap;
static QPixmap *toNextBugPixmap;
static QPixmap *toPrevBugPixmap;
static QPixmap *toScanPixmap;
static QPixmap *toNewPixmap;
static QPixmap *toDebugPixmap;
static QPixmap *toToggleBreakPixmap;
static QPixmap *toEnableBreakPixmap;
static QPixmap *toAddWatchPixmap;
static QPixmap *toChangeWatchPixmap;
static QPixmap *toDelWatchPixmap;

void toDebug::readLog(void)
{
  toLocker lock(Lock);
  if (!TargetLog.isEmpty()) {
    TargetLog.replace(TargetLog.length()-1,1,"");
    RuntimeLog->insertLine(TargetLog);
    RuntimeLog->setCursorPosition(RuntimeLog->numLines()-1,0);
    TargetLog="";
  }
}

static toSQL SQLRuntimeInfo("toDebug:RuntimeInfo",
			    "DECLARE\n"
			    "  info DBMS_DEBUG.RUNTIME_INFO;\n"
			    "  ret BINARY_INTEGER;\n"
			    "BEGIN\n"
			    "  ret:=DBMS_DEBUG.GET_RUNTIME_INFO(DBMS_DEBUG.info_getStackDepth,info);\n"
			    "  SELECT ret,info.stackDepth INTO :ret<int,out>,:depth<int,out> FROM DUAL;\n"
			    "END;",
			    "Get runtime info from debug session, must have same bindings");
static toSQL SQLStackTrace("toDebug:StackTrace",
			   "DECLARE\n"
			   "  info DBMS_DEBUG.PROGRAM_INFO;\n"
			   "  stack DBMS_DEBUG.BACKTRACE_TABLE;\n"
			   "  i BINARY_INTEGER;\n"
			   "BEGIN\n"
			   "  DBMS_DEBUG.PRINT_BACKTRACE(stack);\n"
			   "  i:=:num<int,in>;\n"
			   "  info:=stack(i);\n"
			   "  SELECT info.Name,info.Owner,info.Line#,DECODE(info.LibunitType,DBMS_DEBUG.LibunitType_cursor,'CURSOR',\n"
			   "                                                                 DBMS_DEBUG.LibunitType_function,'FUNCTION',\n"
			   "                                                                 DBMS_DEBUG.LibunitType_procedure,'PROCEDURE',\n"
			   "                                                                 DBMS_DEBUG.LibunitType_package,'PACKAGE',\n"
			   "                                                                 DBMS_DEBUG.LibunitType_package_body,'PACKAGE BODY',\n"
			   "                                                                 DBMS_DEBUG.LibunitType_trigger,'TRIGGEER',\n"
			   "                                                                 'UNKNOWN')\n"
			   "    INTO :name<char[31],out>,:owner<char[31],out>,:line<int,out>,:type<char[31],out> FROM DUAL;\n"
			   "END;",
			   "Get stacktrace from debug session, must have same bindings");
static toSQL SQLLocalWatch("toDebug:LocalWatch",	
			   "DECLARE\n"
			   "  ret BINARY_INTEGER;\n"
			   "  data VARCHAR2(4000);\n"
			   "BEGIN\n"
			   "  ret:=DBMS_DEBUG.GET_VALUE(:name<char[41],in>,0,data,NULL);\n"
			   "  SELECT ret,data INTO :ret<int,out>,:val<char[4001],out> FROM DUAL;\n"
			   "END;",
			   "Get data from local watch, must have same bindings");
static toSQL SQLGlobalWatch("toDebug:GlobalWatch",
			    "DECLARE\n"
			    "  data VARCHAR2(4000);\n"
			    "  proginf DBMS_DEBUG.program_info;\n"
			    "  ret BINARY_INTEGER;\n"
			    "BEGIN\n"
			    "  proginf.Namespace:=DBMS_DEBUG.Namespace_pkg_body;\n"
			    "  proginf.Name:=:object<char[41],in>;\n"
			    "  proginf.Owner:=:owner<char[41],in>;\n"
			    "  proginf.DBLink:=NULL;\n"
			    "  ret:=DBMS_DEBUG.GET_VALUE(:name<char[41],in>,proginf,data,NULL);\n"
			    "  IF ret =DBMS_DEBUG.error_no_such_object THEN\n"
			    "    proginf.Namespace:=DBMS_DEBUG.namespace_pkgspec_or_toplevel;\n"
			    "    ret:=DBMS_DEBUG.GET_VALUE(:name<char[41],in>,proginf,data,NULL);\n"
			    "  END IF;\n"
			    "  SELECT ret          ,data                ,proginf.Namespace\n"
			    "    INTO :ret<int,out>,:val<char[4001],out>,:namespace<int,out>\n"
			    "    FROM DUAL;\n"
			    "END;",
			    "Get data from global watch, must have same bindings");
static toSQL SQLLocalIndex("toDebug:LocalIndex",
			   "DECLARE\n"
			   "  ret BINARY_INTEGER;\n"
			   "  proginf DBMS_DEBUG.program_info;\n"
			   "  i BINARY_INTEGER;\n"
			   "  indata DBMS_DEBUG.index_table;\n"
			   "  outdata VARCHAR2(4000) := '';\n"
			   "BEGIN\n"
			   "  ret:=DBMS_DEBUG.GET_INDEXES(:name<char[41],in>,0,proginf,indata);\n"
			   "  IF ret = DBMS_DEBUG.success THEN\n"
			   "    i:=indata.first;\n"
			   "    WHILE i IS NOT NULL OR LENGTH(outdata)>3900 LOOP\n"
			   "      outdata:=outdata||indata(i)||',';\n"
			   "      i:=indata.next(i);\n"
			   "    END LOOP;\n"
			   "  END IF;\n"
			   "  SELECT outdata INTO :data<char[4001],out> FROM DUAL;\n"
			   "END;",
			   "Get indexes of local watch, must have same bindings");
static toSQL SQLGlobalIndex("toDebug:GlobalIndex",
			    "DECLARE\n"
			    "  ret BINARY_INTEGER;\n"
			    "  proginf DBMS_DEBUG.program_info;\n"
			    "  i BINARY_INTEGER;\n"
			    "  indata DBMS_DEBUG.index_table;\n"
			    "  outdata VARCHAR2(4000) := '';\n"
			    "BEGIN\n"
			    "  proginf.Namespace:=:namespace<int,in>;\n"
			    "  proginf.Name:=:object<char[41],in>;\n"
			    "  proginf.Owner:=:owner<char[41],in>;\n"
			    "  proginf.DBLink:=NULL;\n"
			    "  ret:=DBMS_DEBUG.GET_INDEXES(:name<char[41],in>,NULL,proginf,indata);\n"
			    "  IF ret = DBMS_DEBUG.success THEN\n"
			    "    i:=indata.first;\n"
			    "    WHILE i IS NOT NULL OR LENGTH(outdata)>3900 LOOP\n"
			    "      outdata:=outdata||indata(i)||',';\n"
			    "      i:=indata.next(i);\n"
			    "    END LOOP;\n"
			    "  END IF;\n"
			    "  SELECT outdata INTO :data<char[4001],out> FROM DUAL;\n"
			    "END;",
			    "Get indexes of global watch, must have same bindings");

void toDebug::updateState(int reason)
{
  switch(reason) {
  case TO_REASON_EXIT:
  case TO_REASON_KNL_EXIT:
    ChildSemaphore.down();
  case TO_REASON_NO_SESSION:
    StopButton->setEnabled(false);
    StepOverButton->setEnabled(false);
    StepIntoButton->setEnabled(false);
    ReturnButton->setEnabled(false);
    if (ToolMenu) {
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,false);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STEP_INTO,false);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STEP_OVER,false);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_RETURN_FROM,false);
    }
    HeadEditor->setCurrent(-1);
    BodyEditor->setCurrent(-1);
    StackTrace->clear();
    {
      toLocker lock(Lock);
      if (OutputData.begin()!=OutputData.end()) {
	QListViewItem *head=Parameters->firstChild();
	while(head&&head->nextSibling())
	  head=head->nextSibling();
	head=new toResultViewItem(Parameters,head,"Output");
	head->setOpen(true);
	list<debugParam>::iterator cp;
	for(cp=CurrentParams.begin();cp!=CurrentParams.end()&&!(*cp).Out;cp++)
	  ;

	QListViewItem *last=NULL;
	for (list<QString>::iterator i=OutputData.begin();i!=OutputData.end();i++) {
	  QString name;
	  if (cp!=CurrentParams.end()) {
	    name=(*cp).Name;
	    for(cp++;cp!=CurrentParams.end()&&!(*cp).Out;cp++)
	      ;
	  }
	  if (name.isEmpty())
	    name="Returning";
	  last=new toResultViewItem(head,last,name);
	  last->setText(1,toDeepCopy(*i)); // Deep copy just to be sure
	}
      }
    }
    {
      QListViewItem *next=NULL;
      for (QListViewItem *item=Watch->firstChild();item;item=next) {
	item->setText(4,"NOCHANGE");
	if (item->firstChild())
	  next=item->firstChild();
	else if (item->nextSibling())
	  next=item->nextSibling();
	else {
	  next=item;
	  do {
	    next=next->parent();
	  } while(next&&!next->nextSibling());
	  if (next)
	    next=next->nextSibling();
	}
      }
    }
    break;
  default:
    DebugButton->setOn(true);
    StopButton->setEnabled(true);
    StepOverButton->setEnabled(true);
    StepIntoButton->setEnabled(true);
    ReturnButton->setEnabled(true);
    if (ToolMenu) {
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,true);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STEP_INTO,true);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_STEP_OVER,true);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_RETURN_FROM,true);
    }
    try {
      otl_stream info(1,
		      SQLRuntimeInfo(Connection),
		      Connection.connection());
      int ret,depth;
      info>>ret;
      info>>depth;
      if (ret!=TO_SUCCESS) {
	QString str("Failed to get runtime info (Reason ");
	str+=QString::number(ret);
	str+=")";
	toStatusMessage(str);
	return;
      }
      otl_stream stack(1,
		       SQLStackTrace(Connection),
		       Connection.connection());
      char name[31];
      char schema[31];
      int line;
      char type[31];
      QListViewItem *item=NULL;
      StackTrace->clear();
      for (int num=2;num<=depth;num++) {
	stack<<num;
	stack>>name;
	stack>>schema;
	stack>>line;
	stack>>type;
	
	if (!item)
	  item=new QListViewItem(StackTrace,QString::fromUtf8(name),QString::number(line),QString::fromUtf8(schema),QString::fromUtf8(type));
	else
	  item=new QListViewItem(item,QString::fromUtf8(name),QString::number(line),QString::fromUtf8(schema),QString::fromUtf8(type));
	item->setOpen(true);
      }
      Output->refresh();
      try {
	{
	  for (QListViewItem *item=Watch->firstChild();item;item=item->nextSibling()) {
	    while (item->firstChild())
	      delete item->firstChild();
	  }
	}
	otl_stream local(1,
			 SQLLocalWatch(Connection),
			 Connection.connection());
	otl_stream global(1,
			  SQLGlobalWatch(Connection),
			  Connection.connection());
	otl_stream index(1,
			 SQLLocalIndex(Connection),
			 Connection.connection());
	otl_stream globind(1,
			   SQLGlobalIndex(Connection),
			   Connection.connection());
  
	QListViewItem *next=NULL;
	for (QListViewItem *item=Watch->firstChild();item;item=next) {
	  int ret;
	  int space;
	  char buffer[4001];
	  if (item->text(0).isEmpty()) {
	    local<<item->text(2).utf8();
	    local>>ret;
	    local>>buffer;
	  } else {
	    global<<item->text(1).utf8();
	    global<<item->text(0).utf8();
	    global<<item->text(2).utf8();
	    global>>ret;
	    global>>buffer;
	    global>>space;
	  }
	  item->setText(4,"");
	  if (ret==TO_SUCCESS)
	    item->setText(3,QString::fromUtf8(buffer));
	  else if (ret==TO_ERROR_NULLVALUE) {
	    item->setText(3,"{null}");
	    item->setText(5,"NULL");
	  } else if (ret==TO_ERROR_NULLCOLLECTION) {
	    item->setText(3,"[Count 0]");
	    item->setText(5,"LIST");
	  } else if (ret==TO_ERROR_INDEX_TABLE) {
	    char buffer[4001];
	    if (item->text(0).isEmpty()) {
	      index<<item->text(2).utf8();
	      index>>buffer;
	    } else {
	      globind<<space;
	      globind<<item->text(1).utf8();
	      globind<<item->text(0).utf8();
	      globind<<item->text(2).utf8();
	      globind>>buffer;
	    }
	    char *start=buffer;
	    char *end;
	    QListViewItem *last=NULL;
	    int num=0;
	    for (end=start;*end;end++) {
	      if (*end==',') {
		*end=0;
		if (start<end) {
		  QString name=item->text(2);
		  name+="(";
		  name+=QString::fromUtf8(start);
		  name+=")";
		  last=new toResultViewItem(item,last);
		  last->setText(0,item->text(0));
		  last->setText(1,item->text(1));
		  last->setText(2,name);
		  last->setText(3,"");
		  last->setText(4,"NOCHANGE");
		  last->setText(5,QString::fromUtf8(start));
		  num++;
		}
		start=end+1;
	      }
	    }
	    QString str="[Count ";
	    str+=QString::number(num);
	    str+="]";
	    item->setText(3,str);
	    item->setText(5,"LIST");
	  } else {
	    item->setText(3,"{Unavailable}");
	    item->setText(4,"NOCHANGE");
	  }
	  if (item->firstChild())
	    next=item->firstChild();
	  else if (item->nextSibling())
	    next=item->nextSibling();
	  else {
	    next=item;
	    do {
	      next=next->parent();
	    } while(next&&!next->nextSibling());
	    if (next)
	      next=next->nextSibling();
	  }
	}
      } TOCATCH
      if (depth>=2) {
	viewSource(QString::fromUtf8(schema),QString::fromUtf8(name),QString::fromUtf8(type),line,true);
      } else {
	continueExecution(TO_BREAK_NEXT_LINE);
	return;
      }
    } TOCATCH
    break;
  }
  selectedWatch();
  readLog();
}

bool toDebug::viewSource(const QString &schema,const QString &name,const QString &type,
			 int line,bool setCurrent)
{
  if (HeadEditor->schema()==schema&&
      HeadEditor->object()==name&&
      HeadEditor->type()==type) {
    if (setCurrent)
      HeadEditor->setCurrent(line-1);
    else
      HeadEditor->setCursorPosition(line-1,0);
    HeadEditor->setFocus();
    ShowButton->setOn(true);
  } else if (BodyEditor->schema()==schema&&
	     BodyEditor->object()==name&&
	     BodyEditor->type()==type) {
    if (setCurrent)
      BodyEditor->setCurrent(line-1);
    else
      BodyEditor->setCursorPosition(line-1,0);

    BodyEditor->setFocus();
    ShowButton->setOn(false);
  } else if (!BodyEditor->edited()&&(setCurrent||BodyEditor->current()<0)) {
    BodyEditor->setData(schema,type,name);
    BodyEditor->readData(Connection,StackTrace);
    BodyEditor->setCursorPosition(line-1,0);
    BodyEditor->setFocus();
    updateContent(true);
    ShowButton->setOn(false);
  } else if (!HeadEditor->edited()&&(setCurrent||HeadEditor->current()<0)) {
    toStatusMessage("Reading source into head editor");
    HeadEditor->setData(schema,type,name);
    HeadEditor->readData(Connection,StackTrace);
    HeadEditor->setCursorPosition(line-1,0);
    HeadEditor->setFocus();
    ShowButton->setEnabled(true);
    if (ToolMenu)
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_HEAD_TOGGLE,true);
    ShowButton->setOn(true);
    updateContent(false);
  } else { 
    if (setCurrent) {
      HeadEditor->setCurrent(-1);
      BodyEditor->setCurrent(-1);
    }
    return false;
  }
  return true;
}

void toDebug::setDeferedBreakpoints(void)
{
  for (QListViewItem *item=Breakpoints->firstChild();item;item=item->nextSibling()) {
    toBreakpointItem *point=dynamic_cast<toBreakpointItem *>(item);
    if (point) {
      if (point->text(4)=="DEFERED")
	point->setBreakpoint();
    }
  }
}

static toSQL SQLContinue("toDebug:Continue",
			 "DECLARE\n"
			 "  runinf DBMS_DEBUG.runtime_info;\n"
			 "  ret BINARY_INTEGER;\n"
			 "BEGIN\n"
			 "  ret:=DBMS_DEBUG.CONTINUE(runinf,:break<int,in>,NULL);\n"
			 "  SELECT ret,\n"
			 "         runinf.Reason\n"
			 "    INTO :ret<int,out>,\n"
			 "         :reason<int,out>\n"
			 "    FROM DUAL;\n"
			 "END;",
			 "Continue execution, must have same bindings");

int toDebug::continueExecution(int stopon)
{
  Lock.lock();
  if (RunningTarget) {
    Lock.unlock();
    try {
      int ret,reason;
      setDeferedBreakpoints();
      otl_stream cont(1,
		      SQLContinue(Connection),
		      Connection.connection());
      cont<<stopon;
      cont>>ret>>reason;
      if (reason==TO_REASON_TIMEOUT||ret==TO_ERROR_TIMEOUT) {
	reason=sync();
	if (reason<0)
	  ret=-1;
	else
	  ret=TO_SUCCESS;
      }
      if (ret!=TO_SUCCESS)
	return -1;
      updateState(reason);
      return reason;
    } TOCATCH
  } else {
    toStatusMessage("No running target");
    Lock.unlock();
  }
  return -1;
}

void toDebug::stop(void)
{
  continueExecution(TO_ABORT_EXECUTION);
}

toDebug::toDebug(QWidget *main,toConnection &connection)
  : QVBox(main,NULL,WDestructiveClose),Connection(connection),
    TargetThread()
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toCompilePixmap)
    toCompilePixmap=new QPixmap((const char **)compile_xpm);
  if (!toExecutePixmap)
    toExecutePixmap=new QPixmap((const char **)execute_xpm);
  if (!toStopPixmap)
    toStopPixmap=new QPixmap((const char **)stop_xpm);
  if (!toStepOverPixmap)
    toStepOverPixmap=new QPixmap((const char **)stepover_xpm);
  if (!toStepIntoPixmap)
    toStepIntoPixmap=new QPixmap((const char **)stepinto_xpm);
  if (!toReturnFromPixmap)
    toReturnFromPixmap=new QPixmap((const char **)returnfrom_xpm);
  if (!toShowHeadPixmap)
    toShowHeadPixmap=new QPixmap((const char **)showhead_xpm);
  if (!toShowBodyPixmap)
    toShowBodyPixmap=new QPixmap((const char **)showbody_xpm);
  if (!toNextBugPixmap)
    toNextBugPixmap=new QPixmap((const char **)nextbug_xpm);
  if (!toPrevBugPixmap)
    toPrevBugPixmap=new QPixmap((const char **)prevbug_xpm);
  if (!toScanPixmap)
    toScanPixmap=new QPixmap((const char **)scansource_xpm);
  if (!toNewPixmap)
    toNewPixmap=new QPixmap((const char **)toworksheet_xpm);
  if (!toDebugPixmap)
    toDebugPixmap=new QPixmap((const char **)showdebug_xpm);
  if (!toToggleBreakPixmap)
    toToggleBreakPixmap=new QPixmap((const char **)togglebreak_xpm);
  if (!toEnableBreakPixmap)
    toEnableBreakPixmap=new QPixmap((const char **)enablebreak_xpm);
  if (!toAddWatchPixmap)
    toAddWatchPixmap=new QPixmap((const char **)addwatch_xpm);
  if (!toChangeWatchPixmap)
    toChangeWatchPixmap=new QPixmap((const char **)changewatch_xpm);
  if (!toDelWatchPixmap)
    toDelWatchPixmap=new QPixmap((const char **)delwatch_xpm);

  QToolBar *toolbar=toAllocBar(this,"Debugger",connection.connectString());

  new QToolButton(*toRefreshPixmap,
		  "Update code list",
		  "Update code list",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  Schema=new QComboBox(toolbar);
  connect(Schema,SIGNAL(activated(int)),
	  this,SLOT(changeSchema(int)));

  toolbar->addSeparator();
  new QToolButton(*toNewPixmap,
		  "Clean sheet",
		  "Clean sheet",
		  this,SLOT(newSheet(void)),
		  toolbar);
  new QToolButton(*toScanPixmap,
		  "Rescan source",
		  "Rescan source",
		  this,SLOT(scanSource(void)),
		  toolbar);
  new QToolButton(*toCompilePixmap,
		  "Compile",
		  "Compile",
		  this,SLOT(compile(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(*toExecutePixmap,
		  "Run current block",
		  "Run current block",
		  this,SLOT(execute(void)),
		  toolbar);
  StopButton=new QToolButton(*toStopPixmap,
			     "Stop running",
			     "Stop running",
			     this,SLOT(stop(void)),
			     toolbar);
  StopButton->setEnabled(false);
  toolbar->addSeparator();
  StepIntoButton=new QToolButton(*toStepIntoPixmap,
				 "Step into procedure or function",
				 "Step into procedure or function",
				 this,SLOT(stepInto(void)),
				 toolbar);
  StepIntoButton->setEnabled(false);
  StepOverButton=new QToolButton(*toStepOverPixmap,
				 "Step over procedure or function",
				 "Step over procedure or function",
				 this,SLOT(stepOver(void)),
				 toolbar);
  StepOverButton->setEnabled(false);
  ReturnButton=new QToolButton(*toReturnFromPixmap,
			       "Return from procedure or function",
			       "Return from procedure or function",
			       this,SLOT(returnFrom(void)),
			       toolbar);
  ReturnButton->setEnabled(false);

  toolbar->addSeparator();
  ShowButton=new QToolButton(toolbar);
  ShowButton->setToggleButton(true);
  ShowButton->setIconSet(QIconSet(*toShowHeadPixmap),true);
  ShowButton->setIconSet(QIconSet(*toShowBodyPixmap),false);
  connect(ShowButton,SIGNAL(toggled(bool)),this,SLOT(changeView(bool)));
  QToolTip::add(ShowButton,"Show head or body of packages and procedures.");

  DebugButton=new QToolButton(toolbar);
  DebugButton->setToggleButton(true);
  DebugButton->setIconSet(QIconSet(*toDebugPixmap));
  connect(DebugButton,SIGNAL(toggled(bool)),this,SLOT(showDebug(bool)));
  QToolTip::add(DebugButton,"Show/hide debug info pane.");

  toolbar->addSeparator();
  new QToolButton(*toNextBugPixmap,
		  "Go to next error",
		  "Go to next error",
		  this,SLOT(nextError(void)),
		  toolbar);
  new QToolButton(*toPrevBugPixmap,
		  "Go to previous error",
		  "Go to previous error",
		  this,SLOT(prevError(void)),
		  toolbar);

  toolbar->addSeparator();
  new QToolButton(*toToggleBreakPixmap,
		  "Toggle breakpoint on current line",
		  "Toggle breakpoint on current line",
		  this,SLOT(toggleBreak(void)),
		  toolbar);
  new QToolButton(*toEnableBreakPixmap,
		  "Enable/disable breakpoint on current line",
		  "Enable/disable breakpoint on current line",
		  this,SLOT(toggleEnable(void)),
		  toolbar);

  toolbar->addSeparator();
  new QToolButton(*toAddWatchPixmap,
		  "Add new variable watch",
		  "Add new variable watch",
		  this,SLOT(addWatch(void)),
		  toolbar);
  DelWatchButton=new QToolButton(*toDelWatchPixmap,
				 "Delete variable watch",
				 "Delete variable watch",
				 this,SLOT(deleteWatch(void)),
				 toolbar);
  ChangeWatchButton=new QToolButton(*toChangeWatchPixmap,
				    "Change value of watched variable",
				    "Change value of watched variable",
				    this,SLOT(changeWatch(void)),
				    toolbar);
  DelWatchButton->setEnabled(false);
  ChangeWatchButton->setEnabled(false);

  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QSplitter *splitter=new QSplitter(Vertical,this);
  QSplitter *hsplitter=new QSplitter(Horizontal,splitter);
  DebugTabs=new QTabWidget(splitter);
  DebugTabs->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum));
  DebugTabs->hide();


  {
    QValueList<int> sizes=splitter->sizes();
    sizes[0]+=sizes[1]-200;
    sizes[1]=200;
    splitter->setSizes(sizes);
    splitter->setResizeMode(DebugTabs,QSplitter::KeepSize);
  }

  QSplitter *objSplitter=new QSplitter(Vertical,hsplitter);

  Objects=new toResultView(false,false,Connection,objSplitter);
  Objects->addColumn("Objects");
  Objects->setRootIsDecorated(true);
  Objects->setTreeStepSize(10);
  connect(Objects,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changePackage(QListViewItem *)));
  Contents=new toResultView(false,false,Connection,objSplitter);
  Contents->addColumn("Contents");
  Contents->setRootIsDecorated(true);
  Contents->setSorting(-1);
  Contents->setTreeStepSize(10);
  connect(Contents,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));

  StackTrace=new toResultView(false,false,Connection,DebugTabs);
  StackTrace->addColumn("Object");
  StackTrace->addColumn("Line");
  StackTrace->addColumn("Schema");
  StackTrace->addColumn("Type");
  StackTrace->setColumnAlignment(1,AlignRight);
  StackTrace->setSorting(-1);
  StackTrace->setRootIsDecorated(true);
  StackTrace->setTreeStepSize(10);
  StackTrace->setAllColumnsShowFocus(true);
  DebugTabs->addTab(StackTrace,"&Stack Trace");
  connect(StackTrace,SIGNAL(clicked(QListViewItem *)),
	  this,SLOT(showSource(QListViewItem *)));

  Watch=new toResultView(false,false,Connection,DebugTabs);
  Watch->addColumn("Schema");
  Watch->addColumn("Object");
  Watch->addColumn("Variable");
  Watch->addColumn("Data");
  Watch->setRootIsDecorated(true);
  Watch->setTreeStepSize(10);
  Watch->setAllColumnsShowFocus(true);
  DebugTabs->addTab(Watch,"W&atch");
  connect(Watch,SIGNAL(selectionChanged(void)),
	  this,SLOT(selectedWatch(void)));
  connect(Watch,SIGNAL(doubleClicked(QListViewItem *)),
	  this,SLOT(changeWatch(QListViewItem *)));

  Breakpoints=new toResultView(false,false,Connection,DebugTabs);
  Breakpoints->addColumn("Object");
  Breakpoints->addColumn("Line");
  Breakpoints->addColumn("Schema");
  Breakpoints->addColumn("Object Type");
  Breakpoints->addColumn("Enabled");
  Breakpoints->setColumnAlignment(1,AlignRight);
  Breakpoints->setSorting(-1);
  Breakpoints->setAllColumnsShowFocus(true);
  DebugTabs->addTab(Breakpoints,"&Breakpoints");
  connect(Breakpoints,SIGNAL(clicked(QListViewItem *)),
	  this,SLOT(showSource(QListViewItem *)));

  Parameters=new toResultView(false,false,Connection,DebugTabs);
  Parameters->addColumn("Name");
  Parameters->addColumn("Content");
  Parameters->setSorting(-1);
  Parameters->setTreeStepSize(10);
  Parameters->setRootIsDecorated(true);
  Parameters->setAllColumnsShowFocus(true);
  DebugTabs->addTab(Parameters,"&Parameters");

  Output=new toDebugOutput(this,DebugTabs,Connection);
  DebugTabs->addTab(Output,"Debug &Output");

  RuntimeLog=new toMarkedText(DebugTabs);
  DebugTabs->addTab(RuntimeLog,"&Runtime Log");

  HeadEditor=new toDebugText(Breakpoints,Connection,hsplitter);
  BodyEditor=new toDebugText(Breakpoints,Connection,hsplitter);
  HeadEditor->hide();
  connect(HeadEditor,SIGNAL(insertedLines(int,int)),
	  this,SLOT(reorderContent(int,int)));
  connect(BodyEditor,SIGNAL(insertedLines(int,int)),
	  this,SLOT(reorderContent(int,int)));

  {
    QValueList<int> sizes=hsplitter->sizes();
    sizes[0]=200;
    hsplitter->setSizes(sizes);
    hsplitter->setResizeMode(objSplitter,QSplitter::KeepSize);
  }

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  Connection.addWidget(this);

  refresh();
  startTarget();
  readLog();
}

static toSQL SQLAttach("toDebug:Attach",
		       "DECLARE\n"
		       "    timeout BINARY_INTEGER;\n"
		       "BEGIN\n"
		       "    DBMS_DEBUG.ATTACH_SESSION(:sess<char[201],in>);\n"
		       "    timeout:=DBMS_DEBUG.SET_TIMEOUT(1);\n"
		       "END;",
		       "Connect to the debugging session");

static toSQL SQLDebugEnable("toDebug:EnableDebug",
			    "ALTER SESSION SET PLSQL_DEBUG = TRUE",
			    "Enable PL/SQL debugging");

void toDebug::startTarget(void)
{
  {
    toLocker lock(Lock);
    TargetThread=new toThread(new targetTask(*this));
    TargetThread->start();
  }

  ChildSemaphore.down();
  try {
    otl_stream attach(1,
		      SQLAttach(Connection),
		      Connection.connection());
    attach<<TargetID.utf8();
    otl_cursor::direct_exec(Connection.connection(),
			    SQLDebugEnable(Connection));
  } TOCATCH  // Trying to run somthing after this won't work (And will hang tora I think)
}

toDebugText *toDebug::currentEditor(void)
{
  if (HeadEditor->isHidden())
    return BodyEditor;
  else
    return HeadEditor;
}

void toDebug::changeSchema(int)
{
  refresh();
}

static toSQL SQLListObjects("toDebug:ListObjects",
			    "SELECT Object_Type,Object_Name Type FROM ALL_OBJECTS\n"
			    " WHERE OWNER = :owner<char[31]>\n"
			    "   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
			    "                       'PROCEDURE','TYPE')\n"
			    " ORDER BY Object_Type,Object_Name",
			    "List objects available in a schema, must have same result columns");

void toDebug::refresh(void)
{
  try {
    QString selected=Schema->currentText();
    QString currentSchema;
    if (selected.isEmpty()) {
      selected=Connection.user().upper();
      Schema->clear();
      otl_stream users(1,
		       toSQL::sql(TOSQL_USERLIST,Connection),
		       Connection.connection());
      for(int i=0;!users.eof();i++) {
	char buffer[31];
	users>>buffer;
	Schema->insertItem(QString::fromUtf8(buffer));
      }
    }
    if (!selected.isEmpty()) {
      for (int i=0;i<Schema->count();i++)
	if (Schema->text(i)==selected) {
	  Schema->setCurrentItem(i);
	  break;
	}
      Objects->clear();
      otl_stream code(1,
		      SQLListObjects(Connection),
		      Connection.connection());
      code<<selected.utf8();
      QListViewItem *typeItem=NULL;
      QListViewItem *last=NULL;
      while(!code.eof()) {
	char nameStr[100];
	char typeStr[100];
	code>>typeStr>>nameStr;
	QString name=QString::fromUtf8(nameStr);
	QString type=QString::fromUtf8(typeStr);
	if (!typeItem||typeItem->text(0)!=type) {
	  typeItem=new QListViewItem(Objects,typeItem,type);
#ifndef AUTOEXPAND
	  typeItem->setSelectable(false);
#endif
	}
	QString bodyType(type);
	bodyType+=" BODY";
	last=new QListViewItem(typeItem,last,name);
	last->setText(1,type);
	if (selected==currentEditor()->schema()&&
	    (type==currentEditor()->type()||
	     bodyType==currentEditor()->type())&&
	    name==currentEditor()->object()) {
	  Objects->setOpen(typeItem,true);
	  Objects->setSelected(last,true);
	}
      }
      if (HeadEditor->length()==0)
	HeadEditor->setSchema(selected);
      if (BodyEditor->length()==0)
	BodyEditor->setSchema(selected);
    }
  } TOCATCH
}

bool toDebug::checkStop(void)
{
  Lock.lock();
  if (RunningTarget) {
    Lock.unlock();
    if (QMessageBox::information(this,"Stop execution?",
				 "Do you want to abort the current execution?",
				 "&Ok","Cancel")!=0)
      return false;
    stop();
  } else
    Lock.unlock();
  return true;
}

bool toDebug::checkCompile(void)
{
  if (HeadEditor->edited()) {
    switch (QMessageBox::warning(this,
				 "Header changed",
				 "Header changed. Continuing will discard uncompiled or saved changes",
				 "&Compile",
				 "&Discard changes",
				 "Cancel")) {
    case 0:
      if (!checkStop())
	return false;
      if (!HeadEditor->compile())
	return false;
      break;
    case 1:
      HeadEditor->setEdited(false);
      break;
    case 2:
      return false;
    }
  }
  if (BodyEditor->edited()) {
    switch (QMessageBox::warning(this,
				 "Body changed",
				 "Body changed. Continuing will discard uncompiled or saved changes",
				 "&Compile",
				 "&Discard changes",
				 "Cancel")) {
    case 0:
      if (!checkStop())
	return false;
      if (!BodyEditor->compile())
	return false;
      break;
    case 1:
      BodyEditor->setEdited(false);
      break;
    case 2:
      return false;
    }
  }
  return true;
}

bool toDebug::close(bool del)
{
  if (checkCompile())
    return QVBox::close(del);
  return false;
}

void toDebug::updateCurrent()
{
  QString type=HeadEditor->type();

  if (type.isEmpty())
    type=BodyEditor->type();

  QString bodyType=type;
  bodyType+=" BODY";
  BodyEditor->setType(bodyType);
  if (!BodyEditor->readData(Connection,StackTrace)) {
    BodyEditor->setType(type);
    BodyEditor->readData(Connection,StackTrace);
    HeadEditor->clear();
    BodyEditor->show();
    HeadEditor->hide();
    ShowButton->setEnabled(false);
    ShowButton->setOn(false);
    if (ToolMenu)
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_HEAD_TOGGLE,false);
  } else {
    HeadEditor->readData(Connection,StackTrace);
    ShowButton->setEnabled(true);
    if (ToolMenu)
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_HEAD_TOGGLE,true);
  }

  currentEditor()->setFocus();

  updateContent(true);
  updateContent(false);
}

void toDebug::changePackage(QListViewItem *item)
{
  if (item&&item->parent()&&checkCompile()) {
    BodyEditor->setData(Schema->currentText(),item->text(1),item->text(0));
    HeadEditor->setData(Schema->currentText(),item->text(1),item->text(0));
    updateCurrent();
  }
#ifdef AUTOEXPAND
  else if (item&&!item->parent())
    item->setOpen(true);
#endif
}

void toDebug::showDebug(bool show)
{
  if (show)
    DebugTabs->show();
  else
    DebugTabs->hide();
  if (ToolMenu)
    toMainWidget()->menuBar()->setItemChecked(TO_ID_DEBUG_PANE,show);
}

void toDebug::changeView(bool head)
{
  if (head) {
    HeadEditor->show();
    BodyEditor->hide();    
  } else {
    BodyEditor->show();
    HeadEditor->hide();
  }
  if (ToolMenu)
    toMainWidget()->menuBar()->setItemChecked(TO_ID_HEAD_TOGGLE,head);
  currentEditor()->setFocus();
}

bool toDebugText::compile(void)
{
  QString str=text();
  bool ret=true;
  if (!str.isEmpty()) {
    bool inWord=false;
    int curWord=0;
    QString words[6];
    bool body=false;
    int begin[6];

    for (unsigned int i=0;i<str.length();i++) {
      if (str.at(i).isSpace()) {
	if (inWord) {
	  curWord++;
	  if (curWord>=6)
	    break;
	  inWord=false;
	}
      } else {
	if (!inWord) {
	  begin[curWord]=i;
	  inWord=true;
	}
	words[curWord]+=str.at(i);
      }
    }

    if (words[0].upper()!="PROCEDURE"&&
	words[0].upper()!="TYPE"&&
	words[0].upper()!="FUNCTION"&&
	words[0].upper()!="PACKAGE") {
      toStatusMessage("Invalid start of code");
      return false;
    }

    int word=1;
    if (words[1].upper()=="BODY") {
      body=true;
      word++;
    }
    QString what=words[word];
    word++;
    if(!what.contains('.')&&words[word][0]=='.') {
      what+=words[word];
      word++;
    }
    if (what.right(1)==".") {
      what+=words[word];
      word++;
    }
    if (word>=curWord) {
      toStatusMessage("Invalid start of code");
      return false;
    }

    QString type=words[0];
    QString schema;
    QString object;

    int pos=what.find('.');
    if (pos==-1) {
      schema=Schema;
      object=what;
    } else {
      schema=what.left(pos);
      object=what.right(what.length()-pos-1);
    }

    QString sql="CREATE OR REPLACE ";
    sql.append(words[0]);
    if (body)
      sql.append(" BODY ");
    else
      sql.append(" ");
    sql.append(schema);
    sql.append(".");
    sql.append(object);
    sql.append(" ");
    sql.append(str.right(str.length()-begin[word]));

    try {
      otl_cursor::direct_exec(Connection.connection(),
			      sql.utf8());
      Schema=schema.upper();
      Object=object.upper();
      Type=type.upper();
      if (body)
	Type+=" BODY";
      setEdited(false);
    } catch (const otl_exception &exc) {
      toStatusMessage(QString::fromUtf8((const char *)exc.msg));
      ret=false;
    }
  }
  return ret;
}

void toDebug::compile(void)
{
  if (!checkStop())
    return;

  QString lastSchema=currentEditor()->schema();
  if (HeadEditor->compile()&&
      BodyEditor->compile()) {
    if (lastSchema!=currentEditor()->schema()) {
      for (int i=0;i<Schema->count();i++)
	if (Schema->text(i)==lastSchema) {
	  Schema->setCurrentItem(i);
	  break;
	}
    }
    refresh();
  }
}

toDebug::~toDebug()
{
  stop();
  {
    toLocker lock(Lock);
    TargetSQL="";
    TargetSemaphore.up();
  }
  ChildSemaphore.down();

  DebugTool.closeWindow(Connection);
  Connection.delWidget(this);
}

void toDebug::prevError(void)
{
  currentEditor()->previousError();
}

void toDebug::nextError(void)
{
  currentEditor()->nextError();
}

void toDebug::changeContent(QListViewItem *ci)
{
  toContentsItem *item=dynamic_cast<toContentsItem *>(ci);
  if (item) {
    while(ci->parent())
      ci=ci->parent();
    toHighlightedText *current;
    if (ci->text(0)=="Head") {
      current=HeadEditor;
      ShowButton->setOn(true);
    } else {
      current=BodyEditor;
      ShowButton->setOn(false);
    }
    current->setCursorPosition(item->Line,0);
    current->setFocus();
  }
#ifdef AUTOEXPAND
  else
    ci->setOpen(true);
#endif
}

void toDebug::scanSource(void)
{
  updateContent(true);
  updateContent(false);
}

void toDebug::newSheet(void)
{
  if (checkCompile()) {
    HeadEditor->clear();
    BodyEditor->clear();
    HeadEditor->setSchema(Schema->currentText());
    BodyEditor->setSchema(Schema->currentText());
    scanSource();
    if (ToolMenu)
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_HEAD_TOGGLE,true);
    ShowButton->setEnabled(true);
  }
}

void toDebug::showSource(QListViewItem *item)
{
  if (item) {
    if (!viewSource(item->text(2),item->text(0),item->text(3),item->text(1).toInt(),false)) {
      if (checkCompile()) {
	BodyEditor->setEdited(false); // Write over this editor
	viewSource(item->text(2),item->text(0),item->text(3),item->text(1).toInt(),false);
      }
    }
  }
}

void toDebug::toggleBreak(void)
{
  currentEditor()->toggleBreakpoint();
  currentEditor()->setFocus();
}

void toDebug::toggleEnable(void)
{
  currentEditor()->toggleBreakpoint(-1,true);
  currentEditor()->setFocus();
}

void toDebug::addWatch(void)
{
  toDebugWatch watch(this);
  if (watch.exec()) {
    watch.createWatch(Watch);
    if (isRunning())
      updateState(TO_REASON_WHATEVER);
  }
}

void toDebug::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(*toNewPixmap,"&New Sheet",this,SLOT(newSheet(void)),
			   0,TO_ID_NEW_SHEET);
      ToolMenu->insertItem(*toScanPixmap,"S&can Source",this,SLOT(scanSource(void)),
			   CTRL+Key_F9,TO_ID_SCAN_SOURCE);
      ToolMenu->insertItem(*toCompilePixmap,"&Compile",this,SLOT(compile(void)),
			   Key_F9,TO_ID_COMPILE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(*toExecutePixmap,"&Execute",this,SLOT(execute(void)),
			   CTRL+Key_Return,TO_ID_EXECUTE);
      ToolMenu->insertItem(*toStopPixmap,"&Stop",this,SLOT(stop(void)),
			   Key_F12,TO_ID_STOP);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(*toStepIntoPixmap,"Step &Into",this,SLOT(stepInto(void)),
			   Key_F7,TO_ID_STEP_INTO);
      ToolMenu->insertItem(*toStepOverPixmap,"&Next Line",this,SLOT(stepOver(void)),
			   Key_F8,TO_ID_STEP_OVER);
      ToolMenu->insertItem(*toReturnFromPixmap,"&Return From",this,SLOT(returnFrom(void)),
			   Key_F6,TO_ID_RETURN_FROM);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem("&Head Editor",this,SLOT(toggleHead(void)),
			   CTRL+Key_Space,TO_ID_HEAD_TOGGLE);
      ToolMenu->insertItem("&Debug Pane",this,SLOT(toggleDebug(void)),
			   Key_F11,TO_ID_DEBUG_PANE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(*toNextBugPixmap,"Next &Error",this,SLOT(nextError(void)),
			   CTRL+Key_N);
      ToolMenu->insertItem(*toPrevBugPixmap,"Pre&vious Error",this,SLOT(prevError(void)),
			   CTRL+Key_P);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(*toToggleBreakPixmap,"&Toggle Breakpoint",this,SLOT(toggleBreak(void)),
			   Key_F5);
      ToolMenu->insertItem(*toEnableBreakPixmap,"D&isable Breakpoint",
			   this,SLOT(toggleEnable(void)),
			   CTRL+Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(*toAddWatchPixmap,"&Add Watch",this,SLOT(addWatch(void)),
			   Key_F4);
      ToolMenu->insertItem(*toDelWatchPixmap,"Delete &Watch",this,SLOT(deleteWatch(void)),
			   CTRL+Key_Delete,TO_ID_DEL_WATCH);
      ToolMenu->insertItem(*toChangeWatchPixmap,"Chan&ge Watch",this,SLOT(changeWatch(void)),
			   CTRL+Key_F4,TO_ID_CHANGE_WATCH);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem("Erase Runtime &Log",this,SLOT(clearLog(void)));

      toMainWidget()->menuBar()->insertItem("&Debug",ToolMenu,-1,TO_TOOL_MENU_INDEX);

      if (!isRunning()) {
	toMainWidget()->menuBar()->setItemEnabled(TO_ID_STOP,false);
	toMainWidget()->menuBar()->setItemEnabled(TO_ID_STEP_INTO,false);
	toMainWidget()->menuBar()->setItemEnabled(TO_ID_STEP_OVER,false);
	toMainWidget()->menuBar()->setItemEnabled(TO_ID_RETURN_FROM,false);
      }
      if (currentEditor()==HeadEditor)
	toMainWidget()->menuBar()->setItemChecked(TO_ID_HEAD_TOGGLE,true);
      if (!ShowButton->isEnabled())
	toMainWidget()->menuBar()->setItemEnabled(TO_ID_HEAD_TOGGLE,false);
      if (!DebugTabs->isHidden())
	toMainWidget()->menuBar()->setItemChecked(TO_ID_DEBUG_PANE,true);
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toDebug::toggleHead(void)
{
  ShowButton->setOn(!ShowButton->isOn());
}

void toDebug::toggleDebug(void)
{
  DebugButton->setOn(!DebugButton->isOn());
}

void toDebug::selectedWatch()
{
  QListViewItem *item=Watch->selectedItem();
  if (item) {
    if (!item->text(5).isEmpty()&&item->text(5)!="LIST") {
      DelWatchButton->setEnabled(false);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_DEL_WATCH,false);
    } else {
      DelWatchButton->setEnabled(true);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_DEL_WATCH,true);
    }
    if (item->text(4).isEmpty()) {
      ChangeWatchButton->setEnabled(true);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_CHANGE_WATCH,true);
    } else {
      ChangeWatchButton->setEnabled(false);
      toMainWidget()->menuBar()->setItemEnabled(TO_ID_CHANGE_WATCH,false);
    }
  } else {
    DelWatchButton->setEnabled(false);
    ChangeWatchButton->setEnabled(false);
    toMainWidget()->menuBar()->setItemEnabled(TO_ID_DEL_WATCH,false);
    toMainWidget()->menuBar()->setItemEnabled(TO_ID_CHANGE_WATCH,false);
  }
}

void toDebug::deleteWatch(void)
{
  delete Watch->selectedItem();
}

void toDebug::clearLog(void)
{
  RuntimeLog->clear();
}

void toDebug::changeWatch(void)
{
  changeWatch(Watch->selectedItem());
}

static toSQL SQLChangeLocal("toDebug:ChangeLocalWatch",
			    "DECLARE\n"
			    "  ret BINARY_INTEGER;\n"
			    "  data VARCHAR2(4000);\n"
			    "BEGIN\n"
			    "  ret:=DBMS_DEBUG.SET_VALUE(0,:assign<char[4001],in>);\n"
			    "  SELECT ret INTO :ret<int,out> FROM DUAL;\n"
			    "END;",
			    "Change local watch value, must have same bindings");
static toSQL SQLChangeGlobal("toDebug:ChangeGlobalWatch",
			     "DECLARE\n"
			     "  data VARCHAR2(4000);\n"
			     "  proginf DBMS_DEBUG.program_info;\n"
			     "  ret BINARY_INTEGER;\n"
			     "BEGIN\n"
			     "  proginf.Namespace:=DBMS_DEBUG.Namespace_pkg_body;\n"
			     "  proginf.Name:=:object<char[41],in>;\n"
			     "  proginf.Owner:=:owner<char[41],in>;\n"
			     "  proginf.DBLink:=NULL;\n"
			     "  ret:=DBMS_DEBUG.SET_VALUE(proginf,:assign<char[4001],in>);\n"
			     "  IF ret =DBMS_DEBUG.error_no_such_object THEN\n"
			     "    proginf.Namespace:=DBMS_DEBUG.namespace_pkgspec_or_toplevel;\n"
			     "    ret:=DBMS_DEBUG.SET_VALUE(proginf,:assign<char[4001],in>);\n"
			     "  END IF;\n"
			     "  SELECT ret INTO :ret<int,out> FROM DUAL;\n"
			     "END;",
			     "Change global watch value, must have same bindings");

void toDebug::changeWatch(QListViewItem *item)
{
  if (item&&item->text(4).isEmpty()) {
    QString description="Enter new value to the watch ";
    description+=item->text(2);
    QString data=item->text(3);

    toDebugChangeUI dialog(this,"WatchChange",true);
    dialog.HeadLabel->setText(description);
    QString index=item->text(5);
    if (!index.isEmpty()&&index!="LIST")
      dialog.Index->setValue(item->text(5).toInt());
    if (index!="LIST") {
      dialog.Index->setEnabled(false);
      dialog.Value->setText(data);
    }
    if (item->text(4)=="NULL")
      dialog.NullValue->setChecked(true);

    if (dialog.exec()) {
      int ret=-1;
      QString data;
      QString escdata;
      QString assign;
      if (dialog.NullValue->isChecked()) {
	data="{null}";
	escdata="NULL";
      } else {
	escdata=data=dialog.Value->text();
	escdata.replace(QRegExp("'"),"''");
	escdata.prepend("'");
	escdata+="'";
      }
      assign=item->text(2);
      if (index=="LIST") {
	assign+="(";
	assign+=dialog.Index->text();
	assign+=")";
      }
      
      assign+=":=";
      assign+=escdata;
      assign+=";";
      try {
	if (item->text(0).isEmpty()) {
	  otl_stream local(1,
			   SQLChangeLocal(Connection),
			   Connection.connection());
	  local<<assign;
	  local>>ret;
	} else {
	  otl_stream global(1,
			    SQLChangeGlobal(Connection),
			    Connection.connection());
	  if (item->text(1).isEmpty())
	    global<<"";
	  else
	    global<<item->text(1).utf8();
	  global<<item->text(0).utf8();
	  global<<assign.utf8();
	  global>>ret;
	}
	if (ret==TO_ERROR_UNIMPLEMENTED) {
	  toStatusMessage("Unimplemented in PL/SQL debug interface");
	} else if (ret!=TO_SUCCESS) {
	  QString str("Assignment failed (Reason ");
	  str+=QString::number(ret);
	  str+=")";
	  toStatusMessage(str);
	} else
	  updateState(TO_REASON_WHATEVER);
      } TOCATCH
    }
  }
}
