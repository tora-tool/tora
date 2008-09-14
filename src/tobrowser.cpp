
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

#include "utils.h"

#include "tobrowser.h"
#include "tobrowsertable.h"
#include "tobrowserconstraint.h"
#include "ui_tobrowserfilterui.h"
#include "tobrowserindex.h"
#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "toextract.h"
#include "tohelp.h"
#include "tomain.h"
#include "toresultcols.h"
#include "toresultschema.h"
#include "toresultdata.h"
#include "toresultdepend.h"
#include "toresultextract.h"
#include "toresultfield.h"
#include "toresultitem.h"
#include "toresultstorage.h"
#include "toresultview.h"
#include "toresulttableview.h"
#include "tosql.h"
#include "totabwidget.h"
#include "totool.h"

#ifdef TOEXTENDED_MYSQL
#  include "tomysqluser.h"
#endif

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qinputdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <QList>
#include <QMdiArea>
#include <QMdiSubWindow>

#include <QPixmap>
#include <QString>
#include <QVBoxLayout>
#include <QButtonGroup>

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
//#ifndef TO_NO_ORACLE
#include "icons/offline.xpm"
#include "icons/online.xpm"
//#endif
#ifdef TOEXTENDED_MYSQL
#include "icons/new.xpm"
#endif


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

QWidget *toBrowserTool::toolWindow(QWidget *parent, toConnection &connection)
{
    return new toBrowser(parent, connection);
}

bool toBrowserTool::canHandle(toConnection &conn)
{
    return toIsOracle(conn) || toIsMySQL(conn) || toIsPostgreSQL(conn) || toIsSapDB(conn);
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

void toBrowserTool::addTable(void)
{
    try
    {
        toConnection &conn = toMainWidget()->currentConnection();
        toBrowserTable::newTable(conn,
                                 toIsMySQL(conn) ? conn.database() : conn.user(),
                                 toMainWidget());
    }
    TOCATCH
}

void toBrowserTool::addIndex(void)
{
    try
    {
        toConnection &conn = toMainWidget()->currentConnection();
        toBrowserIndex::modifyIndex(conn,
                                    toIsMySQL(conn) ? conn.database() : conn.user(),
                                    QString::null,
                                    toMainWidget());
    }
    TOCATCH
}

void toBrowserTool::addConstraint(void)
{
    try
    {
        toConnection &conn = toMainWidget()->currentConnection();
        toBrowserConstraint::modifyConstraint(conn,
                                              toIsMySQL(conn) ? conn.database() : conn.user(),
                                              QString::null,
                                              toMainWidget());
    }
    TOCATCH
}

static toBrowserTool BrowserTool;

static toSQL SQLListTablespaces("toBrowser:ListTablespaces",
                                "SELECT Tablespace_Name FROM sys.DBA_TABLESPACES\n"
                                " ORDER BY Tablespace_Name",
                                "List the available tablespaces in a database.");

class toBrowserFilter : public toViewFilter
{
    int                Type;
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
    toBrowserFilter(int type,
                    bool cas,
                    bool invert,
                    const QString &str,
                    int tablespace,
                    const std::list<QString> &tablespaces,
                    bool onlyOwnSchema = false)
            : Type(type),
            IgnoreCase(cas),
            Invert(invert),
            Text(cas ? str.toUpper() : str),
            TablespaceType(tablespace),
            Tablespaces(tablespaces),
            OnlyOwnSchema(onlyOwnSchema),
            Empty(false)
    {

        if (!str.isEmpty())
        {
            Match.setPattern(str);
            Match.setCaseSensitivity(cas ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }

        storeFilterSettings();
    }

    toBrowserFilter(bool empty = true)
            : Type(0),
            IgnoreCase(true),
            Invert(false),
            TablespaceType(0)
    {

        Empty = empty;
        if (!empty)
            readFilterSettings();
        else
            toConfigurationSingle::Instance().setFilterType(0);  // No filter type
    }

    virtual bool isEmpty(void)
    {
        return Empty;
    }

    virtual void storeFilterSettings(void)
    {
        toConfigurationSingle::Instance().setFilterIgnoreCase(IgnoreCase);
        toConfigurationSingle::Instance().setFilterInvert(Invert);
        toConfigurationSingle::Instance().setFilterType(Type);
        toConfigurationSingle::Instance().setFilterTablespaceType(TablespaceType);
        toConfigurationSingle::Instance().setFilterText(Text);
        toConfigurationSingle::Instance().saveConfig();
    }

    virtual void readFilterSettings(void)
    {
        QString t;
        Text = toConfigurationSingle::Instance().filterText();
        IgnoreCase = toConfigurationSingle::Instance().filterIgnoreCase();
        Invert = toConfigurationSingle::Instance().filterInvert();
        OnlyOwnSchema = false;
        Type = toConfigurationSingle::Instance().filterType();
        TablespaceType = toConfigurationSingle::Instance().filterTablespaceType();
    }

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
        for (std::list<QString>::iterator i = Tablespaces.begin();i != Tablespaces.end();i++, id++)
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

    bool onlyOwnSchema(void)
    {
        return OnlyOwnSchema;
    }

    virtual QString wildCard(void)
    {
        switch (Type)
        {
        default:
            return QString::fromLatin1("%");
        case 1:
            return Text.toUpper() + QString::fromLatin1("%");
        case 2:
            return QString::fromLatin1("%") + Text.toUpper();
        case 3:
            return QString::fromLatin1("%") + Text.toUpper() + QString::fromLatin1("%");
        }
    }

    virtual void startingQuery()
    {
        RemoveDuplicates.clear();
    }

    virtual bool check(const toTreeWidgetItem *item)
    {
        return check(item->text(0),
                     item->text(1),
                     item->text(2));
    }

    virtual bool check(const toResultModel *model, const int row)
    {
        return check(model->data(row, 1).toString(),
                     model->data(row, 2).toString(),
                     model->data(row, 3).toString());
    }

    virtual bool check(QString one, QString two, QString three)
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
            default:
                break;
            case 1:
            {
                bool ok = false;
                for (std::list<QString>::iterator i = Tablespaces.begin();i != Tablespaces.end();i++)
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
                for (std::list<QString>::iterator i = Tablespaces.begin();i != Tablespaces.end();i++)
                    if (*i == tablespace)
                        return false;
                break;
            }
        }
        switch (Type)
        {
        default:
            return true;
        case 1:
            if (IgnoreCase)
            {
                if (str.toUpper().startsWith(Text))
                    return !Invert;
            }
            else if (str.startsWith(Text))
                return !Invert;
            break;
        case 2:
            if (IgnoreCase)
            {
                if (str.right(Text.length()).toUpper() == Text)
                    return !Invert;
            }
            else if (str.right(Text.length()) == Text)
                return !Invert;
            break;
        case 3:
            if (str.contains(Text, IgnoreCase ? Qt::CaseSensitive : Qt::CaseInsensitive))
                return !Invert;
            break;
        case 4:
        {
            QStringList lst = Text.split(QRegExp(QString("\\s*,\\s*")));
            for (int i = 0;i < lst.count();i++)
                if (IgnoreCase)
                {
                    if (str.toUpper() == lst[i])
                        return !Invert;
                }
                else if (str == lst[i])
                    return !Invert;
        }
        break;
        case 5:
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

class toBrowserFilterSetup : public QDialog, public Ui::toBrowserFilterUI
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
        ButtonsGroup->addButton(None, 0);
        ButtonsGroup->addButton(StartWith, 1);
        ButtonsGroup->addButton(EndWith, 2);
        ButtonsGroup->addButton(Contains, 3);
        ButtonsGroup->addButton(CommaSeparate, 4);
        ButtonsGroup->addButton(RegExp, 5);

        TypeGroup = new QButtonGroup(TablespaceType);
        TypeGroup->addButton(IncludeAll, 0);
        TypeGroup->addButton(Include, 1);
        TypeGroup->addButton(Exclude, 2);

        if (!temp)
        {
            OnlyOwnSchema->hide();
            Tablespaces->setNumberColumn(false);
            Tablespaces->setReadableColumns(true);
            try
            {
                toConnection &conn = toCurrentConnection(this);
                toQuery query(conn, toSQL::string(SQLListTablespaces, conn));
                Tablespaces->query(SQLListTablespaces);
            }
            catch (...) {}
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

        return new toBrowserFilter(ButtonsGroup->id(ButtonsGroup->checkedButton()),
                                   IgnoreCase->isChecked(),
                                   Invert->isChecked(),
                                   String->text(),
                                   TypeGroup->id(TypeGroup->checkedButton()),
                                   tablespaces,
                                   OnlyOwnSchema->isChecked());
    }
};

toBrowseButton::toBrowseButton(const QIcon &iconSet,
                               const QString &textLabel,
                               const QString & grouptext,
                               QObject * receiver,
                               const char * slot,
                               QToolBar * parent,
                               const char * name)
        : QToolButton(parent)
{

    setIcon(iconSet);
    setText(textLabel);
    connect(this, SIGNAL(triggered(QAction *)), receiver, slot);
    setObjectName(name);
    setToolTip(grouptext);

    try
    {
        connect(toCurrentTool(this), SIGNAL(connectionChange()), this, SLOT(connectionChanged()));
    }
    TOCATCH
    connectionChanged();
}

void toBrowseButton::connectionChanged()
{
    try
    {
        setEnabled(toExtract::canHandle(toCurrentConnection(this)));
    }
    TOCATCH
}

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

#define TAB_ACCESS  "Access"
#define TAB_ACCESS_CONTENT "AccessContent"
#define TAB_ACCESS_USER  "AccessUser"
#define TAB_ACCESS_OBJECTS "AccessObjects"

static toSQL SQLListTablesMysql("toBrowser:ListTables",
                                "SHOW TABLES FROM `:f1<noquote>`",
                                "List the available tables in a schema.",
                                "3.0",
                                "MySQL");
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
                                "7.1",
                                "PostgreSQL");
static toSQL SQLListTablesSapDB("toBrowser:ListTables",
                                "SELECT tablename \"Table Name\"\n"
                                " FROM tables \n"
                                " WHERE tabletype = 'TABLE' and owner = upper(:f1<char[101]>) \n"
                                " ORDER by tablename",
                                "",
                                "",
                                "SapDB");

static toSQL SQLTableIndex("toBrowser:TableIndex",
                           "SELECT IND.index_name AS \"Index Name\",\n"
                           "       ind.column_name AS \"Column Name\",\n"
                           "       al.uniqueness AS \"Unique\",\n"
                           "       AL.index_type AS \"Type\",\n"
                           "       EX.column_expression AS \"Column Expression\"\n"
                           "  FROM SYS.ALL_IND_COLUMNS IND,\n"
                           "       SYS.ALL_IND_EXPRESSIONS EX,\n"
                           "       sys.All_Indexes AL\n"
                           " WHERE IND.INDEX_OWNER = :own<char[101]>\n"
                           "   AND IND.TABLE_NAME = :nam<char[101]>\n"
                           "   AND EX.index_owner ( + ) = IND.index_owner\n"
                           "   AND EX.index_name ( + ) = IND.index_name\n"
                           "   AND IND.index_name = AL.index_name ( + )\n"
                           "   AND IND.index_owner = AL.owner ( + )",
                           "List the indexes on a table",
                           "");
static toSQL SQLTableIndexSapDB("toBrowser:TableIndex",
                                "SELECT owner,\n"
                                "       indexname \"Index_Name\",\n"
                                "       'NORMAL',\n"
                                "       type\n"
                                " FROM indexes \n"
                                " WHERE owner = :f1<char[101]> and tablename = :f2<char[101]> \n"
                                " ORDER by indexname",
                                "",
                                "",
                                "SapDB");

static toSQL SQLTableIndexPG("toBrowser:TableIndex",
                             "SELECT u.usename AS \"Owner\",\n"
                             "       c2.relname AS \"Index Name\",\n"
                             "       pg_get_indexdef(i.indexrelid) as \"Definition\"\n"
                             "  FROM pg_class c,\n"
                             "       pg_class c2,\n"
                             "       pg_index i,\n"
                             "       pg_user u,\n"
                             "       pg_namespace n\n"
                             " WHERE c.relowner = u.usesysid\n"
                             "   AND n.nspname = :f1\n"
                             "   AND c.relname = :f2\n"
                             "   AND c.relowner = u.usesysid\n"
                             "   AND n.OID = c.relnamespace\n"
                             "   AND c.OID = i.indrelid\n"
                             "   AND i.indexrelid = c2.OID",
                             "",
                             "",
                             "PostgreSQL");

static toSQL SQLTableIndexMySQL("toBrowser:TableIndex",
                                "SHOW INDEX FROM `:f1<noquote>`.`:tab<noquote>`",
                                "",
                                "",
                                "MySQL");

static toSQL SQLTableConstraint(
    "toBrowser:TableConstraint",
    "SELECT sub.constraint_name AS \"Constraint Name\",\n"
    "       srch.search_condition AS \"Search Condition\",\n"
    "       sub.condition AS \"Condition\",\n"
    "       sub.status AS \"Status\",\n"
    "       sub.constraint_type AS \"Type\",\n"
    "       sub.delete_rule AS \"Delete Rule\",\n"
    "       sub.GENERATED AS \"Generated\"\n"
    "  FROM ( SELECT c.Constraint_Name,\n"
    "                DECODE ( constraint_type,\n"
    "                         'R',\n"
    "                         'foreign key (' || a.column_name || ') references ' "
    "|| a2.owner || '.' || a2.table_name || '(' || a2.column_name || ')',\n"
    "                         'P',\n"
    "                         'primary key (' || a.column_name || ')',\n"
    "                         'U',\n"
    "                         'unique (' || a.column_name || ')',\n"
    "                         NULL ) AS condition,\n"
    "                c.Status,\n"
    "                c.Constraint_Type,\n"
    "                c.Delete_Rule,\n"
    "                c.GENERATED\n"
    "           FROM sys.All_Constraints c,\n"
    "                sys.all_cons_columns a,\n"
    "                sys.all_cons_columns a2\n"
    "          WHERE c.constraint_name = a.constraint_name\n"
    "            AND c.Owner = :f1<char[101]>\n"
    "            AND c.Table_Name = :f2<char[101]>\n"
    "            AND c.r_constraint_name = a2.constraint_name ( + ) ) sub,\n"
    "       sys.all_constraints srch\n"
    " WHERE sub.constraint_name = srch.constraint_name",
    "List the constraints on a table",
    "");

static toSQL SQLTableConstraintPG(
    "toBrowser:TableConstraint",
    "SELECT conname as \"Constraint Name\",\n"
    "       pg_catalog.pg_get_constraintdef ( r.OID,\n"
    "                                         TRUE ) AS \"Description\"\n"
    "  FROM pg_catalog.pg_constraint r,\n"
    "       pg_catalog.pg_class c,\n"
    "       pg_catalog.pg_namespace n\n"
    " WHERE n.nspname = :f1\n"
    "   AND c.relname = :f2\n"
    "   AND c.relnamespace = n.OID\n"
    "   AND c.OID = r.conrelid\n"
    " ORDER BY 1",
    "",
    "",
    "PostgreSQL");

static toSQL SQLTableReferences(
    "toBrowser:TableReferences",
    "SELECT a.Owner AS \"Owner\",\n"
    "       a.Table_Name AS \"Object\",\n"
    "       a.Constraint_Name AS \"Constraint\",\n"
    "       'foreign key (' || c.column_name || ') references ' || r.owner || "
    "'.' || r.table_name || '(' || r.column_name || ')' AS \"Condition\",\n"
    "       a.Status AS \"Status\",\n"
    "       a.Delete_Rule AS \"Delete Rule\"\n"
    "  FROM sys.all_constraints a,\n"
    "       sys.all_cons_columns r,\n"
    "       sys.all_cons_columns c\n"
    " WHERE a.constraint_type = 'R'\n"
    "   AND a.r_constraint_name IN ( SELECT b.constraint_name\n"
    "                                  FROM sys.all_constraints b\n"
    "                                 WHERE b.OWNER = :owner<char[101]>\n"
    "                                   AND b.TABLE_NAME = :tab<char[101]> )\n"
    "   AND a.r_constraint_name = r.constraint_name\n"
    "   AND a.constraint_name = c.constraint_name\n"
    " UNION SELECT owner,\n"
    "       name,\n"
    "       NULL,\n"
    "       TYPE || ' ' || dependency_type,\n"
    "       'DEPENDENCY',\n"
    "       NULL\n"
    "  FROM sys.all_dependencies\n"
    " WHERE referenced_owner = :owner<char[101]>\n"
    "   AND referenced_name = :tab<char[101]>",
    "List the references on a table",
    "");

static toSQL SQLAnyGrantsSapDB("toBrowser:AnyGrants",
                               "SELECT privilege,grantee,grantor,is_grantable\n"
                               " FROM tableprivileges \n"
                               " WHERE owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>\n"
                               " ORDER by privilege,grantee ",
                               "Display the grants on an object",
                               "",
                               "SapDB");
static toSQL SQLAnyGrants("toBrowser:AnyGrants",
                          "SELECT Privilege,Grantee,Grantor,Grantable FROM SYS.ALL_TAB_PRIVS\n"
                          " WHERE Table_Schema = :f1<char[101]> AND Table_Name = :f2<char[101]>\n"
                          " ORDER BY Privilege,Grantee",
                          "");
static toSQL SQLTableTriggerSapDB("toBrowser:TableTrigger",
                                  "SELECT TriggerName,'UPDATE' \"Event\",''\"Column\",'ENABLED' \"Status\",''\"Description\"\n"
                                  " FROM triggers \n"
                                  " WHERE owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>\n"
                                  "  and update='YES'\n"
                                  "UNION\n"
                                  "SELECT TriggerName,'INSERT','','ENABLED',''\n"
                                  " FROM triggers \n"
                                  " WHERE owner = upper(:f1<char[101]>) and  tablename = :f2<char[101]>\n"
                                  "  and insert='YES'\n"
                                  "UNION\n"
                                  "SELECT TriggerName,'DELETE','','ENABLED',''\n"
                                  " FROM triggers \n"
                                  " WHERE owner = upper(:f1<char[101]>) and  tablename = :f2<char[101]>\n"
                                  "  and delete='YES'\n"
                                  " ORDER by 1 ",
                                  "Display the triggers operating on a table",
                                  "",
                                  "SapDB");
static toSQL SQLTableTrigger("toBrowser:TableTrigger",
                             "SELECT Trigger_Name,Triggering_Event,Column_Name,Status,Description \n"
                             "  FROM SYS.ALL_TRIGGERS\n"
                             " WHERE Table_Owner = :f1<char[101]> AND Table_Name = :f2<char[101]>",
                             "",
                             "0801");
static toSQL SQLTableTrigger8("toBrowser:TableTrigger",
                              "SELECT Trigger_Name,Triggering_Event,Status,Description \n"
                              "  FROM SYS.ALL_TRIGGERS\n"
                              " WHERE Table_Owner = :f1<char[101]> AND Table_Name = :f2<char[101]>");
static toSQL SQLTableTriggerPG("toBrowser:TableTrigger",
                               " SELECT t.tgname AS \"Trigger Name\",\n"
                               "        pg_catalog.pg_get_triggerdef ( t.OID ) AS \"Condition\"\n"
                               "   FROM pg_catalog.pg_trigger t,\n"
                               "        pg_class c,\n"
                               "        pg_tables tab\n"
                               "  WHERE lower ( tab.schemaname ) = lower ( :f1<char[101]> )\n"
                               "    AND c.relname = tab.tablename\n"
                               "    AND lower ( c.relname ) = lower ( :f2<char[101]> )\n"
                               "    AND c.OID = t.tgrelid\n"
                               "    AND ( NOT tgisconstraint OR NOT EXISTS ( SELECT 1\n"
                               "                                               FROM pg_catalog.pg_depend d\n"
                               "                                               JOIN pg_catalog.pg_constraint c\n"
                               "                                                 ON ( d.refclassid = c.tableoid AND d.refobjid = c.OID )\n"
                               "                                              WHERE d.classid = t.tableoid\n"
                               "                                                AND d.objid = t.OID\n"
                               "                                                AND d.deptype = 'i'\n"
                               "                                                AND c.contype = 'f' ) )\n"
                               "  ORDER BY 1\n",
                               "",
                               "",
                               "PostgreSQL");
static toSQL SQLTableInfoMysql("toBrowser:TableInformation",
                               "show table status from `:own<noquote>` like :tab",
                               "Display information about a table",
                               "3.0",
                               "MySQL");
static toSQL SQLTableInfo("toBrowser:TableInformation",
                          "SELECT *\n"
                          "  FROM SYS.ALL_TABLES\n"
                          " WHERE OWNER = :f1<char[101]> AND Table_Name = :f2<char[101]>",
                          "");
static toSQL SQLTableInfoPgSQL("toBrowser:TableInformation",
                               "SELECT c.*\n"
                               "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                               " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                               "   AND c.relkind = 'r'\n"
                               "   AND c.relname = :f2",
                               "",
                               "7.1",
                               "PostgreSQL");
static toSQL SQLTableInfoSapDB("toBrowser:TableInformation",
                               "SELECT TABLENAME,PRIVILEGES,CREATEDATE,CREATETIME,UPDSTATDATE,UPDSTATTIME,ALTERDATE,ALTERTIME,TABLEID \n"
                               " FROM tables \n"
                               " WHERE tabletype = 'TABLE' and owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>",
                               "",
                               "",
                               "SapDB");
static toSQL SQLTableStatistic("toBrowser:TableStatstics",
                               "SELECT description \"Description\", value(char_value,numeric_value) \"Value\" \n"
                               " FROM tablestatistics \n"
                               " WHERE owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>",
                               "Table Statistics",
                               "",
                               "SapDB");
static toSQL SQLTablePartition("toBrowser:TablePartitions",
                               "select    p.partition_name \"Partition\"\n"
                               " , p.composite \"Composite\"\n"
                               " , p.num_rows \"Partition rows\"\n"
                               " , p.high_value \"High value\"\n"
                               " , p.subpartition_count \"Subpartitions\"\n"
                               " , p.partition_position \"Position\"\n"
                               " , s.subpartition_name \"Subpartition\"\n"
                               " , s.num_rows \"Subpartition rows\"\n"
                               " , s.subpartition_position \"Subpartition position\"\n"
                               "  from all_tab_partitions p,\n"
                               "       all_tab_subpartitions s\n"
                               " where p.table_owner = s.table_owner(+)\n"
                               "   and p.table_name = s.table_name(+)\n"
                               "   and p.partition_name = s.partition_name(+)\n"
                               "   and p.table_owner like upper(:table_owner<char[101]>)\n"
                               "   and p.table_name like upper(:table_name<char[101]>)\n"
                               " order by p.partition_name\n"
                               " , s.subpartition_name\n",
                               "Table partitions",
                               "0801");

static toSQL SQLListViewPgSQL("toBrowser:ListView",
                              "SELECT c.relname as View_Name\n"
                              "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                              " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                              "   AND c.relkind = 'v'"
                              " ORDER BY View_Name",
                              "List the available views in a schema",
                              "7.1",
                              "PostgreSQL");
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
static toSQL SQLViewSQLPgSQL("toBrowser:ViewSQL",
                             "SELECT pg_get_viewdef(c.relname)\n"
                             "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                             " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "   AND c.relkind = 'v' AND c.relname = :f2",
                             "Display SQL of a specified view",
                             "7.1",
                             "PostgreSQL");
static toSQL SQLViewSQL("toBrowser:ViewSQL",
                        "SELECT Text SQL\n"
                        "  FROM SYS.ALL_Views\n"
                        " WHERE Owner = :f1<char[101]> AND View_Name = :f2<char[101]>",
                        "");
static toSQL SQLViewSQLSapDb("toBrowser:ViewSQL",
                             "SELECT definition \"SQL\"\n"
                             " FROM viewdefs \n"
                             " WHERE  viewname = :f2<char[101]> and owner = upper(:f1<char[101]>)\n",
                             "",
                             "",
                             "SapDB");

static toSQL SQLListIndexMySQL("toBrowser:ListIndex",
                               "SHOW INDEX FROM `:f1<database>`",
                               "List the available indexes in a schema",
                               "3.23",
                               "MySQL");
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
                               "7.1",
                               "PostgreSQL");
static toSQL SQLListIndexSapDb("toBrowser:ListIndex",
                               "SELECT IndexName \"Index Name\"\n"
                               " FROM indexes \n"
                               " WHERE  owner = upper(:f1<char[101]>)",
                               "",
                               "",
                               "SapDB");

static toSQL SQLIndexColsMySQL("toBrowser:IndexCols",
                               "SHOW INDEX FROM `:f1<noquote>`.`:f2<noquote>`",
                               "Display columns on which an index is built",
                               "3.23",
                               "MySQL");
static toSQL SQLIndexCols("toBrowser:IndexCols",
                          "SELECT a.Table_Name,a.Column_Name,a.Column_Length,a.Descend,b.Column_Expression \" \"\n"
                          "  FROM sys.All_Ind_Columns a,sys.All_Ind_Expressions b\n"
                          " WHERE a.Index_Owner = :f1<char[101]> AND a.Index_Name = :f2<char[101]>\n"
                          "   AND a.Index_Owner = b.Index_Owner(+) AND a.Index_Name = b.Index_Name(+)\n"
                          "   AND a.column_Position = b.Column_Position(+)\n"
                          " ORDER BY a.Column_Position",
                          "",
                          "0801");
static toSQL SQLIndexCols8("toBrowser:IndexCols",
                           "SELECT Table_Name,Column_Name,Column_Length,Descend\n"
                           "  FROM SYS.ALL_IND_COLUMNS\n"
                           " WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
                           " ORDER BY Column_Position",
                           "",
                           "0800");
static toSQL SQLIndexCols7("toBrowser:IndexCols",
                           "SELECT Table_Name,Column_Name,Column_Length\n"
                           "  FROM SYS.ALL_IND_COLUMNS\n"
                           " WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
                           " ORDER BY Column_Position",
                           "",
                           "0703");
static toSQL SQLIndexColsPgSQL("toBrowser:IndexCols",
                               "SELECT a.attname,\n"
                               "       format_type(a.atttypid, a.atttypmod) as FORMAT,\n"
                               "       a.attnotnull,\n"
                               "       a.atthasdef\n"
                               "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid,\n"
                               "       pg_attribute a\n"
                               " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                               "   AND a.attrelid = c.oid AND c.relname = :f2\n"
                               "   AND a.attnum > 0\n"
                               " ORDER BY a.attnum\n",
                               "",
                               "7.1",
                               "PostgreSQL");
static toSQL SQLIndexColsSapDb("toBrowser:IndexCols",
                               "SELECT tablename,columnname,len \"Length\",DataType,Sort \n"
                               " FROM indexcolumns \n"
                               " WHERE  owner = upper(:f1<char[101]>) and indexname = upper(:f2<char[101]>)\n"
                               " ORDER BY indexname,columnno",
                               "",
                               "",
                               "SapDB");

static toSQL SQLIndexInfoSapDb("toBrowser:IndexInformation",
                               "SELECT  INDEXNAME,TABLENAME, TYPE, CREATEDATE,CREATETIME,INDEX_USED, DISABLED \n"
                               " FROM indexes\n"
                               " WHERE  owner = upper(:f1<char[101]>) and indexname = :f2<char[101]>\n",
                               "Display information about an index",
                               "",
                               "SapDB");
static toSQL SQLIndexInfo("toBrowser:IndexInformation",
                          "SELECT * FROM SYS.ALL_INDEXES\n"
                          " WHERE Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>",
                          "");
static toSQL SQLIndexStatistic("toBrowser:IndexStatstics",
                               "SELECT description \"Description\", value(char_value,numeric_value) \"Value\" \n"
                               " FROM indexstatistics \n"
                               " WHERE owner = upper(:f1<char[101]>) and indexname = :f2<char[101]>",
                               "Index Statistics",
                               "",
                               "SapDB");

static toSQL SQLListSequencePgSQL("toBrowser:ListSequence",
                                  "SELECT c.relname AS \"Sequence Name\"\n"
                                  "  FROM pg_class c LEFT OUTER JOIN pg_namespace n ON c.relnamespace=n.oid\n"
                                  " WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                                  "   AND c.relkind = 'S'\n"
                                  " ORDER BY \"Sequence Name\"",
                                  "List the available sequences in a schema",
                                  "7.1",
                                  "PostgreSQL");
static toSQL SQLListSequence("toBrowser:ListSequence",
                             "SELECT Sequence_Name FROM SYS.ALL_SEQUENCES\n"
                             " WHERE SEQUENCE_OWNER = :f1<char[101]>\n"
                             "   AND UPPER(SEQUENCE_NAME) LIKE :f2<char[101]>\n"
                             " ORDER BY Sequence_Name",
                             "");
static toSQL SQLSequenceInfoPgSQL("toBrowser:SequenceInformation",
                                  "SELECT *, substr(:f1,1) as \"Owner\" FROM :f2<noquote>",
                                  "Display information about a sequence",
                                  "7.1",
                                  "PostgreSQL");
static toSQL SQLSequenceInfo("toBrowser:SequenceInformation",
                             "SELECT * FROM SYS.ALL_SEQUENCES\n"
                             " WHERE Sequence_Owner = :f1<char[101]>\n"
                             "   AND Sequence_Name = :f2<char[101]>",
                             "");

static toSQL SQLListSynonym("toBrowser:ListSynonym",
                            "SELECT DECODE(Owner,'PUBLIC','',Owner||'.')||Synonym_Name \"Synonym Name\"\n"
                            "  FROM Sys.All_Synonyms\n"
                            " WHERE Table_Owner = :f1<char[101]>\n"
                            "    OR Owner = :f1<char[101]>\n"
                            "   AND UPPER(Synonym_Name) LIKE :f2<char[101]>\n"
                            " ORDER BY Synonym_Name",
                            "List the available synonyms in a schema");
static toSQL SQLSynonymInfo("toBrowser:SynonymInformation",
                            "SELECT * FROM Sys.All_Synonyms a\n"
                            " WHERE Owner = :f1<char[101]>\n"
                            "   AND Synonym_Name = :f2<char[101]>",
                            "Display information about a synonym");

static toSQL SQLListSQLPgSQL("toBrowser:ListCode",
                             "SELECT p.proname AS Object_Name,\n"
                             "  CASE WHEN p.prorettype = 0 THEN 'PROCEDURE'\n"
                             "       ELSE 'FUNCTION'\n"
                             "   END AS Object_Type\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_namespace n ON p.pronamespace=n.oid\n"
                             "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "ORDER BY Object_Name",
                             "List the available Code objects in a schema",
                             "7.1",
                             "PostgreSQL");
static toSQL SQLListSQL("toBrowser:ListCode",
                        "SELECT Object_Name,Object_Type,Status Type FROM SYS.ALL_OBJECTS\n"
                        " WHERE OWNER = :f1<char[101]>\n"
                        "   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
                        "                       'PROCEDURE','TYPE')\n"
                        "   AND UPPER(OBJECT_NAME) LIKE :f2<char[101]>\n"
                        " ORDER BY Object_Name",
                        "");
static toSQL SQLListSQLShortPgSQL("toBrowser:ListCodeShort",
                                  "SELECT p.proname AS Object_Name\n"
                                  "FROM pg_proc p LEFT OUTER JOIN pg_namespace n ON p.pronamespace=n.oid\n"
                                  "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                                  "ORDER BY Object_Name",
                                  "List the available Code objects in a schema, one column version",
                                  "7.1",
                                  "PostgreSQL");
static toSQL SQLListSQLShort("toBrowser:ListCodeShort",
                             "SELECT Object_Name Type FROM SYS.ALL_OBJECTS\n"
                             " WHERE OWNER = :f1<char[101]>\n"
                             "   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
                             "                       'PROCEDURE','TYPE')\n"
                             "   AND UPPER(OBJECT_NAME) LIKE :f2<char[101]>\n"
                             " ORDER BY Object_Name",
                             "");


static toSQL SQLSQLTemplate("toBrowser:CodeTemplate",
                            "SELECT Text FROM SYS.ALL_SOURCE\n"
                            " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
                            "   AND Type IN ('PACKAGE','PROCEDURE','FUNCTION','PACKAGE','TYPE')",
                            "Declaration of object displayed in template window");
// PostgreSQL does not distinguish between Head and Body for Stored SQL
// package code will be returnd for both Head and Body
static toSQL SQLSQLHeadPgSQL("toBrowser:CodeHead",
                             "SELECT p.prosrc\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_namespace n ON p.pronamespace=n.oid\n"
                             "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "  AND p.proname = :f2\n",
                             "Declaration of object",
                             "7.1",
                             "PostgreSQL");
static toSQL SQLSQLHead("toBrowser:CodeHead",
                        "SELECT Text FROM SYS.ALL_SOURCE\n"
                        " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
                        "   AND Type IN ('PACKAGE','TYPE')",
                        "");

static toSQL SQLSQLBodyPgSQL("toBrowser:CodeBody",
                             "SELECT p.prosrc\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_namespace n ON p.pronamespace=n.oid\n"
                             "WHERE (n.nspname = :f1 OR n.oid IS NULL)\n"
                             "  AND p.proname = :f2\n",
                             "Implementation of object",
                             "7.1",
                             "PostgreSQL");
static toSQL SQLSQLBody("toBrowser:CodeBody",
                        "SELECT Text FROM SYS.ALL_SOURCE\n"
                        " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
                        "   AND Type IN ('PROCEDURE','FUNCTION','PACKAGE BODY','TYPE BODY')",
                        "");


static toSQL SQLListTrigger("toBrowser:ListTrigger",
                            "SELECT Trigger_Name FROM SYS.ALL_TRIGGERS\n"
                            " WHERE OWNER = :f1<char[101]>\n"
                            "   AND UPPER(TRIGGER_NAME) LIKE :f2<char[101]>\n"
                            " ORDER BY Trigger_Name",
                            "List the available triggers in a schema");
static toSQL SQLTriggerInfo("toBrowser:TriggerInfo",
                            "SELECT Owner,Trigger_Name,\n"
                            "       Trigger_Type,Triggering_Event,\n"
                            "       Table_Owner,Base_Object_Type,Table_Name,Column_Name,\n"
                            "       Referencing_Names,When_Clause,Status,\n"
                            "       Description,Action_Type\n"
                            "  FROM SYS.ALL_TRIGGERS\n"
                            "WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                            "Display information about a trigger",
                            "0801");
static toSQL SQLTriggerInfo8("toBrowser:TriggerInfo",
                             "SELECT Owner,Trigger_Name,\n"
                             "       Trigger_Type,Triggering_Event,\n"
                             "       Table_Owner,Table_Name,\n"
                             "       Referencing_Names,When_Clause,Status,\n"
                             "       Description\n"
                             "  FROM SYS.ALL_TRIGGERS\n"
                             "WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                             "",
                             "0800");
static toSQL SQLTriggerBody("toBrowser:TriggerBody",
                            "SELECT Trigger_Body FROM SYS.ALL_TRIGGERS\n"
                            " WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                            "Implementation of trigger");
static toSQL SQLTriggerCols("toBrowser:TriggerCols",
                            "SELECT Column_Name,Column_List \"In Update\",Column_Usage Usage\n"
                            "  FROM SYS.ALL_TRIGGER_COLS\n"
                            " WHERE Trigger_Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
                            "Columns used by trigger");
#if DBLINK
static toSQL SQLListDBLink("toBrowser:ListDBLink",
                           "SELECT Db_Link, Owner FROM SYS.ALL_DB_LINKS\n"
                           " WHERE (Owner = :f1<char[101]> or Owner='PUBLIC') and\n"
                           " UPPER(DB_Link) like :f2<char[101]>",
                           "List database links");
static toSQL SQLListDBLinkDBA("toBrowser:ListDBLinkDBA",
                              "SELECT Owner, Db_Link, Username, Host, Created\n"
                              " FROM SYS.DBA_DB_LINK\n",
                              "List database links as DBA");
static toSQL SQLDBLinkInfo("toBrowser:DBLinkInformation",
                           "SELECT * FROM Sys.all_db_links a\n"
                           " WHERE Owner = :f1<char[101]>\n"
                           "   AND DB_LINK = :f2<char[101]>",
                           "Display information about database link");
static toSQL SQLDBLinkSynonyms("toBrowser:DBLinkSynonyms",
                               "SELECT * FROM Sys.all_synonyms a\n"
                               " WHERE Owner = :f1<char[101]>\n"
                               "   AND DB_LINK = :f2<char[101]>",
                               "Display foreign synonyms");
#endif

static toSQL SQLMySQLAccess("toBrowser:MySQLAcess",
                            "SHOW TABLES FROM mysql",
                            "Show access tables for MySQL databases",
                            "3.23",
                            "MySQL");

static toSQL SQLMySQLUsers("toBrowser:MySQLUsers",
                           "SELECT concat(user,'@',host) Users FROM mysql.user",
                           "Show users for MySQL databases",
                           "3.23",
                           "MySQL");

static toSQL SQLTruncateTable("toBrowser:TruncateTable",
                              "TRUNCATE TABLE %1.%2",
                              "Truncate a table",
                              "",
                              "Any");

QString toBrowser::schema(void)
{
    try
    {
        QString ret = Schema->selected();
        if (ret == tr("No schemas"))
            return connection().database();
        return ret;
    }
    catch (...)
    {
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
    for (std::map<QString, toResultTableView *>::iterator i = Map.begin();i != Map.end();i++)
        (*i).second->setFilter(Filter->clone());
    refresh();
}

toBrowser::toBrowser(QWidget *parent, toConnection &connection)
        : toToolWidget(BrowserTool, "browser.html", parent, connection, "toBrowser")
{
    SecondTab  = NULL;
    SecondText = QString::null;

    Filter = new toBrowserFilter(false);

    QToolBar *toolbar = toAllocBar(this, tr("DB Browser"));
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

    Schema = new toResultSchema(connection, toolbar, TO_TOOLBAR_WIDGET_NAME);
    connect(Schema, SIGNAL(activated(int)),
            this, SLOT(changeSchema(int)));
    toolbar->addWidget(Schema);

    toolbar->addWidget(new QLabel(toolbar));
    new toChangeConnection(toolbar, TO_TOOLBAR_WIDGET_NAME);

    TopTab = new toTabWidget(this);
    layout()->addWidget(TopTab);
    QSplitter *splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_TABLES);
    TopTab->addTab(splitter, tr("T&ables"));
    CurrentTop = splitter;

    QWidget *box = new QWidget(splitter);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    box->setLayout(vbox);

    toolbar = toAllocBar(box, tr("Database browser"));
    vbox->addWidget(toolbar);

    addTableAct = new QAction(QPixmap(const_cast<const char**>(addtable_xpm)),
                              tr("Create new table"),
                              this);
    connect(addTableAct, SIGNAL(triggered()), this, SLOT(addTable(void)));
    toolbar->addAction(addTableAct);

    toolbar->addSeparator();

    modTableAct = new QAction(QPixmap(const_cast<const char**>(modtable_xpm)),
                              tr("Modify table columns"),
                              this);
    connect(modTableAct, SIGNAL(triggered()), this, SLOT(modifyTable(void)));
    toolbar->addAction(modTableAct);

    modConstraintAct = new QAction(QPixmap(const_cast<const char**>(modconstraint_xpm)),
                                   tr("Modify constraints"),
                                   this);
    connect(modConstraintAct, SIGNAL(triggered()), this, SLOT(modifyConstraint(void)));
    toolbar->addAction(modConstraintAct);

    modIndexAct = new QAction(QPixmap(const_cast<const char**>(modindex_xpm)),
                              tr("Modify indexes"),
                              this);
    connect(modIndexAct, SIGNAL(triggered()), this, SLOT(modifyIndex(void)));
    toolbar->addAction(modIndexAct);

    toolbar->addSeparator();

    dropTableAct = new QAction(QPixmap(const_cast<const char**>(trash_xpm)),
                               tr("Drop table"),
                               this);
    connect(dropTableAct, SIGNAL(triggered()), this, SLOT(dropTable(void)));
    toolbar->addAction(dropTableAct);

    toolbar->addSeparator();

    enableConstraintAct = new QAction(QPixmap(const_cast<const char**>(online_xpm)),
                                      tr("Enable constraint or trigger"),
                                      this);
    connect(enableConstraintAct,
            SIGNAL(triggered()),
            this,
            SLOT(enableConstraints(void)));
    toolbar->addAction(enableConstraintAct);

    disableConstraintAct = new QAction(QPixmap(const_cast<const char**>(offline_xpm)),
                                       tr("Disable constraint or trigger"),
                                       this);
    connect(disableConstraintAct,
            SIGNAL(triggered()),
            this,
            SLOT(disableConstraints(void)));
    toolbar->addAction(disableConstraintAct);

    toResultTableView *tableView = new toResultTableView(true, false, box);
    vbox->addWidget(tableView);
    tableView->setReadAll(true);
    tableView->setSQL(SQLListTables);
    setFocusProxy(tableView);
    box->resize(FIRST_WIDTH, tableView->height());
    splitter->setStretchFactor(splitter->indexOf(box), 0);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    FirstTab = tableView;
    Map[TAB_TABLES] = tableView;
    tableView->setTabWidget(TopTab);
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    connect(tableView,
            SIGNAL(displayMenu(QMenu *)),
            this,
            SLOT(displayTableMenu(QMenu *)));


    toTabWidget *curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);

    toResultCols *resultCols = new toResultCols(curr, TAB_TABLE_COLUMNS);
    curr->addTab(resultCols, tr("&Columns"));
    SecondMap[TAB_TABLES] = resultCols;
    SecondMap[TAB_TABLE_COLUMNS] = resultCols;

    tableView = new toResultTableView(true, false, curr, TAB_TABLE_INDEXES);
    tableView->setSQL(SQLTableIndex);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("&Indexes"));
    SecondMap[TAB_TABLE_INDEXES] = tableView;

    tableView = new toResultTableView(true, false, curr, TAB_TABLE_CONS);
    tableView->setSQL(SQLTableConstraint);
    curr->addTab(tableView, tr("C&onstraints"));
    SecondMap[TAB_TABLE_CONS] = tableView;

    tableView = new toResultTableView(true, false, curr, TAB_TABLE_DEPEND);
    tableView->setSQL(SQLTableReferences);
    curr->addTab(tableView, tr("&References"));
    SecondMap[TAB_TABLE_DEPEND] = tableView;

    tableView = new toResultTableView(true, false, curr, TAB_TABLE_GRANTS);
    tableView->setSQL(SQLAnyGrants);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("&Grants"));
    SecondMap[TAB_TABLE_GRANTS] = tableView;

    tableView = new toResultTableView(true, false, curr, TAB_TABLE_TRIGGERS);
    tableView->setSQL(SQLTableTrigger);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("Triggers"));
    SecondMap[TAB_TABLE_TRIGGERS] = tableView;

    TableContent = new toResultData(curr, TAB_TABLE_DATA);
    curr->addTab(TableContent, tr("&Data"));
    SecondMap[TAB_TABLE_DATA] = TableContent;

    toResultItem *resultInfo = new toResultItem(2, true, curr, TAB_TABLE_INFO);
    resultInfo->setSQL(SQLTableInfo);
    curr->addTab(resultInfo, tr("Information"));
    SecondMap[TAB_TABLE_INFO] = resultInfo;

    tableView = new toResultTableView(true, false, curr, TAB_TABLE_STATISTIC);
    tableView->setSQL(SQLTableStatistic);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("Statistic"));
    SecondMap[TAB_TABLE_STATISTIC] = tableView;

    tableView = new toResultTableView(true, false, curr, TAB_TABLE_PARTITION);
    tableView->setSQL(SQLTablePartition);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("Partitions"));
    SecondMap[TAB_TABLE_PARTITION] = tableView;

    toResultExtent *resultExtent = new toResultExtent(curr, TAB_TABLE_EXTENT);
    curr->addTab(resultExtent, tr("Extents"));
    SecondMap[TAB_TABLE_EXTENT] = resultExtent;

    toResultExtract *resultExtract = new toResultExtract(true, this, TAB_TABLE_EXTRACT);
    curr->addTab(resultExtract, tr("Script"));
    SecondMap[TAB_TABLE_EXTRACT] = resultExtract;

    // make sure this gets setup as SecondTab
    //curr->setCurrentWidget(resultCols);
    changeSecondTab(resultCols);
    connect(curr, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeSecondTab(QWidget *)));

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_VIEWS);
    TopTab->addTab(splitter, tr("&Views"));
    tableView = new toResultTableView(true, false, splitter);
    tableView->setReadAll(true);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_VIEWS] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->setSQL(SQLListView);
    tableView->resize(FIRST_WIDTH, tableView->height());
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    splitter->setStretchFactor(splitter->indexOf(tableView), 0);

    curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);
    resultCols = new toResultCols(curr, TAB_VIEW_COLUMNS);
    curr->addTab(resultCols, tr("&Columns"));
    SecondMap[TAB_VIEWS] = resultCols;
    SecondMap[TAB_VIEW_COLUMNS] = resultCols;

    toResultField *resultField = new toResultField(curr, TAB_VIEW_SQL);
    resultField->setSQL(SQLViewSQL);
    curr->addTab(resultField, tr("SQL"));
    SecondMap[TAB_VIEW_SQL] = resultField;

    ViewContent = new toResultData(curr, TAB_VIEW_DATA);
    curr->addTab(ViewContent, tr("&Data"));
    SecondMap[TAB_VIEW_DATA] = ViewContent;

    tableView = new toResultTableView(true, false, curr, TAB_VIEW_GRANTS);
    tableView->setSQL(SQLAnyGrants);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("&Grants"));
    SecondMap[TAB_VIEW_GRANTS] = tableView;

    toResultDepend *resultDepend = new toResultDepend(curr, TAB_VIEW_DEPEND);
    curr->addTab(resultDepend, tr("De&pendencies"));
    SecondMap[TAB_VIEW_DEPEND] = resultDepend;

    resultExtract = new toResultExtract(true, this, TAB_VIEW_EXTRACT);
    curr->addTab(resultExtract, tr("Script"));
    SecondMap[TAB_VIEW_EXTRACT] = resultExtract;
    connect(curr, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeSecondTab(QWidget *)));

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_INDEX);

    TopTab->addTab(splitter, tr("Inde&xes"));

    box = new QWidget(splitter);
    vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    box->setLayout(vbox);

    toolbar = toAllocBar(box, tr("Database browser"));
    vbox->addWidget(toolbar);

    toolbar->addWidget(
        new toBrowseButton(QPixmap(const_cast<const char**>(addindex_xpm)),
                           tr("Add indexes"),
                           tr("Add indexes"),
                           this, SLOT(addIndex()),
                           toolbar));

    toolbar->addSeparator();

    toolbar->addWidget(
        new toBrowseButton(QPixmap(const_cast<const char**>(modindex_xpm)),
                           tr("Modify indexes"),
                           tr("Modify indexes"),
                           this, SLOT(modifyIndex()),
                           toolbar));

    toolbar->addSeparator();

    toolbar->addWidget(
        new toBrowseButton(QPixmap(const_cast<const char**>(trash_xpm)),
                           tr("Drop index"),
                           tr("Drop index"),
                           this, SLOT(dropIndex()),
                           toolbar));

    tableView = new toResultTableView(true, false, box);
    vbox->addWidget(tableView);
    tableView->setReadAll(true);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_INDEX] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->setSQL(SQLListIndex);
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    connect(tableView, SIGNAL(displayMenu(QMenu *)), this, SLOT(displayIndexMenu(QMenu *)));

    box->resize(FIRST_WIDTH, tableView->height());
    splitter->setStretchFactor(splitter->indexOf(box), 0);
    curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);

    tableView = new toResultTableView(true, false, curr, TAB_INDEX_COLS);
    tableView->setSQL(SQLIndexCols);
#if 0
    connect(tableView, SIGNAL(done()), this, SLOT(fixIndexCols()));
#endif
    curr->addTab(tableView, tr("&Columns"));
    SecondMap[TAB_INDEX] = tableView;
    SecondMap[TAB_INDEX_COLS] = tableView;

    toResultItem *resultIdxInfo = new toResultItem(2, true, curr, TAB_INDEX_INFO);
    resultIdxInfo->setSQL(SQLIndexInfo);
    curr->addTab(resultIdxInfo, tr("Info"));
    SecondMap[TAB_INDEX_INFO] = resultIdxInfo;

    tableView = new toResultTableView(true, false, curr, TAB_INDEX_STATISTIC);
    tableView->setSQL(SQLIndexStatistic);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("&Statistic"));
    SecondMap[TAB_INDEX_STATISTIC] = tableView;

    resultExtent = new toResultExtent(curr, TAB_INDEX_EXTENT);
    curr->addTab(resultExtent, tr("Extents"));
    SecondMap[TAB_INDEX_EXTENT] = resultExtent;

    resultExtract = new toResultExtract(true, this, TAB_INDEX_EXTRACT);
    curr->addTab(resultExtract, tr("Script"));
    SecondMap[TAB_INDEX_EXTRACT] = resultExtract;
    connect(curr, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeSecondTab(QWidget *)));

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_SEQUENCES);
    TopTab->addTab(splitter, tr("Se&quences"));
    tableView = new toResultTableView(true, false, splitter);
    tableView->setReadAll(true);
    tableView->setSQL(SQLListSequence);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_SEQUENCES] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->setSQL(SQLListSequence);
    tableView->resize(FIRST_WIDTH, tableView->height());
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    splitter->setStretchFactor(splitter->indexOf(tableView), 0);

    curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);
    toResultItem *resultSequences = new toResultItem(2, true, curr, TAB_SEQUENCES_INFO);
    resultSequences->setSQL(SQLSequenceInfo);
    curr->addTab(resultSequences, tr("Info"));
    SecondMap[TAB_SEQUENCES] = resultSequences;
    SecondMap[TAB_SEQUENCES_INFO] = resultSequences;

    tableView = new toResultTableView(true, false, curr, TAB_SEQUENCES_GRANTS);
    tableView->setSQL(SQLAnyGrants);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("&Grants"));
    SecondMap[TAB_SEQUENCES_GRANTS] = tableView;

    resultExtract = new toResultExtract(true, this, TAB_SEQUENCES_EXTRACT);
    curr->addTab(resultExtract, tr("Script"));
    SecondMap[TAB_SEQUENCES_EXTRACT] = resultExtract;
    connect(curr, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeSecondTab(QWidget *)));

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_SYNONYM);
    TopTab->addTab(splitter, tr("S&ynonyms"));
    tableView = new toResultTableView(true, false, splitter);
    tableView->setReadAll(true);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_SYNONYM] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->setSQL(SQLListSynonym);
    tableView->resize(FIRST_WIDTH, tableView->height());
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    splitter->setStretchFactor(splitter->indexOf(tableView), 0);

    curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);
    toResultItem *resultSynonym = new toResultItem(2, true, curr, TAB_SYNONYM_INFO);
    resultSynonym->setSQL(SQLSynonymInfo);
    curr->addTab(resultSynonym, tr("Info"));
    SecondMap[TAB_SYNONYM] = resultSynonym;
    SecondMap[TAB_SYNONYM_INFO] = resultSynonym;

    tableView = new toResultTableView(true, false, curr, TAB_SYNONYM_GRANTS);
    tableView->setSQL(SQLAnyGrants);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("&Grants"));
    SecondMap[TAB_SYNONYM_GRANTS] = tableView;

    resultExtract = new toResultExtract(true, this, TAB_SYNONYM_EXTRACT);
    curr->addTab(resultExtract, tr("Script"));
    SecondMap[TAB_SYNONYM_EXTRACT] = resultExtract;

    connect(curr, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeSecondTab(QWidget *)));

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_PLSQL);
    TopTab->addTab(splitter, tr("Cod&e"));
    tableView = new toResultTableView(true, false, splitter);
    tableView->setReadAll(true);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_PLSQL] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->setSQL(SQLListSQL);
    tableView->resize(FIRST_WIDTH*2, tableView->height());
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    splitter->setStretchFactor(splitter->indexOf(tableView), 0);

    curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);
    resultField = new toResultField(curr, TAB_PLSQL_SOURCE);
    resultField->setSQL(SQLSQLHead);
    curr->addTab(resultField, tr("&Declaration"));
    SecondMap[TAB_PLSQL] = resultField;
    SecondMap[TAB_PLSQL_SOURCE] = resultField;

    resultField = new toResultField(curr, TAB_PLSQL_BODY);
    resultField->setSQL(SQLSQLBody);
    curr->addTab(resultField, tr("B&ody"));
    SecondMap[TAB_PLSQL_BODY] = resultField;

    tableView = new toResultTableView(true, false, curr, TAB_PLSQL_GRANTS);
    tableView->setReadAll(true);
    tableView->setSQL(SQLAnyGrants);
    curr->addTab(tableView, tr("&Grants"));
    SecondMap[TAB_PLSQL_GRANTS] = tableView;

    resultDepend = new toResultDepend(curr, TAB_PLSQL_DEPEND);
    curr->addTab(resultDepend, tr("De&pendencies"));
    SecondMap[TAB_PLSQL_DEPEND] = resultDepend;

    resultExtract = new toResultExtract(true, this, TAB_PLSQL_EXTRACT);
    curr->addTab(resultExtract, tr("Script"));
    SecondMap[TAB_PLSQL_EXTRACT] = resultExtract;

    connect(curr, SIGNAL(currentTabChanged(QWidget *)), this, SLOT(changeSecondTab(QWidget *)));

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_TRIGGER);
    TopTab->addTab(splitter, tr("Tri&ggers"));
    tableView = new toResultTableView(true, false, splitter);
    tableView->setReadAll(true);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_TRIGGER] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->setSQL(SQLListTrigger);
    tableView->resize(FIRST_WIDTH, tableView->height());
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));
    splitter->setStretchFactor(splitter->indexOf(tableView), 0);

    curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);
    toResultItem *resultTrigger = new toResultItem(2, true, curr, TAB_TRIGGER_INFO);
    resultTrigger->setSQL(SQLTriggerInfo);
    curr->addTab(resultTrigger, tr("Info"));
    SecondMap[TAB_TRIGGER] = resultTrigger;
    SecondMap[TAB_TRIGGER_INFO] = resultTrigger;

    resultField = new toResultField(curr, TAB_TRIGGER_SOURCE);
    resultField->setSQL(SQLTriggerBody);
    curr->addTab(resultField, tr("C&ode"));
    SecondMap[TAB_TRIGGER_SOURCE] = resultField;

    tableView = new toResultTableView(true, false, curr, TAB_TRIGGER_COLS);
    tableView->setSQL(SQLTriggerCols);
    curr->addTab(tableView, tr("&Columns"));
    SecondMap[TAB_TRIGGER_COLS] = tableView;

    tableView = new toResultTableView(true, false, curr, TAB_TRIGGER_GRANTS);
    tableView->setSQL(SQLAnyGrants);
    tableView->setReadAll(true);
    curr->addTab(tableView, tr("&Grants"));
    SecondMap[TAB_TRIGGER_GRANTS] = tableView;

    resultDepend = new toResultDepend(curr, TAB_TRIGGER_DEPEND);
    curr->addTab(resultDepend, tr("De&pendencies"));
    SecondMap[TAB_TRIGGER_DEPEND] = resultDepend;

    resultExtract = new toResultExtract(true, this, TAB_TRIGGER_EXTRACT);
    curr->addTab(resultExtract, tr("Script"));
    SecondMap[TAB_TRIGGER_EXTRACT] = resultExtract;

    connect(curr,
            SIGNAL(currentTabChanged(QWidget *)),
            this,
            SLOT(changeSecondTab(QWidget *)));

#ifdef DBLINK

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_DBLINK);
    TopTab->addTab(splitter, tr("DBLinks"));

    box = new QWidget(splitter);

    vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    box->setLayout(vbox);

    toolbar = toAllocBar(box, tr("Database browser"));
    vbox->addWidget(toolbar);

    testDBLinkAct = new QAction(QPixmap(const_cast<const char**>(modconstraint_xpm)),
                                tr("Test DBLink"), this);
    connect(testDBLinkAct, SIGNAL(triggered()), this, SLOT(testDBLink()));
    toolbar->addAction(testDBLinkAct);
    toolbar->addSeparator();

    tableView = new toResultTableView(true, false, box);
    vbox->addWidget(tableView);
    tableView->setReadAll(true);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_DBLINK] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->setSQL(SQLListDBLink);
    tableView->resize(FIRST_WIDTH, tableView->height());

    connect(tableView,
            SIGNAL(selectionChanged()),
            this,
            SLOT(changeItem()));
    connect(tableView,
            SIGNAL(displayMenu(QMenu *)),
            this,
            SLOT(displayIndexMenu(QMenu *)));

    curr = new toTabWidget(splitter);

    box->resize(FIRST_WIDTH, tableView->height());
    splitter->setStretchFactor(splitter->indexOf(tableView), 0);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);

    toResultItem *resultDBLink = new toResultItem(
        2,
        true,
        curr,
        TAB_DBLINK_INFO);
    resultDBLink->setSQL(SQLDBLinkInfo);
    curr->addTab(resultDBLink, tr("Info"));
    SecondMap[TAB_DBLINK] = resultDBLink;
    SecondMap[TAB_DBLINK_INFO] = resultDBLink;

    tableView = new toResultTableView(
        true,
        false,
        curr,
        TAB_DBLINK_SYNONYMS);
    tableView->setSQL(SQLDBLinkSynonyms);
    curr->addTab(tableView, tr("&Synonyms"));
    SecondMap[TAB_DBLINK_SYNONYMS] = tableView;

    connect(curr,
            SIGNAL(currentTabChanged(QWidget *)),
            this,
            SLOT(changeSecondTab(QWidget *)));
#endif

    splitter = new QSplitter(Qt::Horizontal, TopTab);
    splitter->setObjectName(TAB_ACCESS);
    TopTab->addTab(splitter, tr("Access"));

#ifdef TOEXTENDED_MYSQL
    box = new QWidget(splitter);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->setSpacing(0);
    vbox->setContentsMargins(0, 0, 0, 0);
    setLayout(vbox);

    toolbar = toAllocBar(box, tr("Database browser"));
    vbox->addWidget(box);
    toolbar->addWidget(
        new toBrowseButton(QPixmap(const_cast<const char**>(new_xpm)),
                           tr("Add user"),
                           tr("Add user"),
                           this, SLOT(addUser()),
                           toolbar));
    toolbar->addWidget(
        new toBrowseButton(QPixmap(const_cast<const char**>(trash_xpm)),
                           tr("Drop user"),
                           tr("Drop user"),
                           this, SLOT(dropUser()),
                           toolbar));

    tableView = new toResultTableView(true, false, box);
    vbox->addWidget(tableView);
#else
    tableView = new toResultTableView(true, false, splitter);
#endif

    tableView->setReadAll(true);
    connect(tableView, SIGNAL(done()), this, SLOT(firstDone()));
    Map[TAB_ACCESS] = tableView;
    tableView->setTabWidget(TopTab);
    tableView->resize(FIRST_WIDTH, tableView->height());
    connect(tableView, SIGNAL(selectionChanged()),
            this, SLOT(changeItem()));

    curr = new toTabWidget(splitter);
    splitter->setStretchFactor(splitter->indexOf(curr), 1);

#ifdef TOEXTENDED_MYSQL
    splitter->setResizeMode(box, QSplitter::KeepSize);
    tableView->setSQL(SQLMySQLUsers);

    AccessContent = NULL;

    UserPanel = new toMySQLUser(curr, TAB_ACCESS_USER);
    curr->addTab(UserPanel, tr("&User"));
    SecondMap[TAB_ACCESS] = UserPanel;
    SecondMap[TAB_ACCESS_USER] = UserPanel;

    AccessPanel = new toMySQLUserAccess(curr, TAB_ACCESS_OBJECTS);
    curr->addTab(AccessPanel, tr("&Objects"));
    SecondMap[TAB_ACCESS_OBJECTS] = UserPanel; // Yes, it should be
    // this one, it will
    // signal the
    // TAB_ACCESS_OBJECTS
    // to update.
    connect(AccessPanel, SIGNAL(hasChanged()), UserPanel, SLOT(hasChanged()));
    connect(UserPanel,
            SIGNAL(saveChanges(const QString &, const QString &)),
            AccessPanel,
            SLOT(saveChanges(const QString &, const QString &)));
    connect(UserPanel,
            SIGNAL(changeUser(const QString &)),
            AccessPanel,
            SLOT(changeUser(const QString &)));

    AccessContent = new toResultData(curr, TAB_ACCESS_CONTENT);
    curr->addTab(AccessContent, tr("&Hosts"));
    SecondMap[TAB_ACCESS_CONTENT] = AccessContent;
#else
    splitter->setStretchFactor(splitter->indexOf(tableView), 1);
    tableView->setSQL(SQLMySQLAccess);

    AccessContent = new toResultData(curr, TAB_ACCESS_CONTENT);
    curr->addTab(AccessContent, tr("&Data"));
    SecondMap[TAB_ACCESS] = AccessContent;
    SecondMap[TAB_ACCESS_CONTENT] = AccessContent;
#endif

    connect(AccessContent, SIGNAL(changesSaved()), this, SLOT(flushPrivs()));

    connect(curr,
            SIGNAL(currentTabChanged(QWidget *)),
            this,
            SLOT(changeSecondTab(QWidget *)));

    ToolMenu = NULL;
    connect(toMainWidget()->workspace(),
            SIGNAL(subWindowActivated(QMdiSubWindow *)),
            this,
            SLOT(windowActivated(QMdiSubWindow *)));

    connect(TopTab,
            SIGNAL(currentTabChanged(QWidget *)),
            this,
            SLOT(changeTab(QWidget *)));
    connect(this, SIGNAL(connectionChange()), this, SLOT(changeConnection()));
    Schema->setFocus();

    setNewFilter(NULL);
}

void toBrowser::windowActivated(QMdiSubWindow *widget)
{
    if (!widget)
        return;
    if (widget->widget() == this)
    {
        if (!ToolMenu)
        {
            ToolMenu = new QMenu(tr("&Browser"), this);

            ToolMenu->addAction(refreshAct);

            ToolMenu->addSeparator();

            ToolMenu->addAction(FilterButton);
            ToolMenu->addAction(clearFilterAct);

            toMainWidget()->addCustomMenu(ToolMenu);
        }
    }
    else
    {
        delete ToolMenu;
        ToolMenu = NULL;
    }
}

static toSQL SQLDropUser("toBrowser:DropUser",
                         "DELETE FROM mysql.user WHERE concat(user,'@',host) = :f1<char[255]>",
                         "Drop MYSQL user",
                         "3.23",
                         "MySQL");

void toBrowser::addUser()
{
#ifdef TOEXTENDED_MYSQL
    UserPanel->changeParams(QString::null, QString::null);
#endif
}

void toBrowser::dropUser()
{
#ifdef TOEXTENDED_MYSQL
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

void toBrowser::changeSchema(int)
{
    SecondText = QString::fromLatin1("");
    changeSecond();
    refresh();
    updateTabs();
}

toBrowser::~toBrowser()
{
    delete Filter;
}

void toBrowser::refresh(void)
{
    try
    {
        Schema->refresh();

        if (FirstTab)
        {
            toQList pars = FirstTab->params();
            if (pars.empty())
            {
                updateTabs();
                return;
            }
            FirstTab->refresh();
        }
        if (SecondTab)
        {
            toQList pars = SecondTab->params();
            if (pars.empty())
            {
                updateTabs();
                return ;
            }
            SecondTab->refresh();
        }
    }
    TOCATCH
}

void toBrowser::focusObject(void)
{
    if (FirstTab)
        FirstTab->setFocus();
}

void toBrowser::changeConnection(void)
{
    Schema->query(toSQL::sql(toSQL::TOSQL_USERLIST));

    if ( ! connection().schema().isEmpty() )
    {
        Schema->setSelected(connection().schema().toUpper());
    }
    else if (toIsMySQL(connection()))
    {
        Schema->setSelected(connection().database());
    }
    else if (toIsOracle(connection()) || toIsSapDB(connection()))
    {
        Schema->setSelected(connection().user().toUpper());
    }
    else
    {
        Schema->setSelected(connection().user());
    }
    SecondText = QString::null;
    updateTabs();
}

void toBrowser::updateTabs(void)
{
    try
    {
#if 0
        // Can't remember why I added this, but it's breaking things
        // Without it breaks the default-schema stuff... with it breaks changing schemas...
        // definately have more work to do here
        if (!Schema->selected().isEmpty() && FirstTab)
            Schema->setSelected(connection().schema().toUpper());
#endif

        if (!Schema->selected().isEmpty() && FirstTab)
            FirstTab->changeParams(
                schema(),
                Filter ? Filter->wildCard() : QString::fromLatin1("%"));
        firstDone(); // In case it is ignored cause it is already done.
        if (SecondTab && !SecondText.isEmpty())
            changeSecond();
    }
    TOCATCH;
}

void toBrowser::firstDone(void)
{
    if (!SecondText.isEmpty() && FirstTab)
    {
        for (int row = 0; row < FirstTab->model()->rowCount(); row++)
        {
            QModelIndex index = FirstTab->model()->index(row, 1);
            if (FirstTab->model()->data(
                        index,
                        Qt::DisplayRole).toString() == SecondText)
            {
                FirstTab->setCurrentIndex(index);
//                 FirstTab->setSelected((*it), true);
//                 FirstTab->setCurrentItem((*it));
//                 FirstTab->ensureItemVisible((*it));
                break;
            }
        }
    }
    else
    {
        QModelIndex item = selectedItem();
        if (item.isValid())
            SecondText = item.data(Qt::EditRole).toString();
    }
}

void toBrowser::changeItem()
{
    QModelIndex item = selectedItem();
    if (item.isValid())
    {
        SecondText = item.data(Qt::EditRole).toString();
        if (SecondTab && !SecondText.isEmpty())
            changeSecond();
    }
}

void toBrowser::changeSecond()
{
    QWidget *tab = TopTab->currentWidget();
    QWidget *tab2 = dynamic_cast<QWidget *>(SecondTab);

    if (tab && tab->objectName() == TAB_SYNONYM)
    {
        QString owner;
        QString name;
        int pos = SecondText.indexOf(QString("."));

        if (pos >= 0)
        {
            owner = SecondText.mid(0, pos);
            name = SecondText.mid(pos + 1);
        }
        else
        {
            owner = QString::fromLatin1("PUBLIC");
            name = SecondText;
        }

        SecondTab->changeParams(owner, name);

#ifdef TOEXTENDED_MYSQL
    }
    else if (tab && tab->objectName() == TAB_ACCESS &&
             tab2->objectName() == TAB_ACCESS_CONTENT)
    {
        SecondTab->changeParams("mysql", "host");
#else

    }
    else if (tab && tab->objectName() == TAB_ACCESS)
    {
        SecondTab->changeParams("mysql", SecondText);
#endif

    }
    else if (tab && tab->objectName() == TAB_INDEX &&
             tab2->objectName() == TAB_INDEX_EXTRACT)
    {
        QModelIndex item = selectedItem();
        if (item.isValid())
            SecondTab->changeParams(schema(),
                                    item.data(Qt::EditRole).toString());
    }
    else
        SecondTab->changeParams(schema(), SecondText);
}

void toBrowser::changeSecondTab(QWidget *tab)
{
    if (!tab)
        return;

    toResult *result = dynamic_cast<toResult *>(tab);
    if (!result)
        return;

    SecondTab = result;
    SecondMap[TopTab->currentWidget()->objectName()] = SecondTab;
    if (SecondTab && !SecondText.isEmpty())
        changeSecond();
}

void toBrowser::changeTab(QWidget *tab)
{
    if (tab && this == toMainWidget()->workspace()->activeSubWindow()->widget())
    {
        toResultTableView *newtab = Map[tab->objectName()];

        if (newtab == FirstTab)
            return;

        CurrentTop = tab;
        setFocusProxy(newtab);
        FirstTab = newtab;
        SecondTab = SecondMap[tab->objectName()];
        SecondText = "";

        if (newtab->model() && newtab->model()->rowCount() > 0)
            return;

        if (FirstTab && SecondTab)
            updateTabs();
    }
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

bool toBrowser::canHandle(toConnection &conn)
{
    return toIsOracle(conn) ||
           toIsPostgreSQL(conn) ||
           toIsMySQL(conn) ||
           toIsSapDB(conn);
}

void toBrowser::modifyTable(void)
{
    toBrowserTable::editTable(connection(),
                              Schema->selected(),
                              SecondText,
                              this);
    refresh();
}

void toBrowser::addTable(void)
{
    toBrowserTable::newTable(connection(),
                             Schema->selected(),
                             this);
    refresh();
}

void toBrowser::modifyConstraint(void)
{
    toBrowserConstraint::modifyConstraint(connection(),
                                          Schema->selected(),
                                          SecondText,
                                          this);
    refresh();
}

void toBrowser::modifyIndex(void)
{
    QString index;
    QModelIndex item = selectedItem(1);
    if (FirstTab->model()->columnCount() > 1 && item.isValid())
        index = item.data(Qt::EditRole).toString();

    if (item.isValid())
    {
        toBrowserIndex::modifyIndex(connection(),
                                    Schema->selected(),
                                    item.data(Qt::EditRole).toString(),
                                    this,
                                    index);
    }
    refresh();
}

void toBrowser::addIndex(void)
{
    toBrowserIndex::addIndex(connection(),
                             Schema->selected(),
                             SecondText,
                             this);
    refresh();
}

void toBrowser::displayTableMenu(QMenu *menu)
{
    menu->addSeparator();

    menu->addAction(dropTableAct);
    menu->addAction(tr("Truncate table"), this, SLOT(truncateTable()));

    menu->addSeparator();

    if (toIsMySQL(connection()))
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
    if (what.isEmpty())
        return ;
    if (TOMessageBox::warning(this, tr("Dropping %1?").arg(tr(type.toAscii().constData())),
                              tr("Are you sure you want to drop the %1 %2.%3,\n"
                                 "this action can not be undone?").arg(tr(type.toAscii().constData())).arg(
                                  Schema->selected()).arg(what),
                              tr("&Yes"), tr("&Cancel"), QString::null, 0) == 0)
    {
        std::list<QString> ctx;
        toPush(ctx, Schema->selected());
        toPush(ctx, type.toUpper());
        QStringList parts = what.split(".");
        if (parts.count() > 1)
        {
            toPush(ctx, parts[1]);
            toPush(ctx, QString("ON"));
            toPush(ctx, parts[0]);
        }
        else
        {
            toPush(ctx, what);
        }

        std::list<QString> drop;
        toExtract::addDescription(drop, ctx);

        std::list<QString> empty;

        try
        {
            toExtract extractor(connection(), NULL);
            extractor.setIndexes(false);
            extractor.setConstraints(false);
            extractor.setPrompt(false);
            extractor.setHeading(false);

            std::list<QString> objToDrop;
            QString o = type.toUpper()+ ":" + Schema->selected() + "." + what;
            objToDrop.push_back(o);
            QString sql = extractor.drop(objToDrop);

            std::list<toSQLParse::statement> statements = toSQLParse::parse(sql, connection());
            QProgressDialog prog(tr("Executing %1 change script").arg(tr(type.toAscii().constData())),
                                 tr("Stop"),
                                 0,
                                 statements.size(),
                                 this);
            prog.setWindowTitle(tr("Performing %1 changes").arg(tr(type.toAscii().constData())));

            for (std::list<toSQLParse::statement>::iterator j = statements.begin();j != statements.end();j++)
            {
                QString sql = toSQLParse::indentStatement(*j, connection());
                int i = sql.length() - 1;
                while (i >= 0 && (sql.at(i) == ';' || sql.at(i).isSpace()))
                    i--;
                if (i >= 0)
                    connection().execute(sql.mid(0, i + 1));
                qApp->processEvents();
                if (prog.wasCanceled())
                    throw tr("Canceled ongoing %1 modification, %2 might be corrupt").arg(tr(type.toAscii().constData())).arg(tr(type.toAscii().constData()));
            }
        }
        TOCATCH
    }
    updateTabs();
    refresh();
}

void toBrowser::dropTable(void)
{
    dropSomething("TABLE", SecondText);
}

void toBrowser::truncateTable(void)
{
    bool force = false;
    for (toResultTableView::iterator it(FirstTab); (*it).isValid(); it++)
    {
        if (FirstTab->isRowSelected(*it))
        {
            switch (force ? 0 : TOMessageBox::warning(this, tr("Truncate table?"),
                    tr("Are you sure you want to truncate the table %2.%3,\n"
                       "this action can not be undone?").arg(
                        Schema->selected()).arg((*it).data(Qt::EditRole).toString()),
                    tr("&Yes"), tr("Yes to &all"), tr("&Cancel"), 0))
            {
            case 1 :
                force = true;
                // Intentionally no break here.
            case 0:
                connection().execute(
                    toSQL::string(SQLTruncateTable, connection()).
                    arg(connection().quote(Schema->selected())).
                    arg(connection().quote((*it).data(Qt::EditRole).toString())));
                updateTabs();
                break;
            case 2:
                return;
            }
        }
    }
}

void toBrowser::flushPrivs(void)
{
    try
    {
        connection().execute("FLUSH PRIVILEGES");
    }
    TOCATCH
}

void toBrowser::checkTable(void)
{
    QString sql;

    for (toResultTableView::iterator it(FirstTab); (*it).isValid(); it++)
    {
        if (FirstTab->isRowSelected(*it))
        {
            if (sql.isEmpty())
                sql = "CHECK TABLE ";
            else
                sql += ", ";
            sql += connection().quote(Schema->selected()) + "." +
                   connection().quote((*it).data(Qt::EditRole).toString());
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

    for (toResultTableView::iterator it(FirstTab); (*it).isValid(); it++)
    {
        if (FirstTab->isRowSelected(*it))
        {
            if (sql.isEmpty())
                sql = "OPTIMIZE TABLE ";
            else
                sql += ", ";
            sql += connection().quote(Schema->selected()) + "." +
                   connection().quote((*it).data(Qt::EditRole).toString());
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
    bool ok;
    QString text = QInputDialog::getText(this,
                                         "Change table type",
                                         "Enter new table type",
                                         QLineEdit::Normal,
                                         "MyISAM",
                                         &ok);
    if (ok && !text.isEmpty())
    {
        for (toResultTableView::iterator it(FirstTab); (*it).isValid(); it++)
        {
            if (FirstTab->isRowSelected(*it))
            {
                QString sql = "ALTER TABLE ";
                sql += connection().quote(Schema->selected()) + "." +
                       connection().quote((*it).data(Qt::EditRole).toString());
                sql += " TYPE = " + text;
                try
                {
                    connection().execute(sql);
                }
                TOCATCH
            }
        }
    }
}

void toBrowser::analyzeTable(void)
{
    QString sql;

    for (toResultTableView::iterator it(FirstTab); (*it).isValid(); it++)
    {
        if (FirstTab->isRowSelected(*it))
        {
            if (sql.isEmpty())
                sql = "ANALYZE TABLE ";
            else
                sql += ", ";
            sql += connection().quote(Schema->selected()) + "." +
                   connection().quote((*it).data(Qt::EditRole).toString());
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

QModelIndex toBrowser::selectedItem(int col)
{
    return FirstTab->selectedIndex(col);
}

void toBrowser::dropIndex(void)
{
    dropSomething("INDEX", SecondText);
    // Why there was this?
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


void toBrowser::closeEvent(QCloseEvent *event)
{
    if (ViewContent->maybeSave() &&
            TableContent->maybeSave() &&
            AccessContent->maybeSave())
    {

        // prevent further queries after close
        SecondText = QString::null;
        event->accept();
    }
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

void toBrowser::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    data[prefix + ":Schema"] = Schema->selected();
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
        Filter->exportData(data, prefix + ":Filter");
}

void toBrowser::importData(std::map<QString, QString> &data, const QString &prefix)
{
    disconnect(Schema, SIGNAL(activated(int)),
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
    refresh();
}

#if 0
void toBrowser::fixIndexCols(void)
{
    if (toIsOracle(connection()))
    {
        toResultTableView *tmp = dynamic_cast<toResultTableView *>(SecondMap[TAB_INDEX_COLS]);
        if (tmp)
            for (toTreeWidgetItem *item = tmp->firstChild();item;item = item->nextSibling())
            {
                if (!toUnnull(item->text(4)).isNull())
                {
                    toResultViewItem * resItem = dynamic_cast<toResultViewItem *>(item);
                    if (resItem)
                        resItem->setText(1, resItem->allText(4));
                }
            }
    }
    else if (toIsMySQL(connection()))
    {
        toResultTableView *second = dynamic_cast<toResultTableView *>(SecondMap[TAB_INDEX_COLS]);
        if (FirstTab && second)
        {
            QModelIndex item = selectedItem(1);
            if (item.isValid())
            {
                QString index = item.data(Qt::EditRole).toString();
                for (toTreeWidgetItem *item = second->firstChild();item;)
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

static toBrowseTemplate BrowseTemplate;

void toBrowseTemplate::removeDatabase(const QString &name)
{
    for (std::list<toTemplateItem *>::iterator i = Parents.begin();i != Parents.end();i++)
    {
        for (toTreeWidgetItem *item = (*i)->firstChild();item;item = item->nextSibling())
            if (item->text(0) == name)
            {
                delete item;
                break;
            }
    }
}

void toBrowseTemplate::defineFilter(void)
{
    if (Filter)
    {
        toBrowserFilterSetup filt(true, *Filter, toMainWidget());
        if (filt.exec())
        {
            delete Filter;
            Filter = filt.getSetting();
        }
    }
    else
    {
        toBrowserFilterSetup filt(true, toMainWidget());
        if (filt.exec())
            Filter = filt.getSetting();
    }
    if (Filter)
    {
        disconnect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
        FilterButton->setChecked(true);
        connect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
    }
}

void toBrowseTemplate::clearFilter(void)
{
    delete Filter;
    Filter = new toBrowserFilter;
    disconnect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
    FilterButton->setChecked(false);
    connect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));
}

void toBrowseTemplate::removeItem(toTreeWidgetItem *item)
{
    for (std::list<toTemplateItem *>::iterator i = Parents.begin();i != Parents.end();i++)
        if ((*i) == item)
        {
            Parents.erase(i);
            break;
        }
}

class toTemplateTableItem : public toTemplateItem
{
    toConnection &Connection;
public:
    toTemplateTableItem(toConnection &conn, toTemplateItem *parent,
                        const QString &name)
            : toTemplateItem(parent, name), Connection(conn) {}
    virtual QWidget *selectedWidget(QWidget *par)
    {
        QString ptyp = parent()->parent()->text(0);
        QString object = parent()->text(0);
        QString typ = text(0);
        QString schema = parent()->parent()->parent()->text(0);
        if (ptyp == "Synonyms")
        {
            int pos = object.indexOf(QString::fromLatin1("."));
            if (pos >= 0)
            {
                schema = object.mid(0, pos);
                object = object.mid(pos + 1);
            }
            else
            {
                schema = QString::fromLatin1("PUBLIC");
            }
        }

        if (schema == qApp->translate("toBrowser", "No schemas"))
            schema = Connection.database();

        toResult *res;

        toToolWidget *tool = new toToolWidget(BrowserTool,
                                              "",
                                              par,
                                              Connection);
        if (typ == qApp->translate("toBrowser", "Data"))
        {
            toResultData *cnt = new toResultData(tool);
            cnt->changeParams(schema, object);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Indexes"))
        {
            toResultTableView *tv = new toResultTableView(true, false, tool);
            tv->setReadAll(true);
            tv->setSQL(SQLTableIndex);
            res = tv;
        }
        else if (typ == qApp->translate("toBrowser", "Extents"))
        {
            new toResultExtent(tool);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Constraints"))
        {
            toResultTableView *tv = new toResultTableView(true, false, tool);
            tv->setSQL(SQLTableConstraint);
            res = tv;
        }
        else if (typ == qApp->translate("toBrowser", "Triggers"))
        {
            toResultTableView *tv = new toResultTableView(true, false, tool);
            tv->setReadAll(true);
            tv->setSQL(SQLTableTrigger);
            res = tv;
        }
        else if (typ == qApp->translate("toBrowser", "SQL"))
        {
            toResultField *sql = new toResultField(tool);
            sql->setSQL(SQLViewSQL);
            sql->changeParams(schema, object);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Script"))
        {
            toResultExtract *ext = new toResultExtract(true, tool);
            ext->changeParams(schema, object);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Information"))
        {
            toResultItem *inf = new toResultItem(2, true, tool);
            if (ptyp == qApp->translate("toBrowser", "Tables"))
            {
                inf->setSQL(SQLTableInfo);
            }
            else if (ptyp == qApp->translate("toBrowser", "Triggers"))
            {
                inf->setSQL(SQLTriggerInfo);
            }
            else if (ptyp == qApp->translate("toBrowser", "Indexes"))
            {
                inf->setSQL(SQLIndexInfo);
            }
            inf->changeParams(schema, object);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Columns"))
        {
            res = new toResultTableView(true, false, tool);
            res->setSQL(SQLTriggerCols);
        }
        else if (typ == qApp->translate("toBrowser", "References"))
        {
            res = new toResultTableView(true, false, tool);
            res->setSQL(SQLTableReferences);
        }
        else if (typ == qApp->translate("toBrowser", "Grants"))
        {
            res = new toResultTableView(true, false, tool);
            res->setSQL(SQLAnyGrants);
        }
        else if (typ == qApp->translate("toBrowser", "Dependencies"))
        {
            res = new toResultDepend(tool);
        }
        else
        {
            delete tool;
            return NULL;
        }
        res->changeParams(schema, object);
        return tool;
    }
};

class toTemplateSchemaItem : public toTemplateItem
{
    toConnection &Connection;
public:
    toTemplateSchemaItem(toConnection &conn, toTemplateItem *parent,
                         const QString &name)
            : toTemplateItem(parent, name), Connection(conn)
    {
        QString typ = parent->text(0);
        if (typ == qApp->translate("toBrowser", "Tables"))
        {
            QPixmap image(const_cast<const char**>(table_xpm));
            setPixmap(0, image);
            new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Indexes"));
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Constraints"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "References"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Triggers"));
            }
            new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Data"));
            new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Extents"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
            }
        }
        else if (typ == qApp->translate("toBrowser", "Views"))
        {
            QPixmap image(const_cast<const char**>(view_xpm));
            setPixmap(0, image);
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "SQL"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Data"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Dependencies"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
            }
        }
        else if (typ == qApp->translate("toBrowser", "Sequences"))
        {
            QPixmap image(const_cast<const char**>(sequence_xpm));
            setPixmap(0, image);
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
            }
        }
        else if (typ == qApp->translate("toBrowser", "Code"))
        {
            QPixmap image(const_cast<const char**>(function_xpm));
            setPixmap(0, image);
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Dependencies"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
            }
        }
        else if (typ == qApp->translate("toBrowser", "Triggers"))
        {
            QPixmap image(const_cast<const char**>(function_xpm));
            setPixmap(0, image);
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Columns"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Dependencies"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
            }
        }
        else if (typ == qApp->translate("toBrowser", "Indexes"))
        {
            QPixmap image(const_cast<const char**>(index_xpm));
            setPixmap(0, image);
            if (toIsOracle(conn) || toIsSapDB(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
            }
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Information"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Extents"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
            }
        }
        else if (typ == qApp->translate("toBrowser", "Synonyms"))
        {
            QPixmap image(const_cast<const char**>(synonym_xpm));
            setPixmap(0, image);
            if (toIsOracle(conn))
            {
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Grants"));
                new toTemplateTableItem(conn, this, qApp->translate("toBrowser", "Script"));
            }
        }
    }

    virtual QString allText(int col) const
    {
        QString txt = parent()->parent()->text(0);
        txt += QString::fromLatin1(".");
        txt += text(col);
        return txt;
    }

    virtual QWidget *selectedWidget(QWidget *par)
    {
        QString object = text(0);
        QString typ = parent()->text(0);
        QString schema = parent()->parent()->text(0);
        if (schema == qApp->translate("toBrowser", "No schemas"))
            schema = Connection.database();

        toToolWidget *tool = new toToolWidget(BrowserTool,
                                              "",
                                              par,
                                              Connection);
        if (typ == qApp->translate("toBrowser", "Code") || typ == qApp->translate("toBrowser", "Triggers"))
        {
            toResultField *fld = new toResultField(tool);
            if (typ == qApp->translate("toBrowser", "Code"))
                fld->setSQL(SQLSQLTemplate);
            else
                fld->setSQL(SQLTriggerBody);
            fld->changeParams(schema, object);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Tables") || typ == qApp->translate("toBrowser", "Views"))
        {
            toResultCols *cols = new toResultCols(tool);
            cols->changeParams(schema, object);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Indexes"))
        {
            toResultTableView *tv = new toResultTableView(true, false, tool);
            tv->setSQL(SQLIndexCols);
            tv->changeParams(schema, object);
            return tool;
        }
        else if (typ == qApp->translate("toBrowser", "Synonyms") || typ == qApp->translate("toBrowser", "Sequences"))
        {
            toResultItem *resultItem = new toResultItem(2, true, tool);
            if (typ == qApp->translate("toBrowser", "Synonyms"))
            {
                resultItem->setSQL(SQLSynonymInfo);
                int pos = object.indexOf(QString::fromLatin1("."));
                if (pos >= 0)
                {
                    schema = object.mid(0, pos);
                    object = object.mid(pos + 1);
                }
                else
                {
                    schema = QString::fromLatin1("PUBLIC");
                }
            }
            else
                resultItem->setSQL(SQLSequenceInfo);
            resultItem->changeParams(schema, object);
            return tool;
        }
        else
        {
            delete tool;
            return NULL;
        }
    }
};

class toTemplateSchemaList : public toTemplateSQL
{
public:
    toTemplateSchemaList(toConnection &conn, toTemplateItem *parent,
                         const QString &name, const QString &sql)
            : toTemplateSQL(conn, parent, name, sql) { }
    virtual toTemplateItem *createChild(const QString &name)
    {
        try
        {
            toBrowserFilter *filter = BrowseTemplate.filter();
            toTemplateItem *item = new toTemplateSchemaItem(connection(), this, name);
            if (filter && !filter->check(item))
            {
                delete item;
                return NULL;
            }
            return item;
        }
        catch (...)
        {
            return NULL;
        }
    }
    virtual toQList parameters(void)
    {
        toQList ret;
        ret.insert(ret.end(), parent()->text(0));
        toBrowserFilter *filter = BrowseTemplate.filter();
        if (filter)
            ret.insert(ret.end(), filter->wildCard());
        else
            ret.insert(ret.end(), toQValue(QString::fromLatin1("%")));
        return ret;
    }
};

class toTemplateDBItem : public toTemplateSQL
{
public:
    toTemplateDBItem(toConnection &conn, toTemplateItem *parent,
                     const QString &name)
            : toTemplateSQL(conn, parent, name, toSQL::string(toSQL::TOSQL_USERLIST, conn)) {}
    virtual ~toTemplateDBItem()
    {
        toBrowseTemplate *prov = dynamic_cast<toBrowseTemplate *>(&provider());
        if (prov)
            prov->removeItem(this);
    }
    virtual toTemplateItem *createChild(const QString &name)
    {
        try
        {
            toTemplateItem *item = new toTemplateItem(this, name);
            QPixmap image(const_cast<const char**>(schema_xpm));
            item->setPixmap(0, image);
            QPixmap table(const_cast<const char**>(table_xpm));
            QPixmap view(const_cast<const char**>(view_xpm));
            QPixmap sequence(const_cast<const char**>(sequence_xpm));
            QPixmap function(const_cast<const char**>(function_xpm));
            QPixmap index(const_cast<const char**>(index_xpm));
            QPixmap synonym(const_cast<const char**>(synonym_xpm));

            toBrowserFilter *filter = BrowseTemplate.filter();
            if (filter && filter->onlyOwnSchema() &&
                    name.toUpper() != connection().user().toUpper())
            {
                delete item;
                return NULL;
            }

            (new toTemplateSchemaList(connection(),
                                      item,
                                      qApp->translate("toBrowser", "Tables"),
                                      toSQL::string(SQLListTables, connection())))->setPixmap(0, table);

            if (toIsOracle(connection()) ||
                    toIsSapDB(connection()) ||
                    toIsPostgreSQL(connection()))
            {
                (new toTemplateSchemaList(connection(),
                                          item,
                                          qApp->translate("toBrowser", "Views"),
                                          toSQL::string(SQLListView, connection())))->setPixmap(0, view);
                (new toTemplateSchemaList(connection(),
                                          item,
                                          qApp->translate("toBrowser", "Indexes"),
                                          toSQL::string(SQLListIndex, connection())))->setPixmap(0, index);
            }

            if (toIsOracle(connection()) || toIsPostgreSQL(connection()))
            {
                (new toTemplateSchemaList(connection(),
                                          item,
                                          qApp->translate("toBrowser", "Sequences"),
                                          toSQL::string(SQLListSequence, connection())))->setPixmap(0, sequence);
                (new toTemplateSchemaList(connection(),
                                          item,
                                          qApp->translate("toBrowser", "Code"),
                                          toSQL::string(SQLListSQLShort, connection())))->setPixmap(0, function);
                (new toTemplateSchemaList(connection(),
                                          item,
                                          qApp->translate("toBrowser", "Triggers"),
                                          toSQL::string(SQLListTrigger, connection())))->setPixmap(0, function);
            }

            if (toIsOracle(connection()))
            {
                (new toTemplateSchemaList(connection(),
                                          item,
                                          qApp->translate("toBrowser", "Synonyms"),
                                          toSQL::string(SQLListSynonym, connection())))->setPixmap(0, synonym);
            }
            return item;
        }
        catch (...)
        {
            return NULL;
        }
    }
};

class toBrowseTemplateItem : public toTemplateItem
{
public:
    toBrowseTemplateItem(toTemplateProvider &prov, toTreeWidget *parent, const QString &name)
            : toTemplateItem(prov, parent, name) { }
    virtual ~toBrowseTemplateItem()
    {
        dynamic_cast<toBrowseTemplate &>(provider()).removeItem(this);
    }
};

void toBrowseTemplate::insertItems(toTreeWidget *parent, QToolBar *toolbar)
{
    if (!Registered)
    {
        connect(toMainWidget(),
                SIGNAL(addedConnection(const QString &)),
                this,
                SLOT(addDatabase(const QString &)));
        connect(toMainWidget(),
                SIGNAL(removedConnection(const QString &)),
                this,
                SLOT(removeDatabase(const QString &)));
    }

    toTemplateItem *dbitem = new toBrowseTemplateItem(
        *this,
        parent,
        qApp->translate("toBrowser", "DB Browser"));

    std::list<QString> conn = toMainWidget()->connections();
    for (std::list<QString>::iterator i = conn.begin();i != conn.end();i++)
    {
        toConnection &conn = toMainWidget()->connection(*i);
        new toTemplateDBItem(conn, dbitem, *i);
    }
    Parents.insert(Parents.end(), dbitem);

    FilterButton = new QToolButton(toolbar);
    FilterButton->setCheckable(true);
    FilterButton->setIcon(QIcon(QPixmap(const_cast<const char**>(filter_xpm))));
    FilterButton->setToolTip(tr("Define the object filter for database browser"));
    toolbar->addWidget(FilterButton);
    connect(FilterButton, SIGNAL(toggled(bool)), this, SLOT(defineFilter()));

    toolbar->addAction(QIcon(QPixmap(const_cast<const char**>(nofilter_xpm))),
                       qApp->translate(
                           "toBrowser",
                           "Remove any object filter for database browser"),
                       this,
                       SLOT(clearFilter(void)));
}

void toBrowseTemplate::addDatabase(const QString &name)
{
    try
    {
        for (std::list<toTemplateItem *>::iterator i = Parents.begin();i != Parents.end();i++)
            new toTemplateDBItem(toMainWidget()->connection(name), *i, name);
    }
    TOCATCH
}

void toBrowseTemplate::importData(std::map<QString, QString> &data, const QString &prefix)
{
    if (data.find(prefix + ":Filter:Type") != data.end())
    {
        Filter = new toBrowserFilter;
        Filter->importData(data, prefix + ":Filter");
    }
}

void toBrowseTemplate::exportData(std::map<QString, QString> &data, const QString &prefix)
{
    if (Filter)
        Filter->exportData(data, prefix + ":Filter");
}

void toBrowser::enableDisableConstraints(const QString &what)
{
    if (!SecondTab)
        return;

    try
    {
        toResultTableView *table =
            dynamic_cast<toResultTableView *>(SecondTab);
        toConnection &conn = connection();
        std::list<QString> migrate;
        if (table && table->objectName() == TAB_TABLE_CONS)
        {
            for (toResultTableView::iterator it(table); (*it).isValid(); it++)
            {
                if (table->isRowSelected(*it))
                {
                    toPush(migrate,
                           conn.quote(schema()) + ":" +
                           "TABLE:" +
                           conn.quote(SecondText) + ":" +
                           "CONSTRAINT:" +
                           conn.quote((*it).data(Qt::EditRole).toString()) + ":" +
                           "DEFINITION:" +
                           what);
                }
            }
        }
        else if (table && table->objectName() == TAB_TABLE_DEPEND)
        {
            toResultModel *model = table->model();
            for (toResultTableView::iterator it(table); (*it).isValid(); it++)
            {
                if (table->isRowSelected(*it))
                {
                    toPush(migrate,
                           conn.quote(model->data((*it).row(), 0).toString()) + ":" +
                           "TABLE:" +
                           conn.quote(model->data((*it).row(), 1).toString()) + ":" +
                           "CONSTRAINT:" +
                           conn.quote(model->data((*it).row(), 2).toString()) + ":" +
                           "DEFINITION:" +
                           what);
                }
            }
        }
        else
        {
//             toResultView *lst = dynamic_cast<toResultView *>(SecondTab);
//             if (lst && lst->sqlName() == "toBrowser:TableTrigger") {
            // Need work
//             }
        }

        if (migrate.begin() != migrate.end())
        {
            std::list<QString> drop;
            toExtract extract(conn, this);
            extract.setPrompt(false);
            extract.setHeading(false);
            QString sql = extract.migrate(drop, migrate);
            conn.execute("BEGIN\n" + sql + "\nEND;");
        }
    }
    TOCATCH;
}

void toBrowser::enableConstraints(void)
{
    enableDisableConstraints("ENABLE");
}

void toBrowser::disableConstraints(void)
{
    enableDisableConstraints("DISABLE");
}

void toBrowser::testDBLink(void)
{
    if (SecondText.isEmpty())
        return ;

    toQList resultset;
    try
    {
        resultset = toQuery::readQueryNull(toCurrentConnection(this),
                                           "SELECT * FROM dual@" + SecondText);
    }
    TOCATCH;
    //   } catch (...) {
    //     TOMessageBox::information(this, "Database link", SecondText);
    //   }
    QString status(resultset.empty() ? tr("status: FAILED") : tr("status: OK"));
    TOMessageBox::information(this, "Database link", SecondText + " " + status);
}
