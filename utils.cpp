//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 *      these libraries without written consent from Quest Software, Inc.
 *      Observe that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "tobackground.h"
#include "toconf.h"
#include "toconnection.h"
#include "tohighlightedtext.h"
#include "tomain.h"
#include "tonewconnection.h"
#include "toresult.h"
#include "tosql.h"
#include "tothread.h"
#include "totool.h"

#include <stdlib.h>

#ifdef TO_KDE
#include <kapp.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <ktempfile.h>
#include <kurl.h>
#endif

#include <qapplication.h>
#include <qcombobox.h>
#include <qcstring.h>
#include <qcursor.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qworkspace.h>

#include "utils.moc"

#ifdef WIN32
#include "windows/cregistry.h"
#endif

#if defined(Q_OS_MACX)
#include <sys/param.h>
#include <CoreServices/CoreServices.h>
#endif // Q_OS_MACX

#define CHUNK_SIZE 31

// A little magic to get lrefresh to work and get a check on qApp

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

static toSQL SQLUserNamesMySQL(toSQL::TOSQL_USERLIST,
			       "SHOW DATABASES",
			       "List users in the database",
			       "3.0",
			       "MySQL");

static toSQL SQLUserNames(toSQL::TOSQL_USERLIST,
			  "SELECT UserName FROM sys.All_Users ORDER BY UserName",
			  "");

static toSQL SQLUserNamesPgSQL(toSQL::TOSQL_USERLIST,
			       "SELECT usename AS UserName FROM pg_user ORDER BY UserName",
			       "",
			       "7.1",
			       "PostgreSQL");

static toSQL SQLUserNamesSapDB(toSQL::TOSQL_USERLIST,
			       "SELECT username \"UserName\" FROM users ORDER BY username",
			       "",
			       "",
			       "SapDB");

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
    throw qApp->translate("toSQLString","SQL Address not found in SGA");
  return sql;
}

static toSQL SQLNowMySQL("Global:Now",
			 "SELECT now()",
			 "Get current date/time from database",
			 "3.0",
			 "MySQL");
static toSQL SQLNow("Global:Now",
		    "SELECT TO_CHAR(SYSDATE) FROM sys.DUAL",
		    "");

static toSQL SQLNowPgSQL("Global:Now",
			 "SELECT now()",
			 "",
			 "7.1",
			 "PostgreSQL");

QString toNow(toConnection &conn)
{
  try {
    toQList vals=toQuery::readQuery(conn,SQLNow);
    return toPop(vals);
  } catch(...) {
    return qApp->translate("toNow","Unexpected error");
  }
}

QString toSQLStripSpecifier(const QString &sql)
{
  QString ret;
  char inString=0;
  for(unsigned int i=0;i<sql.length();i++) {
    QChar rc=sql.at(i);
    char c=rc.latin1();
    if (inString) {
      if (c==inString) {
	inString=0;
      }
      ret+=c;
    } else {
      switch(c) {
      case '\'':
	inString='\'';
	ret+=rc;
	break;
      case '\"':
	inString='\"';
	ret+=rc;
	break;
      case ':':
	ret+=rc;
	for (i++;i<sql.length();i++) {
	  rc=sql.at(i);
	  c=rc.latin1();
	  if (!rc.isLetterOrNumber())
	    break;
	  ret+=rc;
	}
	if (c=='<') {
	  ret+=QString::fromLatin1(" ");
	  for (i++;i<sql.length();i++) {
	    rc=sql.at(i);
	    c=rc.latin1();
	    ret+=QString::fromLatin1(" ");
	    if (c=='>') {
	      i++;
	      break;
	    }
	  }
	}
	i--;
	break;
      default:
	ret+=rc;
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
    QChar rc=sql.at(i);
    char c=rc.latin1();
    if (inString) {
      if (c==inString) {
	inString=0;
      }
      ret+=rc;
    } else {
      switch(char(c)) {
      case '\'':
	inString='\'';
	ret+=rc;
	break;
      case '\"':
	inString='\"';
	ret+=rc;
	break;
      case ':':
	ret+=QString::fromLatin1("''");
	for (i++;i<sql.length();i++) {
	  rc=sql.at(i);
	  c=rc.latin1();
	  if (!rc.isLetterOrNumber())
	    break;
	}
	if (c=='<') {
	  for (i++;i<sql.length();i++) {
	    rc=sql.at(i);
	    c=rc.latin1();
	    if (c=='>') {
	      i++;
	      break;
	    }
	  }
	}
	i--;
	break;
      default:
	ret+=rc;
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
  throw qApp->translate("toSQLToAddress","SQL Query not found in SGA");
}

static std::list<QString> LastMessages;

void toStatusMessage(const QString &str,bool save,bool log)
{
  if (!toThread::mainThread())
    return;
  toMain *main=dynamic_cast<toMain *>(qApp->mainWidget());
  if (main) {
    int sec=toTool::globalConfig(CONF_STATUS_MESSAGE,DEFAULT_STATUS_MESSAGE).toInt();
    if (save||sec==0)
      main->statusBar()->message(str.simplifyWhiteSpace());
    else
      main->statusBar()->message(str.simplifyWhiteSpace(),sec*1000);
    if (!save&&!str.isEmpty()&&log) {
      if (toTool::globalConfig(CONF_MESSAGE_STATUSBAR,"").isEmpty())
	main->displayMessage(str);
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
    refresh=new QComboBox(false,parent,name);

  refresh->insertItem(qApp->translate("toRefreshCreate","None"));
  refresh->insertItem(qApp->translate("toRefreshCreate","2 seconds"));
  refresh->insertItem(qApp->translate("toRefreshCreate","5 seconds"));
  refresh->insertItem(qApp->translate("toRefreshCreate","10 seconds"));
  refresh->insertItem(qApp->translate("toRefreshCreate","30 seconds"));
  refresh->insertItem(qApp->translate("toRefreshCreate","1 min"));
  refresh->insertItem(qApp->translate("toRefreshCreate","5 min"));
  refresh->insertItem(qApp->translate("toRefreshCreate","10 min"));
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

  if (t==qApp->translate("toRefreshCreate","None")||t=="None")
    timer->stop();
  else if (t==qApp->translate("toRefreshCreate","2 seconds")||t=="2 seconds")
    timer->start(2*1000);
  else if (t==qApp->translate("toRefreshCreate","5 seconds")||t=="5 seconds")
    timer->start(5*1000);
  else if (t==qApp->translate("toRefreshCreate","10 seconds")||t=="10 seconds")
    timer->start(10*1000);
  else if (t==qApp->translate("toRefreshCreate","30 seconds")||t=="30 seconds")
    timer->start(30*1000);
  else if (t==qApp->translate("toRefreshCreate","1 min")||t=="1 min")
    timer->start(60*1000);
  else if (t==qApp->translate("toRefreshCreate","5 min")||t=="5 min")
    timer->start(300*1000);
  else if (t==qApp->translate("toRefreshCreate","10 min")||t=="10 min")
    timer->start(600*1000);
  else
    throw qApp->translate("toRefreshParse","Unknown timer value");
}

QString toDeepCopy(const QString &str)
{
  return str.copy();
}

#ifdef ENABLE_STYLE
#  if QT_VERSION < 300
#    include <qmotifstyle.h>
#    include <qmotifplusstyle.h>
#    include <qsgistyle.h>
#    include <qcdestyle.h>
#    include <qwindowsstyle.h>
#    include <qplatinumstyle.h>

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
    toStatusMessage(qApp->translate("toSetSessionType","Failed to find style %1").arg(str));
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
  toStatusMessage(qApp->translate("toGetSessionType","Failed to find style match"));
  return DEFAULT_STYLE;
}

QStringList toGetSessionTypes(void)
{
  QStringList ret;
  ret<<"Motif Plus";
  ret<<"SGI";
  ret<<"CDI";
  ret<<"Motif";
  ret<<"Platinum";
  ret<<"Windows";
  return ret;
}

#  else
#    include <qstylefactory.h>
#    include <qstyle.h>

QStringList toGetSessionTypes(void)
{
  return QStyleFactory::keys();
}

QString toGetSessionType(void)
{
  QStyle *style=&qApp->style();

  // This is probably really slow, but who cares.

  QStringList keys=QStyleFactory::keys();
  for(unsigned int i=0;i<keys.size();i++) {
    QString name=keys[i];
    QStyle *t=QStyleFactory::create(name);
    if (!t)
      continue;
    if (t->className()==style->className()) {
      delete t;
      return name;
    }
    delete t;
  }

  // Weird should never get here.
  return style->className();
}

void toSetSessionType(const QString &str)
{
  QStyle *style=QStyleFactory::create(str);
  if (style)
    qApp->setStyle(style);
  else
    toStatusMessage(qApp->translate("toSetSessionType","Failed to find style %1").arg(str));
}

#  endif
#endif

QToolBar *toAllocBar(QWidget *parent,const QString &str)
{
  QString db;
  try {
    db=toCurrentConnection(parent).description(false);
  } catch(...) {
  }
  QString name=str;
  if (!db.isEmpty()&&!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty()) {
    name+=QString::fromLatin1(" ");
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
  tool->setSizePolicy(QSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed));
  return tool;
}

TODock *toAllocDock(const QString &name,
		    const QString &db,
#ifdef TO_KDE
		    const QPixmap &icon
#else
		    const QPixmap &
#endif
		    )
{
  QString str=name;
  if (!db.isEmpty()&&!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty()) {
    str+=QString::fromLatin1(" ");
    str+=db;
  }
#ifdef TO_KDE
  KDockMainWindow *main=(KDockMainWindow *)toMainWidget();
  return main->createDockWidget(str,icon);
#else
#  if QT_VERSION < 300
  if (toTool::globalConfig(CONF_DOCK_TOOLBAR,"Yes").isEmpty()) {
    QVBox *frm=new QVBox(toMainWidget()->workspace());
    frm->setCaption(str);
    return frm;
  } else {
    QToolBar *toolbar=toAllocBar(toMainWidget(),name);
    return toolbar;
  }
#  else
  QDockWindow *dock=new QDockWindow(QDockWindow::InDock,toMainWidget());
  dock->setNewLine(true);
  dock->setCloseMode(QDockWindow::Always);
  return dock;
#  endif
#endif
}

void toAttachDock(TODock *dock,QWidget *container,QMainWindow::ToolBarDock place)
{
#ifdef TO_KDE
  KDockMainWindow *main=dynamic_cast<KDockMainWindow *>(toMainWidget());
  if (main) {
    KDockWidget::DockPosition pos=KDockWidget::DockLeft;
    int pct=20;
    switch (place) {
    case QMainWindow::Top:
      pos=KDockWidget::DockTop;
      break;
    case QMainWindow::Bottom:
      pct=80;
      pos=KDockWidget::DockBottom;
      break;
    default:
      toStatusMessage(qApp->translate("toAttachDock","Unknown dock position"));
      break;
    case QMainWindow::Right:
      pct=80;
      pos=KDockWidget::DockRight;
      break;
    case QMainWindow::Left:
    case QMainWindow::Minimized:
      break;
    }
    KDockWidget *dw=(KDockWidget *)(dock);
    if (dw) {
      dw->setWidget(container);
      if (place==QMainWindow::Minimized)
	main->makeDockInvisible(dw);
      else
	dw->manualDock(main->getMainDockWidget(),pos,pct);
    }
  } else {
    toStatusMessage(qApp->translate("toAttachDock","Main widget not KDockMainWindow"));
    return;
  }
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
  lst.insert(lst.end(),QString::number(int(fnt.weight())));
  lst.insert(lst.end(),QString::number(int(fnt.italic())));
  lst.insert(lst.end(),QString::number(int(fnt.charSet())));
  return lst.join("/");
#  endif
#endif
}

QFont toStringToFont(const QString &str)
{
  if (str.isEmpty())
    return QFont(QString::fromLatin1("Courier"),12);
#if QT_VERSION >= 300
  QFont fnt;
  if (fnt.fromString(str))
    return fnt;
#endif
#ifdef TO_FONT_RAW_NAME
  QFont fnt;
  fnt.setRawName(str);
  return fnt;
#else
  QStringList lst=QStringList::split(QString::fromLatin1("/"),str);
  if (lst.count()!=5)
    return QFont(QString::fromLatin1("Courier"),12);
  return QFont(lst[0],lst[1].toInt(),lst[2].toInt(),
	       bool(lst[3].toInt())
#  if QT_VERSION < 300
	       ,QFont::CharSet(lst[4].toInt())
#  endif
	       );
#endif
}

int toSizeDecode(const QString &str)
{
  if (str==QString::fromLatin1("KB"))
    return 1024;
  if (str==QString::fromLatin1("MB"))
    return 1024*1024;
  return 1;
}

QString toPluginPath(void)
{
  QString str;
#ifdef WIN32
  CRegistry registry;
  DWORD siz=1024;
  char buffer[1024];
  try {
    if (registry.GetStringValue(HKEY_LOCAL_MACHINE,
#  ifdef TOAD
				"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TOAD for MySQL",
#  else
				"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\TOra",
#  endif
				"UninstallString",
				buffer,siz)) {
      if (siz>0) {
	str=buffer;
	static QRegExp findQuotes("\"([^\"]*)\"");
	if (findQuotes.search(str)>=0)
	  str=findQuotes.cap(1);
	int ind=str.findRev('\\');
	if(ind>=0)
	  str=str.mid(0,ind);
      }
    }
  } catch(...) {
  }
#elif defined( Q_OS_MACX )
  { // MacOS
  char resourcePath[MAXPATHLEN];
  memset( &resourcePath[0], 0, MAXPATHLEN );
  CFBundleRef appBundle = ::CFBundleGetMainBundle();
  if ( appBundle ) {
    CFURLRef urlRef = CFBundleCopyResourcesDirectoryURL( appBundle );
    if ( urlRef ) {
      UInt8* _p = (UInt8*) &resourcePath[0];
      bool isOK = CFURLGetFileSystemRepresentation(
          urlRef, TRUE, _p, MAXPATHLEN );
      if ( !isOK ) {
        // QMessageBox::warning( 0, "File error",
        //     QString( "Unexpected: no file system representation") );
      }
    } else {
      // QMessageBox::warning( 0, "File error",
      //     QString( "Unexpected: unable to get resource directory") );
    }
    CFRelease( urlRef );
    str = &resourcePath[0];
  } else {
    // QMessageBox::warning( 0, "File error",
    //     QString( "Unexpected: unable to get main bundle") );
  }
  } // MacOS
#else
  str=toTool::globalConfig(CONF_PLUGIN_DIR,DEFAULT_PLUGIN_DIR);
#endif
  return str;
}

QString toHelpPath(void)
{
  QString str=toTool::globalConfig(CONF_HELP_PATH,"");
  if (str.isEmpty()) {
    str=toPluginPath();
    str+=QString::fromLatin1("/help/toc.htm");
  }
  return str;
}

QString toExpandFile(const QString &file)
{
  QString ret(file);
  QString home;
#ifdef WIN32
  CRegistry registry;
  DWORD siz=1024;
  char buffer[1024];
  try {
    if (registry.GetStringValue(HKEY_CURRENT_USER,
				"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders",
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
  ret.replace(QRegExp(QString::fromLatin1("\\$HOME")),home);
  return ret;
}

QCString toReadFile(const QString &filename)
{
  QString expanded=toExpandFile(filename);
#ifdef TO_KDE
  KURL url(expanded);
  if (!url.isLocalFile()) {
    QString tmpFile;
    if(KIO::NetAccess::download(url,tmpFile
#if KDE_VERSION >= 0x30200
				,toMainWidget()
#endif
				)) {
      QFile file(tmpFile);
      if (!file.open(IO_ReadOnly)) {
	KIO::NetAccess::removeTempFile(tmpFile);
	throw QT_TRANSLATE_NOOP("toReadFile","Couldn't open file %1.").arg(filename);
      }

      int size=file.size();

      char *buf=new char[size+1];
      if (file.readBlock(buf,size)==-1) {
	delete[] buf;
	KIO::NetAccess::removeTempFile(tmpFile);
	throw QT_TRANSLATE_NOOP("toReadFile","Encountered problems read configuration");
      }
      buf[size]=0;
      QCString ret(buf,size+1);
      delete[] buf;
      KIO::NetAccess::removeTempFile(tmpFile);
      return ret;
    }
    throw QT_TRANSLATE_NOOP("toReadFile","Couldn't download file");
  }
#endif
  QFile file(expanded);
  if (!file.open(IO_ReadOnly))
    throw QT_TRANSLATE_NOOP("toReadFile","Couldn't open file %1.").arg(filename);
  
  int size=file.size();
  
  char *buf=new char[size+1];
  if (file.readBlock(buf,size)==-1) {
    delete[] buf;
    throw QT_TRANSLATE_NOOP("toReadFile","Encountered problems read configuration");
  }
  buf[size]=0;
  QCString ret(buf,size+1);
  delete[] buf;
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
      TOMessageBox::warning(toMainWidget(),QT_TRANSLATE_NOOP("toWriteFile","File error"),
			    QT_TRANSLATE_NOOP("toWriteFile","Couldn't write data to tempfile"));
      file.unlink();
      return false;
    }
    file.close();
    if (!KIO::NetAccess::upload(file.name(),url
#if KDE_VERSION >= 0x30200
				,toMainWidget()
#endif
				)) {
      file.unlink();
      TOMessageBox::warning(toMainWidget(),QT_TRANSLATE_NOOP("toWriteFile","File error"),
			    QT_TRANSLATE_NOOP("toWriteFile","Couldn't upload data to URL"));
      return false;
    }
    file.unlink();
    toStatusMessage(QT_TRANSLATE_NOOP("toWriteFile","File saved successfully"),false,false);
    return true;
  }
#endif

  QFile file(expanded);
  if (!file.open(IO_WriteOnly)) {
    TOMessageBox::warning(toMainWidget(),QT_TRANSLATE_NOOP("toWriteFile","File error"),
			  QT_TRANSLATE_NOOP("toWriteFile","Couldn't open file for writing"));
    return false;
  }
  file.writeBlock(data,data.length());
  if (file.status()!=IO_Ok) {
    TOMessageBox::warning(toMainWidget(),QT_TRANSLATE_NOOP("toWriteFile","File error"),
			  QT_TRANSLATE_NOOP("toWriteFile","Couldn't write data to file"));
    return false;
  }
  toStatusMessage(QT_TRANSLATE_NOOP("toWriteFile","File saved successfully"),false,false);
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

static QString GetExtensions(void)
{
  static QRegExp repl(QString::fromLatin1("\\s*,\\s*"));
  QString t=toTool::globalConfig(CONF_EXTENSIONS,DEFAULT_EXTENSIONS);
  t.replace(repl,QString::fromLatin1("\n"));
  return t;
}

static QString AddExt(QString t,const QString &filter)
{
  static QRegExp hasext(QString::fromLatin1("\\.[^\\/]*$"));
  if (t.isEmpty())
    return t;

  toTool::globalSetConfig(CONF_LAST_DIR,t);

  if (hasext.match(t)<0) {
    static QRegExp findext(QString::fromLatin1("\\.[^ \t\r\n\\)\\|]*"));
    int len=0;
    int pos=findext.match(filter,0,&len);
    if (pos>=0)
      t+=filter.mid(pos,len);
    else {
      QFile file(t);
      if (!file.exists())
	t+=QString::fromLatin1(".sql");
    }
  }
  return t;
}

QString toOpenFilename(const QString &filename,const QString &filter,QWidget *parent)
{
  QString t=filter;
  if (t.isEmpty())
    t=GetExtensions();
  
  QString dir=filename;
  if (dir.isNull())
    dir=toTool::globalConfig(CONF_LAST_DIR,"");

#ifdef TO_KDE
  KURL url=TOFileDialog::getOpenURL(dir,t,parent);
  if (url.isEmpty())
    return QString::null;
  if (url.isLocalFile())
    return AddExt(url.path(),t);
  return AddExt(url.url(),t);
#else
  return AddExt(TOFileDialog::getOpenFileName(dir,t,parent),t);
#endif
}

QString toSaveFilename(const QString &filename,const QString &filter,QWidget *parent)
{
  QString t=filter;
  if (t.isEmpty())
    t=GetExtensions();

  QString dir=filename;
  if (dir.isNull())
    dir=toTool::globalConfig(CONF_LAST_DIR,"");

#ifdef TO_KDE
  KURL url=TOFileDialog::getSaveURL(dir,t,parent);
  if (url.hasPass())
    TOMessageBox::warning(toMainWidget(),qApp->translate("toSaveFilename","File open password"),url.pass());
  if (url.isEmpty())
    return QString::null;
  if (url.isLocalFile())
    return AddExt(url.path(),t);
  return AddExt(url.url(),t);
#else
  return AddExt(TOFileDialog::getSaveFileName(dir,t,parent),t);
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

#define COLORS 2

#define TYPES 7

#define STYLES 5

QBrush toChartBrush(int index)
{
  index%=(COLORS*TYPES*STYLES);
  index=COLORS*TYPES*STYLES-1-index;
  int type=index%TYPES;
  int color=(index/TYPES)%COLORS;
  int style=(index/TYPES/COLORS);
  int r=0,g=0,b=0;
  int offset=1;
  switch(type) {
  case 6:
    r=1;
    break;
  case 5:
    g=1;
    break;
  case 4:
    b=1;
    break;
  case 3:
    r=g=1;
    break;
  case 2:
    r=b=1;
    break;
  case 1:
    b=g=1;
    break;
  case 0:
    r=b=g=1;
    offset=0;
    break;
  }

  QBrush::BrushStyle rets;

  switch(style) {
  default:
    rets=QBrush::SolidPattern;
    break;
  case 3:
    rets=QBrush::BDiagPattern;
    break;
  case 2:
    rets=QBrush::FDiagPattern;
    break;
  case 1:
    rets=QBrush::DiagCrossPattern;
    break;
  case 0:
    rets=QBrush::CrossPattern;
    break;
  }

  color+=offset;

  return QBrush(QColor(min(255,r*color*256/(COLORS)),
		       min(255,g*color*256/(COLORS)),
		       min(255,b*color*256/(COLORS))),
		rets);
}

toToolWidget *toCurrentTool(QObject *cur)
{
  while(cur) {
    toToolWidget *tool=dynamic_cast<toToolWidget *>(cur);
    if (tool)
      return tool;
    cur=cur->parent();
  }
  throw qApp->translate("toCurrentTool","Couldn't find parent tool. Internal error.");
}

toConnection &toCurrentConnection(QObject *cur)
{
  while(cur) {
    toConnectionWidget *conn=dynamic_cast<toConnectionWidget *>(cur);
    if (conn)
      return conn->connection();
    cur=cur->parent();
  }
  throw qApp->translate("toCurrentConnection","Couldn't find parent connection. Internal error.");
}

unsigned int toBusy::Count=0;

static toLock BusyLock;

void toBusy::clear()
{
  BusyLock.lock();
  Count=0;
  BusyLock.unlock();
  qApp->restoreOverrideCursor();
}

toBusy::toBusy()
{
  BusyLock.lock();
  if (toThread::mainThread()) {
    if (!Count)
      qApp->setOverrideCursor(Qt::waitCursor);
    Count++;
  }
  BusyLock.unlock();
}

toBusy::~toBusy()
{
  BusyLock.lock();
  if (toThread::mainThread()) {
    if (Count>0)
      Count--;
    if (!Count)
      qApp->restoreOverrideCursor();
  }
  BusyLock.unlock();
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

bool toIsSapDB(const toConnection &conn)
{
  return conn.provider()=="SapDB";
}

bool toIsMySQL(const toConnection &conn)
{
  return conn.provider()=="MySQL";
}

bool toIsPostgreSQL(const toConnection &conn)
{
  return conn.provider()=="PostgreSQL";
}

static QListViewItem *FindItem(QListView *lst,QListViewItem *first,const QString &str)
{
  while(first) {
    QString tmp=first->text(0);
    if (tmp==str)
      return first;
    else {
      tmp+=QString::fromLatin1(":");
      if (str.startsWith(tmp)) {
	QListViewItem *ret=FindItem(lst,first->firstChild(),str.mid(tmp.length()));
	if (ret)
	  return ret;
      }
    }
    first=first->nextSibling();
  }
  return NULL;
}

QListViewItem *toFindItem(QListView *lst,const QString &str)
{
  return FindItem(lst,lst->firstChild(),str);
}

void toToolCaption(toToolWidget *widget,const QString &caption)
{
  QString title;
  if (!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty()) {
    try {
      title=widget->connection().description();
      title+=QString::fromLatin1(" ");
    } catch(...) {
    }
  }
  title+=caption;

  widget->setCaption(title);
  toMainWidget()->windowsMenu();
}

void toMapExport(std::map<QCString,QString> &data,const QCString &prefix,
		 std::map<QCString,QString> &src)
{
  std::map<QCString,QString>::iterator i=src.begin();
  if (i!=src.end()) {
    data[prefix+":First"]=QString::fromLatin1((*i).first);
    QCString key=prefix+":d:";
    do {
      data[key+(*i).first]=(*i).second;
      i++;
    } while(i!=src.end());
  }
}

void toMapImport(std::map<QCString,QString> &data,const QCString &prefix,
		 std::map<QCString,QString> &dst)
{
  dst.clear();
  std::map<QCString,QString>::iterator i=data.find(prefix+":First");
  if (i!=data.end()) {
    QCString key=prefix+":d:";
    i=data.find(key+(*i).second.latin1());
    while(i!=data.end()&&(*i).first.mid(0,key.length())==key) {
      QCString t=(*i).first.mid(key.length());
      if (t.isNull())
	t="";
      dst[t]=(*i).second;
      i++;
    }
  }
#if 0
  for(std::map<QCString,QString>::iterator j=dst.begin();j!=dst.end();j++)
    printf("%s=%s\n",(const char *)((*j).first),(const char *)((*j).second));
#endif
}

bool toCheckModal(QWidget *widget)
{
  QWidget *parent=QApplication::activeModalWidget();
  if (!parent)
    return true;
  while(widget&&widget!=parent)
    widget=widget->parentWidget();
  if (widget==parent)
    return true;
  return false;
}

static bool IndicateEmpty=false;

void toUpdateIndicateEmpty(void)
{
  IndicateEmpty=!toTool::globalConfig(CONF_INDICATE_EMPTY,"").isEmpty();
}

toQValue toNull(const toQValue &str)
{
  if (IndicateEmpty) {
    if (str.isNull())
      return str;
    if (str.toString().length()==0)
      return QString::fromLatin1("''");
  } else if (str.isNull())
    return QString::fromLatin1("{null}");
  return str;
}

toQValue toUnnull(const toQValue &str)
{
  if (IndicateEmpty) {
    if (QString(str)==QString::fromLatin1("''"))
      return QString::fromLatin1("");
  } else if (QString(str)==QString::fromLatin1("{null}"))
    return QString::null;
  return str;
}

QString toTranslateMayby(const QString &ctx,const QString &text)
{
  if (ctx.contains(QString::fromLatin1(" "))||ctx.latin1()!=ctx.utf8()||text.latin1()!=text.utf8()||ctx.isEmpty()||text.isEmpty())
    return text;
#ifdef TODEBUG_TRANSLATION
  static std::map<QString,QString> Context;
  QString t=ctx+QString::fromLatin1(" ")+text;
  if (Context[t].isEmpty()) {
    Context[t]=text;
    printf("QT_TRANSLATE_NOOP(\"%s\",\"%s\"),\n",(const char *)ctx.latin1(),(const char *)text.latin1());
  }
#endif
  return QT_TRANSLATE_NOOP(ctx.latin1(),text.latin1());
}

toPopupButton::toPopupButton(const QIconSet &iconSet,const QString &textLabel,
			     const QString &grouptext,QToolBar *parent,const char *name)
  : QToolButton(iconSet,textLabel,grouptext,NULL,NULL,parent,name)
{
#if QT_VERSION >= 300
  connect(this,SIGNAL(clicked()),this,SLOT(click()));
#endif
  setPopupDelay(0);
}

toPopupButton::toPopupButton(QWidget *parent,const char *name)
  : QToolButton(parent,name)
{
#if QT_VERSION >= 300
  connect(this,SIGNAL(clicked()),this,SLOT(click()));
#endif
  setPopupDelay(0);
}

void toPopupButton::click(void)
{
#if QT_VERSION >= 300
  openPopup();
#endif
}

QString toObfuscate(const QString &str)
{
  if (str.isEmpty())
    return str;

#if QT_VERSION >= 0x030100
  QByteArray arr=qCompress(str.utf8());
  QString ret="\002";
#else
  QByteArray arr=str.utf8();
  QString ret="\001";
#endif
  char buf[100]; // Just to be on the safe side
  for(unsigned int i=0;i<arr.size();i++) {
    sprintf(buf,"%02x",((unsigned int)arr.at(i))%0xff);
    ret+=buf;
  }
  return ret;
}

QString toUnobfuscate(const QString &str)
{
  if (str.isEmpty())
    return str;

  if (str.at(0)!='\001'&&str.at(0)!='\002')
    return str;

  QByteArray arr(int(str.length()/2));
  for(unsigned int i=1;i<str.length();i+=2)
    arr.at(i/2)=str.mid(i,2).toInt(0,16);
  if (str.at(0)=='\002') {
#if QT_VERSION >= 0x030100
    QByteArray ret=qUncompress(arr);
    return QString::fromUtf8(ret);
#else
    toStatusMessage("Tried to unobfuscate obfuscated text from a Qt of version 3.1\n"
		    "from one with Qt version 3.0.x which won't work");
    return QString::null;
#endif
  } else
    return QString::fromUtf8(arr);
}

QKeySequence toKeySequence(const QString &key)
{
  QKeySequence ret=key;
  if (key.isEmpty()&&ret.isEmpty())
    printf("Key sequence %s is not valid\n",(const char *)key);
  return ret;
}

bool toCheckKeyEvent(QKeyEvent *event,const QKeySequence &key)
{
  int state=0;
  if (key.count()!=1)
    return false;
  int val=key[0];
  if ((val&Qt::META)==Qt::META)
    state|=Qt::MetaButton;
  if ((val&Qt::SHIFT)==Qt::SHIFT)
    state|=Qt::ShiftButton;
  if ((val&Qt::CTRL)==Qt::CTRL)
    state|=Qt::ControlButton;
  if ((val&Qt::ALT)==Qt::ALT)
    state|=Qt::AltButton;
  
  val&=0xfffff;

  return (event->state()==state&&event->key()==val);
}
