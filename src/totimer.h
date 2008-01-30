//
// C++ Interface: toTimer
//
// Description:
// A timer descendant which also keep track of the last timer setting sent to it.
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef TOTIMER_H
#define TOTIMER_H

#include <qtimer.h>

class toTimer : public QTimer
{
    int LastTimer;

public:

    /** Create timer.
     * @param parent Parent object of timer.
     * @param name Name of timer.
     */
    toTimer(QObject *parent = 0, const char * name = 0);

    /** Start timer.
     * @param msec Milliseconds to timeout.
     * @param sshot Set to true if only timeout once.
     */
    int start(int msec, bool sshot = false);

    /** Get last timer start timeout.
     * @return Last timeout in millisecond.
     */
    int lastTimer(void);
};

#endif
