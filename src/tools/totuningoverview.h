
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 * 
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#pragma once

#include "core/tobackground.h"
#include "ui_totuningoverviewui.h"

#include <QtCore/QMap>
#include <QtCore/QList>
//class QAction;
//class QMenu;
//class QComboBox;
//class QWidget;
class QLabel;
//class QTabWidget;
//class toBarChart;
class toConnection;
//class toListView;
//class toEventQuery;
//class toResultItem;
//class toResultLine;
//class toResultLock;
//class toResultTableView;
//class toResultParam;
//class toResultStats;
//class toWaitEvents;
class toResultLine;

class toTuningOverview : public QWidget, public Ui::toTuningOverviewUI
{
    Q_OBJECT;

    QMap<QString, QString> Values;
    QString UnitString;

    toBackground Poll;

    QList<QLabel*> Backgrounds;
    void setupChart(toResultLine *chart, const QString &, const QString &, const toSQL &sql);
    void setValue(QLabel *label, const QString &val);
public:
    toTuningOverview(QWidget *parent = 0, const char *name = 0, toWFlags fl = 0);
    ~toTuningOverview();
#if 0
    void stop(void);
    void start(void);
#endif
    void refresh(toConnection &);
public slots:
    void poll(void);

private:
    //QTimer *timer();
};
