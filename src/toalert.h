
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

#ifndef TOALERT_H
#define TOALERT_H

#include "config.h"

#include <list>

#include <qtimer.h>
#include <QMenu>

#include "totool.h"
//#include "tothread.h"

class QComboBox;
class QLineEdit;
class toListView;
class toConnection;

class toAlert : public toToolWidget
{
    Q_OBJECT;

class pollTask : public toTask
    {
        toAlert &Parent;

    public:
        pollTask(toAlert &parent) : Parent(parent)
        { }

        virtual void run(void);
    };

    toListView   *Alerts;
    QComboBox    *Registered;
    QLineEdit    *Name;
    QLineEdit    *Message;
    QTimer        Timer;
    toConnection &Connection;
    toConnection *SendConnection;

    QMenu        *ToolMenu;
    QAction *addAct;
    QAction *removeAct;
    QAction *memoEditAct;
    QAction *sendAct;

    toLock             Lock;
    toSemaphore        Semaphore;
    std::list<QString> AddNames;
    std::list<QString> DelNames;
    std::list<QString> Names;
    QString            Error;

    std::list<QString> NewAlerts;
    std::list<QString> NewMessages;

    std::list<QString> SendAlerts;
    std::list<QString> SendMessages;

    enum
    {
        Started,
        Quit,
        Done
    } State;

protected:
    void closeEvent(QCloseEvent *event);

public:
    toAlert(QWidget *parent, toConnection &connection);
    virtual ~toAlert();

    friend class pollTask;

public slots:
    virtual void poll(void);
    virtual void send(void);
    virtual void memo(void);
    virtual void changeMessage(int, int, const QString &str);
    virtual void add(void);
    virtual void remove(void);
    virtual void windowActivated(QMdiSubWindow *widget);
};

#endif
