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


#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtooltip.h>
#include <qworkspace.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtimer.h>
#include <qstring.h>

#include <map>

#include "totool.h"
#include "tomarkedtext.h"
#include "tomain.h"

#include "tooutput.moc"

#include "icons/refresh.xpm"
#include "icons/tooutput.xpm"
#include "icons/eraselog.xpm"

#define CONF_POLLING    "Refresh"
#define DEFAULT_POLLING	"10 seconds"

class toOutputPrefs : public QFrame, public toSettingTab
{ 
  QComboBox* AutoPolling;
  toTool *Tool;

public:
  toOutputPrefs(toTool *tool,QWidget* parent = 0,const char* name = 0)
    : QFrame(parent,name),Tool(tool)
  {
    QGroupBox *GroupBox1 = new QGroupBox( this, "GroupBox1" );
    GroupBox1->setGeometry( QRect( 10, 10, 380, 380 ) ); 
    GroupBox1->setTitle( tr( "SQL Output"  ) );
    
    QLabel *label=new QLabel("&Polling timeout",GroupBox1);
    label->setGeometry(QRect(20,30,100,20));
    QToolTip::add( label, tr( "Time between trying to poll for output." ) );

    AutoPolling=toRefreshCreate(GroupBox1,NULL,Tool->config(CONF_POLLING,DEFAULT_POLLING));
    AutoPolling->setGeometry( QRect( 130,30,220,20));
    label->setBuddy(AutoPolling);
  }
  virtual void saveSetting(void)
  {
    Tool->setConfig(CONF_POLLING,AutoPolling->currentText());
  }
};

class toOutputTool : public toTool {
protected:
  map<toConnection *,QWidget *> Windows;

  virtual char **pictureXPM(void)
  { return tooutput_xpm; }
public:
  toOutputTool()
    : toTool(103,"SQL Output")
  { }
  virtual QWidget *toolWindow(toMain *main,toConnection &connection)
  {
    map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end()) {
      (*i).second->setFocus();
      return NULL;
    } else {
      QWidget *window=new toOutput(main,connection);
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
  virtual QWidget *configurationTab(QWidget *parent)
  { return new toOutputPrefs(this,parent); }
};

static toOutputTool OutputTool;

static QPixmap *toRefreshPixmap;
static QPixmap *toEraseLogPixmap;

toOutput::toOutput(toMain *main,toConnection &connection)
  : QVBox(main->workspace(),NULL,WDestructiveClose),Connection(connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toEraseLogPixmap)
    toEraseLogPixmap=new QPixmap((const char **)eraselog_xpm);

  QToolBar *toolbar=new QToolBar("SQL Output",main,this);
  new QToolButton(*toRefreshPixmap,
		  "Poll for output now",
		  "Poll for output now",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(*toEraseLogPixmap,
		  "Clear output",	
		  "Clear output",
		  this,SLOT(clear()),
		  toolbar);
  toolbar->addSeparator();
  new QLabel("Refresh",toolbar);
  connect(toRefreshCreate(toolbar,NULL,OutputTool.config(CONF_POLLING,DEFAULT_POLLING)),
	  SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));
  toolbar->setStretchableWidget(new QLabel("",toolbar));

  Output=new toMarkedText(this);

  Timer=new QTimer(this);
  connect(Timer,SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  toRefreshParse(Timer,OutputTool.config(CONF_POLLING,DEFAULT_POLLING));
  try {
    otl_cursor::direct_exec(Connection.connection(),
			    "BEGIN\n"
			    "    DBMS_OUTPUT.ENABLE;"
			    "END;");
  } catch (...) {
    toStatusMessage("Couldn't enable output for session");
  }
  Connection.addWidget(this);
}

toOutput::~toOutput()
{
  try {
    otl_cursor::direct_exec(Connection.connection(),
			    "BEGIN\n"
			    "    DBMS_OUTPUT.DISABLE;"
			    "END;");
  } catch (...) {
    toStatusMessage("Couldn't disable output for session");
  }
  OutputTool.closeWindow(Connection);
  Connection.delWidget(this);
}

void toOutput::refresh(void)
{
  try {
    int numlines;
    do {
      otl_stream query(1,
		       "BEGIN"
		       "    DBMS_OUTPUT.GET_LINES(:lines<char[256],out[255]>,"
		       "                          :numlines<int,inout>);"
		       "END;",
		       Connection.connection());
      query<<254;
      otl_cstr_tab<257,255> lines;
      query>>lines;
      query>>numlines;
      for (int i=0;i<numlines;i++)
	Output->insertLine((const char *)lines.v[i]);
    } while(numlines>=254);
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
  }
}

void toOutput::clear(void)
{
  Output->clear();
}

void toOutput::changeRefresh(const QString &str)
{ 
  toRefreshParse(Timer,str);
}

