
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

#include <QAbstractItemView>

class toTableDetailView : public QAbstractItemView
{
    Q_OBJECT
public:
    toTableDetailView(QWidget *parent = 0);

    QModelIndex indexAt( const QPoint &point ) const override;
    void scrollTo( const QModelIndex &index, ScrollHint hint = EnsureVisible ) override;
    QRect visualRect( const QModelIndex &index ) const override;

protected:
    int horizontalOffset() const override;
    bool isIndexHidden( const QModelIndex &index ) const override;
    QModelIndex moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers modifiers ) override;
    void setSelection( const QRect &rect, QItemSelectionModel::SelectionFlags flags ) override;
    int verticalOffset() const override;
    QRegion visualRegionForSelection( const QItemSelection &selection ) const override;

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )override;

private:
    void updateText();
    QLabel *label;
};

