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

#include <time.h>
#include <stdlib.h>

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qtextview.h>
#include <qregexp.h>

#include "md5.c"
#include "tolicense.h"

#include "tolicense.moc"
#include "tolicenseui.moc"

toLicense::toLicense(QWidget *parent)
  : toLicenseUI(parent,"license",WStyle_Customize|WStyle_DialogBorder|WType_Modal)
{
  QWidget *d=QApplication::desktop();
  move((d->width()-width())/2,(d->height()-height())/2);

  TextView->setText(QString(toAbout::aboutText()).arg(TOVERSION));
  TextView->setPaper(QColor(227,183,54));

  srand(time(NULL));

  switch(rand()%6) {
  case 0:
    Button1->setText("Register");
    connect(Button1,SIGNAL(clicked()),this,SLOT(licenseButton()));
    Button1->setDefault(true);
    Button2->setText("Quit");
    connect(Button2,SIGNAL(clicked()),this,SLOT(quitButton()));
    Button3->setText("Continue");
    connect(Button3,SIGNAL(clicked()),this,SLOT(dontRegisterButton()));
    break;
  case 1:
    Button2->setText("Register");
    connect(Button2,SIGNAL(clicked()),this,SLOT(licenseButton()));
    Button2->setDefault(true);
    Button1->setText("Quit");
    connect(Button1,SIGNAL(clicked()),this,SLOT(quitButton()));
    Button3->setText("Continue");
    connect(Button3,SIGNAL(clicked()),this,SLOT(dontRegisterButton()));
    break;
  case 2:
    Button3->setText("Register");
    connect(Button3,SIGNAL(clicked()),this,SLOT(licenseButton()));
    Button3->setDefault(true);
    Button2->setText("Quit");
    connect(Button2,SIGNAL(clicked()),this,SLOT(quitButton()));
    Button1->setText("Continue");
    connect(Button1,SIGNAL(clicked()),this,SLOT(dontRegisterButton()));
    break;
  case 3:
    Button2->setText("Register");
    connect(Button2,SIGNAL(clicked()),this,SLOT(licenseButton()));
    Button2->setDefault(true);
    Button3->setText("Quit");
    connect(Button3,SIGNAL(clicked()),this,SLOT(quitButton()));
    Button1->setText("Continue");
    connect(Button1,SIGNAL(clicked()),this,SLOT(dontRegisterButton()));
    break;
  case 4:
    Button1->setText("Register");
    connect(Button1,SIGNAL(clicked()),this,SLOT(licenseButton()));
    Button1->setDefault(true);
    Button3->setText("Quit");
    connect(Button3,SIGNAL(clicked()),this,SLOT(quitButton()));
    Button2->setText("Continue");
    connect(Button2,SIGNAL(clicked()),this,SLOT(dontRegisterButton()));
    break;
  default:
    Button3->setText("Register");
    connect(Button3,SIGNAL(clicked()),this,SLOT(licenseButton()));
    Button3->setDefault(true);
    Button1->setText("Quit");
    connect(Button1,SIGNAL(clicked()),this,SLOT(quitButton()));
    Button2->setText("Continue");
    connect(Button2,SIGNAL(clicked()),this,SLOT(dontRegisterButton()));
    break;
  }
  Name->setFocus();
  connect(Name,SIGNAL(returnPressed()),this,SLOT(licenseButton()));
  connect(License,SIGNAL(returnPressed()),this,SLOT(licenseButton()));
  Status=running;
}

void toLicense::dontRegisterButton(void)
{
  Status=dontRegister;
  hide();
}

#include "license.c"

static int DecodeChar(char c)
{
  for (int i=0;i<32;i++)
    if (Base[i]==c)
      return i;
  return -1;
}

static QString CheckLicense(const QString &user,const QString &license)
{
  QString ret;

  MD5_CTX MD5;
  unsigned char output[16];
  int i;

  QString userstr=user.upper();
  userstr.replace(QRegExp("[^A-Z0-9]"),"");
  const char *userp=userstr;

  QString licstr=license.upper();
  licstr.replace(QRegExp("[^0123456789ABCDEFGHJKMNPQRSTUVWXYZ]"),"");

  QString prep=licstr.mid(0,6);
  const char *prepstr=prep;

  /** Calculate hash sum value of secret value and username in argv[1] concatenated.
   */
  MD5Init(&MD5);
  MD5Update(&MD5,(unsigned char *)SECRET_HASH,strlen(SECRET_HASH));
  MD5Update(&MD5,(unsigned char *)prepstr,strlen(prepstr));
  MD5Update(&MD5,(unsigned char *)userp,strlen(userp)>255?255:strlen(userp));
  MD5Final(output,&MD5);
  
  ret=prep;

  ret+=QChar(Base[output[15]&31]);
  ret+=QChar(Base[(output[15]>>3)&31]);

  /* Output MD5 checksum in base 32 with a few characters left out.
   */
  for(i=0;i<3;i++) {
    ret+=QChar(Base[output[i*5]&31]);
    ret+=QChar(Base[((output[i*5+0]>>5)+(output[i*5+1]<<3))&31]);
    ret+=QChar(Base[(output[i*5+1]>>2)&31]);
    ret+=QChar(Base[((output[i*5+1]>>7)+(output[i*5+2]<<1))&31]);
    ret+=QChar(Base[((output[i*5+2]>>4)+(output[i*5+3]<<4))&31]);
    ret+=QChar(Base[(output[i*5+3]>>1)&31]);
    ret+=QChar(Base[((output[i*5+3]>>6)+(output[i*5+4]<<2))&31]);
    ret+=QChar(Base[(output[i*5+4]>>3)&31]);
  }

  if (licstr==ret) {
    int license=DecodeChar(prepstr[4])*32+DecodeChar(prepstr[5]);

    QString ret="Welcome to TOra";
    if (license==0)
      ret+=" (Personal license, not for commerical use)";
    else if (license==1)
      ret+=" (Commercial license one user)";
    else if (license==1000)
      ret+=" (Commerical site license)";
    else
      ret+=" (Commercial license "+QString::number(license)+" users)";

    return ret;
  }
  return QString::null;
}



void toLicense::licenseButton(void)
{
  if (!CheckLicense(Name->text(),License->text()).isNull()) {
    Status=license;
    hide();
  } else {
    TOMessageBox::information(this,
			      "Not a valid license key",
			      "Please check your name and registration key?",
			      "Ok");
  }
}

void toLicense::quitButton(void)
{
  if (TOMessageBox::warning(this,
			    "Really quit?",
			    "Are you sure you want to quit?",
			    "Yes",
			    "No")==0) {
    Status=quit;
    hide();
  }
}

toLicense::status toLicense::exec(void)
{
  Status=running;
  show();
  do {
    qApp->processOneEvent();
  } while(Status==running);
  return Status;
}

QString toCheckLicense(bool force)
{
  QString user=toTool::globalConfig("UserName","");
  QString regkey=toTool::globalConfig("RegKey","");

  QString ret=CheckLicense(user,regkey);
  if (ret.isNull()||force) {
    toLicense license(NULL);
    license.Name->setText(user);
    license.License->setText(regkey);
    if (license.exec()==toLicense::quit)
      exit(0);
    user=license.Name->text();
    regkey=license.License->text();
    toTool::globalSetConfig("UserName",user);
    toTool::globalSetConfig("RegKey",regkey);
    ret=CheckLicense(user,regkey);
    if (ret.isNull())
      ret="Welcome to TOra (Unregistered version, not for commercial use)";
    else
      toTool::saveConfig();
  }
  return ret;
}

bool toFreeware(void)
{
  return false;
}
