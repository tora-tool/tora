
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

#ifndef TOBROWSERSCHEMAWIDGET_H
#define TOBROWSERSCHEMAWIDGET_H

#include <QTreeView>

#include "toresulttableview.h"

class toCodeModel;


/*! \brief A base class for "object schema browsers" used in toBrowser m_objectsMap.
It's inherited in toBrowserSchemaTableView and toBrowserSchemaCodeBrowser.
toBrowser is using toBrowserSchemaTableView as a wrapper for toResultTableView
and toBrowserSchemaCodeBrowser as a common widget with QTreeView with its toCodeModel.
All inherited classes *must* implement pure virtual method to behave correctly
in the toBrowser.
\author Petr Vanek <petr@scribus.info>
*/
class toBrowserSchemaBase
{
    public:
        toBrowserSchemaBase(){};
        virtual ~toBrowserSchemaBase(){};
        //! \brief Return currently selected object name.
        virtual QString objectName() = 0;
        //! \brief Reset widget data depending on new schema and/or filter.
        virtual void changeParams(const QString & schema, const QString & filter) = 0;
};


/*! \brief Wrapper for toResultTableView to display plain SQL results in the
toBrowser as a table.
\author Petr Vanek <petr@scribus.info>
*/
class toBrowserSchemaTableView : public toResultTableView,
                                 public toBrowserSchemaBase
{
    Q_OBJECT

    public:
        toBrowserSchemaTableView(QWidget * parent = 0);
        QString objectName();
        void changeParams(const QString & schema, const QString & filter);
};


/*! \brief This widget implements DB Code Objects tree structure for
toBrowser. All Code objects are displayed as a tree items with
improved functionality of its leafs (e.g. for packages etc.).
QTreeView is joined with toCodeModel for display.
\author Petr Vanek <petr@scribus.info>
*/
class toBrowserSchemaCodeBrowser : public QTreeView,
                                   public toBrowserSchemaBase
{
    Q_OBJECT

    public:
        toBrowserSchemaCodeBrowser(QWidget * parent = 0);
        QString objectName();
        //! \brief Returns uppercased item's db object type.
        QString objectType();
        void changeParams(const QString & schema, const QString & filter);

    private:
        toCodeModel * m_model;
};

#endif
