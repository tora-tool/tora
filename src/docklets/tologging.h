
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

#include "core/todocklet.h"
#include "core/toeditwidget.h"
#include "editor/toeditglobals.h"

#include "loki/Singleton.h"

#include <QListView>
#include <QPlainTextEdit>

class toToolWidget;

class toLoggingDocklet : public toDocklet , public toEditWidget
{
    Q_OBJECT;
    typedef toDocklet super;
public:
    toLoggingDocklet(QWidget *parent = 0);

    /**
     * Get the action icon name for this docklet
     */
    QIcon icon() const override;

    /**
     * Get the docklet's name
     */
    QString name() const override;

    // TODO/FIXME: maybe some of things below do have sense...
    bool editOpen(const QString&) override { return false; };
    bool editSave(bool)  override { return false;};
    void editUndo()      override {};
    void editRedo()      override {};
    void editCut()       override {};
    void editCopy()      override;
    void editPaste()     override {};
    void editSelectAll() override;
    void editReadAll()   override {};
    QString editText()   override { return ""; };

    bool handleSearching(QString const& search, QString const& replace, Search::SearchFlags flags) override;

protected:
    /** Re-implented from toEditWidget */
    void focusInEvent (QFocusEvent *e) override;
    /** Re-implented from toEditWidget */
    void focusOutEvent (QFocusEvent *e) override;

private:
    QPlainTextEdit &log;
};

