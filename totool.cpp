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

#include "totool.h"
#include "tomain.h"
#include "toconf.h"
#include "toconnection.h"

#ifdef WIN32
#  include "windows/cregistry.h"
#  include "windows/cregistry.cpp"
#endif

#include "totool.moc"

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
  setConnection(toCurrentConnection(parentWidget()));
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
      const QPixmap *icon=toolbarImage();
      if (icon)
	newWin->setIcon(*icon);
      toToolWidget *tool=dynamic_cast<toToolWidget *>(newWin);
      if (tool)
	toToolCaption(tool,name());

      newWin->show();
      main->windowsMenu();

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
  if (!Configuration)
    return;
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  saveMap(conf,*Configuration);
#endif
}

void toTool::loadConfig(void)
{
  if (Configuration)
    delete Configuration;
  Configuration=new std::map<QString,QString>;

#ifndef WIN32
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  if (!loadMap(conf,*Configuration))
    loadMap(DEF_CONFIG_FILE,*Configuration);
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
