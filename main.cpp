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
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <qmessagebox.h>
#include <qapplication.h>
#include <qtextcodec.h>

#ifdef TO_KDE
#include <kapp.h>
#endif

#include "tomain.h"
#include "toconf.h"
#include "tohighlightedtext.h"
#include "toabout.h"
#include "tosql.h"
#include "totool.h"

#ifndef TOMONOLITHIC
#include <dlfcn.h>

#include <qdir.h>
#include <qfileinfo.h>
#include <qlabel.h>
#include <qprogressbar.h>
#endif

bool toMonolithic(void)
{
#ifdef TOMONOLITHIC
  return true;
#else
  return false;
#endif
}

int main(int argc,char **argv)
{
#if 0
#ifdef ENABLE_QT_XFT
  toSetEnv("QT_XFT",toTool::globalConfig(CONF_QT_XFT,DEFAULT_QT_XFT).latin1());
#endif
#endif
#ifdef TO_KDE
  new KApplication(argc,argv,"tora");
#else
  if (toTool::globalConfig(CONF_DESKTOP_AWARE,"Yes").isEmpty())
    QApplication::setDesktopSettingsAware(false);
  new QApplication(argc,argv);
#endif
  try {

    if (getenv("LANG"))
      qApp->setDefaultCodec(QTextCodec::codecForName(getenv("LANG")));

#ifdef ENABLE_STYLE
    QString style=toTool::globalConfig(CONF_STYLE,"");
    if (!style.isEmpty())
      toSetSessionType(style);
#endif

#ifndef TOMONOLITHIC
    {
      toSplash splash(NULL,"About TOra",false);
      splash.show();
      std::list<QString> failed;
      QString dirPath=toPluginPath();
      QDir d(dirPath,"*.tso",QDir::Name,QDir::Files);
      for (unsigned int i=0;i<d.count();i++) {
	failed.insert(failed.end(),d.filePath(d[i]));
      }
      QProgressBar *progress=splash.progress();
      QLabel *label=splash.label();
      progress->setTotalSteps(failed.size());
      progress->setProgress(1);
      qApp->processEvents();
      bool success;
      do {
	success=false;
	std::list<QString> current=failed;
	failed.clear();
	for(std::list<QString>::iterator i=current.begin();i!=current.end();i++) {
	  if (!dlopen(*i,RTLD_NOW|RTLD_GLOBAL)) {
	    failed.insert(failed.end(),*i);
	  } else {
	    success=true;
	    progress->setProgress(progress->progress()+1);
	    QString str("Loaded plugin ");
	    QFileInfo file(*i);
	    str+=file.fileName();
	    label->setText(str);
	    qApp->processEvents();
	  }
	}
      } while(failed.begin()!=failed.end()&&success);
      for(std::list<QString>::iterator i=failed.begin();i!=failed.end();i++)
	if (!dlopen(*i,RTLD_NOW|RTLD_GLOBAL))
	  printf("Failed to load %s\n  %s\n",
		 (const char *)(*i),dlerror());
    }
#endif

    toSQL::loadSQL(toTool::globalConfig(CONF_SQL_FILE,DEFAULT_SQL_FILE));

    {
      QString nls=getenv("NLS_LANG");
      if (nls.isEmpty())
	nls="american_america.UTF8";
      else {
	int pos=nls.findRev('.');
	if (pos>0)
	  nls=nls.left(pos);
	nls+=".UTF8";
      }
      toSetEnv("NLS_LANG",nls.latin1());
    }

#ifndef TO_LICENSE
    if (toTool::globalConfig("LastVersion","")!=TOVERSION) {
      toAbout *about=new toAbout(1,NULL,"About TOra",true);
      if (!about->exec()) {
	exit (2);
      }
      delete about;
      toTool::globalSetConfig("LastVersion",TOVERSION);
    }
#endif

    if (toTool::globalConfig("FirstInstall","").isEmpty()) {
      time_t t;
      time(&t);
      toTool::globalSetConfig("FirstInstall",ctime(&t));
    }

    if (qApp->argc()>2||(qApp->argc()==2&&qApp->argv()[1][0]=='-')) {
      printf("Usage:\n\n  tora [{X options}] [connectstring]\n\n");
      exit(2);
    } else if (qApp->argc()==2) {
      QString connect=qApp->argv()[1];
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

    new toMain;

    int ret=qApp->exec();
    return ret;
  } catch (const QString &str) {
    printf("Unhandled exception:\n\n%s\n",
	   (const char *)str);
    TOMessageBox::critical(NULL,
                           "Unhandled exception",
                           str,
                           "Exit");
#if 0
  } catch (...) {
    printf("Unhandled exception of unknown type.\n\n");
    TOMessageBox::critical(NULL,
                           "Unhandled exception",
                           "Unknown type",
                           "Exit");
#endif
  }
  return 1;
}
