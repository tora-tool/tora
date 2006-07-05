/*****
*
* TOra - An Oracle Toolkit for DBA's and developers
* Copyright (C) 2003-2005 Quest Software, Inc
* Portions Copyright (C) 2005 Other Contributors
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
*      these libraries without written consent from Quest Software, Inc.
*      Observe that this does not disallow linking to the Qt Free Edition.
*
*      You may link this product with any GPL'd Qt library such as Qt/Free
*
* All trademarks belong to their respective owners.
*
*****/

#ifndef TOBROWSER_H
#define TOBROWSER_H

#include "config.h"
#include "totemplate.h"
#include "totool.h"

#include <map>

class QComboBox;
class QListViewItem;
class QPopupMenu;
class QTabWidget;
class QToolBar;
class toBrowserFilter;
class toResult;
class toResultCombo;
class toResultContent;
class toResultFilter;
class toResultView;
class toMySQLUser;
class toMySQLUserAccess;

class toBrowser : public toToolWidget
{
    Q_OBJECT

    toResultCombo *Schema;
    QTabWidget *TopTab;
    QPopupMenu *ToolMenu;

    QString SecondText;
    toResultView *FirstTab;
    toResult *SecondTab;
    toBrowserFilter *Filter;
    QWidget *CurrentTop;

    QToolButton *FilterButton;

    toResultContent *ViewContent;
    toResultContent *TableContent;
    toResultContent *AccessContent;

    std::map<QCString, toResultView *> Map;
    std::map<QCString, toResult *> SecondMap;
    void setNewFilter(toBrowserFilter *filter);
    QTimer Poll;

    QString schema(void);
    void enableDisableConstraints(const QString &);
    void dropSomething(const QString &, const QString &);

    QListViewItem *selectedItem();
#ifdef TOEXTENDED_MYSQL

    toMySQLUser *UserPanel;
    toMySQLUserAccess *AccessPanel;
#endif
public:
    toBrowser(QWidget *parent, toConnection &connection);
    virtual ~toBrowser();

    virtual bool canHandle(toConnection &conn);

    virtual void exportData(std::map<QCString, QString> &data, const QCString &prefix);
    virtual void importData(std::map<QCString, QString> &data, const QCString &prefix);

    virtual bool close(bool del);
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
    void windowActivated(QWidget *widget);
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
    void fixIndexCols(void);

    void changeConnection(void);
    void enableConstraints(void);
    void disableConstraints(void);

    void testDBLink(void);

    void displayIndexMenu(QPopupMenu *menu);
    void displayTableMenu(QPopupMenu *menu);

    void addUser(void);
    void dropUser(void);
};

class toBrowseTemplate : public QObject, public toTemplateProvider
{
    Q_OBJECT

    QToolButton *FilterButton;
    toBrowserFilter *Filter;
    std::list<toTemplateItem *> Parents;
    bool Registered;
public:
    toBrowseTemplate(void)
            : QObject(NULL, "browsertemplate"), toTemplateProvider("Browser")
    {
        Registered = false;
        Filter = NULL;
    }
    virtual void insertItems(QListView *parent, QToolBar *toolbar);
    virtual void removeItem(QListViewItem *item);
    toBrowserFilter *filter(void)
    {
        return Filter;
    }
    virtual void exportData(std::map<QCString, QString> &data, const QCString &prefix);
    virtual void importData(std::map<QCString, QString> &data, const QCString &prefix);
public slots:
    void addDatabase(const QString &);
    void removeDatabase(const QString &);
    void defineFilter(void);
    void clearFilter(void);
};

class toBrowseButton : public QToolButton
{
    Q_OBJECT
public:
    toBrowseButton(const QIconSet &iconSet,
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
    Q_OBJECT

protected:
    virtual const char **pictureXPM(void);
public:
    toBrowserTool();
    virtual const char *menuItem();
    virtual QWidget *toolWindow(QWidget *parent, toConnection &connection);
    virtual bool canHandle(toConnection &conn);
    virtual void customSetup(int id);
    virtual void closeWindow(toConnection &connection){};    
public slots:
    void addTable(void);
    void addConstraint(void);
    void addIndex(void);
};


#endif
