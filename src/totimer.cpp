//
// C++ Implementation: totimer
//
// Copyright: See COPYING file that comes with this distribution
//
#include "totimer.h"

toTimer::toTimer(QObject *parent, const char * name): QTimer(parent, name)
{ }

int toTimer::start(int msec, bool sshot)
{
    LastTimer = msec;
    return QTimer::start(msec, sshot);
}
int toTimer::lastTimer(void)
{
    return LastTimer;
}
