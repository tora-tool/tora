//***************************************************************************
/*
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
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
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

TO_NAMESPACE;

#ifndef WIN32
#include <unistd.h>
#endif

#include <qpixmap.h>
#include <qapplication.h>
#include <qstring.h>
#include <qworkspace.h>
#include <qfile.h>
#include <qregexp.h>
#if QT_VERSION >= 300
#  include <qsettings.h>
#endif

#include "totool.h"
#include "tomain.h"
#include "toconf.h"

#include "totool.moc"

map<QString,toTool *> *toTool::Tools;
map<QString,QString> *toTool::Configuration;

#define CONFIG_FILE "/.torarc"
#define DEF_CONFIG_FILE "/etc/torarc"

char **toTool::pictureXPM(void)
{
  return NULL;
}

#include <stdio.h>

toTool::toTool(int priority,const char *name)
  : Name(name)
{
  if (!Tools)
    Tools=new map<QString,toTool *>;
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
	title=main->currentConnection().connectString();
	title.append(" ");
      }
      title.append(name());
      newWin->setCaption(title);
      newWin->show();
      main->windowsMenu(); // Update accellerators
    }
  } TOCATCH
}

bool toTool::saveMap(const QString &file,map<QString,QString> &pairs)
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
    for (map<QString,QString>::iterator i=pairs.begin();i!=pairs.end();i++) {
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

#define APPLICATION_NAME "/tora/"

void toTool::saveConfig(void)
{
#if ! defined(WIN32) && QT_VERSION < 300
  if (!Configuration)
    return;
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  saveMap(conf,*Configuration);
#else
#  if QT_VERSION >= 300
  QSettings settings;
  QRegExp re(":");
  for (map<QString,QString>::iterator i=Configuration->begin();i!=Configuration->end();i++) {
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

static void ReadConfig(QSettings &settings,const QString &path,map<QString,QString> &conf)
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
  Configuration=new map<QString,QString>;

#if ! defined(WIN32) && QT_VERSION < 300
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  if (!loadMap(conf,*Configuration))
    loadMap(DEF_CONFIG_FILE,*Configuration);
#else
#  if QT_VERSION >= 300
  QSettings settings;
  ReadConfig(settings,APPLICATION_NAME,*Configuration);
#  endif
#endif
}

bool toTool::loadMap(const QString &filename,map<QString,QString> &pairs)
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

  map<QString,QString>::iterator i=Configuration->find(tag);
  if (i==Configuration->end())
    return def;
  return (*i).second;
}

void toTool::globalSetConfig(const QString &tag,const QString &value)
{
  if (!Configuration)
    loadConfig();

  (*Configuration)[tag]=value;
}
