
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

#include <QWidget>

#include "tools/tolinechart.h"
#include "tools/tobarchart.h"

#include <map>
#include <list>

class toEventQuery;

class toTuningFileIO : public QWidget
{
    Q_OBJECT
public:
    toTuningFileIO(QWidget *parent = 0);
    ~toTuningFileIO();

public slots:
    void refresh(void);
    void changeCharts(int val);
    void changeConnection(void);
    void receiveData(toEventQuery*);

private slots:
    void queryDone(toEventQuery *, unsigned long);

private:
    QWidget *TablespaceTime;
    QWidget *TablespaceReads;
    QWidget *FileTime;
    QWidget *FileReads;

    QString LastTablespace;
    double TblReads;
    double TblWrites;
    double TblReadBlk;
    double TblWriteBlk;
    double TblAvg;
    double TblMin;
    double TblMaxRead;
    double TblMaxWrite;
    toEventQuery *Query;

    time_t CurrentStamp;
    time_t LastStamp;
    QMap<QString, toBarChart *> ReadsCharts;
    QMap<QString, toLineChart *> TimeCharts;
    std::map<QString, std::list<double> > LastValues;

    void saveSample(const QString &, const QString &,
                    double reads, double writes,
                    double readBlk, double writeBlk,
                    double avgTim, double minTim,
                    double maxRead, double maxWrite);

    void allocCharts(const QString &);
};
