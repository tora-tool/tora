
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

#ifndef TOOUTPUT_H
#define TOOUTPUT_H


#include "widgets/totoolwidget.h"
#include "editor/tomarkededitor.h"
#include "core/tosql.h"
#include "core/toconfenum.h"

#include <QtGui/QCloseEvent>
#include <QAction>
#include <QMenu>
#include <QToolBar>

class QComboBox;
class toConnection;
class toResultView;

namespace ToConfiguration
{
    class Output : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Output() : ConfigContext("Output", ENUM_REF(Output,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                PollingInterval = 12000 // #define CONF_POLLING
                                  , SourceTypeInt         // #define CONF_LOG_TYPE
                , LogUser               // #define CONF_LOG_USER
            };
            QVariant defaultValue(int option) const;
    };
};

class toOutput : public toToolWidget
{
        Q_OBJECT;

    public:
        toOutput(QWidget *parent, toConnection &connection, bool enabled = true);

        virtual ~toOutput();

        bool enabled(void);

        void insertLine(const QString &str);

    public slots:
        void clear(void);
        virtual void refresh(void);
        virtual void disable(bool);
        void changeRefresh(const QString &str);
        virtual void slotWindowActivated(toToolWidget *widget);
        void toggleMenu();

    private:
        QMenu        *ToolMenu;
        QComboBox    *Refresh;
        QAction      *refreshAct;
        QAction      *enableAct;
        QAction      *clearAct;

        void poll(void);

    protected:
        toMarkedEditor *Output;
        QToolBar     *Toolbar;

        virtual void closeEvent(QCloseEvent *);
};

class toLogOutput : public toOutput
{
        Q_OBJECT;

        QComboBox *Type;
        toResultView *Log;

    public:
        toLogOutput(QWidget *parent, toConnection &connection);

    public slots:
        virtual void refresh(void);
        void changeType(void);
};

#endif
