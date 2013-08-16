
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

#ifndef TOANALYZE_H
#define TOANALYZE_H


#include "core/toeventquery.h"
#include "core/totool.h"

#include <QtGui/QAction>
#include <QtGui/QLabel>
#include <QtGui/QToolButton>

class QComboBox;
class QMenu;
class QSpinBox;
class QTabWidget;
class QTimer;
class QLabel;
class toResultSchema;
class toResultPlan;
class toResultTableView;
class toWorksheetStatistic;

class toAnalyze : public toToolWidget
{
    Q_OBJECT;
public:
    toAnalyze(QWidget *parent, toConnection &connection);

    toWorksheetStatistic *worksheet(void);

    static void createTool(void);

    void startQuery(toEventQuery * q);

    QStringList getSQL(void);

public slots:
    virtual void slotDisplaySQL(void);
    virtual void slotChangeOperation(int);
    virtual void slotExecute(void);
    virtual void slotPoll(toEventQuery *);
    virtual void slotQueryDone(toEventQuery *);
    virtual void slotStop(void);
    virtual void slotRefresh(void);
    virtual void slotSelectPlan(void);
    virtual void slotFillOwner(void);
    virtual void slotDisplayMenu(QMenu *);
    virtual void slotWindowActivated(toToolWidget*) {};
private:
    QTabWidget           *Tabs;
    toResultTableView    *Statistics;
    QComboBox            *Analyzed;
    toResultSchema       *Schema;
    QComboBox            *Type;
    QComboBox            *Operation;
    QComboBox            *For;
    QSpinBox             *Sample;
    QSpinBox             *Parallel;
    QLabel               *Current;
    QToolButton          *Stop;
    toResultTableView    *Plans;
    toResultPlan         *CurrentPlan;
    toWorksheetStatistic *Worksheet;
    QList<toEventQuery *> Running;
    QStringList           Pending;
};

#endif
