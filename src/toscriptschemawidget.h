
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2008 Numerous Other Contributors
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

#ifndef TOSCRIPTSCHEMAWIDGET_H
#define TOSCRIPTSCHEMAWIDGET_H

#include "ui_toscriptschemawidgetui.h"

class toScriptTreeModel;


/*! \brief Schema browse widget for toScript tool.
See Source and Destination objects in the toscriptui.ui.
It reads/handles DB objects into proper tree structure.
Items in the tree are selectable by user - and its selection
is used for toScript export/comparation.
*/
class toScriptSchemaWidget : public QWidget, public Ui::toScriptSchemaWidget
{
    Q_OBJECT

    public:
        toScriptSchemaWidget(QWidget * parent = 0);
        ~toScriptSchemaWidget(){};

        //! Set the main group box title.
        void setTitle(const QString & text);

        //! Create commin string list with all selected objects for given QTreeView
        QItemSelectionModel * objectList();

        //! Return current connection string
        QString connectionString();
        /*! Set the connection string from outside of this widget.
        \param c QString which will be set when its already in the
        ConnectionComboBox
        */
        void setConnectionString(const QString & c);

    private:
        //! Model for ObjectView QTreeView
        toScriptTreeModel * Model;
        //! Select ObjectView's child items in recursion
        void subSelectionChanged(QModelIndex ix);

    private slots:
        /*! Handle changes in ConnectionComboBox.
        Resets the SchemaComboBox content too.
        */
        void changeConnection(int val);
        /*! Handle changes in SchemaComboBox.
        Resets the Model too. */
        void changeSchema(int val);

        //! Add new connection from toMain
        void addConnection(const QString &name);
        //! Delete a connection as requested from toMain
        void delConnection(const QString &name);

        void objectsView_selectionChanged(const QItemSelection &,
                                           const QItemSelection &);
};

#endif
