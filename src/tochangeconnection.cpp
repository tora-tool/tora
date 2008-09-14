/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include "tochangeconnection.h"
#include "toconnection.h"
#include "tomain.h"
#include "totool.h"

#include <QMenu>
#include <qtooltip.h>
#include <QPixmap>
#include <QToolBar>


#include "icons/changeconnect.xpm"

toChangeConnection::toChangeConnection(QToolBar *parent, const char *name)
        : toPopupButton(QPixmap(const_cast<const char**>(changeconnect_xpm)),
                        tr("Change the connection of the tool."),
                        tr("Change the connection of the tool."),
                        parent, name)
{
    setMenu(new QMenu(this));
    setPopupMode(QToolButton::MenuButtonPopup);
    connect(menu(), SIGNAL(aboutToShow()), this, SLOT(popupMenu()));
    connect(menu(),
            SIGNAL(triggered(QAction *)),
            this,
            SLOT(changeConnection(QAction *)));

    // addWidget manually or the layout gets weird.
    parent->addWidget(this);
}

void toChangeConnection::popupMenu(void)
{
    try
    {
        menu()->clear();
        toConnection &conn = toCurrentConnection(this);
        std::list<QString> cons = toMainWidget()->connections();
        for (std::list<QString>::iterator i = cons.begin(); i != cons.end(); i++)
        {
            if (toCurrentTool(this)->canHandle(toMainWidget()->connection(*i)))
            {
                QAction *act = menu()->addAction(*i);
                act->setCheckable(true);
                if (conn.description() == *i)
                    act->setChecked(true);
                else
                    act->setChecked(false);
            }
        }
    }
    TOCATCH;
}

void toChangeConnection::changeConnection(QAction *act)
{
    // the act of clicking the menu will invert the checked flag
    if (!act->isChecked())
        return;

    std::list<QString> cons = toMainWidget()->connections();

    try
    {
        for (std::list<QString>::iterator i = cons.begin(); i != cons.end(); i++)
        {
            if (act->text() == (*i))
            {

                QWidget *cur = parentWidget();
                while (cur)
                {
                    toToolWidget *tool = dynamic_cast<toToolWidget *>(cur);

                    if (tool)
                    {
                        tool->setConnection(toMainWidget()->connection(*i));
                        toMainWidget()->windowActivated(qobject_cast<QMdiSubWindow*>(tool->parent()));
                        toMainWidget()->changeConnection();
                        return;
                    }
                    cur = cur->parentWidget();
                }
                throw tr("Couldn't find parent connection. Internal error.");
            }
        }
    }
    TOCATCH;

    toStatusMessage("Couldn't find selected connection");
}
