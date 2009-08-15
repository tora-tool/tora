
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

#ifndef TODOCKLET_H
#define TODOCKLET_H

#include "config.h"

#include <QMap>
#include <QAction>
#include <QWidget>
#include <QDockWidget>
#include <QRect>

class toDocklet;

class toDockletHolder
{
public:
    virtual ~toDockletHolder(){};
    virtual toDocklet* docklet(QWidget *parent = 0,
                               Qt::WindowFlags flags = 0) = 0;
};


class toDocklet : public QDockWidget
{
    Q_OBJECT;

public:
    explicit toDocklet(const QString &title,
                       QWidget *parent = 0,
                       Qt::WindowFlags flags = 0);
    virtual ~toDocklet() {}


    /**
     * Get the icon for this docklet
     *
     */
    virtual QIcon icon() const = 0;


    /**
     * Get the docklet's name
     *
     */
    virtual QString name() const = 0;


    // ---------------------------------------- static

    /**
     * Used by REGISTER_VIEW to init docklets
     *
     */
    static toDockletHolder* registerDocklet(QString name, toDockletHolder *holder);


    /**
     * Get a list of available docklets
     *
     */
    static QList<toDocklet *> docklets(void);


    /**
     * Find the docklet for the given name
     *
     */
    static toDocklet* docklet(QString name);


signals:
    /**
     * Use this signal in a subclass after a child widget has been
     * activated. toDocklet will handle closing the popup, if any.
     *
     */
    void activated(void);


    /**
     * Emitted when the popup is closed
     *
     */
    void popupVisibleToggled(bool shown);


    /**
     * Emitted when docklet moves to new location
     *
     */
    void dockletLocationChanged(toDocklet *let, Qt::DockWidgetArea area);


public slots:
    /**
     * Show or hide popup. This is different from QDockWidget::floating
     *
     */
    void showPopup(bool shown);


protected:
    virtual bool event(QEvent *event);


private slots:
    void childActivated();
    void handleDockletLocationChanged(Qt::DockWidgetArea area);


private:
    bool       isPopup;
};


/**
 * Registers views statically. Simply place:
 *
 * REGISTER_VIEW("some untranslated, unique name", myAwesomeView);
 *
 * call in the .cpp file of your view definition. Uses toDockletHolder
 * to get around limitations with static initialization and QWidgets.
 *
 */
#define REGISTER_VIEW(NAME, TYPE)                           \
class TYPE##Holder : public toDockletHolder                 \
{                                                           \
public:                                                     \
    virtual toDocklet* docklet(QWidget *parent = 0,         \
                               Qt::WindowFlags flags = 0)   \
    {                                                       \
        static TYPE *let = new TYPE(parent, flags);         \
        return let;                                         \
    }                                                       \
};                                                          \
toDockletHolder* TYPE ## _myPrototype =                     \
    toDocklet::registerDocklet(NAME, new TYPE##Holder());

#endif
