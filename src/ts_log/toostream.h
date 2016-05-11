#ifndef __UGLY_OSTREAM
#define __UGLY_OSTREAM

#include "docklets/tologging.h"
#include "core/tora_export.h"

#include <QtCore/QObject>
#include <QPlainTextEdit>

#include <streambuf>
#include <iostream>

/** This widget holds logging information
 * It can we owned by Splash screen or MainWindow
 * Loki does not control lifetime of this singleton. It is deleted by it's parent widget - if any
 * */
typedef Loki::SingletonHolder<QPlainTextEdit, Loki::CreateUsingNew, Loki::NoDestroy> toLoggingWidgetSingle;

class TORA_EXPORT toSender : public QObject
{
        Q_OBJECT;
    public:
        toSender()
            : QObject(NULL)
            , initialized(false)
        {
            QPlainTextEdit const& e = toLoggingWidgetSingle::Instance();
            connect(this, SIGNAL(send(QString const&)), &e, SLOT(appendPlainText(QString const&)), Qt::QueuedConnection);

            initialized = true;
        };

        void emit_send(QString const& s)
        {
            if (initialized)
                emit send(s);
        }

    signals:

        void send(QString const& s);
    private:
        bool initialized;
};

class toBuffer : public std::basic_streambuf< char, std::char_traits< char > >
{

    public:

        inline toBuffer()
        {
            //setp(buf, buf + BUF_SIZE);
        }

    protected:

        //// This is called when buffer becomes full. If
        //// buffer is not used, then this is called every
        //// time when characters are put to stream.
        //inline virtual int overflow(int c = std::char_traits< char >::eof())
        //{
        //    putChars(pbase(), pptr());
        //    if (c != Traits::eof()) {
        //        char c2 = c;
        //        // Handle the one character that didn't fit to buffer
        //        putChars(&c2, &c2 + 1);
        //    }
        //    // This tells that buffer is empty again
        //    setp(buf, buf + BUF_SIZE);
        //    return 0;
        //}

        //// This function is called when stream is flushed,
        //// for example when std::endl is put to stream.
        //virtual int sync(void)
        //{
        //    putChars(pbase(), pptr());
        //    setp(buf, buf + BUF_SIZE);
        //    return 0;
        //}

        std::streamsize xsputn(const char * s, std::streamsize n)
        {
            putChars(s, n);
            return n;
        }

    private:

        // For EOF detection
        typedef std::char_traits< char > Traits;

        // Work in no buffer mode. It is also possible to work with buffer.
        ///static size_t const BUF_SIZE = 32;
        ///char buf[BUF_SIZE];

        toSender sender;

        void putChars(char const* begin, std::streamsize len)
        {
            QString s = QString::fromLatin1(begin, len);
            sender.emit_send(s);
        }

};

class toOStream : public std::basic_ostream< char, std::char_traits< char > >
{
    public:

        inline toOStream() : std::basic_ostream< char, std::char_traits< char > >(&buf)
        {}

    private:
        toBuffer buf;
};

#endif
