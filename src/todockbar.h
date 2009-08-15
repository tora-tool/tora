
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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

#ifndef TODOCKBAR_H
#define TODOCKBAR_H

#include "config.h"
#include "topushbutton.h"

#include <QAction>
#include <QToolBar>
#include <QMap>

class toDocklet;


class toDockbarButton : public toPushButton
{
    Q_OBJECT;

public:
    explicit toDockbarButton(const QIcon &icon,
                             const QString &text,
                             QWidget *parent = 0);
};


class toDockbar : public QToolBar
{
    Q_OBJECT;

private:
    Qt::ToolBarArea Area;

    // docklets managed by this dockbar
    QMap<QString, toDockbarButton *> Docklets;

public:
    toDockbar(Qt::ToolBarArea area,
              const QString &title,
              QWidget *parent = 0);

    /**
     * Adds docklet to be managed by this dockbar
     *
     */
    void addDocklet(toDocklet *let, bool show = true);


    /**
     * Removes a docklet managed by this dockbar
     *
     */
    toDocklet* removeDocklet(QString name);


    /**
     * Removes a docklet managed by this dockbar
     *
     */
    toDocklet* removeDocklet(toDocklet *let);


    /**
     * Show or hide all docklets
     *
     */
    void setAllVisible(bool);


    /**
     * Returns true if this dockbar manages this docklet
     *
     */
    bool contains(toDocklet *let);


    /**
     * Returns the area for the dockbar
     *
     */
    inline Qt::ToolBarArea area() const
    {
        return Area;
    }


    /**
     * Save state to QByteArray. Necessary since main window
     * saveState() will only preserve docked QDockWidgets.
     *
     */
    QByteArray saveState() const;


    /**
     * Restore state of hidden docklets, buttons.
     *
     */
    void restoreState(QByteArray);
};

#endif
