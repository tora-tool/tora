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

#include "tobarchart.h"
#include "toconf.h"
#include "toconnection.h"
#include "tolinechart.h"
#include "tomain.h"
#include "toresultview.h"
#include "tosmtp.h"
#include "totool.h"

#include <ctype.h>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvalidator.h>
#include <qworkspace.h>

#include "tochartalarmui.moc"
#include "tochartmanager.moc"
#include "tochartsetupui.moc"

#include "icons/grid.xpm"
#include "icons/fileopen.xpm"
#include "icons/refresh.xpm"

class toChartTool : public toTool {
protected:
  toChartManager *Window;

public:
  toChartTool()
    : toTool(930,"Chart Manager")
  { Window=NULL; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    if (!Window)
      return NULL;
    Window->refresh();
    Window->show();
    Window->setFocus();
    return Window;
  }
  virtual void customSetup(int toolid)
  {
    toMainWidget()->editMenu()->insertItem(QPixmap((const char **)grid_xpm),
					   qApp->translate("toChartTool","Chart Manager..."),toolid);

    Window=new toChartManager(toMainWidget()->workspace());
    Window->setCaption(qApp->translate("toChartTool","Chart Manager"));
    Window->setIcon(QPixmap((const char **)grid_xpm));
    Window->close();
  }
  void closeWindow(void)
  {
    Window=NULL;
  }
  virtual bool canHandle(toConnection &conn)
  { return true; }
};

static toChartTool ChartTool;

class toChartAlarm : public toChartAlarmUI {
public:
  toChartAlarm(QWidget *parent=0,const char *name=0,bool modal=FALSE,WFlags fl=0)
    : toChartAlarmUI(parent,name,modal,fl)
  { }
  virtual void changeValue(int val)
  { 
    if (val==1)
      ExtraLabel->setText(qApp->translate("toChartAlarm","Email"));
    else
      ExtraLabel->setText(QString::null);

    Extra->setEnabled(val==1);
  }
};

class toChartSetup : public toChartSetupUI {
  toLineChart *Chart;
public:
  toChartSetup(toLineChart *chart,QWidget* parent,const char* name,bool modal)
    : toChartSetupUI(parent,name,modal)
  { Chart=chart; }

  virtual void browseFile()
  {
    QString str=toOpenFilename(Filename->text(),QString::fromLatin1("*.csv"),this);
    if (!str.isEmpty())
      Filename->setText(str);
  }
  virtual QString modifyAlarm(const QString &str,bool &persistent)
  {
    toChartManager::chartAlarm alarm(str,persistent);
    toChartAlarm diag(this,NULL,true);
    diag.Operation->setCurrentItem((int)alarm.Operation);
    diag.Comparison->setCurrentItem((int)alarm.Comparison);
    diag.Action->setCurrentItem((int)alarm.Action);
    diag.Value->setText(QString::number(alarm.Value));
    diag.changeValue((int)alarm.Action);
    diag.Value->setValidator(new QDoubleValidator(diag.Value));
    diag.Extra->setText(alarm.Extra);
    std::list<int>::iterator sel=alarm.Columns.begin();
    diag.Charts->addColumn(qApp->translate("toChartSetup","Charts"));
    diag.Charts->setSelectionMode(QListView::Multi);
    QListViewItem *item=NULL;
    std::list<QString>::iterator lab=Chart->labels().begin();
    for(int i=0;lab!=Chart->labels().end();i++,lab++) {
      item=new QListViewItem(diag.Charts,item,*lab);
      if (sel!=alarm.Columns.end()&&*sel==i) {
	item->setSelected(true);
	sel++;
      }
    }
    diag.Persistent->setChecked(persistent);
    if (diag.exec()) {
      persistent=diag.Persistent->isChecked();
      int i=0;
      alarm.Columns.clear();
      for(QListViewItem *item=diag.Charts->firstChild();item;item=item->nextSibling()) {
	if (item->isSelected())
	  alarm.Columns.insert(alarm.Columns.end(),i);
	i++;
      }
      alarm.Action=(toChartManager::action)diag.Action->currentItem();
      alarm.Operation=(toChartManager::chartAlarm::operation)diag.Operation->currentItem();
      alarm.Comparison=(toChartManager::chartAlarm::comparison)diag.Comparison->currentItem();
      alarm.Value=diag.Value->text().toDouble();
      alarm.Extra=diag.Extra->text();
      return alarm.toString();
    }
    return QString::null;
  }
  virtual void addAlarm()
  {
    toChartManager::chartAlarm alarm;
    QString str=modifyAlarm(alarm.toString(),alarm.Persistent);
    if (!str.isEmpty())
      new QListViewItem(Alarms,str,alarm.Persistent?qApp->translate("toChartSetup","Persistent"):qApp->translate("toChartSetup","Temporary"));
  }
  virtual void modifyAlarm()
  {
    QListViewItem *item=Alarms->selectedItem();
    if (item) {
      bool pers=item->text(1)==qApp->translate("toChartSetup","Persistent");
      QString str=modifyAlarm(item->text(0),pers);
      if (!str.isEmpty()) {
	item->setText(0,str);
	item->setText(1,pers?qApp->translate("toChartSetup","Persistent"):qApp->translate("toChartSetup","Temporary"));
      }
    }
  }
  virtual void removeAlarm()
  {
    delete Alarms->selectedItem();
  }
};

toChartReceiver::toChartReceiver(toChartManager *parent,toLineChart *chart)
  : QObject(parent)
{
  Parent=parent;
  Chart=chart;
  Result=NULL;
  connect(Chart,SIGNAL(valueAdded(std::list<double> &,const QString &)),
	  this,SLOT(valueAdded(std::list<double> &,const QString &)));
}

toResult *toChartReceiver::result(void)
{
  if (!Result)
    Result=dynamic_cast<toResult *>(Chart);
  return Result;
}

QString toChartReceiver::name(void)
{
  if (!Result||Result->sqlName().isEmpty())
    return QString::null;
  try {
    LastName=Result->connection().description(false)+QString::fromLatin1(":")+QString::fromLatin1(Result->sqlName());
  } catch(...) {
  }
  return LastName;
}

void toChartReceiver::valueAdded(std::list<double> &value,const QString &xValues)
{
  Parent->valueAdded(Chart,name(),value,xValues);
}

toChartManager::toChartManager(QWidget *main)
  : QVBox(main),toHelpContext(QString::fromLatin1("chartmanager.html"))
{
  loadSettings();

  connect(toMainWidget(),SIGNAL(chartAdded(toLineChart *)),
	  this,SLOT(addChart(toLineChart *)));
  connect(toMainWidget(),SIGNAL(chartRemoved(toLineChart *)),
	  this,SLOT(removeChart(toLineChart *)));
  connect(toMainWidget(),SIGNAL(chartSetup(toLineChart *)),
	  this,SLOT(setupChart(toLineChart *)));

  QToolBar *toolbar=toAllocBar(this,tr("Chart Manager"));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Refresh list"),
		  tr("Refresh list"),
		  this,SLOT(refresh()),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)fileopen_xpm),
		  tr("Open tracker file"),
		  tr("Open tracker file"),
		  this,SLOT(openChart()),
		  toolbar);
  new QToolButton(QPixmap((const char **)grid_xpm),
		  tr("Setup chart"),
		  tr("Setup chart"),
		  this,SLOT(setupChart()),
		  toolbar);

  toolbar->setStretchableWidget(new QLabel(QString::null,toolbar));

  List=new toListView(this);
  List->addColumn(tr("Connection"));
  List->addColumn(tr("Title"));
  List->addColumn(tr("ID"));
  List->addColumn(tr("Tracking"));
  List->addColumn(tr("Alarms"));
  List->setSorting(2);
  List->setSelectionMode(QListView::Single);

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));
  connect(&Refresh,SIGNAL(timeout()),this,SLOT(refresh()));
  connect(&Timer,SIGNAL(timeout()),this,SLOT(alarm()));

  setFocusProxy(List);
}

void toChartManager::windowActivated(QWidget *widget)
{
  if (widget==this&&!isHidden()) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),
			   this,SLOT(refresh(void)),Key_F5);

      toMainWidget()->menuBar()->insertItem(tr("&Chart Manager"),ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

toChartManager::~toChartManager()
{
  ChartTool.closeWindow();
  Charts.clear();
}

void toChartManager::addChart(toLineChart *chart)
{
  Charts.insert(Charts.end(),new toChartReceiver(this,chart));
  Refresh.start(1,true);
}

void toChartManager::removeChart(toLineChart *chart)
{
  for(std::list<toChartReceiver *>::iterator i=Charts.begin();i!=Charts.end();i++) {
    if ((*i)->chart()==chart) {
      QString name=(*i)->name();
      if (!name.isNull()) {
	std::map<QString,std::list<chartAlarm> >::iterator fnda=Alarms.find(name);
	if (fnda!=Alarms.end()) {
	  bool any=false;
	  for(std::list<chartAlarm>::iterator j=(*fnda).second.begin();j!=(*fnda).second.end();) {
	    if (!(*j).Persistent) {
	      (*fnda).second.erase(j);
	      j=(*fnda).second.begin();
	    } else {
	      j++;
	      any=true;
	    }
	  }
	  if (!any)
	    Alarms.erase(fnda);
	}

	std::map<QString,chartTrack>::iterator fndt=Files.find(name);
	if (fndt!=Files.end()) {
	  if (!(*fndt).second.Persistent)
	    Files.erase(fndt);
	}
      }

      delete *i;
      Charts.erase(i);
      Refresh.start(1,true);
      return;
    }
  }
}

toChartManager::alarmSignal::alarmSignal(void)
{
  Action=Ignore;
}

toChartManager::chartAlarm::chartAlarm(const QString &inp,bool pers)
{
  char oper[100];
  char cols[10000];
  char comp[100];
  char act[100];
  char extra[10000];
  Signal=false;
  Persistent=pers;

  // The \01 is just one character unlikely to be in an email address
  int ret=sscanf(inp.utf8(),"%s %s %s %lf %s %[^\01]",oper,cols,comp,&Value,act,extra);
  if (ret!=5&&ret!=6) {
    Operation=Any;
    Value=0;
    Comparison=Equal;
    Action=Ignore;
    return;
  }

  QString t=QString::fromLatin1(oper);
  if(t==QString::fromLatin1("min"))
    Operation=Min;
  else if (t==QString::fromLatin1("all"))
    Operation=All;
  else if (t==QString::fromLatin1("sum"))
    Operation=Sum;
  else if (t==QString::fromLatin1("average"))
    Operation=Average;
  else if (t==QString::fromLatin1("max"))
    Operation=Max;
  else
    Operation=Any;

  t=QString::fromUtf8(cols);
  if (t.length()>2) {
    QStringList lst=QStringList::split(QString::fromLatin1(","),t.mid(1,t.length()-2));
    for(unsigned int i=0;i<lst.count();i++)
      Columns.insert(Columns.end(),lst[i].toInt());
    Columns.sort();
  }

  t=QString::fromLatin1(comp);
  if (t==QString::fromLatin1("=")) {
    Comparison=Equal;
  } else if (t==QString::fromLatin1("!=")) {
    Comparison=NotEqual;
  } else if (t==QString::fromLatin1("<")) {
    Comparison=Less;
  } else if (t==QString::fromLatin1(">")) {
    Comparison=Greater;
  } else if (t==QString::fromLatin1("<=")) {
    Comparison=LessEqual;
  } else if (t==QString::fromLatin1(">=")) {
    Comparison=GreaterEqual;
  } else {
    Comparison=Equal;
  }

  t=QString::fromLatin1(act);
  if (t==QString::fromLatin1("StatusMessage"))
    Action=StatusMessage;
  else if (t==QString::fromLatin1("Email"))
    Action=Email;
  else
    Action=Ignore;
  if (ret==6)
    Extra=QString::fromUtf8(extra);
}

toChartManager::chartAlarm::chartAlarm()
{
  Signal=false;
  Operation=Any;
  Comparison=Equal;
  Action=StatusMessage;
  Value=0;
  Persistent=true;
}

toChartManager::chartAlarm::chartAlarm(operation oper,comparison comp,action act,
				       double value,std::list<int> &cols,const QString &extra,
				       bool persistent)
{
  Signal=false;
  Operation=oper;
  Comparison=comp;
  Action=act;
  Value=value;
  Columns=cols;
  Persistent=persistent;
  Extra=extra;
}

QString toChartManager::chartAlarm::toString(void)
{
  QString t;
  switch (Operation) {
  case Any:
    t=QString::fromLatin1("any");
    break;
  case All:
    t=QString::fromLatin1("all");
    break;
  case Sum:
    t=QString::fromLatin1("sum");
    break;
  case Average:
    t=QString::fromLatin1("average");
    break;
  case Max:
    t=QString::fromLatin1("max");
    break;
  case Min:
    t=QString::fromLatin1("min");
    break;
  }
  t+=QString::fromLatin1(" (");
  bool first=true;
  for(std::list<int>::iterator i=Columns.begin();i!=Columns.end();i++) {
    if (first)
      first=false;
    else
      t+=QString::fromLatin1(",");
    t+=QString::number(*i);
  }
  t+=QString::fromLatin1(")");
  switch (Comparison) {
  case Equal:
    t+=QString::fromLatin1(" = ");
    break;
  case NotEqual:
    t+=QString::fromLatin1(" != ");
    break;
  case Less:
    t+=QString::fromLatin1(" < ");
    break;
  case Greater:
    t+=QString::fromLatin1(" > ");
    break;
  case LessEqual:
    t+=QString::fromLatin1(" <= ");
    break;
  case GreaterEqual:
    t+=QString::fromLatin1(" >= ");
    break;
  }
  t+=QString::number(Value);
  switch (Action) {
  case StatusMessage:
    t+=QString::fromLatin1(" StatusMessage");
    break;
  case Email:
    t+=QString::fromLatin1(" Email");
    break;
  case Ignore:
    t+=QString::fromLatin1(" Ignore");
    break;
  }
  if (!Extra.isEmpty()) {
    t+=QString::fromLatin1(" ");
    t+=Extra;
  }
  return t;
}

bool toChartManager::chartAlarm::checkValue(double val)
{
  switch(Comparison) {
  case Equal:
    return val==Value;
  case NotEqual:
    return val!=Value;
  case Less:
    return Value<val;
  case Greater:
    return Value>val;
  case LessEqual:
    return Value<=val;
  case GreaterEqual:
    return Value>=val;
  }
  return false;
}

void toChartManager::chartAlarm::valueAdded(toChartManager *manager,
					    const QString &str,
					    std::list<double> &value,
					    const QString &xValue)
{
  std::list<double> vals;
  std::list<double>::iterator i=value.begin();
  std::list<int>::iterator j=Columns.begin();
  for(int id=0;j!=Columns.end()&&i!=value.end();i++,id++) {
    if (id==*j) {
      vals.insert(vals.end(),*i);
      j++;
    }
  }
  bool sig=false;
  switch(Operation) {
  case Any:
    {
      for(std::list<double>::iterator i=vals.begin();i!=vals.end();i++)
	if (checkValue(*i)) {
	  sig=true;
	  break;
	}
    }
    break;
  case All:
    sig=true;
    {
      for(std::list<double>::iterator i=vals.begin();i!=vals.end();i++)
	if (!checkValue(*i)) {
	  sig=false;
	  break;
	}
    }
    break;
  case Sum:
    {
      double val=0;
      for(std::list<double>::iterator i=vals.begin();i!=vals.end();i++)
	val+=*i;
      sig=checkValue(val);
    }
    break;
  case Average:
    {
      double val=0;
      for(std::list<double>::iterator i=vals.begin();i!=vals.end();i++)
	val+=*i;
      sig=checkValue(val/vals.size());
    }
    break;
  case Max:
    {
      bool first=true;
      double val=0;
      for(std::list<double>::iterator i=vals.begin();i!=vals.end();i++) {
	if (first) {
	  first=false;
	  val=*i;
	} else
	  val=max(*i,val);
      }
      sig=checkValue(val);
    }
    break;
  case Min:
    {
      bool first=true;
      double val=0;
      for(std::list<double>::iterator i=vals.begin();i!=vals.end();i++) {
	if (first) {
	  first=false;
	  val=*i;
	} else
	  val=min(*i,val);
      }
      sig=checkValue(val);
    }
    break;
  }
  if (sig) {
    if (Signal)
      return;
    manager->SignalAlarms.insert(manager->SignalAlarms.end(),
				 alarmSignal(Action,xValue,str,toString(),Extra));
    manager->Timer.start(1,true);
    Signal=true;
  } else
    Signal=false;
}

void toChartManager::saveSettings(void)
{
  {
    int num=0;
    for(std::map<QString,chartTrack>::iterator i=Files.begin();i!=Files.end();i++) {
      if ((*i).second.Persistent) {
	num++;
	QCString name=QCString("Files:")+QString::number(num).latin1();
	ChartTool.setConfig(name+":Name",(*i).first);
	ChartTool.setConfig(name+":Spec",(*i).second.File.name());
      }
    }
    ChartTool.setConfig("FilesCount",QString::number(num));
  }
  {
    int num=0;
    for(std::map<QString,std::list<chartAlarm> >::iterator i=Alarms.begin();
	i!=Alarms.end();i++) {
      for(std::list<chartAlarm>::iterator j=(*i).second.begin();j!=(*i).second.end();j++) {
	if ((*j).Persistent) {
	  num++;
	  QCString name=QCString("Alarms:")+QString::number(num).latin1();
	  ChartTool.setConfig(name+":Name",(*i).first);
	  ChartTool.setConfig(name+":Spec",(*j).toString());
	}
      }
    }
    ChartTool.setConfig("AlarmCount",QString::number(num));
  }
  toTool::saveConfig();
}

void toChartManager::loadSettings(void)
{
  {
    for(int num=ChartTool.config("FilesCount","0").toInt();num>0;num--) {
      QCString name=QCString("Files:")+QString::number(num).latin1();
      QString t=ChartTool.config(name+":Name","");
      QString s=ChartTool.config(name+":Spec","");
      if (!t.isEmpty()&&!s.isEmpty())
	Files[t]=chartTrack(s,true);
    }
  }
  {
    for(int num=ChartTool.config("AlarmCount","0").toInt();num>0;num--) {
      QCString name=QCString("Alarms:")+QString::number(num).latin1();
      QString t=ChartTool.config(name+":Name","");
      QString s=ChartTool.config(name+":Spec","");
      if (!t.isEmpty()&&!s.isEmpty())
	Alarms[t].insert(Alarms[t].end(),chartAlarm(s,true));
    }
  }
}

void toChartManager::alarm(void)
{
  while(SignalAlarms.size()>0) {
    alarmSignal signal=toShift(SignalAlarms);
    if (signal.Action==StatusMessage)
      toStatusMessage(tr("ALARM:")+signal.Chart+QString::fromLatin1(": ")+signal.Alarm+QString::fromLatin1(": ")+signal.xValue);
    else if (signal.Action==Email)
      new toSMTP(QString::fromLatin1("TOra <noreply@localhost>"),
		 signal.Extra,
		 tr("TOra alert:")+" "+signal.Chart,
		 tr("A defined alert value was detected:\n\n%1\n\nAt: %2").arg(signal.Alarm).arg(signal.xValue));
  }
}

void toChartManager::valueAdded(toLineChart *chart,
				const QString &chartName,
				std::list<double> &value,
				const QString &xValue)
{
  std::map<QString,std::list<chartAlarm> >::iterator fnda=Alarms.find(chartName);
  if (fnda!=Alarms.end()) {
    std::list<chartAlarm> &alarms=(*fnda).second;
    for(std::list<chartAlarm>::iterator i=alarms.begin();i!=alarms.end();i++)
      (*i).valueAdded(this,chartName,value,xValue);
  }

  std::map<QString,chartTrack>::iterator fndt=Files.find(chartName);
  if (fndt!=Files.end()) {
    QFile &file=(*fndt).second.File;
    bool header=false;
    if (!file.isOpen()) {
      if (!file.exists())
	header=true;
      file.open(IO_Raw|IO_WriteOnly|IO_Append);
    }
    if (file.isOpen()) {
      static QRegExp quote(QString::fromLatin1("\""));
      QCString out="\"";
      if (header) {
	QString t=chart->title();
	t.replace(quote,QString::fromLatin1("\"\""));
	out+=t.utf8();
	std::list<QString> labels=chart->labels();
	for(std::list<QString>::iterator i=labels.begin();i!=labels.end();i++) {
	  out+="\";\"";
	  QString t=(*i);
	  t.replace(quote,QString::fromLatin1("\"\""));
	  out+=t.utf8();
	}
	out+="\"\n\"";
      }
      QString t=xValue;
      t.replace(quote,QString::fromLatin1("\"\""));
      out+=t.utf8();
      for(std::list<double>::iterator i=value.begin();i!=value.end();i++) {
	out+="\";\"";
	out+=QString::number(*i);
      }
      out+="\"\n";
      file.writeBlock(out,out.length());
    }
  }
}

static QString ReadCSV(const QCString &data,unsigned int &pos,bool &nl)
{
  QCString ret;
  if (data.at(pos)!='\"')
    throw qApp->translate("toChartManager","Initial value didn't start with \" in CSV file");
  pos++;
  while(pos<data.length()&&
	(data.at(pos)!='\"'||(pos+1<data.length()&&
			   data.at(pos+1)=='\"'))) {
    ret+=data.at(pos);
    if (data.at(pos)=='\"')
      pos++;
    pos++;
  }
  if (pos>=data.length())
    throw qApp->translate("toChartManager","Missing closing \" in CSV file");
  pos++;
  nl=false;
  while(pos<data.length()&&(isspace(data.at(pos))||data.at(pos)==','||data.at(pos)==';')) {
    if (data.at(pos)=='\n')
      nl=true;
    pos++;
  }
  return QString::fromUtf8(ret);
}

void toChartManager::openChart(void)
{
  QString name=toOpenFilename(QString::null,QString::fromLatin1("*.csv"),this);
  if (!name.isEmpty()) {
    try {
      QCString data=toReadFile(name);
      unsigned int pos=0;
      
      toLineChart *chart;
      switch(TOMessageBox::information(toMainWidget(),
				       tr("Chart format"),
				       tr("Select format of the chart to display"),
				       tr("Barchart"),
				       tr("Linechart"),
				       tr("Cancel"))) {
      case 0:
	chart=new toBarChart(toMainWidget()->workspace(),NULL,
			     WDestructiveClose);
	break;
      case 1:
	chart=new toLineChart(toMainWidget()->workspace(),NULL,
			      WDestructiveClose);
	break;
      default:
	return;
      }
      chart->show();
      chart->setSamples(-1);
      bool eol;
      chart->setTitle(ReadCSV(data,pos,eol));
      std::list<QString> lab;
      while(!eol&&pos<data.length())
	lab.insert(lab.end(),ReadCSV(data,pos,eol));
      chart->setLabels(lab);
      while(pos<data.length()) {
	QString t=ReadCSV(data,pos,eol);
	std::list<double> val;
	while(!eol&&pos<data.length())
	  val.insert(val.end(),ReadCSV(data,pos,eol).toDouble());
	chart->addValues(val,t);
      }
    } TOCATCH
  }
}

void toChartManager::setupChart(void)
{
  toChartReceiver *chart=selectedChart();
  if (chart)
    setupChart(chart->chart());
}

toChartReceiver *toChartManager::selectedChart(void)
{
  try {
    QListViewItem *item=List->selectedItem();
    if (item) {
      for(std::list<toChartReceiver *>::iterator i=Charts.begin();i!=Charts.end();i++) {
	toResult *result=(*i)->result();
	if (result) {
	  if (item->text(0)==result->connection().description(false)&&
	      item->text(2)==result->sqlName())
	    return *i;
	}
      }
    }
  } TOCATCH
  return NULL;
}

void toChartManager::setupChart(toLineChart *chart)
{
  for(std::list<toChartReceiver *>::iterator i=Charts.begin();i!=Charts.end();i++) {
    if ((*i)->chart()==chart) {
      QString name=(*i)->name();
      if (!name.isNull()) {
	std::list<chartAlarm> alarm;
	std::map<QString,std::list<chartAlarm> >::iterator fnda=Alarms.find(name);
	if (fnda!=Alarms.end())
	  alarm=(*fnda).second;

	chartTrack file;
	std::map<QString,chartTrack>::iterator fndt=Files.find(name);
	if (fndt!=Files.end())
	  file=(*fndt).second;

	toChartSetup setup(chart,this,NULL,true);
	setup.Alarms->addColumn(tr("Alarms"));
	setup.Alarms->addColumn(tr("Persistent"));
	setup.Alarms->setSorting(0);
	setup.Alarms->setSelectionMode(QListView::Single);
	setup.Persistent->setChecked(file.Persistent);
	setup.Filename->setText(file.File.name());
	if (!setup.Filename->text().isEmpty())
	  setup.Enabled->setChecked(true);
	setup.ChartFrame->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
						    QSizePolicy::Preferred));

	toLineChart *t=chart->openCopy(setup.ChartFrame);
	t->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
				     QSizePolicy::Preferred));
	t->showLegend(false);
	t->showAxisLegend(false);
	for(std::list<chartAlarm>::iterator j=alarm.begin();j!=alarm.end();j++)
	  new QListViewItem(setup.Alarms,
			    (*j).toString(),
			    (*j).Persistent?tr("Persistent"):tr("Temporary"));

	if (setup.exec()) {
	  if (setup.Filename->text().isEmpty()||!setup.Enabled->isChecked()) {
	    if (fndt!=Files.end())
	      Files.erase(fndt);
	  } else {
	    Files[name]=chartTrack(setup.Filename->text(),
				   setup.Persistent->isChecked());
	  }
	  if (fnda!=Alarms.end())
	    Alarms.erase(fnda);
	  alarm.clear();
	  for(QListViewItem *item=setup.Alarms->firstChild();
	      item;item=item->nextSibling()) {
	    alarm.insert(alarm.end(),chartAlarm(item->text(0),
						item->text(1)==tr("Persistent")));
	  }
	  if (alarm.size()>0)
	    Alarms[name]=alarm;
	  saveSettings();
	}
      }
      break;
    }
  }
}

void toChartManager::refresh(void)
{
  try {
    List->clear();
    for(std::list<toChartReceiver *>::iterator i=Charts.begin();i!=Charts.end();i++) {
      toResult *result=(*i)->result();
      if (result) {
	toResultViewItem *item=new toResultViewMLine(List,
						     NULL,
						     result->connection().description(false));
	item->setText(1,(*i)->chart()->title());
	item->setText(2,result->sqlName());
	QString name=(*i)->name();
	if (!name.isNull()) {
	  std::map<QString,std::list<chartAlarm> >::iterator fnda=Alarms.find(name);
	  if (fnda!=Alarms.end()) {
	    QString t;
	    for(std::list<chartAlarm>::iterator j=(*fnda).second.begin();j!=(*fnda).second.end();j++) {
	      t+=(*j).toString();
	      t+=QString::fromLatin1("\n");
	    }
	    if (t.length()>0)
	      item->setText(4,t.mid(0,t.length()-1));
	  }

	  std::map<QString,chartTrack>::iterator fndt=Files.find(name);
	  if (fndt!=Files.end())
	    item->setText(3,(*fndt).second.File.name());
	}
      }
    }
  } TOCATCH
}
