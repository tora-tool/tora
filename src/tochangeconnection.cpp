/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

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
                        toMainWidget()->windowActivated(tool);
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
