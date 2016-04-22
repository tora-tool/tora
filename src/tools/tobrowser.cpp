
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

#include "tools/tobrowser.h"

#include "widgets/tohelp.h"
#include "ui_tobrowserfilterui.h"
#include "tools/toresultstorage.h"
#include "tools/tobrowsertablewidget.h"
#include "tools/tobrowserviewwidget.h"
#include "tools/tobrowserindexwidget.h"
#include "tools/tobrowsersequencewidget.h"
#include "tools/tobrowsersynonymwidget.h"
#include "tools/tobrowsercodewidget.h"
#include "tools/tobrowsertriggerwidget.h"
#include "tools/tobrowserdblinkswidget.h"
#include "tools/tobrowserdirectorieswidget.h"
#include "tools/tobrowseraccesswidget.h"
#include "tools/tobrowserschemawidget.h"

#include "core/utils.h"
#include "core/tochangeconnection.h"
#include "widgets/toresultschema.h"
#include "core/toconnectionsub.h"
#include "core/toconnectiontraits.h"
#include "core/toglobalevent.h"
#include "core/toconfiguration.h"
#include "toresultview.h"

#ifdef TOEXTENDED_MYSQL
#  include "core/tomysqluser.h"
#endif

#include <QInputDialog>
#include <QSplitter>
#include <QToolBar>

#include "icons/addindex.xpm"
#include "icons/addtable.xpm"
#include "icons/filter.xpm"
#include "icons/function.xpm"
#include "icons/index.xpm"
#include "icons/modconstraint.xpm"
#include "icons/modindex.xpm"
#include "icons/modtable.xpm"
#include "icons/nofilter.xpm"
#include "icons/refresh.xpm"
#include "icons/schema.xpm"
#include "icons/sequence.xpm"
#include "icons/synonym.xpm"
#include "icons/table.xpm"
#include "icons/tobrowser.xpm"
#include "icons/view.xpm"
#include "icons/trash.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#ifdef TOEXTENDED_MYSQL
#include "icons/new.xpm"
#endif

QVariant ToConfiguration::Browser::defaultValue(int option) const
{
    switch (option)
    {
        case FilterIgnoreCase:
            return QVariant((bool)false);
        case FilterInvert:
            return QVariant((bool)false);
        case FilterType:
            return QVariant((int)0);
        case FilterTablespaceType:
            return QVariant((int)0);
        case FilterText:
            return QVariant(QString(""));
        default:
            Q_ASSERT_X( false, qPrintable(__QHERE__), qPrintable(QString("Context Browser un-registered enum value: %1").arg(option)));
            return QVariant();
    }
}

const char **toBrowserTool::pictureXPM(void)
{
    return const_cast<const char**>(tobrowser_xpm);
}

toBrowserTool::toBrowserTool()
    : toTool(20, "Schema Browser")
{ }

const char *toBrowserTool::menuItem()
{
    return "Schema Browser";
}

toToolWidget* toBrowserTool::toolWindow(QWidget *parent, toConnection &connection)
{
    return new toBrowser(parent, connection);
}

bool toBrowserTool::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle") || conn.providerIs("QMYSQL") || conn.providerIs("QPSQL") || conn.providerIs("SapDB") || conn.providerIs("Teradata");
}

void toBrowserTool::customSetup()
{
#if 0                           // todo
    QMenu *createMenu = new QMenu(toMainWidget());
    createMenu->insertItem(QPixmap(const_cast<const char**>(modtable_xpm)), tr("&Table"),
                           this, SLOT(addTable()));
    createMenu->insertItem(QPixmap(const_cast<const char**>(modindex_xpm)), tr("&Index"),
                           this, SLOT(addIndex()));
    createMenu->insertItem(QPixmap(const_cast<const char**>(modconstraint_xpm)), tr("&Constraint"),
                           this, SLOT(addConstraint()));
    toMainWidget()->menuBar()->insertItem(tr("&Create"), createMenu, -1, toToolMenuIndex());
#endif
}

#ifdef TORA3_BROWSER_TOOLS
void toBrowserTool::addTable(void)
{
    //obsolete try
    // {
    //     toConnection &conn = toMainWidget()->currentConnection();
    //     toBrowserTable::newTable(conn,
    //                              conn.providerIs("QMYSQL") ? conn.database() : conn.user(),
    //                              toMainWidget());
    // }
    // TOCATCH
    throw tr("toBrowserTool::addTable(void) not implement yet");
}

void toBrowserTool::addIndex(void)
{
    // try
    // {
    //     toConnection &conn = toMainWidget()->currentConnection();
    //     toBrowserIndex::modifyIndex(conn,
    //                                 conn.providerIs("QMYSQL") ? conn.database() : conn.user(),
    //                                 QString::null,
    //                                 toMainWidget());
    // }
    // TOCATCH
    throw tr("toBrowserTool::addIndex(void) not implement yet");
}

void toBrowserTool::addConstraint(void)
{
    // try
    // {
    //     toConnection &conn = toMainWidget()->currentConnection();
    //     toBrowserConstraint::modifyConstraint(conn,
    //                                           conn.providerIs("QMYSQL") ? conn.database() : conn.user(),
    //                                           QString::null,
    //                                           toMainWidget());
    // }
    // TOCATCH
    throw tr("toBrowserTool::addConstraint(void) not implement yet");
}
#endif

ToConfiguration::Browser toBrowserTool::s_browserConf;

static toBrowserTool BrowserTool;

static toSQL SQLListTablespaces("toBrowser:ListTablespaces",
                                "SELECT Tablespace_Name FROM sys.DBA_TABLESPACES\n"
                                " ORDER BY Tablespace_Name",
                                "List the available tablespaces in a database.");

class toBrowserFilter
    : public toViewFilter
{
        enum FilterType // see toBrowserFilterSetup::ButtonsGroup
        {
            FilterNone = 0,
            FilterStartsWith,
            FilterEndsWith,
            FilterContains,
            FilterCommaSeparated,
            FilterRegExp,
        };
        FilterType         Type;
        bool               IgnoreCase;
        bool               Invert;
        QString            Text;
        int                TablespaceType;
        std::list<QString> Tablespaces;
        QRegExp            Match;
        bool               OnlyOwnSchema;
        bool               Empty;

        std::map<QString, bool> RemoveDuplicates;

    public:
        toBrowserFilter(FilterType type,
                        bool cas,
                        bool invert,
                        const QString &str,
                        int tablespace,
                        const std::list<QString> &tablespaces,
                        bool onlyOwnSchema = false)
            : toViewFilter()
            , Type(type)
            , IgnoreCase(cas)
            , Invert(invert)
            , Text(cas ? str.toUpper() : str)
            , TablespaceType(tablespace)
            , Tablespaces(tablespaces)
            , OnlyOwnSchema(onlyOwnSchema)
            , Empty(false)
        {

            if (!str.isEmpty())
            {
                Match.setPattern(str);
                Match.setCaseSensitivity(cas ? Qt::CaseSensitive : Qt::CaseInsensitive);
            }

            storeFilterSettings();
        }

        toBrowserFilter(bool empty = true)
            : Type(FilterNone),
              IgnoreCase(true),
              Invert(false),
              TablespaceType(0)
        {

            Empty = empty;
            if (!empty)
                readFilterSettings();
            else
                toConfigurationNewSingle::Instance().setOption(ToConfiguration::Browser::FilterType, FilterNone);  // No filter type
        }

        virtual ~toBrowserFilter() {};

        virtual bool isEmpty(void)
        {
            return Empty;
        }

        virtual void storeFilterSettings(void)
        {
            using namespace ToConfiguration;
            toConfigurationNewSingle::Instance().setOption(Browser::FilterIgnoreCase, IgnoreCase);
            toConfigurationNewSingle::Instance().setOption(Browser::FilterInvert, Invert);
            toConfigurationNewSingle::Instance().setOption(Browser::FilterType, Type);
            toConfigurationNewSingle::Instance().setOption(Browser::FilterTablespaceType, TablespaceType);
            toConfigurationNewSingle::Instance().setOption(Browser::FilterText, Text);
            toConfigurationNewSingle::Instance().saveAll();
        }

        virtual void readFilterSettings(void)
        {
            using namespace ToConfiguration;
            QString t;
            Text = toConfigurationNewSingle::Instance().option(Browser::FilterText).toString();
            IgnoreCase = toConfigurationNewSingle::Instance().option(ToConfiguration::Browser::FilterIgnoreCase).toBool();
            Invert = toConfigurationNewSingle::Instance().option(Browser::FilterInvert).toBool();
            OnlyOwnSchema = false;
            Type = (FilterType) toConfigurationNewSingle::Instance().option(Browser::FilterType).toInt();
            TablespaceType = toConfigurationNewSingle::Instance().option(Browser::FilterTablespaceType).toInt();
        }

#ifdef TORA3_SESSION
        virtual void exportData(std::map<QString, QString> &data, const QString &prefix)
        {
            data[prefix + ":Type"] = QString::number(Type);
            if (IgnoreCase)
                data[prefix + ":Ignore"] = "Yes";
            if (Invert)
                data[prefix + ":Invert"] = "Yes";
            data[prefix + ":SpaceType"] = QString::number(TablespaceType);
            data[prefix + ":Text"] = Text;
            int id = 1;
            for (std::list<QString>::iterator i = Tablespaces.begin(); i != Tablespaces.end(); i++, id++)
                data[prefix + ":Space:" + QString::number(id).toLatin1()] = *i;
            if (OnlyOwnSchema)
                data[prefix + ":OwnlyOwnSchema"] = "Yes";
        }

        virtual void importData(std::map<QString, QString> &data, const QString &prefix)
        {
            Type = data[prefix + ":Type"].toInt();
            OnlyOwnSchema = !data[prefix + ":OnlyOwnSchema"].isEmpty();
            TablespaceType = data[prefix + ":SpaceType"].toInt();
            IgnoreCase = !data[prefix + ":Ignore"].isEmpty();
            Invert = !data[prefix + ":Invert"].isEmpty();
            Text = data[prefix + ":Text"];
            if (!Text.isEmpty())
            {
                Match.setPattern(Text);
                Match.setCaseSensitivity(IgnoreCase ? Qt::CaseSensitive : Qt::CaseInsensitive);
            }
            int id = 1;
            std::map<QString, QString>::iterator i;
            Tablespaces.clear();
            while ((i = data.find(prefix + ":Space:" + QString::number(id).toLatin1())) != data.end())
            {
                Tablespaces.insert(Tablespaces.end(), (*i).second);
                i++;
                id++;
            }
        }
#endif

        bool onlyOwnSchema(void)
        {
            return OnlyOwnSchema;
        }

        virtual QString wildCard(void)
        {
            switch (Type)
            {
                case FilterStartsWith:
                    return Text.toUpper() + QString::fromLatin1("%");
                case FilterEndsWith:
                    return QString::fromLatin1("%") + Text.toUpper();
                case FilterContains:
                    return QString::fromLatin1("%") + Text.toUpper() + QString::fromLatin1("%");
                case FilterNone:
                case FilterCommaSeparated:
                case FilterRegExp:
                    return QString::fromLatin1("%");
                default:
                    throw QString("Unknown filter type: %1").arg((int)Type);
            }
        }

        virtual void startingQuery()
        {
            RemoveDuplicates.clear();
        }

//    virtual bool check(const toTreeWidgetItem *item)
//    {
//        return check(item->text(0),
//                     item->text(1),
//                     item->text(2));
//    }

        virtual bool check(const toResultModel *model, const int row)
        {
            return check(model->data(row, 1).toString(),
                         model->data(row, 2).toString(),
                         model->data(row, 3).toString());
        }

        bool check(QString one, QString two, QString three)
        {
            QString key = one + "." + two;
            if (RemoveDuplicates.find(key) != RemoveDuplicates.end())
                return false;
            else
                RemoveDuplicates[key] = true;

            QString str = one;
            QString tablespace = three;
            if (!tablespace.isEmpty())
            {
                switch (TablespaceType)
                {
                    case 1:
                        {
                            bool ok = false;
                            for (std::list<QString>::iterator i = Tablespaces.begin(); i != Tablespaces.end(); i++)
                            {
                                if (*i == tablespace)
                                {
                                    ok = true;
                                    break;
                                }
                            }
                            if (!ok)
                                return false;
                        }
                        break;
                    case 2:
                        for (std::list<QString>::iterator i = Tablespaces.begin(); i != Tablespaces.end(); i++)
                            if (*i == tablespace)
                                return false;
                        break;
                    default:
                        break;
                }
            }
            switch (Type)
            {
                case FilterNone:
                    return true;
                case FilterStartsWith:
                    if (IgnoreCase)
                    {
                        if (str.toUpper().startsWith(Text))
                            return !Invert;
                    }
                    else if (str.startsWith(Text))
                        return !Invert;
                    break;
                case FilterEndsWith:
                    if (IgnoreCase)
                    {
                        if (str.right(Text.length()).toUpper() == Text)
                            return !Invert;
                    }
                    else if (str.right(Text.length()) == Text)
                        return !Invert;
                    break;
                case FilterContains:
                    if (str.contains(Text, IgnoreCase ? Qt::CaseSensitive : Qt::CaseInsensitive))
                        return !Invert;
                    break;
                case FilterCommaSeparated:
                    {
                        QStringList lst = Text.split(QRegExp(QString("\\s*,\\s*")));
                        for (int i = 0; i < lst.count(); i++)
                            if (IgnoreCase)
                            {
                                if (str.toUpper() == lst[i])
                                    return !Invert;
                            }
                            else if (str == lst[i])
                                return !Invert;
                    }
                    break;
                case FilterRegExp:
// qt4             if (Match.match(str) >= 0)
                    if (Match.indexIn(str) >= 0)
                        return !Invert;
                    break;
            }
            return Invert;
        }

        virtual toViewFilter *clone(void)
        {
            return new toBrowserFilter(*this);
        }

        friend class toBrowserFilterSetup;
};

class toBrowserFilterSetup
    : public QDialog
    , public Ui::toBrowserFilterUI
{
    private:
        QButtonGroup *ButtonsGroup;
        QButtonGroup *TypeGroup;

    public:
        void setup(bool temp)
        {
            toHelp::connectDialog(this);

            // qbuttongroup is not a widget. awesome. guess they'll fix
            // that in qt5.
            ButtonsGroup = new QButtonGroup(ButtonsBox);
            ButtonsGroup->addButton(None, toBrowserFilter::FilterNone);
            ButtonsGroup->addButton(StartWith, toBrowserFilter::FilterStartsWith);
            ButtonsGroup->addButton(EndWith, toBrowserFilter::FilterEndsWith);
            ButtonsGroup->addButton(Contains, toBrowserFilter::FilterContains);
            ButtonsGroup->addButton(CommaSeparate, toBrowserFilter::FilterCommaSeparated);
            ButtonsGroup->addButton(RegExp, toBrowserFilter::FilterRegExp);

            TypeGroup = new QButtonGroup(TablespaceType);
            TypeGroup->addButton(IncludeAll, 0);
            TypeGroup->addButton(Include, 1);
            TypeGroup->addButton(Exclude, 2);

            if (!temp)
            {
                OnlyOwnSchema->hide();
                Tablespaces->setNumberColumn(false);
                Tablespaces->setReadableColumns(true);
                Tablespaces->setSQL(SQLListTablespaces);
                try
                {
                    // TODO: WTF is this? the query is executed twice?
                    //toConnection &conn = toConnection::currentConnection(this);
                    //toQuery query(conn, toSQL::string(SQLListTablespaces, conn), toQueryParams());
                    //Tablespaces->query(SQLListTablespaces);

                    Tablespaces->refresh();
                }
                catch (...)
                {
                    TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
                }
                Tablespaces->setSelectionMode(toTreeWidget::Multi);
            }
            else
            {
                TablespaceType->hide();
            }
        }
        toBrowserFilterSetup(bool temp, QWidget *parent)
            : QDialog(parent)
        {
            setupUi(this);
            setup(temp);
        }
        toBrowserFilterSetup(bool temp, toBrowserFilter &cur, QWidget *parent)
            : QDialog(parent)
        {
            setupUi(this);
            setup(temp);

            QAbstractButton *b = ButtonsGroup->button(cur.Type);
            if (b)
                b->setChecked(true);

            if (!TablespaceType->isHidden())
            {
                b = TypeGroup->button(cur.TablespaceType);
                if (b)
                    b->setChecked(true);

                for (std::list<QString>::iterator i = cur.Tablespaces.begin();
                        i != cur.Tablespaces.end();
                        i++)
                {
                    for (toTreeWidgetItemIterator it(Tablespaces); (*it); it++)
                    {
                        if ((*it)->text(0) == *i)
                        {
                            (*it)->setSelected(true);
                            break;
                        }
                    }
                }
                String->setText(cur.Text);
                Invert->setChecked(cur.Invert);
                IgnoreCase->setChecked(cur.IgnoreCase);
                OnlyOwnSchema->setChecked(cur.OnlyOwnSchema);
            }
        }

        toBrowserFilter *getSetting(void)
        {
            std::list<QString> tablespaces;
            for (toTreeWidgetItemIterator it(Tablespaces); (*it); it++)
            {
                if ((*it)->isSelected())
                    tablespaces.insert(tablespaces.end(), (*it)->text(0));
            }

            return new toBrowserFilter((toBrowserFilter::FilterType) ButtonsGroup->id(ButtonsGroup->checkedButton()),
                                       IgnoreCase->isChecked(),
                                       Invert->isChecked(),
                                       String->text(),
                                       TypeGroup->id(TypeGroup->checkedButton()),
                                       tablespaces,
                                       OnlyOwnSchema->isChecked());
        }
};

// toBrowseButton::toBrowseButton(const QIcon &iconSet,
//                                const QString &textLabel,
//                                const QString & grouptext,
//                                QObject * receiver,
//                                const char * slot,
//                                QToolBar * parent,
//                                const char * name)
//         : QToolButton(parent)
// {
//
//     setIcon(iconSet);
//     setText(textLabel);
//     connect(this, SIGNAL(triggered(QAction *)), receiver, slot);
//     setObjectName(name);
//     setToolTip(grouptext);
//
//     try
//     {
//         connect(toCurrentTool(this), SIGNAL(connectionChange()), this, SLOT(connectionChanged()));
//     }
//     TOCATCH
//     connectionChanged();
// }
//
// void toBrowseButton::connectionChanged()
// {
//     try
//     {
//         setEnabled(toExtract::canHandle(toConnection::currentConnection(this)));
//     }
//     TOCATCH
// }

#define FIRST_WIDTH 180

#define TAB_TABLES  "Tables"
#define TAB_TABLE_COLUMNS "TablesColumns"
#define TAB_TABLE_CONS  "TablesConstraint"
#define TAB_TABLE_DEPEND "TablesDepend"
#define TAB_TABLE_INDEXES "TablesIndexes"
#define TAB_TABLE_DATA  "TablesData"
#define TAB_TABLE_GRANTS "TablesGrants"
#define TAB_TABLE_TRIGGERS "TablesTriggers"
#define TAB_TABLE_INFO  "TablesInfo"
#define TAB_TABLE_PARTITION "TablesPartition"
#define TAB_TABLE_STATISTIC "TablesStatistic"
#define TAB_TABLE_EXTENT "TablesExtent"
#define TAB_TABLE_EXTRACT "TablesExtract"

#define TAB_VIEWS  "Views"
#define TAB_VIEW_COLUMNS "ViewColumns"
#define TAB_VIEW_SQL  "ViewSQL"
#define TAB_VIEW_DATA  "ViewData"
#define TAB_VIEW_GRANTS  "ViewGrants"
#define TAB_VIEW_DEPEND  "ViewDepend"
#define TAB_VIEW_EXTRACT "ViewExtract"

#define TAB_SEQUENCES  "Sequences"
#define TAB_SEQUENCES_GRANTS "SequencesGrants"
#define TAB_SEQUENCES_INFO "SequencesInfo"
#define TAB_SEQUENCES_EXTRACT "SequencesExtract"

#define TAB_INDEX  "Index"
#define TAB_INDEX_COLS  "IndexCols"
#define TAB_INDEX_INFO  "IndexInfo"
#define TAB_INDEX_EXTENT "IndexesExtent"
#define TAB_INDEX_EXTRACT "IndexExtract"
#define TAB_INDEX_STATISTIC "IndexStatistic"

#define TAB_SYNONYM  "Synonym"
#define TAB_SYNONYM_GRANTS "SynonymGrants"
#define TAB_SYNONYM_INFO "SynonymInfo"
#define TAB_SYNONYM_EXTRACT "SynonymExtract"

#define TAB_PLSQL  "PLSQL"
#define TAB_PLSQL_SOURCE "PLSQLSource"
#define TAB_PLSQL_BODY  "PLSQLBody"
#define TAB_PLSQL_GRANTS "PLSQLGrants"
#define TAB_PLSQL_DEPEND "PLSQLDepend"
#define TAB_PLSQL_EXTRACT "PLSQLExtract"

#define TAB_TRIGGER  "Trigger"
#define TAB_TRIGGER_INFO "TriggerInfo"
#define TAB_TRIGGER_SOURCE "TriggerSource"
#define TAB_TRIGGER_GRANTS "TriggerGrants"
#define TAB_TRIGGER_COLS "TriggerCols"
#define TAB_TRIGGER_DEPEND "TriggerDepend"
#define TAB_TRIGGER_EXTRACT "TriggerExtract"

#define TAB_DBLINK  "DBLink"
#define TAB_DBLINK_INFO  "DBLinkInfo"
#define TAB_DBLINK_SYNONYMS "DBLinkSynonyms"

#define TAB_DIRECTORIES  "Directories"

#define TAB_ACCESS  "Access"
#define TAB_ACCESS_CONTENT "AccessContent"
#define TAB_ACCESS_USER  "AccessUser"
#define TAB_ACCESS_OBJECTS "AccessObjects"

static toSQL SQLListTablesMysql3("toBrowser:ListTables",
                                 "SHOW TABLES FROM :f1<noquote>",
                                 "List the available tables in a schema.",
                                 "0300",
                                 "QMYSQL");
static toSQL SQLListTablesMysql("toBrowser:ListTables",
                                "SELECT TABLE_NAME TABLES\n"
                                "    FROM information_schema.tables\n"
                                "    WHERE table_schema = :f1",
                                "",
                                "0500",
                                "QMYSQL");
static toSQL SQLListTables("toBrowser:ListTables",
                           "SELECT Table_Name,NULL \" Ignore\",NULL \" Ignore2\",Tablespace_name \" Ignore2\"\n"
                           "  FROM SYS.ALL_ALL_TABLES WHERE OWNER = :f1<char[101]> AND IOT_Name IS NULL\n"
                           "   AND UPPER(TABLE_NAME) LIKE :f2<char[101]>\n"
                           " ORDER BY Table_Name",
                           "",
                           "0800");
// petr vanek 03/01/07 bug #1180847 Error when creating referential constraint
static toSQL SQLListTableNames("toBrowser:ListTableNames",
                               "SELECT Table_Name\n"
                               "  FROM SYS.ALL_ALL_TABLES WHERE OWNER = :f1<char[101]> AND IOT_Name IS NULL\n"
                               " ORDER BY Table_Name",
                               "simplified version of the toBrowser:ListTables",
                               "0800");
static toSQL SQLListTables7("toBrowser:ListTables",
                            "SELECT Table_Name,NULL \" Ignore\",NULL \" Ignore2\",Tablespace_name \" Ignore2\"\n"
                            "  FROM SYS.ALL_TABLES WHERE OWNER = :f1<char[101]>\n"
                            "   AND UPPER(TABLE_NAME) LIKE :f2<char[101]>\n"
                            " ORDER BY Table_Name",
                            "",
                            "0703");
static toSQL SQLListTablesPgSQL("toBrowser:ListTables",
                                "SELECT c.relname AS \"Table Name\"\n"
                                "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                                " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                                "   AND c.relkind = 'r'"
                                " ORDER BY \"Table Name\"",
                                "",
                                "0701",
                                "QPSQL");
static toSQL SQLListTablesSapDB("toBrowser:ListTables",
                                "SELECT tablename \"Table Name\"\n"
                                " FROM tables \n"
                                " WHERE tabletype = 'TABLE' and owner = upper(:f1<char[101]>) \n"
                                " ORDER by tablename",
                                "",
                                "",
                                "SapDB");
static toSQL SQLListTablesTD("toBrowser:ListTables",
                             "SELECT trim ( tablename ) AS \"Table Name\"\n"
                             "  FROM dbc.TABLES\n"
                             " WHERE databasename = trim ( upper ( :f1<char[101]> ) )\n"
                             "   AND tablekind = 'T'\n"
                             " ORDER BY 1",
                             "",
                             "",
                             "Teradata");
static toSQL SQLListViewPgSQL("toBrowser:ListView",
                              "SELECT c.relname as View_Name\n"
                              "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                              " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                              "   AND c.relkind = 'v'"
                              " ORDER BY View_Name",
                              "List the available views in a schema",
                              "0701",
                              "QPSQL");
static toSQL SQLListView("toBrowser:ListView",
                         "SELECT View_Name FROM SYS.ALL_VIEWS WHERE OWNER = :f1<char[101]>\n"
                         "   AND UPPER(VIEW_NAME) LIKE :f2<char[101]>\n"
                         " ORDER BY View_Name",
                         "");
static toSQL SQLListViewSapDb("toBrowser:ListView",
                              "SELECT tablename \"View_Name\"\n"
                              " FROM tables \n"
                              " WHERE tabletype = 'VIEW' and owner = upper(:f1<char[101]>)\n"
                              " ORDER by tablename",
                              "",
                              "",
                              "SapDB");
static toSQL SQLListViewTD("toBrowser:ListView",
                           "SELECT trim ( tablename ) AS \"View_Name\"\n"
                           "  FROM dbc.TABLES\n"
                           " WHERE databasename = trim ( upper ( :f1<char[101]> ) )\n"
                           "   AND tablekind = 'V'\n"
                           " ORDER BY 1",
                           "",
                           "",
                           "Teradata");
static toSQL SQLListIndexMySQL3("toBrowser:ListIndex",
                                "SHOW INDEX FROM :f1<char[100]>",
                                "List the available indexes in a schema",
                                "0323",
                                "QMYSQL");
static toSQL SQLListIndexMySQL("toBrowser:ListIndex",
                               "select distinct table_name \"Tables with Indexes\"\n"
                               "    from information_schema.statistics\n"
                               "    where index_schema = :f1<char[100]>\n"
                               "    order by 1\n",
                               "",
                               "0500",
                               "QMYSQL");
static toSQL SQLListIndex("toBrowser:ListIndex",
                          "SELECT Index_Name,NULL \" Ignore\",NULL \" Ignore2\",Tablespace_name \" Ignore2\"\n"
                          "  FROM SYS.ALL_INDEXES\n"
                          " WHERE OWNER = :f1<char[101]>\n"
                          "   AND UPPER(INDEX_NAME) LIKE :f2<char[101]>\n"
                          " ORDER BY Index_Name\n",
                          "");
static toSQL SQLListIndexPgSQL("toBrowser:ListIndex",
                               "SELECT c.relname AS \"Index Name\"\n"
                               "FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                               "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                               "  AND c.relkind = 'i'\n"
                               "ORDER BY \"Index Name\"",
                               "",
                               "0701",
                               "QPSQL");
static toSQL SQLListIndexSapDb("toBrowser:ListIndex",
                               "SELECT IndexName \"Index Name\"\n"
                               " FROM indexes \n"
                               " WHERE  owner = upper(:f1<char[101]>)",
                               "",
                               "",
                               "SapDB");
static toSQL SQLListSequencePgSQL("toBrowser:ListSequence",
                                  "SELECT c.relname AS \"Sequence Name\"\n"
                                  "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                                  " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                                  "   AND c.relkind = 'S'\n"
                                  " ORDER BY \"Sequence Name\"",
                                  "List the available sequences in a schema",
                                  "0701",
                                  "QPSQL");
static toSQL SQLListSequence("toBrowser:ListSequence",
                             "SELECT Sequence_Name FROM SYS.ALL_SEQUENCES\n"
                             " WHERE SEQUENCE_OWNER = :f1<char[101]>\n"
                             "   AND UPPER(SEQUENCE_NAME) LIKE :f2<char[101]>\n"
                             " ORDER BY Sequence_Name",
                             "");
static toSQL SQLListSynonym("toBrowser:ListSynonym",
                            "SELECT DECODE(Owner,'PUBLIC','',Owner||'.')||Synonym_Name \"Synonym Name\"\n"
                            "  FROM Sys.All_Synonyms\n"
                            " WHERE Table_Owner = :f1<char[101]>\n"
                            "    OR Owner = :f1<char[101]>\n"
                            "   AND UPPER(Synonym_Name) LIKE :f2<char[101]>\n"
                            " ORDER BY Synonym_Name",
                            "List the available synonyms in a schema");
static toSQL SQLListTrigger("toBrowser:ListTrigger",
                            "SELECT Trigger_Name FROM SYS.ALL_TRIGGERS\n"
                            " WHERE OWNER = :f1<char[101]>\n"
                            "   AND UPPER(TRIGGER_NAME) LIKE :f2<char[101]>\n"
                            " ORDER BY Trigger_Name",
                            "List the available triggers in a schema");
static toSQL SQLListDBLink("toBrowser:ListDBLink",
                           "SELECT Db_Link, Owner FROM SYS.ALL_DB_LINKS\n"
                           " WHERE (Owner = :f1<char[101]> or Owner='PUBLIC') and\n"
                           " UPPER(DB_Link) like :f2<char[101]>",
                           "List database links");
static toSQL SQLListDBLinkDBA("toBrowser:ListDBLinkDBA",
                              "SELECT Owner, Db_Link, Username, Host, Created\n"
                              " FROM SYS.DBA_DB_LINK\n",
                              "List database links as DBA");
static toSQL SQLListDirectories("toBrowser:ListDirectories",
                                "SELECT DISTINCT directory_name FROM SYS.ALL_DIRECTORIES\n"
                                " WHERE (owner = 'SYS' or owner=:f1<char[101]>) and\n"
                                " UPPER(directory_name) like :f2<char[101]>"
                                " ORDER BY directory_name",
                                "List database external directories");
static toSQL SQLMySQLAccess("toBrowser:MySQLAcess",
                            "SHOW TABLES FROM mysql",
                            "Show access tables for MySQL databases",
                            "0323",
                            "QMYSQL");
static toSQL SQLMySQLUsers("toBrowser:MySQLUsers",
                           "SELECT concat(user,'@',host) Users FROM mysql.user",
                           "Show users for MySQL databases",
                           "0323",
                           "QMYSQL");
static toSQL SQLTruncateTable("toBrowser:TruncateTable",
                              "TRUNCATE TABLE %1.%2",
                              "Truncate a table",
                              "",
                              "Any");
static toSQL SQLDropUser("toBrowser:DropUser",
                         "DELETE FROM mysql.user WHERE concat(user,'@',host) = :f1<char[255]>",
                         "Drop MYSQL user",
                         "0323",
                         "QMYSQL");

toBrowser::toBrowser(QWidget *parent, toConnection &connection)
    : toToolWidget(BrowserTool, "browser.html", parent, connection, "toBrowser")
    , Filter(new toBrowserFilter(false))
{
    // man toolbar of the tool
    QToolBar *toolbar = Utils::toAllocBar(this, tr("DB Browser"));
    layout()->addWidget(toolbar);

    refreshAct = new QAction(QPixmap(const_cast<const char**>(refresh_xpm)),
                             tr("Update from DB"),
                             this);
    refreshAct->setShortcut(QKeySequence::Refresh);
    connect(refreshAct, SIGNAL(triggered()), this, SLOT(refresh(void)));
    toolbar->addAction(refreshAct);

    toolbar->addSeparator();

    FilterButton = new QAction(tr("&Define filter..."), toolbar);
    FilterButton->setCheckable(true);
    FilterButton->setChecked(!Filter->isEmpty());
    FilterButton->setIcon(QIcon(QPixmap(const_cast<const char**>(filter_xpm))));
    FilterButton->setToolTip(tr("Define the object filter"));
    connect(FilterButton, SIGNAL(triggered(bool)), this, SLOT(defineFilter()));
    FilterButton->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_G);
    toolbar->addAction(FilterButton);

    clearFilterAct = new QAction(QPixmap(const_cast<const char**>(nofilter_xpm)),
                                 tr("Remove any object filter"),
                                 this);
    connect(clearFilterAct, SIGNAL(triggered()), this, SLOT(clearFilter(void)));
    clearFilterAct->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_H);
    toolbar->addAction(clearFilterAct);

    QLabel *strech;
    toolbar->addWidget(strech = new QLabel(toolbar));
    strech->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    strech->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding,
                                      QSizePolicy::Minimum));

    Schema = new toResultSchema(toolbar);
    connect(Schema, SIGNAL(activated(int)),
            this, SLOT(changeSchema(int)));
    toolbar->addWidget(Schema);

    toolbar->addWidget(new QLabel(toolbar));
    new toChangeConnection(toolbar);

    // Main tab widget
    m_mainTab = new QTabWidget(this);
    layout()->addWidget(m_mainTab);

    // Tables
    tableSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    tableSplitter->setObjectName(TAB_TABLES);
//     m_mainTab->addTab(tableSplitter, tr("T&ables"));

    QWidget *tableWidget = new QWidget(tableSplitter);
    QVBoxLayout *tableLayout = new QVBoxLayout;
    tableLayout->setSpacing(0);
    tableLayout->setContentsMargins(0, 0, 0, 0);
    tableWidget->setLayout(tableLayout);

    QToolBar *tableToolbar = Utils::toAllocBar(tableWidget, tr("Database browser"));
    tableLayout->addWidget(tableToolbar);
#ifdef TORA_EXPERIMENTAL
    addTableAct = new QAction(QPixmap(const_cast<const char**>(addtable_xpm)),
                              tr("Create new table"),
                              this);
    //connect(addTableAct, SIGNAL(triggered()), this, SLOT(addTable(void)));
    tableToolbar->addAction(addTableAct);

    tableToolbar->addSeparator();

    modTableAct = new QAction(QPixmap(const_cast<const char**>(modtable_xpm)),
                              tr("Modify table columns"),
                              this);
    //connect(modTableAct, SIGNAL(triggered()), this, SLOT(modifyTable(void)));
    tableToolbar->addAction(modTableAct);

    modConstraintAct = new QAction(QPixmap(const_cast<const char**>(modconstraint_xpm)),
                                   tr("Modify constraints"),
                                   this);
    //connect(modConstraintAct, SIGNAL(triggered()), this, SLOT(modifyConstraint(void)));
    tableToolbar->addAction(modConstraintAct);

    modIndexAct = new QAction(QPixmap(const_cast<const char**>(modindex_xpm)),
                              tr("Modify indexes"),
                              this);
    //connect(modIndexAct, SIGNAL(triggered()), this, SLOT(modifyIndex(void)));
    tableToolbar->addAction(modIndexAct);

    tableToolbar->addSeparator();

    dropTableAct = new QAction(QPixmap(const_cast<const char**>(trash_xpm)),
                               tr("Drop table"),
                               this);
    //connect(dropTableAct, SIGNAL(triggered()), this, SLOT(dropTable(void)));
    tableToolbar->addAction(dropTableAct);

    tableToolbar->addSeparator();

    enableConstraintAct = new QAction(QPixmap(const_cast<const char**>(online_xpm)),
                                      tr("Enable constraint or trigger"),
                                      this);
    //connect(enableConstraintAct, SIGNAL(triggered()), this, SLOT(enableConstraints(void)));
    tableToolbar->addAction(enableConstraintAct);

    disableConstraintAct = new QAction(QPixmap(const_cast<const char**>(offline_xpm)),
                                       tr("Disable constraint or trigger"),
                                       this);
    //connect(disableConstraintAct, SIGNAL(triggered()), this, SLOT(disableConstraints(void)));
    tableToolbar->addAction(disableConstraintAct);
#endif
    // Tables
    tableView = new toBrowserSchemaTableView(tableWidget, "TABLE");
    tableLayout->addWidget(tableView);
    tableView->setReadAll(true);
    tableView->setSQL(SQLListTables);
    setFocusProxy(tableView);
    tableWidget->resize(FIRST_WIDTH, tableView->height());
    tableSplitter->setStretchFactor(tableSplitter->indexOf(tableWidget), 0);

    tableBrowserWidget = new toBrowserTableWidget(tableSplitter);
    tableSplitter->setStretchFactor(tableSplitter->indexOf(tableBrowserWidget), 1);

    connect(tableView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    //connect(tableView, SIGNAL(displayMenu(QMenu *)), this, SLOT(displayTableMenu(QMenu *)));
    connect(tableBrowserWidget, SIGNAL(selected(const QString&)), this, SLOT(slotSelected(const QString&)));
    connect(this,      SIGNAL(filterChanged(toViewFilter*)), tableView, SLOT(setFilter(toViewFilter*)));

    m_objectsMap[tableSplitter] = tableView;
    m_browsersMap[tableSplitter] = tableBrowserWidget;

    // Views
    viewSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    viewSplitter->setObjectName(TAB_VIEWS);
//     m_mainTab->addTab(viewSplitter, tr("&Views"));
    viewView = new toBrowserSchemaTableView(viewSplitter, "VIEW");
    viewView->setReadAll(true);
    viewView->setSQL(SQLListView);
    viewView->resize(FIRST_WIDTH, viewView->height());
    connect(viewView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    // connect context menu for views
    connect(viewView, SIGNAL(displayMenu(QMenu *)), this, SLOT(displayViewMenu(QMenu *)));
    connect(this,     SIGNAL(filterChanged(toViewFilter*)), viewView, SLOT(setFilter(toViewFilter*)));

    viewSplitter->setStretchFactor(viewSplitter->indexOf(viewView), 0);

    viewBrowserWidget = new toBrowserViewWidget(viewSplitter);
    viewSplitter->setStretchFactor(viewSplitter->indexOf(viewBrowserWidget), 1);

    m_objectsMap[viewSplitter] = viewView;
    m_browsersMap[viewSplitter] = viewBrowserWidget;

    dropViewAct = new QAction(QPixmap(const_cast<const char**>(trash_xpm)),
                              tr("Drop view"),
                              this);
#ifdef TORA_EXPERIMENTAL
    //connect(dropViewAct, SIGNAL(triggered()), this, SLOT(dropView(void)));
#endif
    // Indexes
    indexSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    indexSplitter->setObjectName(TAB_INDEX);
//     m_mainTab->addTab(indexSplitter, tr("Inde&xes"));

    QWidget * indexWidget = new QWidget(indexSplitter);
    QVBoxLayout * indexLayout = new QVBoxLayout;
    indexLayout->setSpacing(0);
    indexLayout->setContentsMargins(0, 0, 0, 0);
    indexWidget->setLayout(indexLayout);

    QToolBar * indexToolbar = Utils::toAllocBar(indexWidget, tr("Database browser"));
    indexLayout->addWidget(indexToolbar);
#ifdef TORA_EXPERIMENTAL
    addIndexesAct = new QAction(QPixmap(const_cast<const char**>(addindex_xpm)),
                                tr("Add indexes"),
                                this);
    //connect(addIndexesAct, SIGNAL(triggered()), this, SLOT(addIndex()));
    indexToolbar->addAction(addIndexesAct);

    indexToolbar->addSeparator();

    indexToolbar->addAction(modIndexAct);

    indexToolbar->addSeparator();

    dropIndexesAct = new QAction(QPixmap(const_cast<const char**>(trash_xpm)),
                                 tr("Drop index"),
                                 this);
    //connect(dropIndexesAct, SIGNAL(triggered()), this, SLOT(dropIndex()));
    indexToolbar->addAction(dropIndexesAct);
#endif
    indexView = new toBrowserSchemaTableView(indexWidget, "INDEX");
    indexLayout->addWidget(indexView);
    indexView->setReadAll(true);
    indexView->setSQL(SQLListIndex);
    connect(indexView, SIGNAL(selectionChanged()),   this, SLOT(changeItem()));
    //connect(indexView, SIGNAL(displayMenu(QMenu *)), this, SLOT(displayIndexMenu(QMenu *)));
    connect(this,      SIGNAL(filterChanged(toViewFilter*)), indexView, SLOT(setFilter(toViewFilter*)));

    indexWidget->resize(FIRST_WIDTH, indexView->height());
    indexSplitter->setStretchFactor(indexSplitter->indexOf(indexWidget), 0);

    indexBrowserWidget = new toBrowserIndexWidget(indexSplitter);
    indexSplitter->setStretchFactor(indexSplitter->indexOf(indexBrowserWidget), 1);

    m_objectsMap[indexSplitter] = indexView;
    m_browsersMap[indexSplitter] = indexBrowserWidget;

    // Sequences
    sequenceSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    sequenceSplitter->setObjectName(TAB_SEQUENCES);
//     m_mainTab->addTab(sequenceSplitter, tr("Se&quences"));
    sequenceView = new toBrowserSchemaTableView(sequenceSplitter, "SEQUENCE");
    sequenceView->setReadAll(true);
    sequenceView->setSQL(SQLListSequence);
    sequenceView->resize(FIRST_WIDTH, sequenceView->height());
    connect(sequenceView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    connect(this,         SIGNAL(filterChanged(toViewFilter*)), sequenceView, SLOT(setFilter(toViewFilter*)));

    sequenceSplitter->setStretchFactor(sequenceSplitter->indexOf(sequenceView), 0);

    sequenceBrowserWidget = new toBrowserSequenceWidget(sequenceSplitter);
    sequenceSplitter->setStretchFactor(sequenceSplitter->indexOf(sequenceBrowserWidget), 1);

    m_objectsMap[sequenceSplitter] = sequenceView;
    m_browsersMap[sequenceSplitter] = sequenceBrowserWidget;


    // Synonyms
    synonymSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    synonymSplitter->setObjectName(TAB_SYNONYM);
//     m_mainTab->addTab(synonymSplitter, tr("S&ynonyms"));
    synonymView = new toBrowserSchemaTableView(synonymSplitter); // Note, object cache does not cache synonyms
    synonymView->setReadAll(true);
    synonymView->setSQL(SQLListSynonym);
    synonymView->resize(FIRST_WIDTH, synonymView->height());
    connect(synonymView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    connect(this,        SIGNAL(filterChanged(toViewFilter*)), synonymView, SLOT(setFilter(toViewFilter*)));

    synonymSplitter->setStretchFactor(synonymSplitter->indexOf(synonymView), 0);

    synonymBrowserWidget = new toBrowserSynonymWidget(synonymSplitter);
    synonymSplitter->setStretchFactor(synonymSplitter->indexOf(synonymBrowserWidget), 1);

    m_objectsMap[synonymSplitter] = synonymView;
    m_browsersMap[synonymSplitter] = synonymBrowserWidget;

    // Codes
    codeSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    codeSplitter->setObjectName(TAB_PLSQL);
//     m_mainTab->addTab(codeSplitter, tr("Cod&e"));
    codeView = new toBrowserSchemaCodeBrowser(codeSplitter);
//     codeView->setReadAll(true);
//     codeView->setSQL(SQLListSQL);
    codeView->resize(FIRST_WIDTH * 2, codeView->height());
    connect(codeView, SIGNAL(clicked(const QModelIndex &)),  this, SLOT(changeItem(const QModelIndex &)));
    connect(this, SIGNAL(filterChanged(toViewFilter*)), codeView, SLOT(setFilter(toViewFilter*)));

    codeSplitter->setStretchFactor(codeSplitter->indexOf(codeView), 0);

    codeBrowserWidget = new toBrowserCodeWidget(codeSplitter);
    codeSplitter->setStretchFactor(codeSplitter->indexOf(codeBrowserWidget), 1);

    m_objectsMap[codeSplitter] = codeView;
    m_browsersMap[codeSplitter] = codeBrowserWidget;


    // Triggers
    triggerSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    triggerSplitter->setObjectName(TAB_TRIGGER);
//     m_mainTab->addTab(triggerSplitter, tr("Tri&ggers"));
    triggerView = new toBrowserSchemaTableView(triggerSplitter, "TRIGGER");
    triggerView->setReadAll(true);
    triggerView->setSQL(SQLListTrigger);
    triggerView->resize(FIRST_WIDTH, triggerView->height());
    connect(triggerView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    connect(this, SIGNAL(filterChanged(toViewFilter*)), triggerView, SLOT(setFilter(toViewFilter*)));

    triggerSplitter->setStretchFactor(triggerSplitter->indexOf(triggerView), 0);

    triggerBrowserWidget = new toBrowserTriggerWidget(triggerSplitter);
    triggerSplitter->setStretchFactor(triggerSplitter->indexOf(triggerBrowserWidget), 1);

    m_objectsMap[triggerSplitter] = triggerView;
    m_browsersMap[triggerSplitter] = triggerBrowserWidget;

    dblinkSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    dblinkSplitter->setObjectName(TAB_DBLINK);
//     m_mainTab->addTab(dblinkSplitter, tr("DBLinks"));

    QWidget * dblinkWidget = new QWidget(dblinkSplitter);

    QVBoxLayout * dblinkLayout = new QVBoxLayout;
    dblinkLayout->setSpacing(0);
    dblinkLayout->setContentsMargins(0, 0, 0, 0);
    dblinkWidget->setLayout(dblinkLayout);

    QToolBar * dblinkToolbar = Utils::toAllocBar(dblinkWidget, tr("Database browser"));
    dblinkLayout->addWidget(dblinkToolbar);

    dblinkView = new toBrowserSchemaTableView(dblinkWidget);
    dblinkBrowserWidget = new toBrowserDBLinksWidget(dblinkSplitter);

    testDBLinkAct = new QAction(QPixmap(const_cast<const char**>(modconstraint_xpm)),
                                tr("Test DBLink"), this);
    connect(testDBLinkAct, SIGNAL(triggered()), dblinkBrowserWidget, SLOT(testDBLink()));
    dblinkToolbar->addAction(testDBLinkAct);

    dblinkLayout->addWidget(dblinkView);
    dblinkView->setReadAll(true);
    dblinkView->setSQL(SQLListDBLink);
    dblinkView->resize(FIRST_WIDTH, dblinkView->height());

    connect(dblinkView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    // connect(dblinkView, SIGNAL(displayMenu(QMenu *)), this, SLOT(displayIndexMenu(QMenu *)));
    connect(this, SIGNAL(filterChanged(toViewFilter*)), dblinkView, SLOT(setFilter(toViewFilter*)));

    dblinkWidget->resize(FIRST_WIDTH, dblinkView->height());
    dblinkSplitter->setStretchFactor(dblinkSplitter->indexOf(dblinkView), 0);
    dblinkSplitter->setStretchFactor(dblinkSplitter->indexOf(dblinkBrowserWidget), 1);

    m_objectsMap[dblinkSplitter] = dblinkView;
    m_browsersMap[dblinkSplitter] = dblinkBrowserWidget;

    directoriesSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    directoriesSplitter->setObjectName(TAB_DIRECTORIES);

    QWidget * directoriesWidget = new QWidget(directoriesSplitter);

    QVBoxLayout * directoriesLayout = new QVBoxLayout;
    directoriesLayout->setSpacing(0);
    directoriesLayout->setContentsMargins(0, 0, 0, 0);
    directoriesWidget->setLayout(directoriesLayout);

    directoriesView = new toBrowserSchemaTableView(directoriesWidget, "DIRECTORY");
    directoriesBrowserWidget = new toBrowserDirectoriesWidget(directoriesSplitter);

    directoriesLayout->addWidget(directoriesView);
    directoriesView->setReadAll(true);
    directoriesView->setSQL(SQLListDirectories);
    directoriesView->resize(FIRST_WIDTH, directoriesView->height());

    connect(directoriesView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    // connect(directoriesView, SIGNAL(displayMenu(QMenu *)), this, SLOT(displayIndexMenu(QMenu *)));
    connect(this, SIGNAL(filterChanged(toViewFilter*)), directoriesView, SLOT(setFilter(toViewFilter*)));

    directoriesWidget->resize(FIRST_WIDTH, directoriesView->height());
    directoriesSplitter->setStretchFactor(directoriesSplitter->indexOf(directoriesView), 0);
    directoriesSplitter->setStretchFactor(directoriesSplitter->indexOf(directoriesBrowserWidget), 1);

    m_objectsMap[directoriesSplitter] = directoriesView;
    m_browsersMap[directoriesSplitter] = directoriesBrowserWidget;


    accessSplitter = new QSplitter(Qt::Horizontal, m_mainTab);
    accessSplitter->setObjectName(TAB_ACCESS);
//     m_mainTab->addTab(accessSplitter, tr("Access"));

#ifdef TOEXTENDED_MYSQL
    // This is propably never compiled... and never worked...
    QWidget * accessWidget = new QWidget(accessSplitter);
    QVBoxLayout *accessLayout = new QVBoxLayout;
    accessLayout->setSpacing(0);
    accessLayout->setContentsMargins(0, 0, 0, 0);
    accessWidget->setLayout(accessLayout);

    QToolBar * accessToolBar = Utils::toAllocBar(box, tr("Database browser"));
    accessLayout->addWidget(accessToolBar);

    QAction * addUserAct = accessToolBar->addAction(QPixmap(const_cast<const char**>(new_xpm)),
                           tr("Add user"),
                           this, SLOT(addUser()));
    QAction * dropUserAct = accessToolBar->addAction(QPixmap(const_cast<const char**>(trash_xpm)),
                            tr("Drop user"),
                            this, SLOT(dropUser()));

    accessView = new toBrowserSchemaTableView(accessWidget);
    accessView->setSQL(SQLMySQLUsers);
    AccessContent = NULL;
    accessLayout->addWidget(accessView);
    accessWidget->resize(FIRST_WIDTH, accessView->height());
#else
    accessView = new toBrowserSchemaTableView(accessSplitter);
    accessView->resize(FIRST_WIDTH, accessView->height());
    accessView->setSQL(SQLMySQLAccess);
#endif
    accessView->setReadAll(true);

    accessBrowserWidget = new toBrowserAccessWidget(accessSplitter);

    accessSplitter->setStretchFactor(accessSplitter->indexOf(accessView), 0);
    accessSplitter->setStretchFactor(accessSplitter->indexOf(accessBrowserWidget), 1);

    m_objectsMap[accessSplitter] = accessView;
    m_browsersMap[accessSplitter] = accessBrowserWidget;

    connect(accessView, SIGNAL(selectionChanged()), this, SLOT(changeItem()));
    connect(this, SIGNAL(filterChanged(toViewFilter*)), accessView, SLOT(setFilter(toViewFilter*)));

    // End of tabs. Now the common things are comming...
    ToolMenu = NULL;
    // connect(toMainWidget()->workspace(),
    //         SIGNAL(subWindowActivated(QMdiSubWindow *)),
    //         this,
    //         SLOT(windowActivated(QMdiSubWindow *)));

    Schema->setFocus();
    Schema->refresh();
    changeConnection();

    connect(this, SIGNAL(connectionChange()),
            this, SLOT(changeConnection()));
    connect(m_mainTab, SIGNAL(currentChanged(int)),
            this, SLOT(mainTab_currentChanged(int)));

    emit filterChanged(Filter);
}

void toBrowser::mainTab_currentChanged(int /*ix*/, Caching caching) // caching = USE_CACHE is the default
{
    if (Schema->selected().isEmpty())
        return;

    // It can be called when there is no widget at all
    // e.g. from refresh(). Exit silently in this case.
    if (!m_mainTab->currentWidget())
        return;

    QSplitter * ix = qobject_cast<QSplitter*>(m_mainTab->currentWidget());
    Q_ASSERT_X(ix, "toBrowser::mainTab_currentChanged",
               "main widget of the tab is not QSplitter as is mandatory!");

    if (m_objectsMap.contains(ix))
    {
        if (caching == NO_USE_CACHE)
            m_objectsMap[ix]->forceRequery();
        m_objectsMap[ix]->refreshWithParams(schema(), Filter ? Filter->wildCard() : "%");
        changeItem();
    }
    else
    {
        TLOG(2, toDecorator, __HERE__) << "mainTab_currentChanged unhandled index:" << ix;
    }
}

void toBrowser::slotSelected(const QString& object)
{
    // It can be called when there is no widget at all
    // e.g. from refresh(). Exit silently in this case.
    if (!m_mainTab->currentWidget())
        return;

    QSplitter * ix = qobject_cast<QSplitter*>(m_mainTab->currentWidget());
    if (m_objectsMap.contains(ix) && m_browsersMap.contains(ix))
    {
        toBrowserSchemaBase *b = m_objectsMap[ix];
        if (QTableView *tv = dynamic_cast<QTableView*>(b))
        {
            QAbstractItemModel *model = tv->model();

//    		QSortFilterProxyModel proxy;
//    		proxy.setSourceModel(model);
//    		proxy.setFilterFixedString(object);
//    		int num_rows = model->rowCount();
//    		int num_cols = model->columnCount();
//    		QModelIndex matchingIndex = proxy.mapFromSource(proxy.index(num_rows,1/*num_cols*/));
//    		if(matchingIndex.isValid())
//    		{
//    			tv->setCurrentIndex(matchingIndex);
//    		}

            QModelIndexList matches = model->match(model->index(0,1), Qt::DisplayRole, object, 1, Qt::MatchExactly);
            if (!matches.isEmpty() && matches.first().isValid())
            {
                //tv->setCurrentIndex(matches.first());
                //tv->selectRow(matches.first().row());
                tv->selectionModel()->setCurrentIndex(matches.first(), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
                tv->selectionModel()->select(matches.first(), QItemSelectionModel::ClearAndSelect|QItemSelectionModel::Current);
            }
        }
    }

}

void toBrowser::slotWindowActivated(toToolWidget* widget)
{
    if (!widget)
        return;
    if (widget == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Browser"), this);

            ToolMenu->addAction(refreshAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(FilterButton);
            ToolMenu->addAction(clearFilterAct);

            toGlobalEventSingle::Instance().addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

QString toBrowser::schema(void) const
{
    try
    {
#pragma message WARN("TODO: Override roResutSchema::selected(), handle empty")
        QString ret = Schema->selected();
        if (ret == tr("No schemas"))
            return connection().database();
        return ret;
    }
    catch (...)
    {
        TLOG(1, toDecorator, __HERE__) << "	Ignored exception." << std::endl;
        return QString::null;
    }
}

void toBrowser::setNewFilter(toBrowserFilter *filter)
{
    if (Filter)
    {
        delete Filter;
        Filter = NULL;
    }
    if (filter)
        Filter = filter;
    else
        Filter = new toBrowserFilter();
    FilterButton->setChecked(filter);

    emit filterChanged(Filter);
    refresh();
}

#ifdef TORA3_BROWSER_TOOLS
void toBrowser::addUser()
{
#ifdef TOEXTENDED_MYSQL
    UserPanel->changeParams(QString::null, QString::null);
#endif
}

void toBrowser::dropUser()
{
#ifdef TOEXTENDED_MYSQL
// This propably never woked...
    try
    {
        AccessPanel->dropCurrentAccess();
        connection().execute(SQLDropUser, selectedItem()->text(0));
        flushPrivs();
        refresh();
    }
    TOCATCH
#endif
}
#endif

toBrowser::~toBrowser()
{
    if (Filter)
        delete Filter;
}

void toBrowser::changeSchema(int)
{
    try
    {
        Schema->refresh();
        mainTab_currentChanged(m_mainTab->currentIndex(), USE_CACHE); // just a test do now requery the DB   // true);
    }
    TOCATCH
}

void toBrowser::refresh(void)
{
    try
    {
        mainTab_currentChanged(m_mainTab->currentIndex(), NO_USE_CACHE); // just a test do now requery the DB   // true);
    }
    TOCATCH
}

void toBrowser::changeConnection(void)
{
    try
    {
        m_mainTab->blockSignals(true);

        // enable/disable main tabs depending on DB
        m_mainTab->clear();
        addTab(tableSplitter, tr("T&ables"), true);
        addTab(viewSplitter, tr("&Views"), !connection().providerIs("QMYSQL"));
        addTab(indexSplitter, tr("Inde&xes"), true);
        addTab(sequenceSplitter, tr("Se&quences"), connection().providerIs("Oracle") || connection().providerIs("QPSQL"));
        addTab(synonymSplitter, tr("S&ynonyms"), connection().providerIs("Oracle"));

        // 2010-03-31
        // Starting with version 5.0 MySQL supports stored functions/procedures
        // If TOra is used a lot with older versions of MySQL the "true" parameter
        // should be enhanced with a check for MySQL version
        addTab(codeSplitter, tr("Cod&e"), true);
        addTab(triggerSplitter, tr("Tri&ggers"), !connection().providerIs("QMYSQL") && !connection().providerIs("QPSQL"));
        addTab(dblinkSplitter, tr("DBLinks"), connection().providerIs("Oracle"));
        addTab(directoriesSplitter, tr("Directories"), connection().providerIs("Oracle"));
        addTab(accessSplitter, tr("Access"), connection().providerIs("QMYSQL"));

        foreach (toBrowserBaseWidget * w, m_browsersMap.values())
        w->changeConnection();

        toToolWidget::setCaption(QString());

        m_mainTab->setCurrentIndex(0);

        m_mainTab->blockSignals(false);
    }
    catch (...)
    {
        m_mainTab->blockSignals(false);
        throw;
    }

    refresh();
}

int toBrowser::addTab(QSplitter * page, const QString & label, bool enable)
{
    page->setVisible(enable);
    if (!enable)
        return -1;
    return m_mainTab->addTab(page, label);
}

QString toBrowser::currentItemText(int col)
{
    QSplitter * ix = qobject_cast<QSplitter*>(m_mainTab->currentWidget());
    Q_ASSERT_X(ix, "toBrowser::currentItemText",
               "main widget of the tab is not QSplitter as is mandatory!");

    if (m_objectsMap.contains(ix))
    {
//         return m_objectsMap[ix]->selectedIndex(1).data(Qt::EditRole).toString();
        return m_objectsMap[ix]->objectName();
    }
    return "";
}

void toBrowser::changeItem()
{
    QSplitter * ix = qobject_cast<QSplitter*>(m_mainTab->currentWidget());
    Q_ASSERT_X(ix, "toBrowser::changeItem",
               "main widget of the tab is not QSplitter as is mandatory!");

    if (m_browsersMap.contains(ix))
    {
        if (ix != codeSplitter)
            m_browsersMap[ix]->changeParams(schema(), currentItemText());
        else
        {
            // If code browser has been clicked we need to know type of code (function, procedure...) too
            // as it is not possible to identify code by just schema and object (in MySQL there can be
            // a function and procedure with the same name in the same schema)
            toBrowserSchemaCodeBrowser * browser = dynamic_cast<toBrowserSchemaCodeBrowser*>(m_objectsMap[ix]);
            if (!browser)
            {
                qDebug("Only for code - toBrowserSchemaCodeBrowser cast!");
                return;
            }

            m_browsersMap[ix]->changeParams(schema(), currentItemText(), browser->objectType());
        }
        toToolWidget::setCaption(currentItemText());
    }
    else
    {
        TLOG(2, toDecorator, __HERE__) << "changeItem() unhandled index" << ix;
    }
}

void toBrowser::changeItem(const QModelIndex &)
{
    // It's called only from the code view
    QSplitter * ix = qobject_cast<QSplitter*>(m_mainTab->currentWidget());
    if (ix != codeSplitter)
    {
        qDebug("Only for code - QSplitter cast!");
        return;
    }
    changeItem();

    toBrowserSchemaCodeBrowser * browser = dynamic_cast<toBrowserSchemaCodeBrowser*>(m_objectsMap[ix]);
    if (!browser)
    {
        qDebug("Only for code - toBrowserSchemaCodeBrowser cast!");
        return;
    }
    if (browser->objectType() == "SPEC")
        m_browsersMap[ix]->setCurrentIndex(toBrowserCodeWidget::SpecTab);
    else if (browser->objectType() == "BODY")
        m_browsersMap[ix]->setCurrentIndex(toBrowserCodeWidget::BodyTab);
    else if (browser->objectType() == "TYPE")
        m_browsersMap[ix]->setCurrentIndex(toBrowserCodeWidget::SpecTab);
    else if (browser->objectType() == "PROCEDURE"
             || browser->objectType() == "FUNCTION")
        m_browsersMap[ix]->setCurrentIndex(toBrowserCodeWidget::BodyTab);
}

void toBrowser::clearFilter(void)
{
    setNewFilter(NULL);
}

void toBrowser::defineFilter(void)
{
    if (Filter)
    {
        toBrowserFilterSetup filt(false, *Filter, this);
        if (filt.exec())
            setNewFilter(filt.getSetting());
    }
    else
    {
        toBrowserFilterSetup filt(false, this);
        if (filt.exec())
            setNewFilter(filt.getSetting());
    }

    if (!Filter)
        FilterButton->setChecked(false);
    else
        FilterButton->setChecked(!Filter->isEmpty());
}

bool toBrowser::canHandle(const toConnection &conn)
{
    return conn.providerIs("Oracle") ||
           conn.providerIs("QPSQL") ||
           conn.providerIs("QMYSQL") ||
           conn.providerIs("SapDB");
}

void toBrowser::commitChanges()
{
    //TODO
}

void toBrowser::rollbackChanges()
{
    //TODO
}

#if TORA3_BROWSER_TOOLS
void toBrowser::modifyTable(void)
{
    //obsolete if (m_mainTab->currentWidget() != tableSplitter)
    //     return; // only tabs allowed
    // toBrowserTable::editTable(connection(),
    //                           schema(),
    //                           currentItemText(),
    //                           this);
    // refresh();
    throw tr("toBrowser::modifyTable(void) not implement yet");
}

void toBrowser::addTable(void)
{
    //obsolete  toBrowserTable::newTable(connection(),
    //                          Schema->selected(),
    //                          this);
    // refresh();
    throw tr("toBrowser::addTable(void) not implement yet");
}

void toBrowser::modifyConstraint(void)
{
    //obsolete if (m_mainTab->currentWidget() != tableSplitter)
    //     return;

    // toBrowserConstraint::modifyConstraint(connection(),
    //                                       schema(),
    //                                       tableBrowserWidget->object(),
    //                                       this);
    // refresh();
    throw tr("toBrowser::modifyConstraint(void) not implement yet");
}

void toBrowser::modifyIndex(void)
{
    //obsolete toBrowserIndex::modifyIndex(connection(),
    //                             schema(),
    //                             tableBrowserWidget->object(),
    //                             this,
    //                             currentItemText());
    // refresh();
    throw tr("toBrowser::modifyIndex(void) not implement yet");
}

void toBrowser::addIndex(void)
{
    //obsolete toBrowserIndex::addIndex(connection(),
    //                          schema(),
    //                          tableBrowserWidget->object(),
    //                          this);
    // refresh();
    throw tr("toBrowser::addIndex(void) not implement yet");
}

void toBrowser::displayTableMenu(QMenu *menu)
{
    menu->addSeparator();

    menu->addAction(dropTableAct);
    menu->addAction(tr("Truncate table"), this, SLOT(truncateTable()));

    menu->addSeparator();

    if (connection().providerIs("QMYSQL"))
    {
        menu->addAction(tr("Check table"), this, SLOT(checkTable()));
        menu->addAction(tr("Optimize table"), this, SLOT(optimizeTable()));
        menu->addAction(tr("Analyze table"), this, SLOT(analyzeTable()));
        menu->addAction(tr("Change type"), this, SLOT(changeType()));

        menu->addSeparator();
    }

    menu->addAction(modConstraintAct);
    menu->addAction(modIndexAct);
    menu->addAction(addTableAct);

    menu->addSeparator();

    menu->addAction(refreshAct);
}

void toBrowser::displayViewMenu(QMenu *menu)
{
    menu->addSeparator();
    menu->addAction(dropViewAct);
} // displayViewMenu

void toBrowser::displayIndexMenu(QMenu *menu)
{
    menu->addSeparator();

    menu->addAction(QIcon(trash_xpm), tr("Drop index"), this, SLOT(dropIndex()));
    menu->addAction(QIcon(modindex_xpm), tr("Modify index"), this, SLOT(modifyIndex()));
    menu->addAction(QIcon(addindex_xpm), tr("Create index"), this, SLOT(addIndex()));

    menu->addSeparator();

    menu->addAction(QIcon(refresh_xpm), tr("Refresh"), this, SLOT(refresh()));
}

void toBrowser::dropSomething(const QString &type, const QString &what)
{
    //obsolete if (what.isEmpty())
    //     return ;
    // if (TOMessageBox::warning(this, tr("Dropping %1?").arg(tr(type.toLatin1().constData())),
    //                           tr("Are you sure you want to drop the %1 %2.%3?\n"
    //                              "This action can not be undone!").arg(tr(type.toLatin1().constData())).arg(
    //                               Schema->selected()).arg(what),
    //                           tr("&Yes"), tr("&Cancel"), QString::null, 0) == 0)
    // {
    //     std::list<QString> ctx;
    //     Utils::toPush(ctx, Schema->selected());
    //     Utils::toPush(ctx, type.toUpper());
    //     QStringList parts = what.split(".");
    //     if (parts.count() > 1)
    //     {
    //         Utils::toPush(ctx, parts[1]);
    //         Utils::toPush(ctx, QString("ON"));
    //         Utils::toPush(ctx, parts[0]);
    //     }
    //     else
    //     {
    //         Utils::toPush(ctx, what);
    //     }

    //     std::list<QString> drop;
    //     toExtract::addDescription(drop, ctx);

    //     std::list<QString> empty;

    //     try
    //     {
    //         toExtract extractor(connection(), NULL);
    //         extractor.setIndexes(false);
    //         extractor.setConstraints(false);
    //         extractor.setPrompt(false);
    //         extractor.setHeading(false);

    //         std::list<QString> objToDrop;
    //         QString o = type.toUpper() + ":" + Schema->selected() + "." + what;
    //         objToDrop.push_back(o);
    //         QString sql = extractor.drop(objToDrop);

    //         std::list<toSQLParse::statement> statements = toSQLParse::parse(sql, connection());
    //         QProgressDialog prog(tr("Executing %1 change script").arg(tr(type.toLatin1().constData())),
    //                              tr("Stop"),
    //                              0,
    //                              statements.size(),
    //                              this);
    //         prog.setWindowTitle(tr("Performing %1 changes").arg(tr(type.toLatin1().constData())));

    //         for (std::list<toSQLParse::statement>::iterator j = statements.begin(); j != statements.end(); j++)
    //         {
    //             QString sql = toSQLParse::indentStatement(*j, connection());
    //             int i = sql.length() - 1;
    //             while (i >= 0 && (sql.at(i) == ';' || sql.at(i).isSpace()))
    //                 i--;
    //             if (i >= 0)
    //             {
    //                 toConnectionSubLoan connSub(connection());
    //                 connSub->execute(sql.mid(0, i + 1));
    //             }
    //             qApp->processEvents();
    //             if (prog.wasCanceled())
    //                 throw tr("Canceled ongoing %1 modification, %2 might be corrupt").arg(tr(type.toLatin1().constData())).arg(tr(type.toLatin1().constData()));
    //         }
    //     }
    //     TOCATCH
    // }
    throw tr("toBrowser::dropSomething(const QString &type, const QString &what)");

//     refresh(); no refresh goes here as it can be called from loop
}

void toBrowser::dropTable(void)
{
    if (m_mainTab->currentWidget() != tableSplitter)
        return; // only tabs allowed

    for (toResultTableView::iterator it(tableView); (*it).isValid(); it++)
    {
        if (tableView->isRowSelected(*it))
            dropSomething("TABLE", (*it).data(Qt::EditRole).toString());
    }
    refresh();
}

void toBrowser::dropView(void)
{
    if (m_mainTab->currentWidget() != viewSplitter)
        return; // only views allowed

    for (toResultTableView::iterator it(viewView); (*it).isValid(); it++)
    {
        if (viewView->isRowSelected(*it))
            dropSomething("VIEW", (*it).data(Qt::EditRole).toString());
    }
    refresh();
} // dropView

void toBrowser::truncateTable(void)
{
    if (m_mainTab->currentWidget() != tableSplitter)
        return; // only tabs allowed

    bool force = false;
    for (toResultTableView::iterator it(tableView); (*it).isValid(); it++)
    {
        if (tableView->isRowSelected(*it))
        {
            switch (force ? 0 : TOMessageBox::warning(this, tr("Truncate table?"),
                    tr("Are you sure you want to truncate the table %2.%3,\n"
                       "this action can not be undone?").arg(
                        schema()).arg((*it).data(Qt::EditRole).toString()),
                    tr("&Yes"), tr("Yes to &all"), tr("&Cancel"), 0))
            {
                case 1 :
                    force = true;
                    // Intentionally no break here.
                case 0:
                    {
                        toConnectionSubLoan connSub(connection());
                        connSub->execute(toSQL::string(SQLTruncateTable, connection()).
                                         arg(connection().getTraits().quote(schema())).
                                         arg(connection().getTraits().quote((*it).data(Qt::EditRole).toString())));
                    }
                    break;
                case 2:
                    return;
            }
        }
    }
    refresh();
}

void toBrowser::checkTable(void)
{
    QString sql;
    if (m_mainTab->currentWidget() != tableSplitter)
        return; // only tabs allowed

    for (toResultTableView::iterator it(tableView); (*it).isValid(); it++)
    {
        if (tableView->isRowSelected(*it))
        {
            if (sql.isEmpty())
                sql = "CHECK TABLE ";
            else
                sql += ", ";
            sql += connection().getTraits().quote(schema()) + "." +
                   connection().getTraits().quote((*it).data(Qt::EditRole).toString());
        }
    }

    if (!sql.isEmpty())
    {
        toResultTableView *result = new toResultTableView(true, false, this);
        result->setWindowFlags(Qt::Window);
        result->setAttribute(Qt::WA_DeleteOnClose);
        result->query(sql);
        result->show();
    }

}

void toBrowser::optimizeTable(void)
{
    QString sql;
    if (m_mainTab->currentWidget() != tableSplitter)
        return; // only tabs allowed

    for (toResultTableView::iterator it(tableView); (*it).isValid(); it++)
    {
        if (tableView->isRowSelected(*it))
        {
            if (sql.isEmpty())
                sql = "OPTIMIZE TABLE ";
            else
                sql += ", ";
            sql += connection().getTraits().quote(Schema->selected()) + "." +
                   connection().getTraits().quote((*it).data(Qt::EditRole).toString());
        }
    }

    if (!sql.isEmpty())
    {
        toResultTableView *result = new toResultTableView(true, false, this);
        result->setWindowFlags(Qt::Window);
        result->setAttribute(Qt::WA_DeleteOnClose);
        result->query(sql);
        result->show();
    }
}

void toBrowser::changeType(void)
{
    if (m_mainTab->currentWidget() != tableSplitter)
        return; // only tabs allowed

    bool ok;
    QString text = QInputDialog::getText(this,
                                         "Change table type",
                                         "Enter new table type",
                                         QLineEdit::Normal,
                                         "MyISAM",
                                         &ok);
    if (ok && !text.isEmpty())
    {
        for (toResultTableView::iterator it(tableView); (*it).isValid(); it++)
        {
            if (tableView->isRowSelected(*it))
            {
                QString sql = "ALTER TABLE ";
                sql += connection().getTraits().quote(schema()) + "." +
                       connection().getTraits().quote((*it).data(Qt::EditRole).toString());
                sql += " TYPE = " + text;
                try
                {
                    toConnectionSubLoan connSub(connection());
                    connSub->execute(sql);
                }
                TOCATCH
            }
        }
    }
}

void toBrowser::analyzeTable(void)
{
    if (m_mainTab->currentWidget() != tableSplitter)
        return; // only tabs allowed

    QString sql;

    for (toResultTableView::iterator it(tableView); (*it).isValid(); it++)
    {
        if (tableView->isRowSelected(*it))
        {
            if (sql.isEmpty())
                sql = "ANALYZE TABLE ";
            else
                sql += ", ";
            sql += connection().getTraits().quote(schema()) + "." +
                   connection().getTraits().quote((*it).data(Qt::EditRole).toString());
        }
    }

    if (!sql.isEmpty())
    {
        toResultTableView *result = new toResultTableView(true, false, this);
        result->setWindowFlags(Qt::Window);
        result->setAttribute(Qt::WA_DeleteOnClose);
        result->query(sql);
        result->show();
    }
}

void toBrowser::dropIndex(void)
{
    if (m_mainTab->currentWidget() == indexSplitter)
    {
        for (toResultTableView::iterator it(indexView); (*it).isValid(); it++)
        {
            if (indexView->isRowSelected(*it))
            {
                dropSomething("INDEX", (*it).data(Qt::EditRole).toString());
            }
        }
        return;
    }
    // TODO/FIXME: implement deleting from table tab
    TLOG(2, toDecorator, __HERE__) << "toBrowser::dropIndex()" << "indexes can be dropped only directly from indexes tab";

//     for (toResultTableView::iterator it(FirstTab); (*it).isValid(); it++)
//     {
//         if (FirstTab->isRowSelected(*it))
//         {
//             QModelIndex sec = FirstTab->model()->index((*it).row(), 2);
//             QString index = sec.data(Qt::EditRole).toString();
//             if (index != "PRIMARY" && !(*it).data(Qt::EditRole).toString().isEmpty())
//                 dropSomething("INDEX", (*it).data(Qt::EditRole).toString() + "." + index);
//             else
//                 dropSomething("INDEX", (*it).data(Qt::EditRole).toString());
//         }
//     }
}
#endif

void toBrowser::closeEvent(QCloseEvent *event)
{
    bool acceptEvent = true;

    foreach (toBrowserBaseWidget * w, m_browsersMap.values())
    acceptEvent &= w->maybeSave();

    if (acceptEvent)
        event->accept();
    else
        event->ignore();
}


bool toBrowser::close()
{
#ifdef TOEXTENDED_MYSQL
    UserPanel->saveChanges();
#endif

    return toToolWidget::close();
}

#ifdef TORA3_SESSION
void toBrowser::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    // TODO/FIXME
    TLOG(2, toDecorator, __HERE__) << "void toBrowser::exportData(std::map<QString, QString> &data, const QString &prefix)";
    /*    data[prefix + ":Schema"] = Schema->selected();
        data[prefix + ":FirstTab"] = TopTab->currentWidget()->objectName();
        data[prefix + ":SecondText"] = SecondText;
        for (std::map<QString, toResult *>::iterator i = SecondMap.begin();i != SecondMap.end();i++)
        {
            if ((*i).second == SecondTab && Map.find((*i).first) == Map.end())
            {
                data[prefix + ":SecondTab"] = (*i).first;
                break;
            }
        }
        ViewContent->exportData(data, prefix + ":View");
        TableContent->exportData(data, prefix + ":Table");
        if (AccessContent)
            AccessContent->exportData(data, prefix + ":Hosts");
        toToolWidget::exportData(data, prefix);
        if (Filter)
            Filter->exportData(data, prefix + ":Filter");*/
}

void toBrowser::importData(std::map<QString, QString> &data, const QString &prefix)
{
    // TODO/FIXME
    TLOG(2, toDecorator, __HERE__) << "void toBrowser::importData(std::map<QString, QString> &data, const QString &prefix)";
    /*    disconnect(Schema, SIGNAL(activated(int)),
                   this, SLOT(changeSchema(int)));
        disconnect(TopTab, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeTab(QWidget *)));

        ViewContent->importData(data, prefix + ":View");
        TableContent->importData(data, prefix + ":Table");
        if (AccessContent)
            AccessContent->importData(data, prefix + ":Hosts");

        if (data.find(prefix + ":Filter:Type") != data.end())
        {
            toBrowserFilter *filter = new toBrowserFilter;
            filter->importData(data, prefix + ":Filter");
            setNewFilter(filter);
        }
        else
            setNewFilter(NULL);

        toToolWidget::importData(data, prefix);
        QString str = data[prefix + ":Schema"];
        Schema->setSelected(str);
        for (int i = 0;i < Schema->count();i++)
            if (Schema->itemText(i) == str)
                Schema->setCurrentIndex(i);

        str = data[prefix + ":FirstTab"];
        QWidget *chld = findChild<QWidget *>(str);
        if (chld && str.length())
        {
            SecondText = QString::null;
            TopTab->setCurrentIndex(TopTab->indexOf(chld));

            toResultTableView *newtab = Map[chld->objectName()];
            if (newtab != FirstTab && newtab)
            {
                CurrentTop = chld;
                setFocusProxy(newtab);
                FirstTab = newtab;
            }

            str = data[prefix + ":SecondTab"];
            chld = findChild<QWidget *>(str);
            if (chld && str.length())
            {
                QWidget *par = chld->parentWidget();
                while (par && !par->inherits("toTabWidget"))
                    par = par->parentWidget();
                if (par)
                {
                    toTabWidget *tab = dynamic_cast<toTabWidget *>(par);
                    if (tab)
                        tab->setCurrentIndex(tab->indexOf(chld));
                }
                changeSecondTab(chld);
            }
            SecondText = data[prefix + ":SecondText"];
        }

        connect(Schema, SIGNAL(activated(int)),
                this, SLOT(changeSchema(int)));
        connect(TopTab, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeTab(QWidget *)));
        refresh();*/
}
#endif

#if 0
void toBrowser::fixIndexCols(void)
{
    if (providerIs("Oracle")(connection()))
    {
        toResultTableView *tmp = dynamic_cast<toResultTableView *>(SecondMap[TAB_INDEX_COLS]);
        if (tmp)
            for (toTreeWidgetItem *item = tmp->firstChild(); item; item = item->nextSibling())
            {
                if (!toUnnull(item->text(4)).isNull())
                {
                    toResultViewItem * resItem = dynamic_cast<toResultViewItem *>(item);
                    if (resItem)
                        resItem->setText(1, resItem->allText(4));
                }
            }
    }
    else if (connection().providerIs("QMYSQL"))
    {
        toResultTableView *second = dynamic_cast<toResultTableView *>(SecondMap[TAB_INDEX_COLS]);
        if (FirstTab && second)
        {
            QModelIndex item = selectedItem(1);
            if (item.isValid())
            {
                QString index = item.data(Qt::EditRole).toString();
                for (toTreeWidgetItem *item = second->firstChild(); item;)
                {
                    toTreeWidgetItem *t = item->nextSibling();
                    if (item->text(2) != index)
                    {
                        delete item;
                        second->clearParams(); // Make sure it is reexecuted even if same table.
                    }
                    item = t;
                }
            }
        }
    }
}
#endif

// static toBrowseTemplate BrowseTemplate;

// void toBrowseTemplate::removeDatabase(const QString &name)
// {
//     for (std::list<toTemplateItem *>::iterator i = Parents.begin();i != Parents.end();i++)
//     {
//         for (toTreeWidgetItem *item = (*i)->firstChild();item;item = item->nextSibling())
//             if (item->text(0) == name)
//             {
//                 delete item;
//                 break;
//             }
//     }
// }
//
// void toBrowseTemplate::defineFilter(void)
// {
//     if (Filter)
//     {
//         toBrowserFilterSetup filt(true, *Filter, toMainWidget());
//         if (filt.exec())
//         {
//             delete Filter;
//             Filter = filt.getSetting();
//         }
//     }
//     else
//     {
//         toBrowserFilterSetup filt(true, toMainWidget());
//         if (filt.exec())
//             Filter = filt.getSetting();
//     }
//     if (Filter)
//     {
//         disconnect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
//         FilterButton->setChecked(true);
//         connect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
//     }
// }
//
// void toBrowseTemplate::clearFilter(void)
// {
//     delete Filter;
//     Filter = new toBrowserFilter;
//     disconnect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
//     FilterButton->setChecked(false);
//     connect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
// }
//
// void toBrowseTemplate::removeItem(toTreeWidgetItem *item)
// {
//     for (std::list<toTemplateItem *>::iterator i = Parents.begin();i != Parents.end();i++)
//         if ((*i) == item)
//         {
//             Parents.erase(i);
//             break;
//         }
// }
//
// class toTemplateTableItem : public toTemplateItem
// {
//     toConnection &Connection;
// public:
//     toTemplateTableItem(toConnection &conn, toTemplateItem *parent,
//                         const QString &name)
//             : toTemplateItem(parent, name), Connection(conn) {}
//     virtual QWidget *selectedWidget(QWidget *par)
//     {
//         QString ptyp = parent()->parent()->text(0);
//         QString object = parent()->text(0);
//         QString typ = text(0);
//         QString schema = parent()->parent()->parent()->text(0);
//         if (ptyp == "Synonyms")
//         {
//             int pos = object.indexOf(QString::fromLatin1("."));
//             if (pos >= 0)
//             {
//                 schema = object.mid(0, pos);
//                 object = object.mid(pos + 1);
//             }
//             else
//             {
//                 schema = QString::fromLatin1("PUBLIC");
//             }
//         }
//
//         if (schema == qApp->translate("toBrowser", "No schemas"))
//             schema = Connection.database();
//
//         toResult *res;
//
//         toToolWidget *tool = new toToolWidget(BrowserTool,
//                                               "",
//                                               par,
//                                               Connection);
//         if (typ == qApp->translate("toBrowser", "Data"))
//         {
//             toResultData *cnt = new toResultData(tool);
//             cnt->changeParams(schema, object);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Indexes"))
//         {
//             toResultTableView *tv = new toResultTableView(true, false, tool);
//             tv->setReadAll(true);
//             tv->setSQL(SQLTableIndex);
//             res = tv;
//         }
//         else if (typ == qApp->translate("toBrowser", "Extents"))
//         {
//             new toResultExtent(tool);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Constraints"))
//         {
//             toResultTableView *tv = new toResultTableView(true, false, tool);
//             tv->setSQL(SQLTableConstraint);
//             res = tv;
//         }
//         else if (typ == qApp->translate("toBrowser", "Triggers"))
//         {
//             toResultTableView *tv = new toResultTableView(true, false, tool);
//             tv->setReadAll(true);
//             tv->setSQL(SQLTableTrigger);
//             res = tv;
//         }
//         else if (typ == qApp->translate("toBrowser", "SQL"))
//         {
//             toResultField *sql = new toResultField(tool);
//             sql->setSQL(SQLViewSQL);
//             sql->changeParams(schema, object);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Script"))
//         {
//             toResultExtract *ext = new toResultExtract(true, tool);
//             ext->changeParams(schema, object);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Information"))
//         {
//             toResultItem *inf = new toResultItem(2, true, tool);
//             if (ptyp == qApp->translate("toBrowser", "Tables"))
//             {
//                 inf->setSQL(SQLTableInfo);
//             }
//             else if (ptyp == qApp->translate("toBrowser", "Triggers"))
//             {
//                 inf->setSQL(SQLTriggerInfo);
//             }
//             else if (ptyp == qApp->translate("toBrowser", "Indexes"))
//             {
//                 inf->setSQL(SQLIndexInfo);
//             }
//             inf->changeParams(schema, object);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Columns"))
//         {
//             res = new toResultTableView(true, false, tool);
//             res->setSQL(SQLTriggerCols);
//         }
//         else if (typ == qApp->translate("toBrowser", "References"))
//         {
//             res = new toResultTableView(true, false, tool);
//             res->setSQL(SQLTableReferences);
//         }
//         else if (typ == qApp->translate("toBrowser", "Grants"))
//         {
//             res = new toResultTableView(true, false, tool);
//             res->setSQL(SQLAnyGrants);
//         }
//         else if (typ == qApp->translate("toBrowser", "Dependencies"))
//         {
//             res = new toResultDepend(tool);
//         }
//         else
//         {
//             delete tool;
//             return NULL;
//         }
//         res->changeParams(schema, object);
//         return tool;
//     }
// };
//
// class toTemplateSchemaItem : public toTemplateItem
// {
//     toConnection &Connection;
// public:
//     toTemplateSchemaItem(toConnection &conn, toTemplateItem *parent,
//                          const QString &name)
//             : toTemplateItem(parent, name), Connection(conn)
//     {
//         QString typ = parent->text(0);
//         if (typ == qApp->translate("toBrowser", "Tables"))
//         {
//             QPixmap image(const_cast<const char**>(table_xpm));
//             setPixmap(0, image);
//             new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Indexes"));
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Constraints"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "References"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Triggers"));
//             }
//             new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Data"));
//             new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Extents"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
//             }
//         }
//         else if (typ == qApp->translate("toBrowser", "Views"))
//         {
//             QPixmap image(const_cast<const char**>(view_xpm));
//             setPixmap(0, image);
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "SQL"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Data"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Dependencies"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
//             }
//         }
//         else if (typ == qApp->translate("toBrowser", "Sequences"))
//         {
//             QPixmap image(const_cast<const char**>(sequence_xpm));
//             setPixmap(0, image);
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
//             }
//         }
//         else if (typ == qApp->translate("toBrowser", "Code"))
//         {
//             QPixmap image(const_cast<const char**>(function_xpm));
//             setPixmap(0, image);
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Dependencies"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
//             }
//         }
//         else if (typ == qApp->translate("toBrowser", "Triggers"))
//         {
//             QPixmap image(const_cast<const char**>(function_xpm));
//             setPixmap(0, image);
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Columns"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Dependencies"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
//             }
//         }
//         else if (typ == qApp->translate("toBrowser", "Indexes"))
//         {
//             QPixmap image(const_cast<const char**>(index_xpm));
//             setPixmap(0, image);
//             if (providerIs("Oracle")(conn) || conn.providerIs("SapDB"))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
//             }
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Extents"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
//             }
//         }
//         else if (typ == qApp->translate("toBrowser", "Synonyms"))
//         {
//             QPixmap image(const_cast<const char**>(synonym_xpm));
//             setPixmap(0, image);
//             if (providerIs("Oracle")(conn))
//             {
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
//                 new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
//             }
//         }
//     }
//
//     virtual QString allText(int col) const
//     {
//         QString txt = parent()->parent()->text(0);
//         txt += QString::fromLatin1(".");
//         txt += text(col);
//         return txt;
//     }
//
//     virtual QWidget *selectedWidget(QWidget *par)
//     {
//         QString object = text(0);
//         QString typ = parent()->text(0);
//         QString schema = parent()->parent()->text(0);
//         if (schema == qApp->translate("toBrowser", "No schemas"))
//             schema = Connection.database();
//
//         toToolWidget *tool = new toToolWidget(BrowserTool,
//                                               "",
//                                               par,
//                                               Connection);
//         if (typ == qApp->translate("toBrowser", "Code") || typ == qApp->translate("toBrowser", "Triggers"))
//         {
//             toResultField *fld = new toResultField(tool);
//             if (typ == qApp->translate("toBrowser", "Code"))
//                 fld->setSQL(SQLSQLTemplate);
//             else
//                 fld->setSQL(SQLTriggerBody);
//             fld->changeParams(schema, object);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Tables") || typ == qApp->translate("toBrowser", "Views"))
//         {
//             toResultCols *cols = new toResultCols(tool);
//             cols->changeParams(schema, object);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Indexes"))
//         {
//             toResultTableView *tv = new toResultTableView(true, false, tool);
//             tv->setSQL(SQLIndexCols);
//             tv->changeParams(schema, object);
//             return tool;
//         }
//         else if (typ == qApp->translate("toBrowser", "Synonyms") || typ == qApp->translate("toBrowser", "Sequences"))
//         {
//             toResultItem *resultItem = new toResultItem(2, true, tool);
//             if (typ == qApp->translate("toBrowser", "Synonyms"))
//             {
//                 resultItem->setSQL(SQLSynonymInfo);
//                 int pos = object.indexOf(QString::fromLatin1("."));
//                 if (pos >= 0)
//                 {
//                     schema = object.mid(0, pos);
//                     object = object.mid(pos + 1);
//                 }
//                 else
//                 {
//                     schema = QString::fromLatin1("PUBLIC");
//                 }
//             }
//             else
//                 resultItem->setSQL(SQLSequenceInfo);
//             resultItem->changeParams(schema, object);
//             return tool;
//         }
//         else
//         {
//             delete tool;
//             return NULL;
//         }
//     }
// };

// class toTemplateSchemaList : public toTemplateSQL
// {
// public:
//     toTemplateSchemaList(toConnection &conn, toTemplateItem *parent,
//                          const QString &name, const QString &sql)
//             : toTemplateSQL(conn, parent, name, sql) { }
//     virtual toTemplateItem *createChild(const QString &name)
//     {
//         try
//         {
//             toBrowserFilter *filter = BrowseTemplate.filter();
//             toTemplateItem *item = new toTemplateSchemaItem(connection(), this, name);
//             if (filter && !filter->check(item))
//             {
//                 delete item;
//                 return NULL;
//             }
//             return item;
//         }
//         catch (...)
//         {
//             return NULL;
//         }
//     }
//     virtual toQList parameters(void)
//     {
//         toQList ret;
//         ret.insert(ret.end(), parent()->text(0));
//         toBrowserFilter *filter = BrowseTemplate.filter();
//         if (filter)
//             ret.insert(ret.end(), filter->wildCard());
//         else
//             ret.insert(ret.end(), toQValue(QString::fromLatin1("%")));
//         return ret;
//     }
// };
//
// class toTemplateDBItem : public toTemplateSQL
// {
// public:
//     toTemplateDBItem(toConnection &conn, toTemplateItem *parent,
//                      const QString &name)
//             : toTemplateSQL(conn, parent, name, toSQL::string(toSQL::TOSQL_USERLIST, conn)) {}
//     virtual ~toTemplateDBItem()
//     {
//         toBrowseTemplate *prov = dynamic_cast<toBrowseTemplate *>(&provider());
//         if (prov)
//             prov->removeItem(this);
//     }
//     virtual toTemplateItem *createChild(const QString &name)
//     {
//         try
//         {
//             toTemplateItem *item = new toTemplateItem(this, name);
//             QPixmap image(const_cast<const char**>(schema_xpm));
//             item->setPixmap(0, image);
//             QPixmap table(const_cast<const char**>(table_xpm));
//             QPixmap view(const_cast<const char**>(view_xpm));
//             QPixmap sequence(const_cast<const char**>(sequence_xpm));
//             QPixmap function(const_cast<const char**>(function_xpm));
//             QPixmap index(const_cast<const char**>(index_xpm));
//             QPixmap synonym(const_cast<const char**>(synonym_xpm));
//
//             toBrowserFilter *filter = BrowseTemplate.filter();
//             if (filter && filter->onlyOwnSchema() &&
//                     name.toUpper() != connection().user().toUpper())
//             {
//                 delete item;
//                 return NULL;
//             }
//
//             (new toTemplateSchemaList(connection(),
//                                       item,
//                                       qApp->translate("toBrowser", "Tables"),
//                                       toSQL::string(SQLListTables, connection())))->setPixmap(0, table);
//
//             if (providerIs("Oracle")(connection()) ||
//                     toIsSapDB(connection()) ||
//                     connection().providerIs("QPSQL"))
//             {
//                 (new toTemplateSchemaList(connection(),
//                                           item,
//                                           qApp->translate("toBrowser", "Views"),
//                                           toSQL::string(SQLListView, connection())))->setPixmap(0, view);
//                 (new toTemplateSchemaList(connection(),
//                                           item,
//                                           qApp->translate("toBrowser", "Indexes"),
//                                           toSQL::string(SQLListIndex, connection())))->setPixmap(0, index);
//             }
//
//             if (providerIs("Oracle")(connection()) || connection().providerIs("QPSQL"))
//             {
//                 (new toTemplateSchemaList(connection(),
//                                           item,
//                                           qApp->translate("toBrowser", "Sequences"),
//                                           toSQL::string(SQLListSequence, connection())))->setPixmap(0, sequence);
//                 (new toTemplateSchemaList(connection(),
//                                           item,
//                                           qApp->translate("toBrowser", "Code"),
//                                           toSQL::string(SQLListSQLShort, connection())))->setPixmap(0, function);
//                 (new toTemplateSchemaList(connection(),
//                                           item,
//                                           qApp->translate("toBrowser", "Triggers"),
//                                           toSQL::string(SQLListTrigger, connection())))->setPixmap(0, function);
//             }
//
//             if (providerIs("Oracle")(connection()))
//             {
//                 (new toTemplateSchemaList(connection(),
//                                           item,
//                                           qApp->translate("toBrowser", "Synonyms"),
//                                           toSQL::string(SQLListSynonym, connection())))->setPixmap(0, synonym);
//             }
//             return item;
//         }
//         catch (...)
//         {
//             return NULL;
//         }
//     }
// };
//
// class toBrowseTemplateItem : public toTemplateItem
// {
// public:
//     toBrowseTemplateItem(toTemplateProvider &prov, toTreeWidget *parent, const QString &name)
//             : toTemplateItem(prov, parent, name) { }
//     virtual ~toBrowseTemplateItem()
//     {
//         dynamic_cast<toBrowseTemplate &>(provider()).removeItem(this);
//     }
// };
//
// void toBrowseTemplate::insertItems(toTreeWidget *parent, QToolBar *toolbar)
// {
//     if (!Registered)
//     {
//         connect(toMainWidget(),
//                 SIGNAL(addedConnection(const QString &)),
//                 this,
//                 SLOT(addDatabase(const QString &)));
//         connect(toMainWidget(),
//                 SIGNAL(removedConnection(const QString &)),
//                 this,
//                 SLOT(removeDatabase(const QString &)));
//     }
//
//     toTemplateItem *dbitem = new toBrowseTemplateItem(
//         *this,
//         parent,
//         qApp->translate("toBrowser", "DB Browser"));
//
//     std::list<QString> conn = toMainWidget()->connections();
//     for (std::list<QString>::iterator i = conn.begin();i != conn.end();i++)
//     {
//         toConnection &conn = toMainWidget()->connection(*i);
//         new toTemplateDBItem(conn, dbitem, *i);
//     }
//     Parents.insert(Parents.end(), dbitem);
//
//     FilterButton = new QToolButton(toolbar);
//     FilterButton->setCheckable(true);
//     FilterButton->setIcon(QIcon(QPixmap(const_cast<const char**>(filter_xpm))));
//     FilterButton->setToolTip(tr("Define the object filter for database browser"));
//     toolbar->addWidget(FilterButton);
//     connect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
//
//     toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(nofilter_xpm))),
//                        qApp->translate(
//                            "toBrowser",
//                            "Remove any object filter for database browser"),
//                        this,
//                        SLOT(clearFilter(void)));
// }
//
// void toBrowseTemplate::addDatabase(const QString &name)
// {
//     try
//     {
//         for (std::list<toTemplateItem *>::iterator i = Parents.begin();i != Parents.end();i++)
//             new toTemplateDBItem(toMainWidget()->connection(name), *i, name);
//     }
//     TOCATCH
// }
//
// void toBrowseTemplate::importData(std::map<QString, QString> &data, const QString &prefix)
// {
//     if (data.find(prefix + ":Filter:Type") != data.end())
//     {
//         Filter = new toBrowserFilter;
//         Filter->importData(data, prefix + ":Filter");
//     }
// }
//
// void toBrowseTemplate::exportData(std::map<QString, QString> &data, const QString &prefix)
// {
//     if (Filter)
//         Filter->exportData(data, prefix + ":Filter");
// }

//void toBrowser::enableConstraints(void)
//{
//    if (m_mainTab->currentWidget() == tableSplitter)
//        tableBrowserWidget->enableConstraints(true);
//}
//
//void toBrowser::disableConstraints(void)
//{
//    if (m_mainTab->currentWidget() == tableSplitter)
//        tableBrowserWidget->enableConstraints(false);
//}
