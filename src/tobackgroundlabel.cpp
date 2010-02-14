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
// C++ Implementation: toBackgroundLabel
//
#include "tobackgroundlabel.h"

#include "utils.h"

#include <qtooltip.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QLabel>

#include "tomain.h"
#include "toconnection.h"

QMovie* toBackgroundLabel::Animation = 0;


toBackgroundLabel::toBackgroundLabel(QWidget *parent) : QLabel(parent)
{
    Animation = new QMovie(":/icons/backgroundlabel.gif");
    setMovie(Animation);
}

void toBackgroundLabel::unpause()
{
    Animation->setPaused(false);
}

void toBackgroundLabel::pause()
{
    Animation->setPaused(true);
}

void toBackgroundLabel::setTip(QString tip)
{
    setToolTip(tip);
}

void toBackgroundLabel::setSpeed(int Running)
{
    Animation->setSpeed(std::min(Running, 1)*100);
}

void toBackgroundLabel::mouseReleaseEvent(QMouseEvent *e)
{
    try
    {
        if (e->button() == Qt::LeftButton)
        {
            QString str = "Currently running SQL:\n\n";
            std::list<QString> conns = toMainWidget()->connections();
            for (std::list<QString>::iterator i = conns.begin();i != conns.end();i++)
            {
                toConnection &conn = toMainWidget()->connection(*i);
                std::list<QString> running = conn.running();
                if (running.begin() != running.end())
                {
                    str += "On connection " + conn.description(false) + ":\n\n";
                }
                int num = 1;
                for (std::list<QString>::iterator j = running.begin();j != running.end();j++)
                {
                    str += QString::number(num) + ". " + (*j) + "\n\n";
                    num++;
                }
            }
            toStatusMessage(str);
            e->accept();
        }
        else
            QLabel::mouseReleaseEvent(e);
    }
    TOCATCH
}
