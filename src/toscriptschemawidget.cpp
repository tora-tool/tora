
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

#include "utils.h"
#include "tosql.h"
#include "tomain.h"
#include "toconnection.h"
#include "toscripttreemodel.h"
#include "toscripttreeitem.h"
#include "toscriptschemawidget.h"
#include "toworkingwidget.h"


static toSQL SQLSchemasMySQL("toScriptSchemaWidget:ExtractSchema",
                             "SHOW DATABASES",
                             "Get usernames available in database, must have same columns",
                             "3.23",
                             "MySQL");

static toSQL SQLSchemas("toScriptSchemaWidget:ExtractSchema",
                        "SELECT username FROM sys.all_users ORDER BY username",
                        "");


toScriptSchemaWidget::toScriptSchemaWidget(QWidget * parent)
    : QWidget(parent)
{
    setupUi(this);
    WorkingWidget->hide();
    WorkingWidget->setType(toWorkingWidget::NonInteractive);

    Model = new toScriptTreeModel(this);
    ObjectsView->setModel(Model);

    std::list<QString> cons = toMainWidget()->connections();
    while (cons.size() > 0)
        ConnectionComboBox->addItem(toShift(cons));

    connect(ConnectionComboBox, SIGNAL(currentIndexChanged(int)),
             this, SLOT(changeConnection(int)));
    connect(SchemaComboBox, SIGNAL(currentIndexChanged(int)),
             this, SLOT(changeSchema(int)));

    connect(toMainWidget(), SIGNAL(addedConnection(const QString &)),
            this, SLOT(addConnection(const QString &)));
    connect(toMainWidget(), SIGNAL(removedConnection(const QString &)),
            this, SLOT(delConnection(const QString &)));

    connect(ObjectsView->selectionModel(),
             SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
             this,
             SLOT(objectsView_selectionChanged(const QItemSelection &, const QItemSelection &)));
};

void toScriptSchemaWidget::setTitle(const QString & text)
{
    GroupBox->setTitle(text);
}

void toScriptSchemaWidget::changeConnection(int val)
{
//     qDebug() << "toScriptSchemaWidget::changeConnection" << val;
    if (val == -1)
        return;

    SchemaComboBox->blockSignals(true);

    SchemaComboBox->clear();
    toConnection &conn = toMainWidget()->connection(connectionString());
    toQList schema = toQuery::readQuery(conn, SQLSchemas);
    SchemaComboBox->addItem(tr("All"));
    while (schema.size() > 0)
        SchemaComboBox->addItem(toShift(schema));

    int ix = SchemaComboBox->findText(conn.user().toUpper(), Qt::MatchExactly);
    if (ix == -1)
        ix = 0;
    SchemaComboBox->blockSignals(false);
    SchemaComboBox->setCurrentIndex(ix);
}

QString toScriptSchemaWidget::connectionString()
{
    return ConnectionComboBox->currentText();
}

void toScriptSchemaWidget::setConnectionString(const QString & c)
{
    int i = ConnectionComboBox->findText(c);
    if (i == -1)
        return;
    ConnectionComboBox->setCurrentIndex(i);
    changeConnection(i);
}

void toScriptSchemaWidget::delConnection(const QString &name)
{
    ConnectionComboBox->removeItem(ConnectionComboBox->findText(name));
}

void toScriptSchemaWidget::addConnection(const QString & name)
{
    ConnectionComboBox->addItem(name);
}

void toScriptSchemaWidget::changeSchema(int val)
{
//     qDebug() << "toScriptSchemaWidget::changeSchema" << val;
    if (val == -1)
        return;
    QString schema;
    if (val != 0)
        schema = SchemaComboBox->itemText(val);

    setEnabled(false);
    WorkingWidget->show();
    QCoreApplication::processEvents();
    Model->setupModelData(ConnectionComboBox->currentText(), schema);
    WorkingWidget->hide();
    setEnabled(true);
}

QItemSelectionModel * toScriptSchemaWidget::objectList()
{
    return ObjectsView->selectionModel();
}

void toScriptSchemaWidget::objectsView_selectionChanged(const QItemSelection & selected,
                                                         const QItemSelection & deselected)
{
    // all other widgets are disabled until it ends
    setEnabled(false);
    WorkingWidget->show();

    foreach (QModelIndex i, selected.indexes())
        subSelectionChanged(i);

    WorkingWidget->hide();
    setEnabled(true);
}

void toScriptSchemaWidget::subSelectionChanged(QModelIndex ix)
{
    QCoreApplication::processEvents();

    QItemSelectionModel * sel = ObjectsView->selectionModel();
    toScriptTreeItem * item = static_cast<toScriptTreeItem*>(ix.internalPointer());
    toScriptTreeItem * subItem;
    QModelIndex subIx;

    for (int i = 0; i < item->childCount(); ++i)
    {
        subIx = Model->index(i, 0, ix);
        subItem = static_cast<toScriptTreeItem*>(subIx.internalPointer());

        if (!sel->isSelected(subIx))
            sel->select(subIx, QItemSelectionModel::Select);

        if (subItem->childCount() > 0)
            subSelectionChanged(subIx);
    }
}

