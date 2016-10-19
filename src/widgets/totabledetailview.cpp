
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

#include "widgets/totabledetailview.h"

#include <QScrollBar>
#include <QGridLayout>
#include <QLabel>

toTableDetailView::toTableDetailView (QWidget* parent)
    : QAbstractItemView(parent)
{
  QGridLayout *layout = new QGridLayout( this->viewport() );
  label = new QLabel();

  layout->addWidget( label, 0, 0 );

  label->setAlignment( Qt::AlignCenter );
  label->setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );
  label->setText( tr("<i>No data.</i>") );
}

QRect toTableDetailView::visualRect( const QModelIndex &index ) const
{
  if( selectionModel()->selection().indexes().count() != 1 )
    return QRect();

  if( currentIndex() != index )
    return QRect();

  return rect();
}

QRegion toTableDetailView::visualRegionForSelection( const QItemSelection &selection ) const
{
  if( selection.indexes().count() != 1 )
    return QRect();

  if( currentIndex() != selection.indexes()[0] )
    return QRect();

  return rect();
}

bool toTableDetailView::isIndexHidden( const QModelIndex &index ) const
{
  if( selectionModel()->selection().indexes().count() != 1 )
    return true;

  if( currentIndex() != index )
    return true;

  return false;
}

QModelIndex toTableDetailView::indexAt( const QPoint &point ) const
{
  if( selectionModel()->selection().indexes().count() != 1 )
    return QModelIndex();

  return currentIndex();
}

int toTableDetailView::horizontalOffset() const
{
  return horizontalScrollBar()->value();
}

int toTableDetailView::verticalOffset() const
{
  return verticalScrollBar()->value();
}

QModelIndex toTableDetailView::moveCursor( CursorAction cursorAction, Qt::KeyboardModifiers modifiers )
{
  return currentIndex();
}

void toTableDetailView::setSelection( const QRect &rect, QItemSelectionModel::SelectionFlags flags )
{
  // do nothing
}

void toTableDetailView::scrollTo( const QModelIndex &index, ScrollHint hint )
{
  // cannot scroll
}

void toTableDetailView::dataChanged( const QModelIndex &topLeft, const QModelIndex &bottomRight )
{
  updateText();
}

void toTableDetailView::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  updateText();
}

void toTableDetailView::updateText()
{
  switch( selectionModel()->selection().indexes().count() )
  {
    case 0:
      label->setText( tr("<i>No data.</i>") );
      break;

    case 1:
      label->setText( model()->data( currentIndex(), Qt::UserRole).toString() );
      break;

    default:
      label->setText( tr("<i>Too many items selected.<br>Can only show one item at a time.</i>") );
      break;
  }
}
