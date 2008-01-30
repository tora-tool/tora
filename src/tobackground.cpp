//
// C++ Implementation: toBackground
//
// Copyright: See COPYING file that comes with this distribution
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
{
    //createToBackgroundLabel()
    main = toMainWidget();
}

void toBackground::start(int msec)
{
    main = toMainWidget();

    if (!isActive())
    {
        Running++;
        main->getBackgroundLabel()->unpause();
    }
    main->getBackgroundLabel()->setSpeed(std::min(Running, 1)*100);
    //Animation->setSpeed(std::min(Running, 1)*100);
    if (Running > 1)
    {
        main->getBackgroundLabel()->setTip(tr("%1 queries running in background.").arg(Running));
        //QToolTip::add(Label, tr("%1 queries running in background.").arg(Running));
    }
    else
    {
        main->getBackgroundLabel()->setTip(tr("One query running in background."));
        //   QToolTip::add(Label, tr("One query running in background."));
    }
    toTimer::start(msec);
}

void toBackground::stop(void)
{
    main = toMainWidget();

    /*if (!Animation)
        init(); */
    if (isActive())
    {
        Running--;
        if (Running == 0 )
            main->getBackgroundLabel()->pause();
        else
            main->getBackgroundLabel()->setSpeed(Running*100);
        if (Running > 1)
            main->getBackgroundLabel()->setTip(tr("%1 queries running in background.").arg(Running));
        else if (Running == 1)
            main->getBackgroundLabel()->setTip("One query running in background.");
        else
            main->getBackgroundLabel()->setTip(tr("No background queries."));

    }
    toTimer::stop();
}

/*void toBackground::init(void)
{
    toMain *main = toMainWidget();
    if (!main || Label)
        return ;

    Label = theToBackgroundLabel(main->statusBar());
    Label->show();
    main->statusBar()->addWidget(Label, 0, true);
    QToolTip::add
        (Label, tr("No background queries."));
}*/

toBackground::~toBackground()
{
    stop();
}
