/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "utils.h"

#include "toconf.h"
#include "toeventquery.h"
#include "toeventquerytask.h"
#include "toresultstats.h"
#include "totool.h"
#include "torunnable.h"


toQValue toEventQuery::readValueNull() {
    if(Values.isEmpty())
        throw tr("Read past end of query");

    if((Values.size() == 15 || Values.size() == 1) && !eof()) {
        if(Task)
            Task->read();           // getting close to end, request read
    }

    return Values.takeFirst();
}


toQValue toEventQuery::readValue() {
    return toNull(readValueNull());
}


toEventQuery::toEventQuery(toConnection &conn,
                           const QString &sql,
                           const toQList &param,
                           toResultStats *stats)
    : SQL(sql),
      Param(param),
      Statistics(stats),
      Task(0)
{

    Connection = &conn;
    Processed  = 0;
    Columns    = 0;
    TaskDone   = false;
}

toEventQuery::toEventQuery(toConnection &conn,
                           toQuery::queryMode mode,
                           const QString &sql,
                           const toQList &param,
                           toResultStats *stats)
    : SQL(sql),
      Param(param),
      Statistics(stats),
      Task(0) {

    Connection = &conn;
    Processed  = 0;
    Columns    = 0;
    TaskDone   = false;
}


void toEventQuery::start() {
    Task = new toEventQueryTask(this, *Connection, SQL, Param, Statistics);
    Task->ThreadAlive.lock();

    qRegisterMetaType<toQDescList>("toQDescList&");
    qRegisterMetaType<ValuesList>("ValuesList&");

    connect(Task,
            SIGNAL(headers(toQDescList &, int)),
            this,
            SLOT(taskDesc(toQDescList &, int)),
            Qt::QueuedConnection);

    connect(Task,
            SIGNAL(data(ValuesList &)),
            this,
            SLOT(taskData(ValuesList &)),
            Qt::QueuedConnection);

    connect(Task,
            SIGNAL(error(const toConnection::exception &)),
            this,
            SLOT(taskError(const toConnection::exception &)),
            Qt::QueuedConnection);

    connect(Task,
            SIGNAL(done()),
            this,
            SLOT(taskFinished()),
            Qt::QueuedConnection);

    (new toRunnableThread(Task))->start();
}


toEventQuery::~toEventQuery() {
    try {
        if(Task) {
            disconnect(Task, 0, 0, 0);
            if(Task->thread())
                Task->thread()->exit();
            Task->ThreadAlive.unlock();
        }
    }
    catch(...) {
        ; // ignored
    }
}


bool toEventQuery::eof(void) const {
    if(hasMore())
        return false;
    return TaskDone;
}


void toEventQuery::stop(void) {
    if(!TaskDone && Task)
        Task->close();
}


void toEventQuery::taskData(ValuesList &values) {
    Values += values;
    emit dataAvailable();

    try {
        if(Statistics)
            Statistics->refreshStats(false);
    }
    catch(...) {
        // ignored
    }
}


void toEventQuery::taskDesc(toQDescList &desc, int column) {
    Description = desc;
    Columns = column;
    emit descriptionAvailable();
}


void toEventQuery::taskError(const toConnection::exception &msg) {
    emit error(msg);
}


void toEventQuery::taskFinished() {
    TaskDone = true;
    emit done();
}
