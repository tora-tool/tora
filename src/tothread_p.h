/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOTHREAD_P_H
#define TOTHREAD_P_H

#include "config.h"
#include "tothread.h"

#include <QThread>

class taskRunner : public QThread
{
    Q_OBJECT
public:
    toSemaphore StartSemaphore;
    toTask *Task;
    taskRunner(toTask *);
    virtual void run(void);
    friend class toThread;
};

#endif
