//
// C++ Implementation: toBackgroundLabel
//
// Copyright: See COPYING file that comes with this distribution
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
