//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "toconnection.h"
#include "tomain.h"
#include "tomarkedtext.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#include <map>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qworkspace.h>

#include "tooutput.moc"

#include "icons/eraselog.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#include "icons/refresh.xpm"
#include "icons/tooutput.xpm"

#define CONF_POLLING    "Refresh"
#define DEFAULT_POLLING	"10 seconds"

#define CONF_TYPE	"Type"
#define DEFAULT_TYPE	"0"

#define CONF_LOG_USER	"LogUser"
#define DEFAULT_LOG_USER "ULOG"

class toOutputPrefs : public QGroupBox, public toSettingTab
{ 
  QComboBox *AutoPolling;
  QComboBox *Type;
  QLineEdit *User;
  toTool *Tool;

public:
  toOutputPrefs(toTool *tool,QWidget* parent = 0,const char* name = 0)
    : QGroupBox(1,Horizontal,parent,name),toSettingTab("output.html"),Tool(tool)
  {
    setTitle(qApp->translate("toOutputPrefs","SQL Output"));

    QLabel *label=new QLabel(qApp->translate("toOutputPrefs","&Polling timeout"),this);
    label->setGeometry(QRect(20,30,100,20));
    QToolTip::add(label,qApp->translate("toOutputPrefs","Time between trying to poll for output."));

    AutoPolling=toRefreshCreate(this,NULL,Tool->config(CONF_POLLING,DEFAULT_POLLING));
    label->setBuddy(AutoPolling);

    label=new QLabel(qApp->translate("toOutputPrefs","Default &source"),this);
    Type=new QComboBox(this);
    Type->insertItem(qApp->translate("toLogOutput","SQL Output"));
    Type->insertItem(qApp->translate("toLogOutput","Log4PL/SQL"));
    Type->setCurrentItem(Tool->config(CONF_TYPE,DEFAULT_TYPE).toInt());
    label->setBuddy(Type);

    label=new QLabel(qApp->translate("toOutputPrefs","Log4PL/SQL &User"),this);
    User=new QLineEdit(Tool->config(CONF_LOG_USER,DEFAULT_LOG_USER),this);
    label->setBuddy(User);
  }
  virtual void saveSetting(void)
  {
    Tool->setConfig(CONF_POLLING,AutoPolling->currentText());
    Tool->setConfig(CONF_TYPE,QString::number(Type->currentItem()));
    Tool->setConfig(CONF_LOG_USER,User->text());
  }
};

class toOutputTool : public toTool {
protected:
  std::map<toConnection *,QWidget *> Windows;

  virtual char **pictureXPM(void)
  { return tooutput_xpm; }
public:
  toOutputTool()
    : toTool(340,"SQL Output")
  { }
  virtual const char *menuItem()
  { return "SQL Output"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    std::map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end()) {
      (*i).second->setFocus();
      return NULL;
    } else {
      QWidget *window=new toLogOutput(parent,connection);
      Windows[&connection]=window;
      return window;
    }
  }
  void closeWindow(toConnection &connection)
  {
    std::map<toConnection *,QWidget *>::iterator i=Windows.find(&connection);
    if (i!=Windows.end())
      Windows.erase(i);
  }
  virtual QWidget *configurationTab(QWidget *parent)
  { return new toOutputPrefs(this,parent); }
};

static toOutputTool OutputTool;

toOutput::toOutput(QWidget *main,toConnection &connection,bool enabled)
  : toToolWidget(OutputTool,"output.html",main,connection)
{
  ToolBar=toAllocBar(this,tr("SQL Output"));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Poll for output now"),
		  tr("Poll for output now"),
		  this,SLOT(refresh(void)),
		  ToolBar);
  ToolBar->addSeparator();
  DisableButton=new QToolButton(ToolBar);
  DisableButton->setToggleButton(true);
#if QT_VERSION >= 300
  QIconSet iconset;
  iconset.setPixmap(QPixmap((const char **)online_xpm),QIconSet::Automatic,QIconSet::Normal,QIconSet::Off);
  iconset.setPixmap(QPixmap((const char **)offline_xpm),QIconSet::Automatic,QIconSet::Normal,QIconSet::On);
  DisableButton->setIconSet(iconset);
#else
  DisableButton->setIconSet(QIconSet(QPixmap((const char **)online_xpm)),false);
  DisableButton->setIconSet(QIconSet(QPixmap((const char **)offline_xpm)),true);
#endif
  DisableButton->setOn(!enabled);
  connect(DisableButton,SIGNAL(toggled(bool)),this,SLOT(disable(bool)));
  QToolTip::add(DisableButton,tr("Enable or disable getting SQL output."));

  new QToolButton(QPixmap((const char **)eraselog_xpm),
		  tr("Clear output"),
		  tr("Clear output"),
		  this,SLOT(clear()),
		  ToolBar);
  ToolBar->addSeparator();
  new QLabel(tr("Refresh")+" ",ToolBar);
  connect(Refresh=toRefreshCreate(ToolBar,NULL,OutputTool.config(CONF_POLLING,DEFAULT_POLLING)),
	  SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));
  ToolBar->setStretchableWidget(new QLabel(QString::null,ToolBar));

  Output=new toMarkedText(this);

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  try {
    connect(timer(),SIGNAL(timeout(void)),this,SLOT(refresh(void)));
    toRefreshParse(timer(),OutputTool.config(CONF_POLLING,DEFAULT_POLLING));
  } TOCATCH
  if (enabled)
    disable(false);

  setFocusProxy(Output);
}

#define TO_ID_TOGGLE		(toMain::TO_TOOL_MENU_ID+ 0)

void toOutput::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),
			   this,SLOT(refresh(void)),Key_F5);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(tr("Output enabled"),
			   this,SLOT(toggleMenu()),Key_F4,TO_ID_TOGGLE);
      ToolMenu->insertItem(QPixmap((const char **)eraselog_xpm),tr("Clear output"),
			   this,SLOT(clear()),CTRL+Key_Backspace);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(tr("&Change Refresh"),Refresh,SLOT(setFocus(void)),
			   Key_R+ALT);

      toMainWidget()->menuBar()->insertItem(tr("&Output"),ToolMenu,-1,toToolMenuIndex());
      ToolMenu->setItemChecked(TO_ID_TOGGLE,!DisableButton->isOn());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

static toSQL SQLEnable("toOutput:Enable",
		       "BEGIN\n"
		       "    SYS.DBMS_OUTPUT.ENABLE;\n"
		       "END;",
		       "Enable output collection");
static toSQL SQLDisable("toOutput:Disable",
			"BEGIN\n"
			"    SYS.DBMS_OUTPUT.DISABLE;\n"
			"END;",
			"Disable output collection");

void toOutput::toggleMenu()
{
  DisableButton->setOn(!DisableButton->isOn());
}

void toOutput::disable(bool dis)
{
  try {
    if (dis)
      connection().allExecute(SQLDisable);
    else
      connection().allExecute(SQLEnable);
    QString str=toSQL::string(SQLEnable,connection());
    if (dis)
      connection().delInit(str);
    else
      connection().addInit(str);
    if (ToolMenu)
      ToolMenu->setItemChecked(TO_ID_TOGGLE,!DisableButton->isOn());
  } catch (...) {
    toStatusMessage(tr("Couldn't enable/disable output for session"));
  }
}

toOutput::~toOutput()
{
  try {
    disable(true);
    OutputTool.closeWindow(connection());
  } TOCATCH
}

static toSQL SQLLines("toOutput:Poll",
		      "BEGIN\n"
		      "    SYS.DBMS_OUTPUT.GET_LINE(:lines<char[1000],out>,\n"
		      "                             :stat<int,out>);\n"
		      "END;",
		      "Get lines from SQL Output, must use same bindings");

void toOutput::poll()
{
  try {
    bool any;
    do {
      toQList params;
      toQuery query(connection(),toQuery::All,SQLLines,params);

      any=false;
      while(!query.eof()) {
	QString line=query.readValueNull();
	int status=query.readValueNull().toInt();
	if (status==0) {
	  any=true;
	  insertLine(line);
	}
      }
    } while(any);
  } TOCATCH
}

void toOutput::refresh(void)
{
  poll();
}

void toOutput::clear(void)
{
  Output->clear();
}

void toOutput::changeRefresh(const QString &str)
{ 
  try {
    toRefreshParse(timer(),str);
  } TOCATCH
}

bool toOutput::enabled(void)
{
  return !DisableButton->isOn();
}

static toSQL SQLLog("toLogOutput:Poll",
		    "SELECT LDATE||'.'||to_char(mod(LHSECS,100),'09') \"Timestamp\",\n"
		    "       decode(llevel,1,'OFF',\n"
		    "                     2,'FATAL',\n"
		    "                     3,'ERROR',\n"
		    "                     4,'WARNING',\n"
		    "                     5,'INFO',\n"
		    "                     6,'DEBUG',\n"
		    "                     7,'ALL' ,\n"
		    "                     'UNDEFINED') \"Level\",\n"
		    "       LUSER \"User\",\n"
		    "       LSECTION \"Section\",\n"
		    "       LTEXTE \"Text\"\n"
		    "  from %1.tlog order by id desc\n",
		    "Poll data from PL/SQL log table");

toLogOutput::toLogOutput(QWidget *parent,toConnection &connection)
  : toOutput(parent,connection)
{
  Type=new QComboBox(toolBar());
  Type->insertItem(tr("SQL Output"));
  Type->insertItem(tr("Log4PL/SQL"));
  Type->setCurrentItem(OutputTool.config(CONF_TYPE,DEFAULT_TYPE).toInt());
  connect(Type,SIGNAL(activated(int)),this,SLOT(changeType()));

  Log=new toResultView(false,false,this);
  changeType();
}

void toLogOutput::refresh(void)
{
  if (Type->currentItem()==1) {
    Log->setSQL(QString::null);
    Log->query(SQLLog(connection()).arg(OutputTool.config(CONF_LOG_USER,DEFAULT_LOG_USER)));
  }
  toOutput::refresh();
}

void toLogOutput::changeType(void)
{
  if (Type->currentItem()==1) {
    output()->hide();
    Log->show();
    refresh();
  } else {
    output()->show();
    Log->hide();
  }
}
