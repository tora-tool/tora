/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toeventquerytask.h"
#include "toresultstats.h"
#include "totool.h"
#include "toresultstats.h"
#include "toconfiguration.h"

#include <QApplication>
#include <QMutexLocker>
#include <QTimer>


static const int FIREWALL_TIMEOUT = 240000;


#define CATCH_ALL                                   \
    catch(const toConnection::exception &str) {     \
        if(!Closed) {                               \
            emit error(str);                        \
            close();                                \
        }                                           \
    }                                               \
    catch(const QString &str) {                     \
        if(!Closed) {                               \
            emit error(str);                        \
            close();                                \
        }                                           \
    }                                               \
    catch(...) {                                    \
        if(!Closed) {                               \
            emit error(tr("Unknown exception"));    \
            close();                                \
        }                                           \
    }


toEventQueryTask::toEventQueryTask(QObject *parent,
                                   toConnection &conn,
                                   const QString &sql,
                                   const toQList &param,
                                   toResultStats *stats)
    : toRunnable(),
      SQL(sql),
      Params(param),
      Statistics(stats) {
    Query      = 0;
    Columns    = 0;
    Connection = &conn;
    Closed     = false;
    setObjectName("toEventQueryTask");
}


void toEventQueryTask::run(void) {
    try {
        Query = new toQuery(*Connection);
        Query->execute(SQL, Params);

        connect(this,
                SIGNAL(readRequested(bool)),
                this,
                SLOT(pread(bool)),
                Qt::QueuedConnection);

        toQDescList desc = Query->describe();
        Columns = Query->columns();
        emit headers(desc, Columns);

        if(Query->eof()) {
            // emit empty result
            ValuesList values;
            emit data(values);
        }
        else {
            read();

            if(toConfigurationSingle::Instance().firewallMode())
                QTimer::singleShot(FIREWALL_TIMEOUT, this, SLOT(timeout()));

            // begin thread's event loop
            thread()->exec();
        }
    }
    CATCH_ALL;

    close();

    try {
        if(Statistics)
            Statistics->changeSession(*Query);
    }
    catch(...) {
        // ignored
    }

    try {
        QMutexLocker lock(&CloseLock);
        Closed = true;
        delete Query;
        Query = 0;
    }
    catch(...) {
        // ignored
    }

    ThreadAlive.lock();
}


toEventQueryTask::~toEventQueryTask() {
}


void toEventQueryTask::close() {
    try {
        emit done();

        disconnect(this, 0, 0, 0);

        QMutexLocker lock(&CloseLock);
        if(Query && !Closed)
            Query->cancel();
    }
    catch(...) {
        // noop
    }

    Closed = true;

    // exit thread event loop. safe to call before event loop starts.
    if(thread())
        thread()->exit();
}


void toEventQueryTask::read(bool all) {
    emit readRequested(all);
}


void toEventQueryTask::pread(bool all) {
    if(!Query || Columns < 1) {
        close();
        return;
    }

    int maxRead = toConfigurationSingle::Instance().maxNumber();

    try {
        do {
            ValuesList values;

            for(int row = 0; row < maxRead; row++) {
                for(int i = 0; i < Columns && !Query->eof(); i++)
                    values.append(Query->readValueNull());
            }

            if(values.size() > 0)
                emit data(values);    // must not access after this line
        } while(all && !Query->eof());
    }
    CATCH_ALL;

    try {
        if(!Query || Query->eof())
            close();
    }
    CATCH_ALL;
}


void toEventQueryTask::timeout()
{
    if(Closed)
        return;

    pread(false);

    // check again in case config changes
    if(toConfigurationSingle::Instance().firewallMode())
        QTimer::singleShot(FIREWALL_TIMEOUT, this, SLOT(timeout()));
}
