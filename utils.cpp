#include <qapplication.h>
#include <qstatusbar.h>
#include <qcombobox.h>
#include <qtimer.h>
#include <qtooltip.h>

#include "tohighlightedtext.h"
#include "tonewconnection.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"

#define CHUNK_SIZE 63

QString toReadValue(const otl_column_desc &dsc,otl_stream &q,int maxSize)
{
  switch (dsc.otl_var_dbtype) {
  default:  // Try using char if all else fails
    {
      char buffer[dsc.dbsize*2+1]; // The *2 is for raw columns
      q>>buffer;
      if (q.is_null())
	return "{null}";
      return buffer;
    }
    break;
  case otl_var_varchar_long:
  case otl_var_raw_long:
  case otl_var_clob:
  case otl_var_blob:
    {
      char buffer[maxSize+1];
      otl_long_string data(buffer,maxSize);
      q>>data;
      if (q.is_null())
	return "{null}";
      buffer[data.len()]=0; // Not sure if this is needed
      return buffer;
    }
    break;
  }
}

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
  toMain *main=dynamic_cast<toMain *>(qApp->mainWidget());
  if (main) {
    main->statusBar()->message(str);
    QToolTip::add(main->statusBar(),str);
  }
}

toMain *toMainWidget(void)
{
  return dynamic_cast<toMain *>(qApp->mainWidget());
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

