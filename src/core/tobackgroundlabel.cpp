
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

#include "core/tobackgroundlabel.h"
#include "core/utils.h"
#include "core/toconnectionregistry.h"
#include "core/toconnection.h"

#include <QtGui/QMovie>
#include <QtGui/QMouseEvent>

QMovie* toBackgroundLabel::Animation = 0;

toBackgroundLabel::toBackgroundLabel(QWidget *parent) : QLabel(parent)
{
	if (Animation == NULL)
		Animation = new QMovie(":/icons/backgroundlabel.gif");
	setMovie(Animation);

	// Start animation and stop it again
	// this make the image displayed immediately
	movie()->start();
	Animation->setPaused(true);
}

toBackgroundLabel::~toBackgroundLabel()
{}

void toBackgroundLabel::unpause()
{
    Animation->setPaused(false);
}

void toBackgroundLabel::pause()
{
    Animation->setPaused(true);
}

void toBackgroundLabel::setTip(const QString & tip)
{
    setToolTip(tip);
}

void toBackgroundLabel::setSpeed(int Running)
{
    Animation->setSpeed( (::std::min)(Running, 1) * 100);
}

void toBackgroundLabel::mouseReleaseEvent(QMouseEvent *e)
{
    try
    {
        if (e->button() == Qt::LeftButton)
        {
            QString str = "Currently running SQL:\n\n";
            QList<toConnection*> const& conns = toConnectionRegistrySing::Instance().connections();
            foreach (toConnection const * conn, conns)
            {
                QList<QString> running = conn->running();
                if (running.empty())
                	continue;

                str += "On connection " + conn->description(false) + ":\n\n";
                int num = 1;
                foreach (QString const & q, running)
                {
                    str += QString::number(num) + ". " + q + "\n\n";
                    num++;
                }
            }
            Utils::toStatusMessage(str);
            e->accept();
        }
        else
            QLabel::mouseReleaseEvent(e);
    }
    TOCATCH
}
