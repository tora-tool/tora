/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX, Qt/Windows or Qt Non Commercial products of TrollTech.
 *      And you are not permitted to distribute binaries compiled against
 *      these libraries.
 *
 *      You may link this product with any GPL'd Qt library.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

//
// C++ Implementation: toBackground
//

#include "utils.h"

#include "tobackground.h"
#include "toconnection.h"

#include <qlabel.h>
#include <qstatusbar.h>
#include <qtooltip.h>

#include "tobackgroundlabel.h"
#include "tomain.h"

int toBackground::Running = 0;

toBackground::toBackground(QObject* parent, const char* name)
        : toTimer(parent, name)
		, main(toMainWidget())
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
    emit SignalSetSpeed((std::min)(Running, 1)*100);
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
			emit SignalSetSpeed(Running*100);

		if (Running > 1)
			emit SignalSetTip(tr("%1 queries running in background.").arg(Running));
		else if (Running == 1)
			emit SignalSetTip("One query running in background.");
		else
			emit SignalSetTip(tr("No background queries."));
    }
    toTimer::stop();
}

toBackground::~toBackground()
{
    stop();
}

void toBackground::pause()
{
	if(main && main->getBackgroundLabel())
	{
		main->getBackgroundLabel()->pause();
	}
}

void toBackground::unpause()
{
	if(main && main->getBackgroundLabel())
	{
		main->getBackgroundLabel()->unpause();
	}
}

void toBackground::setSpeed(int speed)
{
	if(main && main->getBackgroundLabel())
	{
		main->getBackgroundLabel()->setSpeed(speed);
	}
}

void toBackground::setTip(QString tip)
{
	if(main && main->getBackgroundLabel())
	{
		main->getBackgroundLabel()->setTip(tip);
	}
}
