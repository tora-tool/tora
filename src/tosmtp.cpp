/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "tosmtp.h"
#include "toconf.h"
#include "toconfiguration.h"

#include <qapplication.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <QTextStream>
#include <qtimer.h>

#include <QTcpSocket>


toSMTP::toSMTP(const QString &from,
               const QString &to,
               const QString &subject,
               const QString &body)
{
    From      = from;
    Recipient = to;
    State     = Init;
    Stream    = 0;

    Socket = new QTcpSocket(this);
    connect(Socket, SIGNAL(readyRead()),
            this, SLOT(readyRead()));
    connect(Socket, SIGNAL(connected()),
            this, SLOT(connected()));

    Message = QString::fromLatin1("From: ") + from +
              QString::fromLatin1("\nTo: ") + to +
              QString::fromLatin1("\nSubject: ") + subject +
              QString::fromLatin1("\n\n") + body + "\n";
    Message.replace(QRegExp(QString::fromLatin1("\n")),
                    QString::fromLatin1("\r\n"));
    Message.replace(QRegExp(QString::fromLatin1("\r\n.\r\n")),
                    QString::fromLatin1("\r\n..\r\n"));

    QString server(toConfigurationSingle::Instance().smtp());
    int port = toConfigurationSingle::Instance().smtpPort();

    if (server.isNull() || server.isEmpty())
        toStatusMessage("No SMTP Server configured. Please check the preferences dialog.");
    else
    {
        Socket->connectToHost(server, port);
        Stream = new QTextStream(Socket);
    }
}

toSMTP::~toSMTP()
{
    delete Stream;
    delete Socket;
}

void toSMTP::connected()
{
    toStatusMessage(tr("Connected to %1").arg(Socket->peerName()), false, false);
}

void toSMTP::readyRead()
{
    // SMTP is line-oriented
    if (!Socket->canReadLine())
        return ;

    QString responseLine;
    do
    {
        responseLine = Socket->readLine();
        Response += responseLine;
    }
    while (Socket->canReadLine() && responseLine[3] != ' ');
    responseLine.truncate(3);

    if (State == Init && responseLine[0] == '2')
    {
        // banner was okay, let's go on
        *Stream << "HELO there\r\n";
        State = Mail;
    }
    else if (State == Mail && responseLine[0] == '2')
    {
        // HELO response was okay (well, it has to be)
        *Stream << "MAIL FROM: <" << From << ">\r\n";
        State = Rcpt;
    }
    else if (State == Rcpt && responseLine[0] == '2')
    {
        *Stream << "RCPT TO: <" << Recipient << ">\r\n";
        State = Data;
    }
    else if (State == Data && responseLine[0] == '2')
    {
        *Stream << "DATA\r\n";
        State = Body;
    }
    else if (State == Body && responseLine[0] == '3')
    {
        *Stream << Message << ".\r\n";
        State = Quit;
    }
    else if (State == Quit && responseLine[0] == '2')
    {
        *Stream << "QUIT\r\n";
        // here, we just close.
        State = Close;
        toStatusMessage(tr("Message sent"), false, false);
    }
    else if (State == Close)
    {
        delete this;
    }
    else
    {
        toStatusMessage(tr("Unexpected reply from SMTP server:\n\n") +
                        Response);
        State = Close;
    }

    Response = "";
}
