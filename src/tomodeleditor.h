/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOMEMOEDITOR_H
#define TOMEMOEDITOR_H

#include "config.h"

#include <qdialog.h>
#include <QLabel>
#include <QToolBar>
#include <QModelIndex>

class QCheckBox;
class QToolBar;
class toMarkedText;
class QAbstractItemModel;


/**
 * A dialog for displaying and editing a row and column of a model.
 *
 */
class toModelEditor : public QDialog
{
    Q_OBJECT;

    // Editor of widget
    toMarkedText *Editor;

    QToolBar  *Toolbar;
    QLabel    *Label;
    QCheckBox *NullCheck;

    bool                Editable;
    QModelIndex         Current;
    QAbstractItemModel *Model;

private slots:
    void openFile(void);
    void saveFile(void);
    void readSettings(void);
    void writeSettings(void) const;
    virtual void setNull(bool);

protected:
    QToolBar *toolbar()
    {
        return Toolbar;
    }
    toMarkedText *editor()
    {
        return Editor;
    }
    QLabel *label()
    {
        return Label;
    }

public:
    /**
     * Create this editor.
     *
     * @param parent Parent widget.
     * @param model data source
     * @param row Optional location specifier, pass on in @ref changeData call.
     * @param col Optional location specifier, pass on in @ref changeData call.
     * @param sql Use SQL syntax highlighting of widget.
     * @param modal Display in modal dialog
     */
    toModelEditor(QWidget *parent,
                  QAbstractItemModel *model,
                  QModelIndex current,
                  bool sql = false,
                  bool modal = false);

public slots:
    /** Save changes and close.
     */
    virtual void store(void);

    /** Goto first column.
     */
    virtual void firstColumn();

    /** Goto next column.
     */
    virtual void nextColumn();

    /** Goto previous column.
     */
    virtual void previousColumn();

    /** Goto last column.
     */
    virtual void lastColumn();

    /** Change position in whatever it is your displaying.
     */
    virtual void changePosition(QModelIndex index);

    /** Change value of editor.
     */
    virtual void setText(const QString &);
};

#endif
