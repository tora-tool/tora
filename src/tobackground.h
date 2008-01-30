//
// C++ Interface: toBackground
//
// Copyright: See COPYING file that comes with this distribution
//

#ifndef TOBACKGROUND_H
#define TOBACKGROUND_H

#include "config.h"
#include "totimer.h"

class toMain;


/**
 * A descendant of timer which will indicate in the statusbar of the
 * main window when any timer is running. Can not do singleshots, only
 * periodic intervals.
 */
class toBackground : public toTimer
{
public:
    /**
     * Create a background timer widget.
     * @param parent Parent widget.
     * @param name Name of widget.
     */
    toBackground(QObject* parent = 0, const char* name = 0);
    ~toBackground();
    /** Start repeating timer with msec interval.
     * @param msec Millsecond repeat interval.
     */
    void start(int msec);
    /** Stop timer
     */
    void stop(void);

    static void init(void);
private:
    static int Running;
    toMain* main;
};

#endif
