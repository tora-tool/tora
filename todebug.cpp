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

#include "totool.h"
#include "tomarkedtext.h"
#include "tomain.h"
#include "todebug.h"
#include "tohighlightedtext.h"
#include "toresultview.h"

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
#include "icons/scansource.xpm"

class toDebugTool : public toTool {
  virtual char **pictureXPM(void)
  { return todebug_xpm; }
public:
  toDebugTool()
    : toTool(10,"PL/SQL Debugger")
  { }
  virtual QWidget *toolWindow(toMain *main,toConnection &connection)
  {
    QWidget *window=new toDebug(main,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
};

static toDebugTool DebugTool;

#include <stdio.h>

void toDebug::targetTask::run(void)
{
  printf("Started task\n");
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
    char buffer[201];
    {
      toLocker lock(Parent.Lock);
      init>>buffer;
      Parent.TargetID=buffer;
      init>>buffer;
      Parent.TargetSession=buffer;
      Parent.ChildSemaphore.up();
    }
    printf("Connected to database\n");
    while(1) {
      {
	toLocker lock(Parent.Lock);
	Parent.RunningTarget=false;
      }
      Parent.TargetSemaphore.down();
      printf("Anything to do?\n");
      QString sql;
      {
	toLocker lock(Parent.Lock);
	sql=Parent.TargetSQL;
	Parent.TargetSQL="";
      }
      if (sql.isEmpty())
	break;

      {
	toLocker lock(Parent.Lock);
	Parent.RunningTarget=true;
      }
      try {
      
      } catch (...) {
	printf("Encountered error in target SQL\n");
      }
    }

    printf("Target thread exiting\n");
  }
  toLocker lock(Parent.Lock);
  Parent.TargetThread=NULL;
  Parent.ChildSemaphore.up();
  printf("Done exiting\n");
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

void toDebug::reorderContent(int start,int diff)
{
  QString name;
  if (BodyEditor->isHidden())
    name="Head";
  else
    name="Body";
  for (QListViewItem *item=Contents->firstChild();item;item=item->nextSibling()) {
    if (item->text(0)==name)
      reorderContent(item,start,diff);
  }
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

void toDebug::stop(void)
{
}

toDebug::toDebug(toMain *main,toConnection &connection)
  : QVBox(main->workspace(),NULL,WDestructiveClose),Connection(connection),
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

  QToolBar *toolbar=new QToolBar("SQL Output",main,this);

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
		  this,SLOT(refresh(void)),
		  toolbar);
  StopButton=new QToolButton(*toStopPixmap,
			     "Stop running",
			     "Stop running",
			     this,SLOT(refresh(void)),
			     toolbar);
  StopButton->setEnabled(false);
  toolbar->addSeparator();
  StepIntoButton=new QToolButton(*toStepIntoPixmap,
				 "Step into procedure or function",
				 "Step into procedure or function",
				 this,SLOT(refresh(void)),
				 toolbar);
  StepIntoButton->setEnabled(false);
  StepOverButton=new QToolButton(*toStepOverPixmap,
				 "Step over procedure or function",
				 "Step over procedure or function",
				 this,SLOT(refresh(void)),
				 toolbar);
  StepOverButton->setEnabled(false);
  ReturnButton=new QToolButton(*toReturnFromPixmap,
			       "Return from procedure or function",
			       "Return from procedure or function",
			       this,SLOT(refresh(void)),
			       toolbar);
  ReturnButton->setEnabled(false);

  toolbar->addSeparator();
  ShowButton=new QToolButton(toolbar);
  ShowButton->setToggleButton(true);
  ShowButton->setIconSet(QIconSet(*toShowHeadPixmap),true);
  ShowButton->setIconSet(QIconSet(*toShowBodyPixmap),false);
  ShowButton->setOn(true);
  connect(ShowButton,SIGNAL(toggled(bool)),this,SLOT(changeView(bool)));
  QToolTip::add(ShowButton,"Show head or body of packages and procedures.");

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

  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QSplitter *splitter=new QSplitter(Vertical,this);
  QSplitter *hsplitter=new QSplitter(Horizontal,splitter);
  DebugTabs=new QTabWidget(splitter);

  QSplitter *objSplitter=new QSplitter(Vertical,hsplitter);

  Objects=new QListView(objSplitter);
  Objects->addColumn("Objects");
  Objects->setRootIsDecorated(true);
  connect(Objects,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changePackage(QListViewItem *)));
  Contents=new QListView(objSplitter);
  Contents->addColumn("Contents");
  Contents->setRootIsDecorated(true);
  Contents->setSorting(-1);
  connect(Contents,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeContent(QListViewItem *)));

  HeadEditor=new toHighlightedText(hsplitter);
  BodyEditor=new toHighlightedText(hsplitter);
  HeadEditor->hide();
  connect(HeadEditor,SIGNAL(insertedLines(int,int)),
	  this,SLOT(reorderContent(int,int)));
  connect(BodyEditor,SIGNAL(insertedLines(int,int)),
	  this,SLOT(reorderContent(int,int)));

  QValueList<int> sizes=hsplitter->sizes();
  sizes[0]=200;
  hsplitter->setSizes(sizes);
  hsplitter->setResizeMode(objSplitter,QSplitter::KeepSize);

  DebugTabs->hide();

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
}

void toDebug::changeSchema(int)
{
  CurrentSchema=Schema->currentText();
  refresh();
}

void toDebug::refresh(void)
{
  try {
    QString selected=Schema->currentText();
    if (selected.isEmpty()) {
      CurrentSchema=Connection.user().upper();
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
    if (!CurrentSchema.isEmpty()) {
      for (int i=0;i<Schema->count();i++)
	if (Schema->text(i)==CurrentSchema) {
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
      code<<(const char *)CurrentSchema;
      QListViewItem *typeItem=NULL;
      QListViewItem *last=NULL;
      while(!code.eof()) {
	char name[100];
	char type[100];
	code>>type>>name;
	if (!typeItem||typeItem->text(0)!=type) {
	  typeItem=new QListViewItem(Objects,typeItem,type);
	}
	last=new QListViewItem(typeItem,last,name);
	last->setText(1,type);
	if (type==CurrentType&&name==CurrentObject) {
	  Objects->setOpen(typeItem,true);
	  Objects->setSelected(last,true);
	}
      }
    }
  } TOCATCH
}

bool toDebug::checkCompile(void)
{
  return true;
}

void toDebug::updateCurrent()
{
  HeadEditor->clear();
  BodyEditor->clear();
  try {
    otl_stream lines(1,
		     "SELECT Text FROM All_Source"
		     " WHERE OWNER = :f1<char[31]>"
		     "   AND NAME = :f2<char[31]>"
		     "   AND TYPE = :f3<char[31]>"
		     " ORDER BY Type,Line",
		     Connection.connection());
    otl_stream errors(1,
		      "SELECT Line-1,Text FROM All_Errors"
		      " WHERE OWNER = :f1<char[31]>"
		      "   AND NAME = :f2<char[31]>"
		      "   AND TYPE = :f3<char[31]>"
		      " ORDER BY Type,Line",
		      Connection.connection());

    QString type=CurrentType;

    map<int,QString> Errors;

    type+=" BODY";
    lines<<(const char *)CurrentSchema;
    lines<<(const char *)CurrentObject;
    lines<<(const char *)type;
    QString str;
    while(!lines.eof()) {
      char buffer[4001];
      lines>>buffer;
      str+=buffer;
    }
    BodyEditor->setText(str);
    bool noBody=false;
    if (str.isEmpty()) {
      noBody=true;
      HeadEditor->setErrors(Errors);
    } else {
      errors<<(const char *)CurrentSchema;
      errors<<(const char *)CurrentObject;
      errors<<(const char *)type;
      while(!errors.eof()) {
	char buffer[4001];
	int line;
	errors>>line;
	errors>>buffer;
	Errors[line]+=" ";
	Errors[line]+=buffer;
      }
      BodyEditor->setErrors(Errors);
    }
    Errors.clear();

    str="";
    lines<<(const char *)CurrentSchema;
    lines<<(const char *)CurrentObject;
    lines<<(const char *)CurrentType;
    while(!lines.eof()) {
      char buffer[4001];
      lines>>buffer;
      str+=buffer;
    }
    errors<<(const char *)CurrentSchema;
    errors<<(const char *)CurrentObject;
    errors<<(const char *)CurrentType;
    while(!errors.eof()) {
      char buffer[4001];
      int line;
      errors>>line;
      errors>>buffer;
      Errors[line]+=" ";
      Errors[line]+=buffer;
    }
    if (noBody) {
      BodyEditor->setText(str);
      BodyEditor->setErrors(Errors);
      HeadEditor->setText("");
      BodyEditor->show();
      HeadEditor->hide();
      changeView(false);
      ShowButton->setEnabled(false);
    } else {
      ShowButton->setEnabled(true);
      HeadEditor->setText(str);
      HeadEditor->setErrors(Errors);
      changeView(ShowButton->isOn());
    }
    updateContent(true);
    updateContent(false);
  } TOCATCH
}

void toDebug::changePackage(QListViewItem *item)
{
  if (item&&item->parent()&&checkCompile()) {
    CurrentSchema=Schema->currentText();
    CurrentObject=item->text(0);
    CurrentType=item->text(1);
    updateCurrent();
  } else if (item&&!item->parent())
    item->setOpen(true);
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
}

bool toDebug::compile(const QString &str)
{
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
      schema=CurrentSchema;
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

    printf("%s\n",(const char *)sql);

    try {
      otl_cursor::direct_exec(Connection.connection(),
			      (const char *)sql);
      CurrentSchema=schema.upper();
      CurrentObject=object.upper();
      CurrentType=type.upper();
      refresh();
    } catch (const otl_exception &exc) {
      toStatusMessage((const char *)exc.msg);
      ret=false;
    }
  }
  return ret;
}

void toDebug::compile(void)
{
  if (compile(HeadEditor->text())&&
      compile(BodyEditor->text()))
    updateCurrent();
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

  Connection.delWidget(this);
}

void toDebug::prevError(void)
{
  if (HeadEditor->isHidden())
    BodyEditor->previousError();
  else
    HeadEditor->previousError();
}

void toDebug::nextError(void)
{
  if (HeadEditor->isHidden())
    BodyEditor->nextError();
  else
    HeadEditor->nextError();
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
  } else
    ci->setOpen(true);
}

void toDebug::scanSource(void)
{
  updateContent(true);
  updateContent(false);
}
