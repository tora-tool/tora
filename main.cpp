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


#include <stdio.h>

#include <qapplication.h>
#include <qstatusbar.h>
#include <qcombobox.h>
#include <qtimer.h>

#include "tonewconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "toabout.h"

toMain *mainWidget;

#define CHUNK_SIZE 63

QString toSQLString(toConnection &conn,const QString &address)
{
  QString sql;
  otl_stream q(1,
	       "SELECT SQL_Text"
	       "  FROM V$SQLText_With_Newlines"
	       " WHERE Address||':'||Hash_Value = :f1<char[100]>"
	       " ORDER BY Piece",
	       conn.connection());

  q<<(const char *)address;

  while(!q.eof()) {
    char buffer[100];
    q>>buffer;
    sql.append(buffer);
  }
  if (sql.isEmpty())
    throw QString("SQL Address not found in SGA");
  return sql;
}

QString toNow(toConnection &conn)
{
  otl_stream q(1,
	       "SELECT TO_CHAR(SYSDATE) FROM DUAL",
	       conn.connection());
  char buffer[1024];
  q>>buffer;
  return buffer;
}

QString toSQLToAddress(toConnection &conn,const QString &sql)
{
  QString search;

  search.sprintf("SELECT Address||':'||Hash_Value"
		 "  FROM V$SQLText_With_Newlines"
		 " WHERE SQL_Text LIKE :f1<char[%d]>||'%%'",
		 CHUNK_SIZE+1);

  otl_stream q(1,(const char *)search,conn.connection());

  q<<(const char *)sql.left(CHUNK_SIZE);

  while(!q.eof()) {
    char buf[100];
    q>>buf;
    if (sql==toSQLString(conn,buf))
      return buf;
  }
  throw QString("SQL Query not found in SGA");
}

void toStatusMessage(const QString &str)
{
  if (mainWidget)
    mainWidget->statusBar()->message(str);
}

toMain *toMainWidget(void)
{
  return mainWidget;
}

QComboBox *toRefreshCreate(QWidget *parent,const char *name,const char *def)
{
  QComboBox *refresh=new QComboBox(false,parent);

  refresh->insertItem("None");
  refresh->insertItem("10 seconds");
  refresh->insertItem("30 seconds");
  refresh->insertItem("1 min");
  refresh->insertItem("10 min");
  QString str;
  if (def)
    str=def;
  else
    str=toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH);
  if (str=="10 seconds")
    refresh->setCurrentItem(1);
  else if (str=="30 seconds")
    refresh->setCurrentItem(2);
  else if (str=="1 min")
    refresh->setCurrentItem(3);
  else if (str=="10 min")
    refresh->setCurrentItem(4);
  else
    refresh->setCurrentItem(0);
  return refresh;
}

void toRefreshParse(QTimer *timer,const QString &str)
{
  if (str=="None")
    timer->stop();
  else if (str=="10 seconds")
    timer->start(10*1000);
  else if (str=="30 seconds")
    timer->start(30*1000);
  else if (str=="1 min")
    timer->start(60*1000);
  else if (str=="10 min")
    timer->start(600*1000);
  else
    throw QString("Unknown timer value");
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
  if (dynamic_cast<QMotifPlusStyle *>(style))
    return "Motif Plus";
  else if (dynamic_cast<QSGIStyle *>(style))
    return "SGI";
  else if (dynamic_cast<QCDEStyle *>(style))
    return "CDE";
  else if (dynamic_cast<QMotifStyle *>(style))
    return "Motif";
  else if (dynamic_cast<QPlatinumStyle *>(style))
    return "Platinum";
  else if (dynamic_cast<QWindowsStyle *>(style))
    return "Windows";
  throw QString("Failed to find style match");
}
#endif

int main(int argc,char **argv)
{
  try {
    QApplication mainApp(argc,argv);

#ifdef ENABLE_STYLE
    QString style=toTool::globalConfig(CONF_STYLE,"");
    if (!style.isEmpty())
      toSetSessionType(style);
#endif

    otl_connect::otl_initialize(0);

    if (toTool::globalConfig("LastVersion","")!=TOVERSION) {
      toAbout *about=new toAbout(NULL,"About TOra",true);
      if (!about->exec()) {
	exit (2);
      }
      delete about;
      toTool::globalSetConfig("LastVersion",TOVERSION);
    }

    if (argc>2||(argc==2&&argv[1][0]=='-')) {
      printf("Usage:\n\n  tora [{X options}] [connectstring]\n\n");
      exit(2);
    } else if (argc==2) {
      QString connect=argv[1];
      QString user;
      int pos=connect.find("@");
      if (pos>-1) {
	user=connect.left(pos);
	connect=connect.right(connect.length()-pos-1);
      } else {
	user=connect;
	if (getenv("ORACLE_SID"))
	  connect=getenv("ORACLE_SID");
      }
      if (!connect.isEmpty())
	toTool::globalSetConfig(CONF_DATABASE,connect);
      pos=user.find("/");
      if (pos>-1) {
	toTool::globalSetConfig(CONF_PASSWORD,user.right(user.length()-pos-1));
	user=user.left(pos);
      }
      if (!user.isEmpty())
	toTool::globalSetConfig(CONF_USER,user);
    }

    mainWidget=new toMain;

    mainApp.setMainWidget(mainWidget);
    mainWidget->createDefault();
    
    return mainApp.exec();
  } catch (const otl_exception &exc) {
    printf("Unhandled exception:\n%s\n",exc.msg);
  } catch (const QString &str) {
    printf("Unhandled exception:\n%s\n",(const char *)str);
  } catch (...) {
    printf("Unhandled exception:\nUnknown type\n");
  }
}
