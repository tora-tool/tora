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


#include <ctype.h>
#include <stack>

#include <qsizepolicy.h>
#include <qmessagebox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qworkspace.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtimer.h>
#include <qstring.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qnamespace.h>

#include "toparamget.h"
#include "totool.h"
#include "tomarkedtext.h"
#include "tomain.h"
#include "todebug.h"
#include "tohighlightedtext.h"
#include "toresultview.h"
#include "toconf.h"
#include "tooutput.h"
#include "todebugtext.h"

#include "todebug.moc"

#include "icons/todebug.xpm"
#include "icons/compile.xpm"
#include "icons/refresh.xpm"
#include "icons/execute.xpm"
#include "icons/stop.xpm"
#include "icons/stepover.xpm"
#include "icons/stepinto.xpm"
#include "icons/returnfrom.xpm"
#include "icons/showhead.xpm"
#include "icons/showbody.xpm"
#include "icons/nextbug.xpm"
#include "icons/prevbug.xpm"
#include "icons/showdebug.xpm"
#include "icons/scansource.xpm"
#include "icons/toworksheet.xpm"
#include "icons/togglebreak.xpm"
#include "icons/enablebreak.xpm"
#include "icons/addwatch.xpm"
#include "icons/delwatch.xpm"

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

#include <stdio.h>

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
			  "
DECLARE
   coll sys.dbms_debug_vc2coll;
   ret INTEGER;
   i INTEGER;
   line VARCHAR2(100);
   errm VARCHAR2(100);
BEGIN
  DBMS_DEBUG.EXECUTE('DECLARE '||
                     '  pp SYS.dbms_debug_vc2coll := SYS.dbms_debug_vc2coll(); '||
                     '  line VARCHAR2(100); '||
                     '  ret INTEGER; '||
                     'BEGIN '||
                     '  DBMS_OUTPUT.GET_LINE(line,ret); '||
                     '  pp.EXTEND(2); '||
                     '  pp(1):=ret; '||
                     '  pp(2):=line; '||
                     '  :1 := pp; '||
                     'END;',
                     -1,
                     1,
                     coll,
                     line);
  i:=coll.first;
  IF i IS NOT NULL THEN
    ret:=coll(i);
    IF ret = 0 THEN
      i:=coll.next(i);
      line:=coll(i);
    ELSE
      line:='';
    END IF;
  ELSE
    ret:=1;
  END IF;
  SELECT ret,line INTO :ret<int,out>,:line<char[101,out> FROM DUAL;
END;",
			  Connection.connection());
	  poll>>ret;
	  char buffer[101];
	  poll>>buffer;
	  if (ret==0||strlen(buffer))
	    insertLine(buffer);
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
				"
DECLARE
   coll sys.dbms_debug_vc2coll;
   errm VARCHAR2(100);
BEGIN
  DBMS_DEBUG.EXECUTE('BEGIN DBMS_OUTPUT.DISABLE; END;',
                     -1,
                     0,
                     coll,
                     errm);
END;");
	else
	  otl_cursor::direct_exec(Connection.connection(),
				"
DECLARE
   coll sys.dbms_debug_vc2coll;
   errm VARCHAR2(100);
BEGIN
  DBMS_DEBUG.EXECUTE('BEGIN DBMS_OUTPUT.ENABLE; END;',
                     -1,
                     0,
                     coll,
                     errm);
END;");
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

void toDebug::targetTask::run(void)
{
#ifdef TO_DEBUG
  printf("Started task\n");
#endif
  {
    toConnection Connection(Parent.Connection.user(),
			    Parent.Connection.password(),
			    Parent.Connection.host());
    otl_stream init(1,"
DECLARE
  ret VARCHAR2(200);
BEGIN
  
  ret:=DBMS_DEBUG.INITIALIZE;
  DBMS_DEBUG.DEBUG_ON;
  SELECT ret,dbms_session.unique_session_id INTO :f2<char[201],out>,:f3<char[25],out> FROM DUAL;
END;
",
		    Connection.connection());
    int colSize;
    {
      char buffer[201];
      toLocker lock(Parent.Lock);
      init>>buffer;
      Parent.TargetID=buffer;
      init>>buffer;
      Parent.TargetSession=buffer;
      Parent.ChildSemaphore.up();
    }
#ifdef TO_DEBUG
    printf("Connected to database\n");
#endif
    while(1) {
      {
	toLocker lock(Parent.Lock);
	Parent.RunningTarget=false;
	colSize=Parent.ColumnSize;
      }
      Parent.TargetSemaphore.down();
#ifdef TO_DEBUG
      printf("Anything to do?\n");
#endif

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
#ifdef TO_DEBUG
	printf("Running SQL\n%s\n",(const char *)sql);
#endif

	otl_stream q(1,
		     (const char *)sql,
		     Connection.connection());

	for(list<QString>::iterator i=inParams.begin();i!=inParams.end();i++)
	  q<<(const char *)(*i);

	outParams=new list<QString>;

#ifdef TO_DEBUG
	printf("Checking output\n");
#endif
	while(!q.eof()) {
	  char buffer[colSize];
	  q>>buffer;
	  outParams->insert(outParams->end(),buffer);
	}
      } catch (const QString &str) {
	printf("Encountered error:\n%s\n",(const char *)str);
      } catch (const otl_exception &exc) {
	printf("Running SQL\n%s\n",(const char *)sql);
	printf("Encountered SQL error:\n%s\n",exc.msg);
      } catch (...) {
	printf("Encountered error in target SQL\n");
      }

#ifdef TO_DEBUG
      printf("Storing result\n");
#endif
      {
	toLocker lock(Parent.Lock);
	if (outParams) {
	  Parent.OutputData=(*outParams);
	  delete outParams;
	}
      }
      Parent.ChildSemaphore.up();
    }

#ifdef TO_DEBUG
    printf("Target thread exiting\n");
#endif
  }
  toLocker lock(Parent.Lock);
  Parent.TargetThread=NULL;
  Parent.ChildSemaphore.up();
#ifdef TO_DEBUG
  printf("Done exiting\n");
#endif
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
    
  for(QListViewItem *item=parent->firstChild();item;item=item->nextSibling())
    if (item->text(0)==dsc)
      return item;

  QListViewItem *lastItem=NULL;
  for(QListViewItem *item=parent->firstChild();item;item=item->nextSibling()) {
    if (dsc<item->text(0))
      break;
    lastItem=item;
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
	if (retType.isEmpty()||retType[retType.length()-1]=='.'||token==".")
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
    sql+="BEGIN\n  ";
    if (!retType.isEmpty())
      sql+="SELECT ";
    sql+=currentEditor()->schema();
    sql+=".";
    if (hasMembers(currentEditor()->type())) {
      sql+=currentEditor()->object();
      sql+=".";
    }
    sql+=callName;

    Parameters->clear();
    QListViewItem *head=new QListViewItem(Parameters,NULL,"Input");
    QListViewItem *last=NULL;
    head->setOpen(true);

    for(list<debugParam>::iterator i=CurrentParams.begin();i!=CurrentParams.end();i++) {
      if ((*i).In)
	last=new QListViewItem(head,last,(*i).Name);
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
      sql+=" INTO :tora_int_return<char[";
      sql+=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE);
      sql+="],out> FROM DUAL";
    }
    sql+=";\nEND;\n";

    try {
      {
	toLocker lock(Lock);
	InputData=toParamGet::getParam(this,sql);
	last=head->firstChild();
	if (InputData.begin()!=InputData.end())
	  for(list<QString>::iterator i=InputData.begin();
	      last&&i!=InputData.end();
	      i++,last=last->nextSibling())
	    last->setText(1,(const char *)(*i)); // Deep copy
	else
	  delete head;
	ColumnSize=toTool::globalConfig(CONF_MAX_COL_SIZE,DEFAULT_MAX_COL_SIZE).toInt();
	TargetSQL=(const char *)sql; // Deep copy of SQL
	TargetSemaphore.up(); // Go go power rangers!
      }
      if (sync()>=0)
	continueExecution(TO_BREAK_ANY_CALL);
    } TOCATCH
  } else
    toStatusMessage("Couldn't find any function or procedure under cursor.");
}

int toDebug::sync(void)
{
  try {
    otl_stream sync(1,"
DECLARE
  ret binary_integer;
  runinf DBMS_DEBUG.Runtime_Info;
BEGIN
  ret:=DBMS_DEBUG.SYNCHRONIZE(runinf,:info<int,in>);
  SELECT ret,
         runinf.Reason
    INTO :ret<int,out>,
         :reason<int,out>
    FROM DUAL;
END;
",
		    Connection.connection());
    int ret;
    int reason;
    do {
      sync<<0;
      sync>>ret>>reason;
      {
	toLocker lock(Lock);
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
  } state,beforeState;

  for (QListViewItem *item=Contents->firstChild();item;item=item->nextSibling()) {
    if (item->text(0)==topName) {
      delete item;
      break;
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
  int startFirst;
  bool firstIgnore=true;

  QString is("IS");
  for(int cline=0;cline<current->numLines();cline++) {
    QString line=current->textLine(cline);
    QChar lc='\n';
    toClearBlocks();
    for (unsigned int i=0;i<line.length();i++) {
      QChar c=line[i];
      QChar nc;
      if (i==line.length()-1)
	nc='\n';
      else
	nc=line[i+1];
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
static QPixmap *toDelWatchPixmap;

void toDebug::updateState(int reason)
{
  switch(reason) {
  case TO_REASON_EXIT:
  case TO_REASON_KNL_EXIT:
    ChildSemaphore.down();
    StopButton->setEnabled(false);
    StepOverButton->setEnabled(false);
    StepIntoButton->setEnabled(false);
    ReturnButton->setEnabled(false);
    HeadEditor->setCurrent(-1);
    BodyEditor->setCurrent(-1);
    StackTrace->clear();
    {
      toLocker lock(Lock);
      if (OutputData.begin()!=OutputData.end()) {
	QListViewItem *head=Parameters->firstChild();
	while(head&&head->nextSibling())
	  head=head->nextSibling();
	head=new QListViewItem(Parameters,head,"Output");
	head->setOpen(true);
	list<debugParam>::iterator cp=CurrentParams.begin();

	QListViewItem *last=NULL;
	for (list<QString>::iterator i=OutputData.begin();i!=OutputData.end();i++) {
	  QString name;
	  for(;cp!=CurrentParams.end()&&!(*cp).Out;cp++)
	    ;
	  if (name.isEmpty())
	    name="Returning";
	  last=new QListViewItem(head,last,name);
	  last->setText(1,(const char *)*i); // Deep copy just to be sure
	}
      }
    }
    break;
  default:
    DebugTabs->show();
    StopButton->setEnabled(true);
    StepOverButton->setEnabled(true);
    StepIntoButton->setEnabled(true);
    ReturnButton->setEnabled(true);
    try {
      otl_stream info(1,
		      "
DECLARE
  info DBMS_DEBUG.RUNTIME_INFO;
  ret BINARY_INTEGER;
BEGIN
  ret:=DBMS_DEBUG.GET_RUNTIME_INFO(DBMS_DEBUG.info_getStackDepth,info);
  SELECT ret,info.stackDepth INTO :ret<int,out>,:depth<int,out> FROM DUAL;
END;",
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
		       "
DECLARE
  info DBMS_DEBUG.PROGRAM_INFO;
  stack DBMS_DEBUG.BACKTRACE_TABLE;
  i BINARY_INTEGER;
BEGIN
  DBMS_DEBUG.PRINT_BACKTRACE(stack);
  i:=:num<int,in>;
  info:=stack(i);
  SELECT info.Name,info.Owner,info.Line#,DECODE(info.LibunitType,DBMS_DEBUG.LibunitType_cursor,'CURSOR',
                                                                 DBMS_DEBUG.LibunitType_function,'FUNCTION',
                                                                 DBMS_DEBUG.LibunitType_procedure,'PROCEDURE',
                                                                 DBMS_DEBUG.LibunitType_package,'PACKAGE',
                                                                 DBMS_DEBUG.LibunitType_package_body,'PACKAGE BODY',
                                                                 DBMS_DEBUG.LibunitType_trigger,'TRIGGEER',
                                                                 'UNKNOWN')
    INTO :name<char[31],out>,:owner<char[31],out>,:line<int,out>,:type<char[31],out> FROM DUAL;
END;",
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
	  item=new QListViewItem(StackTrace,name,QString::number(line),schema,type);
	else
	  item=new QListViewItem(item,name,QString::number(line),schema,type);
	item->setOpen(true);
      }
      Output->refresh();
      if (depth>=2)
	viewSource(schema,name,type,line,true);
      else
	continueExecution(TO_BREAK_NEXT_LINE);
    } TOCATCH
    break;
  }
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

int toDebug::continueExecution(int stopon)
{
  Lock.lock();
  if (RunningTarget) {
    Lock.unlock();
    try {
      int ret,reason;
      setDeferedBreakpoints();
      otl_stream cont(1,
		   "
DECLARE
  runinf DBMS_DEBUG.runtime_info;
  ret BINARY_INTEGER;
BEGIN
  ret:=DBMS_DEBUG.CONTINUE(runinf,:break<int,in>,NULL);
  SELECT ret,
         runinf.Reason
    INTO :ret<int,out>,
         :reason<int,out>
    FROM DUAL;
END;",
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
  if (!toDelWatchPixmap)
    toDelWatchPixmap=new QPixmap((const char **)delwatch_xpm);

  QToolBar *toolbar=new QToolBar("SQL Output",toMainWidget(),this);

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
		  this,SLOT(toggleBreak(void)),
		  toolbar);
  new QToolButton(*toDelWatchPixmap,
		  "Delete variable watch",
		  "Delete variable watch",
		  this,SLOT(toggleEnable(void)),
		  toolbar);

  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QSplitter *splitter=new QSplitter(Vertical,this);
  QSplitter *hsplitter=new QSplitter(Horizontal,splitter);
  DebugTabs=new QTabWidget(splitter);
  DebugTabs->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum));
  DebugTabs->hide();


  {
    QValueList<int> sizes=splitter->sizes();
    sizes[1]=200;
    splitter->setSizes(sizes);
    splitter->setResizeMode(DebugTabs,QSplitter::KeepSize);
  }

  QSplitter *objSplitter=new QSplitter(Vertical,hsplitter);

  Objects=new QListView(objSplitter);
  Objects->addColumn("Objects");
  Objects->setRootIsDecorated(true);
  Objects->setTreeStepSize(10);
  connect(Objects,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changePackage(QListViewItem *)));
  Contents=new QListView(objSplitter);
  Contents->addColumn("Contents");
  Contents->setRootIsDecorated(true);
  Contents->setSorting(-1);
  Contents->setTreeStepSize(10);
  connect(Contents,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));

  StackTrace=new QListView(DebugTabs);
  StackTrace->addColumn("Object");
  StackTrace->addColumn("Line");
  StackTrace->addColumn("Schema");
  StackTrace->addColumn("Type");
  StackTrace->setColumnAlignment(1,AlignRight);
  StackTrace->setSorting(-1);
  StackTrace->setRootIsDecorated(true);
  StackTrace->setTreeStepSize(10);
  StackTrace->setAllColumnsShowFocus(true);
  DebugTabs->addTab(StackTrace,"Stack Trace");
  connect(StackTrace,SIGNAL(clicked(QListViewItem *)),
	  this,SLOT(showSource(QListViewItem *)));

  Watch=new toResultView(false,false,Connection,DebugTabs);
  Watch->addColumn("Schema");
  Watch->addColumn("Type");
  Watch->addColumn("Object");
  Watch->addColumn("Data");
  Watch->setSorting(-1);
  Watch->setRootIsDecorated(true);
  Watch->setTreeStepSize(10);
  Watch->setAllColumnsShowFocus(true);
  DebugTabs->addTab(Watch,"Watch");

  Breakpoints=new QListView(DebugTabs);
  Breakpoints->addColumn("Object");
  Breakpoints->addColumn("Line");
  Breakpoints->addColumn("Schema");
  Breakpoints->addColumn("Object Type");
  Breakpoints->addColumn("Enabled");
  Breakpoints->setColumnAlignment(1,AlignRight);
  Breakpoints->setSorting(-1);
  Breakpoints->setAllColumnsShowFocus(true);
  DebugTabs->addTab(Breakpoints,"Breakpoints");
  connect(Breakpoints,SIGNAL(clicked(QListViewItem *)),
	  this,SLOT(showSource(QListViewItem *)));

  Parameters=new QListView(DebugTabs);
  Parameters->addColumn("Name");
  Parameters->addColumn("Content");
  Parameters->setSorting(-1);
  Parameters->setTreeStepSize(10);
  Parameters->setRootIsDecorated(true);
  Parameters->setAllColumnsShowFocus(true);
  DebugTabs->addTab(Parameters,"Parameters");

  Output=new toDebugOutput(this,DebugTabs,Connection);
  DebugTabs->addTab(Output,"Debug Output");

  HeadEditor=new toDebugText(Breakpoints,Connection,hsplitter);
  BodyEditor=new toDebugText(Breakpoints,Connection,hsplitter);
  HeadEditor->hide();
  connect(HeadEditor,SIGNAL(insertedLines(int,int)),
	  this,SLOT(reorderContent(int,int)));
  connect(BodyEditor,SIGNAL(insertedLines(int,int)),
	  this,SLOT(reorderContent(int,int)));
  connect(HeadEditor,SIGNAL(execute(void)),
	  this,SLOT(execute(void)));
  connect(BodyEditor,SIGNAL(execute(void)),
	  this,SLOT(execute(void)));

  {
    QValueList<int> sizes=hsplitter->sizes();
    sizes[0]=200;
    hsplitter->setSizes(sizes);
    hsplitter->setResizeMode(objSplitter,QSplitter::KeepSize);
  }

  Connection.addWidget(this);

  refresh();
  startTarget();
}

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
		      "
DECLARE
    timeout BINARY_INTEGER;
BEGIN
    DBMS_DEBUG.ATTACH_SESSION(:sess<char[201],in>);
    timeout:=DBMS_DEBUG.SET_TIMEOUT(1);
END;
",
		      Connection.connection());
    attach<<(const char *)TargetID;
    otl_cursor::direct_exec(Connection.connection(),
			    "ALTER SESSION SET PLSQL_DEBUG = TRUE");
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

void toDebug::refresh(void)
{
  try {
    QString selected=Schema->currentText();
    QString currentSchema;
    if (selected.isEmpty()) {
      selected=Connection.user().upper();
      Schema->clear();
      otl_stream users(1,
		       "SELECT Username FROM ALL_Users ORDER BY Username",
		       Connection.connection());
      for(int i=0;!users.eof();i++) {
	char buffer[31];
	users>>buffer;
	Schema->insertItem(buffer);
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
		      "SELECT Object_Type,Object_Name Type FROM ALL_OBJECTS"
		      " WHERE OWNER = :Long_Name_Etc<char[31]>"
		      "   AND Object_Type IN ('FUNCTION','PACKAGE',"
		      "                       'PROCEDURE','TYPE')"
		      " ORDER BY Object_Type,Object_Name",
		      Connection.connection());
      code<<(const char *)selected;
      QListViewItem *typeItem=NULL;
      QListViewItem *last=NULL;
      while(!code.eof()) {
	char name[100];
	char type[100];
	code>>type>>name;
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
  } else {
    HeadEditor->readData(Connection,StackTrace);
    ShowButton->setEnabled(true);
    ShowButton->setOn(true);
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
      if (str[i].isSpace()) {
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
	words[curWord]+=str[i];
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
			      (const char *)sql);
      Schema=schema.upper();
      Object=object.upper();
      Type=type.upper();
      if (body)
	Type+=" BODY";
      setEdited(false);
    } catch (const otl_exception &exc) {
      toStatusMessage((const char *)exc.msg);
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
