/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSGATRACE_H
#define TOSGATRACE_H

#include "config.h"
#include "totool.h"

#include <qstring.h>

#define TOSQL_LONGOPS "toSGATrace:LongOps"

class QComboBox;
class QMenu;
class QTabWidget;
class toConnection;
class toMain;
class toResultCombo;
class toResultTableView;
class toSGAStatement;
class toTool;

class toSGATrace : public toToolWidget
{
    Q_OBJECT;

    toResultTableView *Trace;
    QTabWidget *ResultTab;

    QAction       *FetchAct;
    toResultCombo *Schema;
    QComboBox     *Type;
    QComboBox     *Refresh;
    QComboBox     *Limit;

    toSGAStatement *Statement;
    QString CurrentSchema;

    void updateSchemas(void);

public:
    toSGATrace(QWidget *parent, toConnection &connection);

public slots:
    void changeSchema(const QString &str);
    void changeItem(void);
    void changeRefresh(const QString &str);
    void refresh(void);
};

#endif
