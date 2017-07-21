
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

#include "ui_tosearchreplaceui.h"
#include "editor/toeditglobals.h"
#include "core/tohelpcontext.h"

#include <QWidget>

class toEditWidget;

class toSearchReplace : public QWidget
    , public Ui::toSearchReplaceUI
    , public toHelpContext
{
    Q_OBJECT;

public:
    toSearchReplace(QWidget *parent);

    QString searchText();
    QString replaceText();

    void setReadOnly(bool ro);

    void activate();
    void registerEdit(toEditWidget*);
    void unregisterEdit(toEditWidget*);

public slots:
    void act_searchNext();
    void act_replace();

private slots:
    void act_replaceAll();
    void act_searchChanged(const QString & text);
    void displayHelp();
    void act_searchPrevious();

protected:
    void showEvent(QShowEvent * e) override;
    void closeEvent(QCloseEvent *e) override;

    bool eventFilter(QObject *target, QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    Search::SearchFlags sharedFlags();
    toEditWidget* m_editWidget;
};
