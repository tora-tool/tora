//
// C++ Implementation: totimer
//
// Copyright: See COPYING file that comes with this distribution
//
#include "totimer.h"

toTimer::toTimer(QObject *parent, const char * name):
        QTimer(parent)
{

    if (name)
        setObjectName(name);
}

int toTimer::start(int msec, bool sshot)
{
    LastTimer = msec;

    if (sshot)
        setSingleShot(msec);
    QTimer::start(msec);
    return timerId();
}

int toTimer::lastTimer(void)
{
    return LastTimer;
}
