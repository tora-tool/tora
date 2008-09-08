/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2008 Quest Software, Inc
* Portions Copyright (C) 2008 Other Contributors
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

#ifndef TOWAITEVENTS_H
#define TOWAITEVENTS_H

#include "config.h"
#include "tobackground.h"

#include <list>
#include <map>
#include <algorithm>

#include <QWidget>
#include <QString>

class toTreeWidget;
class toNoBlockQuery;
class toPieChart;
class toResultBar;
class QSplitter;


class toWaitEvents : public QWidget
{
    Q_OBJECT;

    QSplitter * splitter;
    toResultBar *Delta;
    toResultBar *DeltaTimes;
    toPieChart *AbsolutePie;
    toPieChart *DeltaPie;
    toTreeWidget *Types;
    toNoBlockQuery *Query;
    toBackground Poll;

    bool First;
    bool ShowTimes;
    QString Now;
    std::list<QString> Labels;
    time_t LastTime;
    std::list<double> LastCurrent;
    std::list<double> LastTimes;
    std::list<double> Current;
    std::list<double> CurrentTimes;
    std::list<double> Relative;
    std::list<double> RelativeTimes;
    std::list<bool> Enabled;

    int Session;

    std::map<QString, bool> HideMap;

    void setup(int session);
public:
    toWaitEvents(QWidget *parent, const char *name);
    toWaitEvents(int session, QWidget *parent, const char *name);
    ~toWaitEvents();

    void setSession(int session);

    virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
    virtual void importData(std::map<QString, QString> &data, const QString &prefix);
public slots:
    virtual void connectionChanged(void);
    virtual void changeSelection(void);
    virtual void poll(void);
    virtual void refresh(void);
    virtual void start(void);
    virtual void stop(void);
    virtual void changeType(int);
};

#endif
