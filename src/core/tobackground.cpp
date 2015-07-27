
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

//
// C++ Implementation: toBackground
//


#include "core/tobackground.h"
#include "core/tobackgroundlabel.h"
#include "core/tomainwindow.h"

int toBackground::Running = 0;

toBackground::toBackground(QObject* parent, const char* name)
    : toTimer(parent, name)
    , label(NULL)
{
    /* It it not allowed to touch the gui(instance of toBackgroundLabel) from parallel thread,
    * therefore we connect it(if it exists) to our signals and will update it using signal emits.
    * QT signal mechanism will satisfy handover of singal data between our thread and mainWindowThread
    */

    {
        connect(this, SIGNAL(SignalPause(void)), this, SLOT(pause(void)));
        connect(this, SIGNAL(SignalUnpause(void)), this, SLOT(unpause(void)));
        connect(this, SIGNAL(SignalSetSpeed(int)), this, SLOT(setSpeed(int)));
        connect(this, SIGNAL(SignalSetTip(QString)), this, SLOT(setTip(QString)));
    }
}

void toBackground::start(int msec)
{
    if (!isActive())
    {
        Running++;
        emit SignalUnpause();
    }
    emit SignalSetSpeed( (::std::min)(Running, 1) * 100);
    if (Running > 1)
    {
        emit SignalSetTip(tr("%1 queries running in background.").arg(Running));
    }
    else
    {
        emit SignalSetTip(tr("One query running in background."));
    }
    toTimer::start(msec);
}

void toBackground::stop(void)
{
    if (isActive())
    {
        Running--;
        if (Running == 0)
            emit SignalPause();
        else
            emit SignalSetSpeed(Running * 100);

        if (Running > 1)
            emit SignalSetTip(tr("%1 queries running in background.").arg(Running));
        else if (Running == 1)
            emit SignalSetTip("One query running in background.");
        else
            emit SignalSetTip(tr("No background queries."));
    }
    toTimer::stop();
}

void toBackground::setBackgroundLabel(toBackgroundLabel* label)
{
    this->label = label;
}

toBackground::~toBackground()
{
    stop();
}

void toBackground::pause()
{
    if (label)
    {
        label->pause();
    }
}

void toBackground::unpause()
{
    if (label)
    {
        label->unpause();
    }
}

void toBackground::setSpeed(int speed)
{
    if (label)
    {
        label->setSpeed(speed);
    }
}

void toBackground::setTip(QString tip)
{
    if (label)
    {
        label->setTip(tip);
    }
}
