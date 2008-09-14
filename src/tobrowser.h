
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

#ifndef TOBROWSER_H
#define TOBROWSER_H

#include "config.h"
#include "totemplate.h"
#include "totool.h"

#include <map>
#include <QString>
#include <QMenu>
#include <QAction>

class QComboBox;
class toTreeWidgetItem;
class toTabWidget;
class QToolBar;
class toBrowserFilter;
class toResult;
class toResultCombo;
class toResultData;
class toResultData;
class toResultFilter;
class toMySQLUser;
class toMySQLUserAccess;
class toResultTableView;
class QMdiSubWindow;


class toBrowser : public toToolWidget
{
    Q_OBJECT;

    toResultCombo *Schema;
    toTabWidget   *TopTab;
    QMenu         *ToolMenu;

    QString            SecondText;
    toResultTableView *FirstTab;
    toResult          *SecondTab;
    toBrowserFilter   *Filter;
    QWidget           *CurrentTop;

    toResultData *ViewContent;
    toResultData *TableContent;
    toResultData *AccessContent;

    std::map<QString, toResultTableView *> Map;
    std::map<QString, toResult *> SecondMap;
    void setNewFilter(toBrowserFilter *filter);

    QString schema(void);
    void enableDisableConstraints(const QString &);
    void dropSomething(const QString &, const QString &);

    QModelIndex selectedItem(int col = 1);

    QAction *refreshAct;
    QAction *FilterButton;
    QAction *clearFilterAct;
    QAction *addTableAct;
    QAction *modTableAct;
    QAction *modConstraintAct;
    QAction *modIndexAct;
    QAction *dropTableAct;
    QAction *enableConstraintAct;
    QAction *disableConstraintAct;
    QAction *testDBLinkAct;

#ifdef TOEXTENDED_MYSQL
    toMySQLUser *UserPanel;
    toMySQLUserAccess *AccessPanel;
#endif

public:
    toBrowser(QWidget *parent, toConnection &connection);
    virtual ~toBrowser();

    virtual bool canHandle(toConnection &conn);

    virtual void exportData(std::map<QString, QString> &data,
                            const QString &prefix);
    virtual void importData(std::map<QString, QString> &data,
                            const QString &prefix);

    virtual bool close();

public slots:
    void refresh(void);
    void updateTabs(void);
    void changeSchema(int);
    void changeTab(QWidget *tab);
    void changeSecond(void);
    void changeSecondTab(QWidget *tab);
    void changeItem();
    void clearFilter(void);
    void defineFilter(void);
    void windowActivated(QMdiSubWindow *widget);
    void firstDone(void);
    void focusObject(void);

    void modifyTable(void);
    void addTable(void);
    void addIndex(void);
    void modifyConstraint(void);
    void modifyIndex(void);

    void changeType(void);
    void dropTable(void);
    void truncateTable(void);

    void checkTable(void);
    void optimizeTable(void);
    void analyzeTable(void);
    void flushPrivs(void);

    void dropIndex(void);
#if 0
    void fixIndexCols(void);
#endif

    void changeConnection(void);
    void enableConstraints(void);
    void disableConstraints(void);

    void testDBLink(void);

    void displayIndexMenu(QMenu *menu);
    void displayTableMenu(QMenu *menu);

    void addUser(void);
    void dropUser(void);

protected:
    virtual void closeEvent(QCloseEvent *);
};

class toBrowseTemplate : public QObject, public toTemplateProvider
{
    Q_OBJECT;

    QToolButton                 *FilterButton;
    toBrowserFilter             *Filter;
    std::list<toTemplateItem *>  Parents;
    bool                         Registered;

public:
    toBrowseTemplate(void)
            : QObject(NULL), toTemplateProvider("Browser")
    {
        Registered = false;
        Filter = NULL;
    }

    virtual void insertItems(toTreeWidget *parent, QToolBar *toolbar);
    virtual void removeItem(toTreeWidgetItem *item);
    toBrowserFilter *filter(void)
    {
        return Filter;
    }
    virtual void exportData(std::map<QString, QString> &data,
                            const QString &prefix);
    virtual void importData(std::map<QString, QString> &data,
                            const QString &prefix);

public slots:
    void addDatabase(const QString &);
    void removeDatabase(const QString &);
    void defineFilter(void);
    void clearFilter(void);
};


class toBrowseButton : public QToolButton
{
    Q_OBJECT;

public:
    toBrowseButton(const QIcon &iconSet,
                   const QString &textLabel,
                   const QString & grouptext,
                   QObject * receiver,
                   const char * slot,
                   QToolBar * parent,
                   const char * name = 0);

private slots:
    void connectionChanged(void);
};

class toBrowserTool : public toTool
{
    Q_OBJECT;

protected:
    virtual const char **pictureXPM(void);

public:
    toBrowserTool();
    virtual const char *menuItem();
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection);
    virtual bool canHandle(toConnection &conn);
    virtual void customSetup(void);
    virtual void closeWindow(toConnection &connection) {};

public slots:
    void addTable(void);
    void addConstraint(void);
    void addIndex(void);
};


#endif
