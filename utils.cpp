//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
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

#include <stdlib.h>

#ifdef TO_KDE
#include <kurl.h>
#include <ktempfile.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kmenubar.h>
#include <kapp.h>
#endif

#include <qapplication.h>
#include <qlayout.h>
#include <qcursor.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qtoolbar.h>
#include <qworkspace.h>
#include <qvbox.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qmenubar.h>

#include "tohighlightedtext.h"
#include "tonewconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"
#include "toresult.h"
#include "tothread.h"
#include "toconnection.h"

#ifdef WIN32
#include "windows/cregistry.h"
#endif

#define CHUNK_SIZE 31

static toSQL SQLUserNames(toSQL::TOSQL_USERLIST,
			  "SELECT UserName FROM All_Users ORDER BY UserName",
			  "List users in the database");

static toSQL SQLUserNamesMySQL(toSQL::TOSQL_USERLIST,
			       "SELECT 'No schemas'",
			       QString::null,
			       "3.0",
			       "MySQL");

static toSQL SQLTextPiece("Global:SQLText",
			  "SELECT SQL_Text\n"
			  "  FROM V$SQLText_With_Newlines\n"
			  " WHERE Address||':'||Hash_Value = :f1<char[100]>\n"
			  " ORDER BY Piece",
			  "Get text of SQL statement.");

QString toSQLString(toConnection &conn,const QString &address)
{
  QString sql;

  toQList vals=toQuery::readQuery(conn,SQLTextPiece,address);

  for(toQList::iterator i=vals.begin();i!=vals.end();i++) {
    sql.append(*i);
  }
  if (sql.isEmpty())
    throw QString("SQL Address not found in SGA");
  return sql;
}

static toSQL SQLNow("Global:Now",
		    "SELECT TO_CHAR(SYSDATE) FROM DUAL",
		    "Get current date/time from database");

static toSQL SQLNowMySQL("Global:Now",
			 "SELECT now()",
			 QString::null,
			 "3.0",
			 "MySQL");

QString toNow(toConnection &conn)
{
  try {
    toQList vals=toQuery::readQuery(conn,SQLNow);
    return toPop(vals);
  } catch(...) {
    return "Unexpected error";
  }
}

QString toSQLStripSpecifier(const QString &sql)
{
  QString ret;
  char inString=0;
  for(unsigned int i=0;i<sql.length();i++) {
    QChar c=sql.at(i);
    if (inString) {
      if (char(c)==inString) {
	inString=0;
      }
      ret+=c;
    } else {
      switch(char(c)) {
      case '\'':
	inString='\'';
	ret+=c;
	break;
      case '\"':
	inString='\"';
	ret+=c;
	break;
      case ':':
	ret+=c;
	for (i++;i<sql.length();i++) {
	  c=sql.at(i);
	  if (!c.isLetterOrNumber())
	    break;
	  ret+=c;
	}
	if (c=='<') {
	  ret+=" ";
	  for (i++;i<sql.length();i++) {
	    c=sql.at(i);
	    ret+=" ";
	    if (c=='>') {
	      i++;
	      break;
	    }
	  }
	}
	i--;
	break;
      default:
	ret+=c;
      }
    }
  }
  return ret;
}

QString toSQLStripBind(const QString &sql)
{
  QString ret;
  char inString=0;
  for(unsigned int i=0;i<sql.length();i++) {
    QChar c=sql.at(i);
    if (inString) {
      if (char(c)==inString) {
	inString=0;
      }
      ret+=c;
    } else {
      switch(char(c)) {
      case '\'':
	inString='\'';
	ret+=c;
	break;
      case '\"':
	inString='\"';
	ret+=c;
	break;
      case ':':
	ret+="''";
	for (i++;i<sql.length();i++) {
	  c=sql.at(i);
	  if (!c.isLetterOrNumber())
	    break;
	}
	if (c=='<') {
	  for (i++;i<sql.length();i++) {
	    c=sql.at(i);
	    if (c=='>') {
	      i++;
	      break;
	    }
	  }
	}
	i--;
	break;
      default:
	ret+=c;
      }
    }
  }
  return ret;
}

static toSQL SQLAddress("Global:Address",
			"SELECT Address||':'||Hash_Value\n"
			"  FROM V$SQLText_With_Newlines\n"
			" WHERE SQL_Text LIKE :f1<char[150]>||'%'",
			"Get address of an SQL statement");

QString toSQLToAddress(toConnection &conn,const QString &sql)
{
  QString search=toSQLStripSpecifier(sql);

  toQList vals=toQuery::readQuery(conn,SQLAddress,search.left(CHUNK_SIZE));

  for(toQList::iterator i=vals.begin();i!=vals.end();i++) {
    if (search==toSQLString(conn,*i))
      return *i;
  }
  throw QString("SQL Query not found in SGA");
}

static std::list<QString> LastMessages;

void toStatusMessage(const QString &str,bool save,bool log)
{
  toMain *main=dynamic_cast<toMain *>(qApp->mainWidget());
  if (main) {
    int sec=toTool::globalConfig(CONF_STATUS_MESSAGE,DEFAULT_STATUS_MESSAGE).toInt();
    if (save||sec==0)
      main->statusBar()->message(str);
    else
      main->statusBar()->message(str,sec*1000);
    if (!save&&!str.isEmpty()&&log) {
      toPush(LastMessages,str);
      if (int(LastMessages.size())>toTool::globalConfig(CONF_STATUS_SAVE,
							DEFAULT_STATUS_SAVE).toInt())
	toShift(LastMessages);
    }
    QToolTip::add(main->statusBar(),str);
  }
}

std::list<QString> toStatusMessages(void)
{
  return LastMessages;
}

toMain *toMainWidget(void)
{
  return dynamic_cast<toMain *>(qApp->mainWidget());
}

QComboBox *toRefreshCreate(QWidget *parent,const char *name,const QString &def,QComboBox *item)
{
  QComboBox *refresh;
  if (item)
    refresh=item;
  else
    refresh=new QComboBox(false,parent);

  refresh->insertItem("None");
  refresh->insertItem("2 seconds");
  refresh->insertItem("5 seconds");
  refresh->insertItem("10 seconds");
  refresh->insertItem("30 seconds");
  refresh->insertItem("1 min");
  refresh->insertItem("5 min");
  refresh->insertItem("10 min");
  QString str;
  if (def)
    str=def;
  else
    str=toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH);
  if (str=="2 seconds")
    refresh->setCurrentItem(1);
  else if (str=="5 seconds")
    refresh->setCurrentItem(2);
  else if (str=="10 seconds")
    refresh->setCurrentItem(3);
  else if (str=="30 seconds")
    refresh->setCurrentItem(4);
  else if (str=="1 min")
    refresh->setCurrentItem(5);
  else if (str=="5 min")
    refresh->setCurrentItem(6);
  else if (str=="10 min")
    refresh->setCurrentItem(7);
  else
    refresh->setCurrentItem(0);
  return refresh;
}

void toRefreshParse(toTimer *timer,const QString &str)
{
  QString t=str;
  if (t.isEmpty())
    t=toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH);

  if (t=="None")
    timer->stop();
  else if (t=="2 seconds")
    timer->start(2*1000);
  else if (t=="5 seconds")
    timer->start(5*1000);
  else if (t=="10 seconds")
    timer->start(10*1000);
  else if (t=="30 seconds")
    timer->start(30*1000);
  else if (t=="1 min")
    timer->start(60*1000);
  else if (t=="5 min")
    timer->start(300*1000);
  else if (t=="10 min")
    timer->start(600*1000);
  else
    throw QString("Unknown timer value");
}

QString toDeepCopy(const QString &str)
{
  QString ret=str;
  ret+=" ";
  return ret.left(ret.length()-1);
}

#ifdef ENABLE_STYLE
#include <qmotifstyle.h>
#include <qmotifplusstyle.h>
#include <qsgistyle.h>
#include <qcdestyle.h>
#include <qwindowsstyle.h>
#include <qplatinumstyle.h>

void toSetSessionType(const QString &str)
{
  if (str=="Motif")
    qApp->setStyle(new QMotifStyle());
  else if (str=="Motif Plus")
    qApp->setStyle(new QMotifPlusStyle());
  else if (str=="SGI")
    qApp->setStyle(new QSGIStyle());
  else if (str=="CDE")
    qApp->setStyle(new QCDEStyle());
  else if (str=="Windows")
    qApp->setStyle(new QWindowsStyle());
  else if (str=="Platinum")
    qApp->setStyle(new QPlatinumStyle());
  else
    throw QString("Failed to find style match");
}

QString toGetSessionType(void)
{
  QStyle *style=&qApp->style();
  if (style->isA("QMotifPlusStyle"))
    return "Motif Plus";
  else if (style->isA("QSGIStyle"))
    return "SGI";
  else if (style->isA("QCDEStyle"))
    return "CDE";
  else if (style->isA("QMotifStyle"))
    return "Motif";
  else if (style->isA("QPlatinumStyle"))
    return "Platinum";
  else if (style->isA("QWindowsStyle"))
    return "Windows";
  throw QString("Failed to find style match");
}
#endif

QToolBar *toAllocBar(QWidget *parent,const QString &str,const QString &db)
{
  QString name=str;
  if (!db.isEmpty()&&!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty()) {
    name+=" ";
    name+=db;
  }
  QToolBar *tool;
#ifdef TO_KDE
  if (parent==toMainWidget())
    tool=new KToolBar(toMainWidget(),QMainWindow::Top);
  else {
#if KDE_VERSION < 220
    tool=new QToolBar(name,toMainWidget(),parent);
#else
    tool=new KToolBar(toMainWidget(),parent);
#endif
  }
#else
  if (parent==toMainWidget())
    tool=new QToolBar(name,toMainWidget());
  else
    tool=new QToolBar(name,toMainWidget(),parent);
#endif
  return tool;
}

TODock *toAllocDock(const QString &name,
		    const QString &db,
		    const QPixmap &icon)
{
  QString str=name;
  if (!db.isEmpty()&&!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty()) {
    str+=" ";
    str+=db;
  }
#ifdef TO_KDE
  KDockMainWindow *main=dynamic_cast<KDockMainWindow *>(toMainWidget());
  if (main)
    return main->createDockWidget(str,icon);
  else
    throw QString("Main widget not KDockMainWindow");
#else
#  if QT_VERSION < 300
  if (toTool::globalConfig(CONF_DOCK_TOOLBAR,"Yes").isEmpty()) {
    QVBox *frm=new QVBox(toMainWidget()->workspace());
    frm->setCaption(str);
    return frm;
  } else {
    QToolBar *toolbar=toAllocBar(toMainWidget(),name,db);
    return toolbar;
  }
#  else
  return new QDockWindow(QDockWindow::InDock,toMainWidget());
#  endif
#endif
}

void toAttachDock(TODock *dock,QWidget *container,QMainWindow::ToolBarDock place)
{
#ifdef TO_KDE
  KDockMainWindow *main=dynamic_cast<KDockMainWindow *>(toMainWidget());
  if (main) {
    KDockWidget::DockPosition pos;
    switch (place) {
    case QMainWindow::Top:
      pos=KDockWidget::DockTop;
      break;
    case QMainWindow::Bottom:
      pos=KDockWidget::DockBottom;
      break;
    case QMainWindow::Left:
      pos=KDockWidget::DockLeft;
      break;
    case QMainWindow::Right:
      pos=KDockWidget::DockRight;
      break;
    default:
      throw QString("Unknown dock position");
    }
    KDockWidget *dw=(KDockWidget *)(dock);
    if (dw) {
      dw->setWidget(container);
      dw->manualDock(main->getMainDockWidget(),pos,20);
    }
  } else
    throw QString("Main widget not KDockMainWindow");
#else
#  if QT_VERSION < 300
  if (!toTool::globalConfig(CONF_DOCK_TOOLBAR,"Yes").isEmpty()) {
    QToolBar *bar=(QToolBar *)dock;
    if (bar) {
      toMainWidget()->moveToolBar(bar,place);
      bar->setStretchableWidget(container);
    }
  }
#  else
  QDockWindow *d=(QDockWindow *)dock;
  if (d) {
    toMainWidget()->moveDockWindow(d,place);
    d->setResizeEnabled(true);
    d->setWidget(container);
    container->show();
  }
#  endif
#endif
}

QString toFontToString(const QFont &fnt)
{
#if QT_VERSION >= 300
  return fnt.toString();
#else
#  ifdef TO_FONT_RAW_NAME
  return fnt.rawName();
#  else
  QStringList lst;
  lst.insert(lst.end(),fnt.family());
  lst.insert(lst.end(),QString::number(fnt.pointSize()));
  lst.insert(lst.end(),QString::number(fnt.weight()));
  lst.insert(lst.end(),QString::number(fnt.italic()));
  lst.insert(lst.end(),QString::number(fnt.charSet()));
  return lst.join("/");
#  endif
#endif
}

QFont toStringToFont(const QString &str)
{
  if (str.isEmpty())
    return QFont("Courier",12);
#if QT_VERSION >= 300
  QFont fnt;
  fnt.fromString(str);
  return fnt;
#else
#  ifdef TO_FONT_RAW_NAME
  QFont fnt;
  fnt.setRawName(str);
  return fnt;
#  else
  QStringList lst=QStringList::split("/",str);
  if (lst.count()!=5)
    return QFont("Courier",12);
  QFont font(lst[0],lst[1].toInt(),lst[2].toInt(),
	     bool(lst[3].toInt()),QFont::CharSet(lst[4].toInt()));
  return font;
#  endif
#endif
}

int toSizeDecode(const QString &str)
{
  if (str=="KB")
    return 1024;
  if (str=="MB")
    return 1024*1024;
  return 1;
}

QString toPluginPath(void)
{
  QString str;
#ifndef WIN32
  str=toTool::globalConfig(CONF_PLUGIN_DIR,DEFAULT_PLUGIN_DIR);
#else
  CRegistry registry;
  DWORD siz=1024;
  char buffer[1024];
  try {
    if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
				"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\tora.exe",
				"Path",
				buffer,siz)) {
      if (siz>0)
	str=buffer;
    }
  } catch(...) {
  }
#endif
  return str;
}

QString toHelpPath(void)
{
  QString str=toTool::globalConfig(CONF_HELP_PATH,"");
  if (str.isEmpty()) {
    str=toPluginPath();
    str+="/help/toc.htm";
  }
  return str;
}

static QString toExpandFile(const QString &file)
{
  QString ret(file);
  QString home;
#ifdef WIN32
  CRegistry registry;
  DWORD siz=1024;
  char buffer[1024];
  try {
    if (registry.GetStringValue(HKEY_CURRENT_USER,
				"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
				"Personal",
				buffer,siz)) {
      if (siz>0)
	home=buffer;
    }
  } catch(...) {
  }
#else
  const char *homet=getenv("HOME");
  if (!homet)
    home="";
  else
    home=homet;
#endif
  ret.replace(QRegExp("\\$HOME"),home);
  return ret;
}

QCString toReadFile(const QString &filename)
{
  QString expanded=toExpandFile(filename);
#ifdef TO_KDE
  KURL url(expanded);
  if (!url.isLocalFile()) {
    QString tmpFile;
    if(KIO::NetAccess::download(url,tmpFile)) {
      QFile file(tmpFile);
      if (!file.open(IO_ReadOnly)) {
	KIO::NetAccess::removeTempFile(tmpFile);
	throw QString("Couldn't open file %1.").arg(filename);
      }

      int size=file.size();

      char *buf=new char[size+1];
      if (file.readBlock(buf,size)==-1) {
	delete buf;
	KIO::NetAccess::removeTempFile(tmpFile);
	throw QString("Encountered problems read configuration");
      }
      buf[size]=0;
      QCString ret(buf,size+1);
      delete buf;
      KIO::NetAccess::removeTempFile(tmpFile);
      return ret;
    }
    throw QString("Couldn't download file");
  }
#endif
  QFile file(expanded);
  if (!file.open(IO_ReadOnly))
    throw QString("Couldn't open file %1.").arg(filename);
	    
  int size=file.size();
	    
  char *buf=new char[size+1];
  if (file.readBlock(buf,size)==-1) {
    delete buf;
    throw QString("Encountered problems read configuration");
  }
  buf[size]=0;
  QCString ret(buf,size+1);
  delete buf;
  return ret;
}

bool toWriteFile(const QString &filename,const QCString &data)
{
  QString expanded=toExpandFile(filename);
#ifdef TO_KDE
  KURL url(expanded);
  if (!url.isLocalFile()) {
    KTempFile file;
    file.file()->writeBlock(data,data.length());
    if (file.status()!=IO_Ok) {
      TOMessageBox::warning(toMainWidget(),"File error","Couldn't write data to tempfile");
      file.unlink();
      return false;
    }
    file.close();
    if (!KIO::NetAccess::upload(file.name(),url)) {
      file.unlink();
      TOMessageBox::warning(toMainWidget(),"File error","Couldn't upload data to URL");
      return false;
    }
    file.unlink();
    toStatusMessage("File saved successfully",false,false);
    return true;
  }
#endif

  QFile file(expanded);
  if (!file.open(IO_WriteOnly)) {
    TOMessageBox::warning(toMainWidget(),"File error","Couldn't open file for writing");
    return false;
  }
  file.writeBlock(data,data.length());
  if (file.status()!=IO_Ok) {
    TOMessageBox::warning(toMainWidget(),"File error","Couldn't write data to file");
    return false;
  }
  toStatusMessage("File saved successfully",false,false);
  return true;
}

bool toWriteFile(const QString &filename,const QString &data)
{
  return toWriteFile(filename,data.local8Bit());
}

bool toCompareLists(QStringList &lst1,QStringList &lst2,unsigned int len)
{
  if (lst1.count()<len||lst2.count()<len)
    return false;
  for (unsigned int i=0;i<len;i++)
    if (lst1[i]!=lst2[i])
      return false;
  return true;
}

QString toOpenFilename(const QString &filename,const QString &filter,QWidget *parent)
{
#ifdef TO_KDE
  KURL url=TOFileDialog::getOpenURL(filename,filter,parent);
  if (url.isEmpty())
    return QString::null;
  if (url.isLocalFile())
    return url.path();
  return url.url();
#else
  return TOFileDialog::getOpenFileName(filename,filter,parent);
#endif
}

QString toSaveFilename(const QString &filename,const QString &filter,QWidget *parent)
{
#ifdef TO_KDE
  KURL url=TOFileDialog::getSaveURL(filename,filter,parent);
  if (url.hasPass())
    TOMessageBox::warning(toMainWidget(),"File open password",url.pass());
  if (url.isEmpty())
    return QString::null;
  if (url.isLocalFile())
    return url.path();
  return url.url();
#else
  return TOFileDialog::getSaveFileName(filename,filter,parent);
#endif
}

void toSetEnv(const QCString &var,const QCString &val)
{
#ifndef TO_HAS_SETENV
  // Has a memory leak, but just a minor one.

  char *env=new char[var.length()+val.length()+2];
  strcpy(env,var);
  strcat(env,"=");
  strcat(env,val);
  putenv(env);
#else
  setenv(var,val,1);
#endif
}

void toUnSetEnv(const QCString &var)
{
#ifndef TO_HAS_SETENV
  toSetEnv(var,"");
#else
  unsetenv(var);
#endif
}

int toToolMenuIndex(void)
{
  return toMainWidget()->menuBar()->indexOf(toMain::TO_WINDOWS_MENU);
}

static QColor ChartColors[]={
  Qt::red,
  Qt::green,
  Qt::blue,
  Qt::cyan,
  Qt::magenta,
  Qt::yellow,
  Qt::darkRed,
  Qt::darkGreen,
  Qt::darkBlue,
  Qt::darkCyan,
  Qt::darkMagenta,
  Qt::darkYellow
};

QColor toChartColor(int index)
{
  return ChartColors[index%(sizeof(ChartColors)/sizeof(QColor))];
}

toToolWidget *toCurrentTool(QObject *cur)
{
  while(cur) {
    try {
      toToolWidget *tool=dynamic_cast<toToolWidget *>(cur);
      if (tool)
	return tool;
    } catch(...) {
      // Catch problems with Visual C++ missing RTTI
    }
    cur=cur->parent();
  }
  throw QString("Couldn't find parent connection. Internal error.");
}

toConnection &toCurrentConnection(QObject *cur)
{
  return toCurrentTool(cur)->connection();
}

unsigned int toBusy::Count=0;

toBusy::toBusy()
{
  if (!Count&&toThread::mainThread())
    qApp->setOverrideCursor(Qt::waitCursor);
  Count++;
}

toBusy::~toBusy()
{
  Count--;
  if (!Count&&toThread::mainThread())
    qApp->restoreOverrideCursor();
}

void toReadableColumn(QString &name)
{
  bool inWord=false;
  for(unsigned int i=0;i<name.length();i++) {
    if (name.at(i)=='_') {
      name.ref(i)=' ';
      inWord=false;
    } else if (name.at(i).isSpace()) {
      inWord=false;
    } else if (name.at(i).isLetter()) {
      if (inWord)
	name.ref(i)=name.at(i).lower();
      else
	name.ref(i)=name.at(i).upper();
      inWord=true;
    }
  }
}

bool toIsOracle(const toConnection &conn)
{
  return conn.provider()=="Oracle";
}

QString toGetToken(toMarkedText *text,int &curLine,int &pos,bool forward)
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

  int inc=forward?1:-1;

  while(curLine<int(text->numLines())) {
    QString line=text->textLine(curLine);
    while((forward&&pos<int(line.length()))||(!forward&&pos>=0)) {
      if (!forward)
	pos--;
      c=line[pos];
      if ((forward&&pos<int(line.length()-1))||(!forward&&pos>0))
	nc=line[pos+inc];
      else
	nc='\n';
      if (forward)
	pos++;
      if (state==space) {
	if(forward&&c=='-'&&nc=='-')
	  break;
	if(!c.isSpace())
	  state=any;
      }
      if (state!=space) {
	if(forward)
	  token+=c;
	else
	  token.prepend(c);
	switch(state) {
	case space:
	  break;
	case any:
	  if (toIsIdent(c)) {
	    if (!toIsIdent(nc))
	      return token;
	    state=identifier;
	  } else if (c=='\''||c=='\"') {
	    endString=c;
	    state=string;
	  } else {
	    if ((forward&&c==':'&&nc=='=')||
		(!forward&&nc==':'&&c=='=')) {
	      if(forward)
		token+=c;
	      else
		token.prepend(c);
	      pos+=inc;
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
    curLine+=inc;
    if (forward)
      pos=0;
    else if (curLine>=0) {
      line=text->textLine(curLine);
      pos=line.length();
    } else
      break;
  }
  return QString::null;
}

#ifndef TO_LICENSE

bool toCheckLicense(void)
{
  return true;
}

void toEnterLicense(void)
{
  throw QString("Should never need to ask for default license");
}

#endif
