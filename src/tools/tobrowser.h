
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOBROWSER_H
#define TOBROWSER_H

//template #include "core/totemplate.h"
#include "widgets/totoolwidget.h"
#include "core/totool.h"
#include "core/toconfenum.h"

#include <map>
#include <QtCore/QString>
#include <QMenu>
#include <QAction>

class QComboBox;
class QTabWidget;
class QSplitter;
class QModelIndex;

class toTreeWidgetItem;
class toTabWidget;
class QToolBar;
class toBrowserFilter;
class toViewFilter;
class toResult;
class toResultSchema;
class toResultData;
class toResultData;
class toResultFilter;
class toMySQLUser;
class toMySQLUserAccess;
class toResultTableView;
class QMdiSubWindow;

class toBrowserTableWidget;
class toBrowserViewWidget;
class toBrowserIndexWidget;
class toBrowserSequenceWidget;
class toBrowserSynonymWidget;
class toBrowserCodeWidget;
class toBrowserTriggerWidget;
class toBrowserDBLinksWidget;
class toBrowserDirectoriesWidget;
class toBrowserAccessWidget;
class toBrowserBaseWidget;
class toBrowserSchemaBase;
class toBrowserSchemaCodeBrowser;
class toBrowserSchemaTableView;

namespace ToConfiguration
{
    class Browser : public ConfigContext
    {
            Q_OBJECT;
            Q_ENUMS(OptionTypeEnum);
        public:
            Browser() : ConfigContext("Browser", ENUM_REF(Browser,OptionTypeEnum)) {};
            enum OptionTypeEnum
            {
                FilterIgnoreCase  = 9000  // #define CONF_FILTER_IGNORE_CASE
                                    , FilterInvert            // #define CONF_FILTER_INVERT
                , FilterType              // #define CONF_FILTER_TYPE
                , FilterTablespaceType    // #define CONF_FILTER_TABLESPACE_TYPE
                , FilterText              // #define CONF_FILTER_TEXT
            };
            virtual QVariant defaultValue(int option) const;
    };
};

/*! \brief Main GUI for Schema Browser.
toBrowser holds the GUI stuff for schema browsing. It contains
some slots/actions related to the DB object manipulations - mainly
the "mass" actions available for multiple objects (e.g. deleting
all selected tables etc.). Actions related to the one object (e.g.
test the DB link) are moved to toBrowserBaseWidget inherited
classes.

The main widget of toBrowser is the tab widget m_mainTab. Every its
tab widget is based on QSplitter (\see tableSplitter, etc.).
This QSplitter has two main children - the "object browser" (usually
the toResultTableView - dblinkView etc. - or QWidget with toolbar
and toResultTableView) and the "browser" - the toBrowserBaseWidget
inherited object.
QSplitters are keys in the m_browsersMap and m_objectsMap implicitly
shared QMap structures to allow easy and quick access with
QTabWidget::currentWidget() casted to QSplitter (\see e.g.
mainTab_currentChanged() as an example of data transfer to subwidgets).

Availability of the tabs for given connection is checked in
changeConnection() method. Some tabs are available only for
some DBMS, depending on features. The tab is hidden when there
is no available fature in DBMS.

Hidden tabs, resp. widgets, are not deleted. All is still available
in QMap (\see m_objectsMap, m_browsersMap) structures waiting for
resfresh.
*/
class toBrowser : public toToolWidget
{
        Q_OBJECT;

        enum Caching
        {
            USE_CACHE = 0,
            NO_USE_CACHE
        };

#ifdef TOEXTENDED_MYSQL
        toMySQLUser *UserPanel;
        toMySQLUserAccess *AccessPanel;
#endif

    public:
        toBrowser(QWidget *parent, toConnection &connection);
        virtual ~toBrowser();

        void setNewFilter(toBrowserFilter *filter);

        QString schema(void);

        /*! \brief Get text from the active "object browser" (toResultTableView).
        \see m_objectsMap.
        \param col a column name. It's 1 by default
         */
        QString currentItemText(int col = 1);

        /*! \brief Add a page to the m_mainTab widget;
        \see changeConnection().
        \param page a QSplitter main widget (tableSplitter etc.)
        \param label text to display as a tab title
        \param enable true when it should be visible. False on missing feature.
         */
        int addTab(QSplitter * page, const QString & label, bool enable);

        virtual bool canHandle(const toConnection &conn);

        void commitChanges();
        void rollbackChanges();

#ifdef TORA3_SESSION
        virtual void exportData(std::map<QString, QString> &data,
                                const QString &prefix);
        virtual void importData(std::map<QString, QString> &data,
                                const QString &prefix);
#endif
        virtual bool close();

    public slots:
        void refresh(void);
        void changeSchema(int);
        void changeItem();
        void changeItem(const QModelIndex &);
        void clearFilter(void);
        void defineFilter(void);
        virtual void slotWindowActivated(toToolWidget*);

        void changeConnection(void);

#if TORA3_BROWSER_TOOLS
        /*! \brief A wrapper method to drop any object from DB.
        \param type a uppercase string with e.g. 'TABLE', 'INDEX', etc.
        			Only objects supported by toExtract can be dropped.
        \param what a object name to drop.
         */
        void dropSomething(const QString & type, const QString & what);

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

        void dropView(void);

        void dropIndex(void);

        void enableConstraints(void);
        void disableConstraints(void);

        void displayTableMenu(QMenu *menu);
        void displayViewMenu(QMenu *menu);
        void displayIndexMenu(QMenu *menu);

        void addUser(void);
        void dropUser(void);
#endif

    private slots:
        /** Handle main tabwidget and its tabs switch
         * @param int Tab which has been activated
         * @param Caching caching Do(not) try using the cache but query the database instead
         */
        void mainTab_currentChanged(int, Caching caching = USE_CACHE);

        void slotSelected(const QString&);
    signals:
        void filterChanged(toViewFilter*);

    protected:
        virtual void closeEvent(QCloseEvent *);

    private:
        toResultSchema *Schema;
        QTabWidget   *m_mainTab;
        QMenu         *ToolMenu;

        QSplitter * tableSplitter;
        toBrowserSchemaTableView *tableView;
        toBrowserTableWidget * tableBrowserWidget;

        QSplitter * viewSplitter;
        toBrowserSchemaTableView * viewView;
        toBrowserViewWidget * viewBrowserWidget;

        QSplitter * indexSplitter;
        toBrowserSchemaTableView * indexView;
        toBrowserIndexWidget * indexBrowserWidget;

        QSplitter * sequenceSplitter;
        toBrowserSchemaTableView * sequenceView;
        toBrowserSequenceWidget * sequenceBrowserWidget;

        QSplitter * synonymSplitter;
        toBrowserSchemaTableView * synonymView;
        toBrowserSynonymWidget * synonymBrowserWidget;

        QSplitter * codeSplitter;
        toBrowserSchemaCodeBrowser * codeView;
        toBrowserCodeWidget * codeBrowserWidget;

        QSplitter * triggerSplitter;
        toBrowserSchemaTableView * triggerView;
        toBrowserTriggerWidget * triggerBrowserWidget;

        QSplitter * dblinkSplitter;
        toBrowserSchemaTableView * dblinkView;
        toBrowserDBLinksWidget * dblinkBrowserWidget;

        QSplitter * directoriesSplitter;
        toBrowserSchemaTableView * directoriesView;
        toBrowserDirectoriesWidget * directoriesBrowserWidget;

        QSplitter * accessSplitter;
        toBrowserSchemaTableView * accessView;
        toBrowserAccessWidget * accessBrowserWidget;

        QMap<QSplitter*, toBrowserSchemaBase*> m_objectsMap;
        QMap<QSplitter*, toBrowserBaseWidget*> m_browsersMap;

        toBrowserFilter   *Filter;

        QAction *refreshAct;
        QAction *FilterButton;
        QAction *clearFilterAct;
        QAction *addTableAct;
        QAction *modTableAct;
        QAction *modConstraintAct;
        QAction *modIndexAct;
        QAction *addIndexesAct;
        QAction *dropIndexesAct;
        QAction *dropTableAct;
        QAction *dropViewAct;
        QAction *enableConstraintAct;
        QAction *disableConstraintAct;
        QAction *testDBLinkAct;
};

// class toBrowseTemplate : public QObject, public toTemplateProvider
// {
//     Q_OBJECT;
//
//     QToolButton                 *FilterButton;
//     toBrowserFilter             *Filter;
//     std::list<toTemplateItem *>  Parents;
//     bool                         Registered;
//
// public:
//     toBrowseTemplate(void)
//             : QObject(NULL), toTemplateProvider("Browser")
//     {
//         Registered = false;
//         Filter = NULL;
//     }
//
//     virtual void insertItems(toTreeWidget *parent, QToolBar *toolbar);
//     virtual void removeItem(toTreeWidgetItem *item);
//     toBrowserFilter *filter(void)
//     {
//         return Filter;
//     }
//     virtual void exportData(std::map<QString, QString> &data,
//                             const QString &prefix);
//     virtual void importData(std::map<QString, QString> &data,
//                             const QString &prefix);
//
// public slots:
//     void addDatabase(const QString &);
//     void removeDatabase(const QString &);
//     void defineFilter(void);
//     void clearFilter(void);
// };


// class toBrowseButton : public QToolButton
// {
//     Q_OBJECT;
//
// public:
//     toBrowseButton(const QIcon &iconSet,
//                    const QString &textLabel,
//                    const QString & grouptext,
//                    QObject * receiver,
//                    const char * slot,
//                    QToolBar * parent,
//                    const char * name = 0);
//
// private slots:
//     void connectionChanged(void);
// };

class toBrowserTool : public toTool
{
        Q_OBJECT;

    protected:
        virtual const char **pictureXPM(void);

    public:
        toBrowserTool();
        virtual const char *menuItem();
        virtual toToolWidget *toolWindow(QWidget *parent, toConnection &connection);
        virtual bool canHandle(const toConnection &conn);
        virtual void customSetup(void);
        virtual void closeWindow(toConnection &connection) {};

    public slots:
#ifdef TORA3_BROWSER_TOOLS
        void addTable(void);
        void addConstraint(void);
        void addIndex(void);
#endif
    private:
        static ToConfiguration::Browser s_browserConf;
};


#endif
