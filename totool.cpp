//***************************************************************************
/* $Id$
**
** Copyright (C) 2000-2001 GlobeCom AB.  All rights reserved.
**
** This file is part of the Toolkit for Oracle.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE included in the packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.globecom.net/tora/ for more information.
**
** Contact tora@globecom.se if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef WIN32
#include <unistd.h>
#endif

#include <qpixmap.h>
#include <qapplication.h>
#include <qstring.h>
#include <qworkspace.h>
#include <qfile.h>
#include <qtimer.h>
#include <qregexp.h>
#ifdef WIN32
#  include "windows/cregistry.h"
#  include "windows/cregistry.cpp"
#else
#  if QT_VERSION >= 300
#    include <qsettings.h>
#  endif
#endif

#include "totool.h"
#include "tomain.h"
#include "toconf.h"
#include "toconnection.h"

#include "totool.moc"

toToolWidget::toToolWidget(toTool &tool,const QString &ctx,QWidget *parent,toConnection &conn,const char *name)
  : QVBox(parent,name,WDestructiveClose),toHelpContext(ctx),Tool(tool)
{
  Connection=&conn;
  Timer=NULL;
  Connection->addWidget(this);
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

std::map<QString,toTool *> *toTool::Tools;
std::map<QString,QString> *toTool::Configuration;

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
    Tools=new std::map<QString,toTool *>;
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
    QWidget *newWin=toolWindow(main->workspace(),main->currentConnection());

    if (newWin) {
      QString title;
      if (!toTool::globalConfig(CONF_DB_TITLE,"Yes").isEmpty()) {
	title=main->currentConnection().description();
	title.append(" ");
      }
      title.append(name());
      newWin->setCaption(title);
      newWin->show();
      main->windowsMenu(); // Update accellerators

      // Maximize window if only window
      if (toMainWidget()->workspace()->windowList().count()==1&&
	  toMainWidget()->workspace()->windowList().at(0)==newWin)
	newWin->showMaximized();

#if 1
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

bool toTool::saveMap(const QString &file,std::map<QString,QString> &pairs)
{
  QCString data;
  QString newfile(file);

  newfile.append(".new");
  {
    QFile file(newfile);
    if (!file.open(IO_WriteOnly))
      return false;

    QRegExp newline("\n");
    QRegExp backslash("\\");
    for (std::map<QString,QString>::iterator i=pairs.begin();i!=pairs.end();i++) {
      QString line=(*i).first;
      line.append("=");
      line.replace(backslash,"\\\\");
      line.replace(newline,"\\n");
      QCString str=line.latin1();

      file.writeBlock(str,str.length());
      line=(*i).second;
      line.replace(backslash,"\\\\");
      line.replace(newline,"\\n");
      str+=line.utf8();
      str+="\n";
      data+=str;
    }
  }
  return toWriteFile(file,data);
}

#ifdef WIN32
#  define APPLICATION_NAME "SOFTWARE\\GlobeCom\\tora\\"

static char *toKeyPath(const QString &str,CRegistry &registry)
{
  static char *buf=NULL;
  for(int pos=str.length()-1;pos>=0&&str.at(pos)!='\\';pos--)
    ;
  if (pos<0)
    throw QString("Couldn't find \\ in path");
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
    throw QString("Couldn't find \\ in path");
  if (buf)
    free(buf);
  buf=strdup(str.mid(pos+1));
  return buf;
}

#else
#  if QT_VERSION >= 300
#    define APPLICATION_NAME "/tora/"
#  endif
#endif

void toTool::saveConfig(void)
{
#ifdef WIN32
  CRegistry registry;
  QRegExp re(":");
  for (std::map<QString,QString>::iterator i=Configuration->begin();i!=Configuration->end();i++) {
    QString path=(*i).first;
    QString value=(*i).second;
    path.prepend(APPLICATION_NAME);
    path.replace(re,"\\");
    if (value.isNull())
      registry.SetStringValue(HKEY_CURRENT_USER,
			      toKeyPath(path,registry),
			      toKeyValue(path),
			      "");
    else {
      char *t=strdup(value.utf8());
      registry.SetStringValue(HKEY_CURRENT_USER,
			      toKeyPath(path,registry),
			      toKeyValue(path),
			      t);
      free(t);
    }
  }
#else
#  if QT_VERSION < 300
  if (!Configuration)
    return;
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  saveMap(conf,*Configuration);
#  else
  QSettings settings;
  QRegExp re(":");
  for (std::map<QString,QString>::iterator i=Configuration->begin();i!=Configuration->end();i++) {
    QString path=(*i).first;
    QString value=(*i).second;
    path.prepend(APPLICATION_NAME);
    path.replace(re,"/");
    if (value.isNull())
      settings.writeEntry(path,"");
    else
      settings.writeEntry(path,value);
  }
#  endif
#endif
}

#if QT_VERSION >= 300

static void ReadConfig(QSettings &settings,const QString &path,std::map<QString,QString> &conf)
{
  QStringList lst=settings.entryList(path);
  for(unsigned int i=0;i<lst.count();i++) {
    QString fullpath=path;
    fullpath+=lst[i];
    QString confname=fullpath;
    confname.replace(QRegExp("/"),":");
    confname=confname.right(confname.length()-strlen(APPLICATION_NAME));
    conf[confname]=settings.readEntry(fullpath);
  }
  lst=settings.subkeyList(path);
  for (unsigned int j=0;j<lst.count();j++) {
    QString fullpath=path;
    fullpath+=lst[j];
    fullpath+="/";
    ReadConfig(settings,fullpath,conf);
  }
}

#endif

void toTool::loadConfig(void)
{
  if (Configuration)
    delete Configuration;
  Configuration=new std::map<QString,QString>;

#ifndef WIN32
#  if QT_VERSION < 300
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  if (!loadMap(conf,*Configuration))
    loadMap(DEF_CONFIG_FILE,*Configuration);
#  else
  QSettings settings;
  ReadConfig(settings,APPLICATION_NAME,*Configuration);
#  endif
#endif
}

bool toTool::loadMap(const QString &filename,std::map<QString,QString> &pairs)
{
  QCString data;
  try {
    data=toReadFile(filename);
  } catch (...) {
    return false;
  }

  int pos=0;
  int bol=0;
  int endtag=-1;
  int wpos=0;
  int size=data.length();
  while(pos<size) {
    switch(data[pos]) {
    case '\n':
      if (endtag==-1)
	throw QString("Malformed tag in config file. Missing = on row. (%1)").
	  arg(((const char *)data)+bol);
      data[wpos]=0;
      {
	QString tag=((const char *)data)+bol;
	QString val=((const char *)data)+endtag+1;
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
	throw QString("Unknown escape character in string (Only \\\\ and \\n recognised)");
      }
      break;
    default:
      data[wpos]=data[pos];
    }
    wpos++;
    pos++;
  }
  return true;
}

const QString &toTool::config(const QString &tag,const QString &def)
{
  QString str=name();
  str.append(":");
  str.append(tag);
  return globalConfig(str,def);
}

void toTool::setConfig(const QString &tag,const QString &def)
{
  QString str=name();
  str.append(":");
  str.append(tag);
  globalSetConfig(str,def);
}

const QString &toTool::globalConfig(const QString &tag,const QString &def)
{
  if (!Configuration)
    loadConfig();

  std::map<QString,QString>::iterator i=Configuration->find(tag);
  if (i==Configuration->end()) {
#if defined(WIN32)
    CRegistry registry;
    QRegExp re(":");
    QString path=tag;
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
    }
#endif
    return def;
  }
  return (*i).second;
}

void toTool::globalSetConfig(const QString &tag,const QString &value)
{
  if (!Configuration)
    loadConfig();

  (*Configuration)[tag]=value;
}

bool toTool::canHandle(toConnection &conn)
{
  return (conn.provider()=="Oracle");
}
