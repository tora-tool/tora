
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

#include <QCoreApplication>

#include "tobrowserschemawidget.h"
#include "tocodemodel.h"
#include "utils.h"


toBrowserSchemaTableView::toBrowserSchemaTableView(QWidget * parent)
    : toResultTableView(true, false, parent),
      toBrowserSchemaBase()
{
}

QString toBrowserSchemaTableView::objectName()
{
    return selectedIndex(1).data(Qt::EditRole).toString();
}

void toBrowserSchemaTableView::changeParams(const QString & schema, const QString & filter)
{
    toResultTableView::changeParams(schema, filter);
}


toBrowserSchemaCodeBrowser::toBrowserSchemaCodeBrowser(QWidget * parent)
    : QTreeView(parent),
      toBrowserSchemaBase()
{
    m_model = new toCodeModel(this);
    setModel(m_model);

    setAlternatingRowColors(true);

    connect(this, SIGNAL(clicked(const QModelIndex &)),
             m_model, SLOT(addChildContent(const QModelIndex &)));
    connect(m_model, SIGNAL(dataReady()), this, SLOT(expandAll()));
}

QString toBrowserSchemaCodeBrowser::objectName()
{
    toCodeModelItem *item = static_cast<toCodeModelItem*>(currentIndex().internalPointer());
    if (!item || item->type() == "NULL")
        return "";

    if (item->type() == "PACKAGE"
        || item->type() == "FUNCTION"
        || item->type() == "PROCEDURE"
        || item->type() == "TYPE")
    {
        return item->display();
    }
    if (item->type() == "SPEC" || item->type() == "BODY")
        return item->parent()->display();

    return "";
}

QString toBrowserSchemaCodeBrowser::objectType()
{
    toCodeModelItem *item = static_cast<toCodeModelItem*>(currentIndex().internalPointer());
    if (item && item->parent())
    {
        return item->type();
    }
    return "";
}

void toBrowserSchemaCodeBrowser::changeParams(const QString & schema, const QString & filter)
{
    // WARNING: QCoreApplication::processEvents() is mandatory here
    //          to prevent random crashing on parent-tab widget switching
    //          in toBrowser. Dunno why...
    QCoreApplication::processEvents();
    m_model->refresh(toCurrentConnection(this), schema);

}
