/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOROLLBACK_H
#define TOROLLBACK_H

#include "config.h"
#include "ui_torollbackdialogui.h"
#include "totool.h"

#include <qdialog.h>
#include <totreewidget.h>

#include <algorithm>

class QComboBox;
class QMenu;
class toConnection;
class toResultItem;
class toResultView;
class toSGAStatement;
class toStorageDefinition;
class toResultTableView;


class toRollbackDialog : public QDialog, public Ui::toRollbackDialogUI
{
    Q_OBJECT

    toStorageDefinition *Storage;

public:
    toRollbackDialog(toConnection &conn, QWidget *parent = 0, const char *name = 0);

    std::list<QString> sql(void);

public slots:
    void valueChanged(const QString &str);
    void displaySQL(void);
};

class toRollback : public toToolWidget
{
    Q_OBJECT

    toResultView *Segments;
    toResultView *Statements;

    QWidget *CurrentTab;

    toSGAStatement *CurrentStatement;
    toResultTableView *TransactionUsers;

    QAction * UpdateSegmentsAct;
    QAction * enableOldAct;
    QAction * OnlineAct;
    QAction * OfflineAct;
    QAction * NewAct;
    QAction * DropAct;

    QMenu *ToolMenu;
    QComboBox *Refresh;

    QString currentSegment(void);

public:
    toRollback(QWidget *parent, toConnection &connection);
public slots:
    void changeStatement(toTreeWidgetItem *item);
    void changeItem(toTreeWidgetItem *item);
    void changeRefresh(const QString &str);

    void enableOld(bool);
    void refresh(void);
    void online(void);
    void offline(void);
    void addSegment(void);
    void dropSegment(void);
    void windowActivated(QMdiSubWindow *widget);
};

#endif
