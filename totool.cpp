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
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#ifndef WIN32
#include <unistd.h>
#endif

#include <qapplication.h>
#include <qstring.h>
#include <qworkspace.h>

#include "totool.h"
#include "tomain.h"

#include <qfile.h>

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
      QString title(main->currentConnection().connectString());
      title.append(" ");
      title.append(name());
      newWin->setCaption(title);
      newWin->show();
      main->windowsMenu(); // Update accellerators
    }
  } TOCATCH
}

void toTool::saveConfig(void)
{
#ifndef WIN32
  if (!Configuration)
    return;
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  QString newconf(conf);
  newconf.append(".new");
  {
    QFile file(newconf);
    if (!file.open(IO_WriteOnly))
      return;

    QRegExp newline("\n");
    QRegExp backslash("\\");
    for (map<QString,QString>::iterator i=Configuration->begin();i!=Configuration->end();i++) {
      QString line=(*i).first;
      line.append("=");
      line.replace(backslash,"\\\\");
      line.replace(newline,"\\n");
      file.writeBlock(line,line.length());
      line=(*i).second;
      line.replace(backslash,"\\\\");
      line.replace(newline,"\\n");
      QCString str=line.utf8();
      file.writeBlock(str,str.length());
      file.writeBlock("\n",1);
    }
    if (file.status()!=IO_Ok) {
      file.close();
      unlink(newconf);
    }
  }
  QString oldconf=(conf);
  oldconf.append(".old");
  unlink(oldconf);
  if (!link(conf,oldconf))
    unlink(conf);
  link(newconf,conf);
  unlink(newconf);
#endif
}

void toTool::loadConfig(void)
{
  if (Configuration)
    delete Configuration;
  Configuration=new map<QString,QString>;
#ifndef WIN32
  QString conf;
  if (getenv("HOME")) {
    conf=getenv("HOME");
  }
  conf.append(CONFIG_FILE);
  QFile file(conf);
  if (!file.open(IO_ReadOnly)) {
    file.setName(DEF_CONFIG_FILE);
    if (!file.open(IO_ReadOnly))
      return;
  }

  int size=file.size();
  
  char buf[size+1];
  if (file.readBlock(buf,size)==-1) {
    throw QString("Encountered problems read configuration");
  }
  buf[size]=0;
  int pos=0;
  int bol=0;
  int endtag=-1;
  int wpos=0;
  while(pos<size) {
    switch(buf[pos]) {
    case '\n':
      if (endtag==-1)
	throw QString("Malformed tag in config file. Missing = on row.");
      buf[wpos]=0;
      {
	QString tag=buf+bol;
	QString val=buf+endtag+1;
	(*Configuration)[tag]=QString::fromUtf8(val);
      }
      bol=pos+1;
      endtag=-1;
      wpos=pos;
      break;
    case '=':
      if (endtag==-1) {
	endtag=pos;
	buf[wpos]=0;
	wpos=pos;
      } else
	buf[wpos]=buf[pos];
      break;
    case '\\':
      pos++;
      switch(buf[pos]) {
      case 'n':
	buf[wpos]='\n';
	break;
      case '\\':
	if (endtag>=0)
	  buf[wpos]='\\';
	else
	  buf[wpos]=':';
	break;
      default:
	throw QString("Unknown escape character in string (Only \\\\ and \\n recognised)");
      }
      break;
    default:
      buf[wpos]=buf[pos];
    }
    wpos++;
    pos++;
  }
#endif
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
