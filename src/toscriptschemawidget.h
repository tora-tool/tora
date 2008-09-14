/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
