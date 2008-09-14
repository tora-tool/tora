/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "torunnable.h"

#include <QCoreApplication>

static toRunnableDeleter *Deleter = new toRunnableDeleter();

void toRunnableDeleter::customEvent(QEvent *event) {
    FinishedEvent *e = dynamic_cast<FinishedEvent *>(event);
    if(e) {
        toRunnableThread *t = e->thread();
        if(t) {
            t->exit();
            t->wait();
            delete t;
        }
    }
}


void toRunnableThread::run() {
    Runner->setThread(this);
    Runner->run();

    delete Runner;
    QCoreApplication::postEvent(Deleter, new FinishedEvent(this));
}
