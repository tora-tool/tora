/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2008 Quest Software, Inc
* Portions Copyright (C) 2008 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TOSMTP_H
#define TOSMTP_H

#include "config.h"

#include <qobject.h>
#include <qstring.h>

class QTcpSocket;
class QTextStream;


// Adapted from Qt smtp example.

/**
 * Used to send mail asyncronously. Just create an instance and forget
 * it.  It will delete itself when done.
 */
class toSMTP : public QObject
{
    Q_OBJECT;

public:
    /** Send an email.
     * @param from From field of email.
     * @param to To field of email header.
     * @param subject Subject of mail.
     * @param body Body of email message.
     */
    toSMTP(const QString &from,
           const QString &to,
           const QString &subject,
           const QString &body);
    ~toSMTP();

private slots:
    void readyRead();
    void connected();

private:
    enum state
    {
        Init,
        Mail,
        Rcpt,
        Data,
        Body,
        Quit,
        Close
    };

    QString      Message;
    QString      From;
    QString      Recipient;
    QTcpSocket  *Socket;
    QTextStream *Stream;
    int          State;
    QString      Response;
};

#endif
