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

#include "tobackground.h"
#include "toconf.h"
#include "toconnection.h"
#include "tomain.h"
#include "totool.h"

#ifndef WIN32
#include <unistd.h>
#endif

#include <qapplication.h>
#include <qfile.h>
#include <qpixmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qtimer.h>
#include <qworkspace.h>

#ifdef WIN32
#  include "windows/cregistry.h"
#  include "windows/cregistry.cpp"
#endif

#include "totool.moc"

// A little magic to get lrefresh to work and get a check on qApp

#undef QT_TRANSLATE_NOOP
#define QT_TRANSLATE_NOOP(x,y) QTRANS(x,y)

toToolWidget::toToolWidget(toTool &tool,const QString &ctx,QWidget *parent,toConnection &conn,const char *name)
  : QVBox(parent,name,WDestructiveClose),toHelpContext(ctx),Tool(tool)
{
  Connection=&conn;
  Timer=NULL;
  Connection->addWidget(this);

  if (parent) {
    // Voodoo for making connection changing cascade to sub tools.
    try {
      toToolWidget *tool=toCurrentTool(parent);
      if (tool)
	connect(tool,SIGNAL(connectionChange()),this,SLOT(parentConnection()));
    } catch(...) {
    }
  }
}

void toToolWidget::parentConnection(void)
{
  try {
    setConnection(toCurrentConnection(parentWidget()));
  } TOCATCH
}

toToolWidget::~toToolWidget()
{
  Connection->delWidget(this);
}

void toToolWidget::setConnection(toConnection &conn)
{
  bool connCap=false;
  QString name=Connection->description();
  QString capt=caption();
  if (capt.startsWith(name)) {
    connCap=true;
    capt=capt.mid(name.length());
  }
  Connection->delWidget(this);
  Connection=&conn;
  Connection->addWidget(this);
  if (connCap) {
    capt.prepend(Connection->description());
    setCaption(capt);
  }
  emit connectionChange();
}

toTimer *toToolWidget::timer(void)
{
  if (!Timer)
    Timer=new toTimer(this);
  return Timer;
}

void toToolWidget::exportData(std::map<QCString,QString> &data,const QCString &prefix)
{
  QWidget *par=parentWidget();
  if (!par)
    par=this;
  if (isMaximized()||par->width()>=toMainWidget()->workspace()->width())
    data[prefix+":State"]=QString::fromLatin1("Maximized");
  else if (isMinimized())
    data[prefix+":State"]=QString::fromLatin1("Minimized");
  else {
    data[prefix+":X"]=QString::number(par->x());
    data[prefix+":Y"]=QString::number(par->y());
    data[prefix+":Width"]=QString::number(par->width());
    data[prefix+":Height"]=QString::number(par->height());
  }
}

void toToolWidget::importData(std::map<QCString,QString> &data,const QCString &prefix)
{
  QWidget *par=parentWidget();
  if (!par)
    par=this;
  if (data[prefix+":State"]==QString::fromLatin1("Maximized"))
    showMaximized();
  else if (data[prefix+":State"]==QString::fromLatin1("Minimized"))
    showMinimized();
  else {
    par->showNormal();
    par->setGeometry(data[prefix+":X"].toInt(),
		     data[prefix+":Y"].toInt(),
		     data[prefix+":Width"].toInt(),
		     data[prefix+":Height"].toInt());
  }
}

std::map<QCString,toTool *> *toTool::Tools;
std::map<QCString,QString> *toTool::Configuration;

#define CONFIG_FILE "/.torarc"
#define DEF_CONFIG_FILE "/etc/torarc"

char **toTool::pictureXPM(void)
{
  return NULL;
}

toTool::toTool(int priority,const char *name)
  : Name(name)
{
  if (!Tools)
    Tools=new std::map<QCString,toTool *>;
  Priority=priority;
  Key.sprintf("%05d%s",priority,name);
  (*Tools)[Key]=this;
  ButtonPicture=NULL;
}

toTool::~toTool()
{
  Tools->erase(Key);
}

const QPixmap *toTool::toolbarImage()
{
  if (!ButtonPicture) {
    char **picture=pictureXPM();
    if (picture)
      ButtonPicture=new QPixmap((const char **)picture);
  }
  return ButtonPicture;
}

void toTool::createWindow(void)
{
  toMain *main=(toMain *)qApp->mainWidget();
  try {
    if (!canHandle(main->currentConnection()))
      throw QString("The tool %1 doesn't support the current database").arg(name());
    QWidget *newWin=toolWindow(main->workspace(),main->currentConnection());

    if (newWin) {
      const QPixmap *icon=toolbarImage();
      if (icon)
	newWin->setIcon(*icon);
      toToolWidget *tool=dynamic_cast<toToolWidget *>(newWin);
      if (tool)
	toToolCaption(tool,name());

      newWin->show();
      main->windowsMenu();

      // Maximize window if only window
      {
	bool max=true;
	for (unsigned int i=0;i<toMainWidget()->workspace()->windowList().count();i++) {
	  QWidget *widget=toMainWidget()->workspace()->windowList().at(i);
	  if (widget&&widget!=newWin&&!widget->isHidden())
	    max=false;
	}
	if (max)
	  newWin->showMaximized();
      }

#if QT_VERSION < 0x030100
      // This is a really ugly workaround for a Qt layout bug
      QWidget *tmp=NULL;
      QWidget *tmp2=NULL;
      for (unsigned int i=0;i<toMainWidget()->workspace()->windowList().count();i++) {
        QWidget *widget=toMainWidget()->workspace()->windowList().at(i);
        if (newWin!=widget)
	  tmp2=widget;
	else
	  tmp=newWin;
	if (tmp2&&tmp)
	  break;
      }
      if(tmp2&&tmp) {
        tmp2->setFocus();
        tmp->setFocus();
      }
#endif
    }
  } TOCATCH
}

bool toTool::saveMap(const QString &file,std::map<QCString,QString> &pairs)
{
  QCString data;

  {
    QRegExp newline(QString::fromLatin1("\n"));
    QRegExp backslash(QString::fromLatin1("\\"));
    for (std::map<QCString,QString>::iterator i=pairs.begin();i!=pairs.end();i++) {
      QCString str=(*i).first;
      str.append(QString::fromLatin1("="));
      str.replace(backslash,QString::fromLatin1("\\\\"));
      str.replace(newline,QString::fromLatin1("\\n"));
      QString line=(*i).second;
      line.replace(backslash,QString::fromLatin1("\\\\"));
      line.replace(newline,QString::fromLatin1("\\n"));
      str+=line.utf8();
      str+=QString::fromLatin1("\n");
      data+=str;
    }
  }
  return toWriteFile(file,data);
}

#ifdef WIN32
#  define APPLICATION_NAME "SOFTWARE\\Underscore\\tora\\"
#  define FALLBACK_NAME    "SOFTWARE\\GlobeCom\\tora\\"

static char *toKeyPath(const QString &str,CRegistry &registry)
{
  static char *buf=NULL;
  for(int pos=str.length()-1;pos>=0&&str.at(pos)!='\\';pos--)
    ;
  if (pos<0)
    throw QT_TRANSLATE_NOOP("toKeyPath","Couldn't find \\ in path");
  QString ret=str.mid(0,pos);
  if (buf)
    free(buf);
  buf=strdup(ret);
  registry.CreateKey(HKEY_CURRENT_USER,buf);
  return buf;
}

static char *toKeyValue(const QString &str)
{
  static char *buf=NULL;
  for(int pos=str.length()-1;pos>=0&&str.at(pos)!='\\';pos--)
    ;
  if (pos<0)
    throw QT_TRANSLATE_NOOP("toKeyValue","Couldn't find \\ in path");
  if (buf)
    free(buf);
  buf=strdup(str.mid(pos+1));
  return buf;
}

#endif

void toTool::saveConfig(void)
{
  try {
#ifdef WIN32
    CRegistry registry;
    QRegExp re(":");
    for (std::map<QCString,QString>::iterator i=Configuration->begin();i!=Configuration->end();i++) {
      QCString path=(*i).first;
      QString value=(*i).second;
      path.prepend(APPLICATION_NAME);
      path.replace(re,"\\");
      if (value.isEmpty()) {
	if (!registry.SetStringValue(HKEY_CURRENT_USER,
				     toKeyPath(path,registry),
				     toKeyValue(path),
				     ""))
	  toStatusMessage(QT_TRANSLATE_NOOP("toTool","Couldn't save empty value at key %1").arg(path));
      } else {
	char *t=strdup(value.utf8());
	if (!registry.SetStringValue(HKEY_CURRENT_USER,
				     toKeyPath(path,registry),
				     toKeyValue(path),
				     t))
	  toStatusMessage(QT_TRANSLATE_NOOP("toTool","Couldn't save %1 value at key %2").arg(value).arg(path));
	free(t);
      }
    }
#else
    if (!Configuration)
      return;
    QCString conf;
    if (getenv("HOME")) {
      conf=getenv("HOME");
    }
    conf.append(CONFIG_FILE);
    saveMap(conf,*Configuration);
#endif
  } TOCATCH
}

void toTool::loadConfig(void)
{
  if (Configuration)
    delete Configuration;
  Configuration=new std::map<QCString,QString>;

#ifndef WIN32
  QString conf;
  if (getenv("HOME")) {
    conf=QString::fromLatin1(getenv("HOME"));
  }
  conf.append(QString::fromLatin1(CONFIG_FILE));
  try {
    loadMap(conf,*Configuration);
  } catch(...) {
    try {
      loadMap(QString::fromLatin1(DEF_CONFIG_FILE),*Configuration);
    } catch(...) {
    }
  }
#endif
}

void toTool::loadMap(const QString &filename,std::map<QCString,QString> &pairs)
{
  QCString data=toReadFile(filename);

  int pos=0;
  int bol=0;
  int endtag=-1;
  int wpos=0;
  int size=data.length();
  while(pos<size) {
    switch(data[pos]) {
    case '\n':
      data[wpos]=0;
      if (endtag==-1)
	throw QT_TRANSLATE_NOOP("toTool","Malformed tag in config file. Missing = on row. (%1)").
	  arg(data.mid(bol,wpos-bol));
      {
	QCString tag=((const char *)data)+bol;
	QCString val=((const char *)data)+endtag+1;
	pairs[tag]=QString::fromUtf8(val);
      }
      bol=pos+1;
      endtag=-1;
      wpos=pos;
      break;
    case '=':
      if (endtag==-1) {
	endtag=pos;
	data[wpos]=0;
	wpos=pos;
      } else
	data[wpos]=data[pos];
      break;
    case '\\':
      pos++;
      switch(data[pos]) {
      case 'n':
	data[wpos]='\n';
	break;
      case '\\':
	if (endtag>=0)
	  data[wpos]='\\';
	else
	  data[wpos]=':';
	break;
      default:
	throw QT_TRANSLATE_NOOP("toTool","Unknown escape character in string (Only \\\\ and \\n recognised)");
      }
      break;
    default:
      data[wpos]=data[pos];
    }
    wpos++;
    pos++;
  }
  return;
}

const QString &toTool::config(const QCString &tag,const QCString &def)
{
  QCString str=name();
  str.append(":");
  str.append(tag);
  return globalConfig(str,def);
}

void toTool::eraseConfig(const QCString &tag)
{
  QCString str=name();
  str.append(":");
  str.append(tag);
  globalEraseConfig(str);
}

void toTool::setConfig(const QCString &tag,const QString &def)
{
  QCString str=name();
  str.append(":");
  str.append(tag);
  globalSetConfig(str,def);
}

const QString &toTool::globalConfig(const QCString &tag,const QCString &def)
{
  if (!Configuration)
    loadConfig();

  std::map<QCString,QString>::iterator i=Configuration->find(tag);
  if (i==Configuration->end()) {
#if defined(WIN32)
    CRegistry registry;
    QRegExp re(QString::fromLatin1(":"));
    QCString path=tag;
    path.prepend(APPLICATION_NAME);
    path.replace(re,"\\");
    DWORD siz=1024;
    char buffer[1024];
    try {
      if (registry.GetStringValue(HKEY_CURRENT_USER,
  				  toKeyPath(path,registry),
				  toKeyValue(path),
				  buffer,siz)) {
	if (siz>0) {
	  QString ret=QString::fromUtf8(buffer);
	  (*Configuration)[tag]=ret;
	} else {
	  (*Configuration)[tag]="";
	}
	return (*Configuration)[tag];
      }
    } catch (...) {
      try {
	path=tag;
	path.prepend(FALLBACK_NAME);
	path.replace(re,"\\");
	if (registry.GetStringValue(HKEY_CURRENT_USER,
				    toKeyPath(path,registry),
				    toKeyValue(path),
				    buffer,siz)) {
	  if (siz>0) {
	    QString ret=QString::fromUtf8(buffer);
	    (*Configuration)[tag]=ret;
	  } else {
	    (*Configuration)[tag]="";
	  }
	  return (*Configuration)[tag];
	}
      } catch (...) {
      }
    }
#endif
    (*Configuration)[tag]=QString::fromLatin1(def);
    return (*Configuration)[tag];
  }
  return (*i).second;
}

void toTool::globalEraseConfig(const QCString &tag)
{
  if (!Configuration)
    loadConfig();
  std::map<QCString,QString>::iterator i=Configuration->find(tag);
  if (i!=Configuration->end()) {
    Configuration->erase(i);
#if defined(WIN32)
    CRegistry registry;
    QRegExp re(QString::fromLatin1(":"));
    QCString path=tag;
    path.prepend(APPLICATION_NAME);
    path.replace(re,"\\"); 
    registry.DeleteKey(HKEY_CURRENT_USER,path); // Don't really care if it works.
#endif
  }
}

void toTool::globalSetConfig(const QCString &tag,const QString &value)
{
  if (!Configuration)
    loadConfig();

  (*Configuration)[tag]=value;
}

bool toTool::canHandle(toConnection &conn)
{
  return (conn.provider()=="Oracle");
}

void toTool::customSetup(int)
{
}

QWidget *toTool::configurationTab(QWidget *)
{
  return NULL;
}

toTool *toTool::tool(const QCString &key)
{
  if (!Tools)
    Tools=new std::map<QCString,toTool *>;
  std::map<QCString,toTool *>::iterator i=Tools->find(key);
  if (i==Tools->end())
    return NULL;
  
  return (*i).second;
}

void toTool::about(QWidget *)
{
}
