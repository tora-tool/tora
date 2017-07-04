
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

class toSqlText;
class QAbstractItemModel;
class toSearchReplace;

#include "core/toeditwidget.h"
#include "editor/toeditglobals.h"
#include "core/toconfiguration.h"
#include "core/toeditorconfiguration.h"
#include "core/utils.h"

#include <QWidget>
#include <QAbstractItemView>
#include <QtCore/QSet>

namespace Views
{

/** A tora "Highlighted sql source" version of the @ref QPlainTextEdit widget.
 */
class toSqlTextView : public QAbstractItemView, public toEditWidget
{
    Q_OBJECT;
public:

    toSqlTextView(QWidget *parent = 0, const char *name = 0);

    void setReadOnly(bool ro);
    void setText(const QString &t);
    void setFilename(const QString &f);

    void editCopy(void) override;
    void editSelectAll(void) override;
    bool editSave(bool) override;
    bool editOpen(const QString&) override { return false; }
    void editPrint() override {}
    void editUndo() override {}
    void editRedo() override {}
    void editCut() override {}
    void editPaste() override {}
    void editReadAll() override {}
    QString editText() override;

    bool searchNext() override;
    void searchReplace() override {};

    void setSqlColumn(unsigned column) { m_model_column = column; };

    void setModel(QAbstractItemModel *model);

    void setContextMenuPolicy(Qt::ContextMenuPolicy policy);

    /* QAbstractItemModel method overrides */
    QRect visualRect(const QModelIndex &index) const override { return QRect(); };
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override {};
    QModelIndex indexAt(const QPoint &point) const override { return QModelIndex(); };
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) override { return QModelIndex(); };
    int horizontalOffset() const override { return 0; };
    int verticalOffset() const override { return 0; };
    bool isIndexHidden(const QModelIndex &index) const override { return false; };
    void setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command) override {};
    QRegion visualRegionForSelection(const QItemSelection &selection) const override { return QRegion(); };

protected:
    void focusInEvent (QFocusEvent *e) override;

protected slots:
    void setEditorFocus();
    void handleSearching(Search::SearchFlags flags);

    // handle just some of model's signals
    void modelReset();
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsRemoved(const QModelIndex &parent, int first, int last);
private:
    toSqlText *m_view;
    toSearchReplace *m_search;
    QString m_filename;
    QAbstractItemModel *m_model;
    QSet<int> m_lines;
    unsigned m_model_column;
};

template<typename _T>
class DefaultSqlTextViewPolicy
{
    private:
        typedef _T Traits;
        typedef typename Traits::View View;
    public:
        void setup(View* pView);
};

template<typename Traits>
void DefaultSqlTextViewPolicy<Traits>::setup(View* pView)
{
    pView->setContextMenuPolicy( (Qt::ContextMenuPolicy) Traits::ContextMenuPolicy);
    QFont fixed(Utils::toStringToFont(toConfigurationNewSingle::Instance().option(ToConfiguration::Editor::ConfCodeFont).toString()));
    pView->setFont(fixed);
}

}
