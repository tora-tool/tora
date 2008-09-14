/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOPLSQLEDITOR_H
#define TOPLSQLEDITOR_H


#include "totool.h"
#include "tohighlightedtext.h"
#include "tosqlparse.h"

class QMdiSubWindow;
class QTreeView;
class toCodeModel;
class toConnection;
class toPLSQLWidget;
class toTreeWidget;
class toTreeWidgetItem;


/*! \brief Advanced PL/SQL Editor. It's based on
Debugger code. But it can compile PL/SQL unit in all cases.
Even if is the user without debug grants/roles.

It can compile for "production" and for "warnings" as well.
There is implemented navigation for bugs etc.
*/
class toPLSQLEditor : public toToolWidget
{
    Q_OBJECT;

    // Toolbar
    void createActions(void);

    QComboBox *Schema;
    QAction   *refreshAct;
    QAction   *newSheetAct;
    QAction   *compileAct;
    QAction   *compileWarnAct;
    QAction   *closeAct;
    QAction   *closeAllAct;
    QAction   *nextErrorAct;
    QAction   *previousErrorAct;

    // Extra menu
    QMenu *ToolMenu;

    // Content pane
    QTreeView   *Objects;
    toCodeModel *CodeModel;
    QSplitter   *splitter;
    // Editors
    QTabWidget *Editors;

    void updateCurrent(void);
    QString editorName(const QString &schema,
                       const QString &object,
                       const QString &type);
    QString editorName(toPLSQLWidget *text);


    bool viewSource(const QString &schema,
                    const QString &name,
                    const QString &type,
                    int line,
                    bool current = false);

    void closeEditor(toPLSQLWidget* &editor);

protected:
    virtual void closeEvent(QCloseEvent *);

public:
    toPLSQLEditor(QWidget *parent, toConnection &connection);

    virtual ~toPLSQLEditor();

    toPLSQLWidget *currentEditor(void);
    QString currentSchema(void);

public slots:
    void compile(void);
    void compileWarn(void);
    void changeSchema(int);
    void refresh(void);
    void changePackage(const QModelIndex &, const QModelIndex &);
    void prevError(void);
    void nextError(void);
    void showSource(toTreeWidgetItem *);
    void newSheet(void);
    void windowActivated(QMdiSubWindow *w);
    void closeEditor(void);
    void closeAllEditor(void);
};

#endif
