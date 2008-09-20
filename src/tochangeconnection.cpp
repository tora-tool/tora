
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

    setFocusPolicy(Qt::NoFocus);
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
