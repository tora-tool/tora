
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

#include "ui_test18ui.h"
#include "core/tocontextmenu.h"

#include <QtCore/QObject>
#include <QMainWindow>

class toSqlText;
class QAction;
class QMenu;
class QCloseEvent;
class toTableModelPriv;

#include "result/tomvc.h"
#include "widgets/totableview.h"
#include "widgets/totreeview.h"

#if 1
namespace XSessions
{
    struct Traits : public MVCTraits
    {
        static const bool AlternatingRowColorsEnabled = true;
        static const int  ShowRowNumber = NoRowNumber;
        //static const int  ColumnResize = RowColumResize;

        typedef Views::toTableView View;
    };

    class MVC
            : public TOMVC
              <
              ::XSessions::Traits,
              Views::DefaultTableViewPolicy,
              ::DefaultDataProviderPolicy
              >
    {
        Q_OBJECT;
    public:
        typedef TOMVC<
                ::XSessions::Traits,
                 Views::DefaultTableViewPolicy,
                ::DefaultDataProviderPolicy
                  > _s;
        MVC(QWidget *parent) : _s(parent)
        {};
        virtual ~MVC() {};
    };
}

/**
 * A result table displaying information about sessions
 */
class toResultXSessions: public XSessions::MVC
{
    Q_OBJECT;

public:
    toResultXSessions(QWidget *parent, const char *name = NULL)
    : XSessions::MVC(parent)
    {}
};
#endif

class Test18Window : public QMainWindow, Ui::Test18Window, public toContextMenuHandler
{
    Q_OBJECT;

public:
    Test18Window();
    virtual ~Test18Window() {};

protected slots:
    void timerEvent();

protected:
    virtual void closeEvent(QCloseEvent *event);
    void handle(QObject *,QMenu *) override;

    QTimer *timer;
    toTableModelPriv *model;
};
