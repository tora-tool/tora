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

#include "tosmtp.h"

#include <qapplication.h>
#include <qdns.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qsocket.h>
#include <qtextstream.h>
#include <qtimer.h>

#include "tosmtp.moc"

toSMTP::toSMTP(const QString &from,const QString &to,
	       const QString &subject,const QString &body)
{
  Socket=new QSocket( this );
  connect(Socket,SIGNAL(readyRead()),
	  this,SLOT( readyRead()));
  connect(Socket,SIGNAL(connected()),
	  this,SLOT(connected()));
  
  Stream=0;
  MXLookup=new QDns(to.mid(to.find('@')+1),QDns::Mx);
  connect(MXLookup,SIGNAL(resultsReady()),
	  this,SLOT(dnsLookupHelper()) );
  
  Message=QString::fromLatin1("From: ")+from+
    QString::fromLatin1("\nTo: ")+to+
    QString::fromLatin1("\nSubject: ")+subject+
    QString::fromLatin1("\n\n")+body+"\n";
  Message.replace(QRegExp(QString::fromLatin1("\n")),
		  QString::fromLatin1("\r\n"));
  Message.replace(QRegExp(QString::fromLatin1("\r\n.\r\n")),
		  QString::fromLatin1("\r\n..\r\n"));

  From=from;
  Recipient=to;

  State=Init;
}

toSMTP::~toSMTP()
{
  delete Stream;
  delete Socket;
}

void toSMTP::dnsLookupHelper()
{
  QValueList<QDns::MailServer> s=MXLookup->mailServers();
  if(s.isEmpty()&&MXLookup->isWorking())
    return;

  toStatusMessage(tr("Connecting to %1").arg(s.first().name),false,false);

  Socket->connectToHost(s.first().name,25);
  Stream=new QTextStream(Socket);
}

void toSMTP::connected()
{
  toStatusMessage(tr("Connected to %1" ).arg(Socket->peerName()),false,false);
}

void toSMTP::readyRead()
{
  // SMTP is line-oriented
  if(!Socket->canReadLine())
    return;

  QString responseLine;
  do {
    responseLine=Socket->readLine();
    Response+=responseLine;
  } while(Socket->canReadLine()&&responseLine[3]!=' ');
  responseLine.truncate(3);

  if (State==Init&&responseLine[0]=='2') {
    // banner was okay, let's go on
    *Stream<<"HELO there\r\n";
    State=Mail;
  } else if (State==Mail&&responseLine[0]=='2') {
    // HELO response was okay (well, it has to be)
    *Stream<<"MAIL FROM: <"<<From<<">\r\n";
    State=Rcpt;
  } else if (State==Rcpt&&responseLine[0]=='2') {
    *Stream<<"RCPT TO: <"<<Recipient<<">\r\n";
    State=Data;
  } else if (State==Data&&responseLine[0]=='2') {
    *Stream<<"DATA\r\n";
    State=Body;
  } else if (State==Body&&responseLine[0]=='3') {
    *Stream<<Message<<".\r\n";
    State=Quit;
  } else if (State==Quit&&responseLine[0]=='2') {
    *Stream<<"QUIT\r\n";
    // here, we just close.
    State=Close;
    toStatusMessage(tr("Message sent"),false,false);
  } else if (State==Close) {
    delete this;
  } else {
    toStatusMessage(tr("Unexpected reply from SMTP server:\n\n")+
		    Response);
    State=Close;
  }

  Response="";
}
