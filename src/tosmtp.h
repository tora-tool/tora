/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

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
