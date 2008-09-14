/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOMEMOEDITOR_H
#define TOMEMOEDITOR_H

#include "config.h"

#include <qdialog.h>
#include <QLabel>
#include <QToolBar>

class QCheckBox;
class QLabel;
class toTreeWidgetItem;
class QToolBar;
class toListView;
class toMarkedText;
class toResultContentEditor;

/** Displays an text in a widget which can optionally be modified and saved back.
 */

class toMemoEditor : public QDialog
{
    Q_OBJECT

    /** Editor of widget
     */
    toMarkedText *Editor;
    /** Row of this field
     */
    int Row;
    /** Column of this field
     */
    int Col;
    QToolBar *Toolbar;
    QLabel *Label;
    QCheckBox *Null;

    toListView *listView(void);
private slots:
    void openFile(void);
    void saveFile(void);
    void readSettings(void);
    void writeSettings(void) const;
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
    /** Create this editor. If row and col is specified, editing is posible.
     * @param parent Parent widget.
     * @param data Data of editor.
     * @param row Optional location specifier, pass on in @ref changeData call.
     * @param col Optional location specifier, pass on in @ref changeData call.
     * @param sql Use SQL syntax highlighting of widget.
     * @param modal Display in modal dialog
     * @param navigation Display navigation buttons
     */
    toMemoEditor(QWidget *parent, const QString &data, int row = -1, int col = -1,
                 bool sql = false, bool modal = false, bool navigation = false);

    /** Get editor text.
     * @return String of editor.
     */
    QString text(void);
    /** Current row being edited.
     */
    int row() const
    {
        return Row;
    }
    /** Current column being edited.
     */
    int column() const
    {
        return Col;
    }
signals:
    /** Emitted when data is to be stored back to the creator
     * @param row Pass on from creation.
     * @param col Pass on from creation.
     * @param data New data to save.
     */
    void changeData(int row, int col, const QString &data);
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
    virtual void changePosition(int row, int cols);
    /** Change value of editor.
     */
    virtual void setText(const QString &);
private slots:
    void changeCurrent(toTreeWidgetItem *item);
    virtual void null(bool);
};

#endif
