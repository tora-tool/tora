//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "tobrowser.h"
#include "tobrowserfilterui.h"
#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tohelp.h"
#include "tomain.h"
#include "toresultcols.h"
#include "toresultcombo.h"
#include "toresultconstraint.h"
#include "toresultcontent.h"
#include "toresultcontent.h"
#include "toresultdepend.h"
#include "toresultextract.h"
#include "toresultfield.h"
#include "toresultindexes.h"
#include "toresultitem.h"
#include "toresultlong.h"
#include "toresultreferences.h"
#include "toresultstorage.h"
#include "toresultview.h"
#include "tosql.h"
#include "totool.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qvaluelist.h>
#include <qworkspace.h>

#include "tobrowser.moc"
#include "tobrowserconstraintui.moc"
#include "tobrowserfilterui.moc"
#include "tobrowserindexui.moc"
#include "tobrowsertableui.moc"

#include "icons/filter.xpm"
#include "icons/function.xpm"
#include "icons/index.xpm"
#include "icons/nofilter.xpm"
#include "icons/refresh.xpm"
#include "icons/schema.xpm"
#include "icons/sequence.xpm"
#include "icons/synonym.xpm"
#include "icons/table.xpm"
#include "icons/tobrowser.xpm"
#include "icons/view.xpm"
#include "icons/addtable.xpm"
#include "icons/modconstraint.xpm"
#include "icons/modindex.xpm"
#include "icons/modtable.xpm"
#define TO_DEBUGOUT(x) fprintf(stderr,(const char *)x);

#define CONF_FILTER_IGNORE_CASE "FilterIgnoreCase"
#define CONF_FILTER_INVERT  	"FilterInvert"
#define CONF_FILTER_TYPE  	"FilterType"
#define CONF_FILTER_TABLESPACE_TYPE  	"FilterTablespaceType"
#define CONF_FILTER_TEXT  	"FilterText"

class toBrowserTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return tobrowser_xpm; }
public:
  toBrowserTool()
    : toTool(20,"Schema Browser")
  { }
  virtual const char *menuItem()
  { return "Schema Browser"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toBrowser(parent,connection);
  }
  virtual bool canHandle(toConnection &conn)
  { return toIsOracle(conn)||toIsMySQL(conn)||toIsPostgreSQL(conn) || toIsSapDB(conn); }
};

static toBrowserTool BrowserTool;

static toSQL SQLListTablespaces("toBrowser:ListTablespaces",
				"SELECT Tablespace_Name FROM sys.DBA_TABLESPACES\n"
				" ORDER BY Tablespace_Name",
				"List the available tablespaces in a database.");

class toBrowserFilter : public toResultFilter {
  int Type;
  bool IgnoreCase;
  bool Invert;
  QString Text;
  int TablespaceType;
  std::list<QString> Tablespaces;
  QRegExp Match;
  bool OnlyOwnSchema;
public:
  toBrowserFilter(int type,bool cas,bool invert,
		  const QString &str,int tablespace,
		  const std::list<QString> &tablespaces,
		  bool onlyOwnSchema=false)
    : Type(type),IgnoreCase(cas),Invert(invert),Text(cas?str.upper():str),
      TablespaceType(tablespace),Tablespaces(tablespaces),OnlyOwnSchema(onlyOwnSchema)
  {
    if (!str.isEmpty()) {
      Match.setPattern(str);
      Match.setCaseSensitive(cas);
    }
    storeFilterSettings();
  }
  
  toBrowserFilter(void)
    : Type(0),IgnoreCase(true),Invert(false),TablespaceType(0)
  {
  	readFilterSettings();
  }
  
  virtual void storeFilterSettings(void) 
  {
        BrowserTool.setConfig(CONF_FILTER_IGNORE_CASE,IgnoreCase?"Yes":"No");
        BrowserTool.setConfig(CONF_FILTER_INVERT,Invert?"Yes":"No");
        BrowserTool.setConfig(CONF_FILTER_TYPE,QString("%1").arg(Type));
        BrowserTool.setConfig(CONF_FILTER_TABLESPACE_TYPE,QString("%1").arg(TablespaceType));
	BrowserTool.setConfig(CONF_FILTER_TEXT,Text);
	toTool::saveConfig();
  }

  virtual void readFilterSettings(void) 
  {
  	QString t;
        Text = BrowserTool.config(CONF_FILTER_TEXT, "");
  
        if (BrowserTool.config(CONF_FILTER_IGNORE_CASE,"No") == "Yes") 
 		IgnoreCase=true;
 	else
 		IgnoreCase=false;
		
	if (BrowserTool.config(CONF_FILTER_INVERT,"No") == "Yes") 
 		Invert=true;
 	else
 		Invert=false;
	Type=QString(BrowserTool.config(CONF_FILTER_TYPE,"0")).toInt();
	TablespaceType=QString(BrowserTool.config(CONF_FILTER_TABLESPACE_TYPE,"0")).toInt();
  }
  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix)
  {
    data[prefix+":Type"]=QString::number(Type);
    if (IgnoreCase)
      data[prefix+":Ignore"]="Yes";
    if (Invert)
      data[prefix+":Invert"]="Yes";
    data[prefix+":SpaceType"]=QString::number(TablespaceType);
    data[prefix+":Text"]=Text;
    int id=1;
    for(std::list<QString>::iterator i=Tablespaces.begin();i!=Tablespaces.end();i++,id++)
      data[prefix+":Space:"+QString::number(id).latin1()]=*i;
    if (OnlyOwnSchema)
      data[prefix+":OwnlyOwnSchema"]="Yes";
  }
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix)
  {
    Type=data[prefix+":Type"].toInt();
    OnlyOwnSchema=!data[prefix+":OnlyOwnSchema"].isEmpty();
    TablespaceType=data[prefix+":SpaceType"].toInt();
    IgnoreCase=!data[prefix+":Ignore"].isEmpty();
    Invert=!data[prefix+":Invert"].isEmpty();
    Text=data[prefix+":Text"];
    if (!Text.isEmpty()) {
      Match.setPattern(Text);
      Match.setCaseSensitive(IgnoreCase);
    }
    int id=1;
    std::map<QCString,QString>::iterator i;
    Tablespaces.clear();
    while((i=data.find(prefix+":Space:"+QString::number(id).latin1()))!=data.end()) {
      Tablespaces.insert(Tablespaces.end(),(*i).second);
      i++;
      id++;
    }
  }
  bool onlyOwnSchema(void)
  { return OnlyOwnSchema; }
  virtual QString wildCard(void)
  {
    switch(Type) {
    default:
      return QString::fromLatin1("%");
    case 1:
      return Text.upper()+QString::fromLatin1("%");
    case 2:
      return QString::fromLatin1("%")+Text.upper();
    case 3:
      return QString::fromLatin1("%")+Text.upper()+QString::fromLatin1("%");
    }
  }
  virtual bool check(const QListViewItem *item)
  {
    QString str=item->text(0);
    QString tablespace=item->text(3);
    if (!tablespace.isEmpty()) {
      switch(TablespaceType) {
      default:
	break;
      case 1:
	{
	  bool ok=false;
	  for(std::list<QString>::iterator i=Tablespaces.begin();i!=Tablespaces.end();i++) {
	    if (*i==tablespace) {
	      ok=true;
	      break;
	    }
	  }
	  if (!ok)
	    return false;
	}
	break;
      case 2:
	for(std::list<QString>::iterator i=Tablespaces.begin();i!=Tablespaces.end();i++)
	  if (*i==tablespace)
	    return false;
	break;
      }
    }
    switch(Type) {
    default:
      return true;
    case 1:
      if (IgnoreCase) {
	if (str.upper().startsWith(Text))
	  return !Invert;
      } else if (str.startsWith(Text))
	return !Invert;
      break;
    case 2:
      if (IgnoreCase) {
	if (str.right(Text.length()).upper()==Text)
	  return !Invert;
      } else if (str.right(Text.length())==Text)
	return !Invert;
      break;
    case 3:
      if (str.contains(Text,!IgnoreCase))
	return !Invert;
      break;
    case 4:
      {
	QStringList lst=QStringList::split(QRegExp(QString::fromLatin1("\\s*,\\s*")),Text);
	for(unsigned int i=0;i<lst.count();i++)
	  if (IgnoreCase) {
	    if (str.upper()==lst[i])
	      return !Invert;
	  } else if (str==lst[i])
	    return !Invert;
      }
      break;
    case 5:
      if (Match.match(str)>=0)
	return !Invert;
      break;
    }
    return Invert;
  }
  virtual toResultFilter *clone(void)
  { return new toBrowserFilter(*this); }
  friend class toBrowserFilterSetup;
};

class toBrowserFilterSetup : public toBrowserFilterUI {
public:
  void setup(bool temp)
  {
    toHelp::connectDialog(this);
    if (!temp) {
      OnlyOwnSchema->hide();
      Tablespaces->setNumberColumn(false);
      Tablespaces->setReadableColumns(true);
      try {
	toConnection &conn=toCurrentConnection(this);
	toQuery query(conn,toSQL::string(SQLListTablespaces,conn));
	Tablespaces->query(SQLListTablespaces);
      } catch(...) {
      }
      Tablespaces->setSelectionMode(QListView::Multi);
    } else {
      TablespaceType->hide();
    }
  }
  toBrowserFilterSetup(bool temp,QWidget *parent)
    : toBrowserFilterUI(parent,"Filter Setting",true)
  {
    setup(temp);
  }
  toBrowserFilterSetup(bool temp,toBrowserFilter &cur,QWidget *parent)
    : toBrowserFilterUI(parent,"Filter Setting",true)
  {
    setup(temp);
    Buttons->setButton(cur.Type);
    if (!TablespaceType->isHidden()) {
      TablespaceType->setButton(cur.TablespaceType);
      for(std::list<QString>::iterator i=cur.Tablespaces.begin();i!=cur.Tablespaces.end();i++) {
	for (QListViewItem *item=Tablespaces->firstChild();item;item=item->nextSibling())
	  if (item->text(0)==*i) {
	    item->setSelected(true);
	    break;
	  }
      }
    }
    String->setText(cur.Text);
    Invert->setChecked(cur.Invert);
    IgnoreCase->setChecked(cur.IgnoreCase);
    OnlyOwnSchema->setChecked(cur.OnlyOwnSchema);
  }
  toBrowserFilter *getSetting(void)
  {
    std::list<QString> tablespaces;
    for (QListViewItem *item=Tablespaces->firstChild();item;item=item->nextSibling())
      if (item->isSelected())
	tablespaces.insert(tablespaces.end(),item->text(0));
    return new toBrowserFilter(Buttons->id(Buttons->selected()),
			       IgnoreCase->isChecked(),
			       Invert->isChecked(),
			       String->text(),
			       TablespaceType->id(TablespaceType->selected()),
			       tablespaces,
			       OnlyOwnSchema->isChecked());
  }
};

#define FIRST_WIDTH 150

#define TAB_TABLES		"Tables"
#define TAB_TABLE_COLUMNS	"TablesColumns"
#define TAB_TABLE_CONS		"TablesConstraint"
#define TAB_TABLE_DEPEND	"TablesDepend"
#define TAB_TABLE_INDEXES	"TablesIndexes"
#define TAB_TABLE_DATA		"TablesData"
#define TAB_TABLE_GRANTS	"TablesGrants"
#define TAB_TABLE_TRIGGERS	"TablesTriggers"
#define TAB_TABLE_INFO		"TablesInfo"
#define TAB_TABLE_STATISTIC	"TablesStatistic"
#define TAB_TABLE_EXTENT	"TablesExtent"
#define TAB_TABLE_EXTRACT	"TablesExtract"

#define TAB_VIEWS		"Views"
#define TAB_VIEW_COLUMNS	"ViewColumns"
#define TAB_VIEW_SQL		"ViewSQL"
#define TAB_VIEW_DATA		"ViewData"
#define TAB_VIEW_GRANTS		"ViewGrants"
#define TAB_VIEW_DEPEND		"ViewDepend"
#define TAB_VIEW_EXTRACT	"ViewExtract"

#define TAB_SEQUENCES		"Sequences"
#define TAB_SEQUENCES_GRANTS	"SequencesGrants"
#define TAB_SEQUENCES_INFO	"SequencesInfo"
#define TAB_SEQUENCES_EXTRACT	"SequencesExtract"

#define TAB_INDEX		"Index"
#define TAB_INDEX_COLS		"IndexCols"
#define TAB_INDEX_INFO		"IndexInfo"
#define TAB_INDEX_EXTENT	"IndexesExtent"
#define TAB_INDEX_EXTRACT	"IndexExtract"
#define TAB_INDEX_STATISTIC	"IndexStatistic"

#define TAB_SYNONYM		"Synonym"
#define TAB_SYNONYM_GRANTS	"SynonymGrants"
#define TAB_SYNONYM_INFO	"SynonymInfo"
#define TAB_SYNONYM_EXTRACT	"SynonymExtract"

#define TAB_PLSQL		"PLSQL"
#define TAB_PLSQL_SOURCE	"PLSQLSource"
#define TAB_PLSQL_BODY		"PLSQLBody"
#define TAB_PLSQL_GRANTS	"PLSQLGrants"
#define TAB_PLSQL_DEPEND	"PLSQLDepend"
#define TAB_PLSQL_EXTRACT	"PLSQLExtract"

#define TAB_TRIGGER		"Trigger"
#define TAB_TRIGGER_INFO	"TriggerInfo"
#define TAB_TRIGGER_SOURCE	"TriggerSource"
#define TAB_TRIGGER_GRANTS	"TriggerGrants"
#define TAB_TRIGGER_COLS	"TriggerCols"
#define TAB_TRIGGER_DEPEND	"TriggerDepend"
#define TAB_TRIGGER_EXTRACT	"TriggerExtract"

#define TAB_DBLINK		"DBLink"
#define TAB_DBLINK_INFO		"DBLinkInfo"
#define TAB_DBLINK_SYNONYMS	"DBLinkSynonyms"

static toSQL SQLListTables("toBrowser:ListTables",
			   "SELECT Table_Name,NULL \" Ignore\",NULL \" Ignore2\",Tablespace_name \" Ignore2\"\n"
			   "  FROM SYS.ALL_ALL_TABLES WHERE OWNER = :f1<char[101]> AND IOT_Name IS NULL\n"
			   "   AND UPPER(TABLE_NAME) LIKE :f2<char[101]>\n"
			   " ORDER BY Table_Name",
			   "List the available tables in a schema.",
			   "8.0");
static toSQL SQLListTables7("toBrowser:ListTables",
			    "SELECT Table_Name,NULL \" Ignore\",NULL \" Ignore2\",Tablespace_name \" Ignore2\"\n"
			    "  FROM SYS.ALL_TABLES WHERE OWNER = :f1<char[101]>\n"
			    "   AND UPPER(TABLE_NAME) LIKE :f2<char[101]>\n"
			    " ORDER BY Table_Name",
			    "",
			    "7.3");
static toSQL SQLListTablesMysql("toBrowser:ListTables",
				"SHOW TABLES FROM :f1<noquote>",
				"",
				"3.0",
				"MySQL");
static toSQL SQLListTablesPgSQL("toBrowser:ListTables",
                                "SELECT c.relname AS \"Table Name\"\n"
                                "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
                                " WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
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
static toSQL SQLAnyGrants("toBrowser:AnyGrants",
			  "SELECT Privilege,Grantee,Grantor,Grantable FROM SYS.ALL_TAB_PRIVS\n"
			  " WHERE Table_Schema = :f1<char[101]> AND Table_Name = :f2<char[101]>\n"
			  " ORDER BY Privilege,Grantee",
			  "Display the grants on an object");
static toSQL SQLAnyGrantsSapDB("toBrowser:AnyGrants",
                            "SELECT privilege,grantee,grantor,is_grantable\n"
                            " FROM tableprivileges \n"
                            " WHERE owner = upper(:f1<char[101]>) and tablename = :f2<char[101]>\n"
                            " ORDER by privilege,grantee ",
                            "",
                            "",
                            "SapDB");
static toSQL SQLTableTrigger("toBrowser:TableTrigger",
			     "SELECT Trigger_Name,Triggering_Event,Column_Name,Status,Description \n"
			     "  FROM SYS.ALL_TRIGGERS\n"
			     " WHERE Table_Owner = :f1<char[101]> AND Table_Name = :f2<char[101]>",
			     "Display the triggers operating on a table",
			     "8.1");
static toSQL SQLTableTrigger8("toBrowser:TableTrigger",
			      "SELECT Trigger_Name,Triggering_Event,Status,Description \n"
			      "  FROM SYS.ALL_TRIGGERS\n"
			      " WHERE Table_Owner = :f1<char[101]> AND Table_Name = :f2<char[101]>",
			      "",
			      "8.0");
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
                            "",
                            "",
                            "SapDB");
static toSQL SQLTableInfo("toBrowser:TableInformation",
			  "SELECT *\n"
			  "  FROM SYS.ALL_TABLES\n"
			  " WHERE OWNER = :f1<char[101]> AND Table_Name = :f2<char[101]>",
			  "Display information about a table");
static toSQL SQLTableInfoMysql("toBrowser:TableInformation",
			       "show table status from :own<noquote> like :tab",
			       "",
			       "3.0",
			       "MySQL");
static toSQL SQLTableInfoPgSQL("toBrowser:TableInformation",
			       "SELECT c.*\n"
			       "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
			       " WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
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
static toSQL SQLListView("toBrowser:ListView",
			 "SELECT View_Name FROM SYS.ALL_VIEWS WHERE OWNER = :f1<char[101]>\n"
			 "   AND UPPER(VIEW_NAME) LIKE :f2<char[101]>\n"
			 " ORDER BY View_Name",
			 "List the available views in a schema");
static toSQL SQLListViewPgSQL("toBrowser:ListView",
                         "SELECT c.relname as View_Name\n"
                         "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
                         " WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                         "   AND c.relkind = 'v'"
                         " ORDER BY View_Name",
                         "",
                         "7.1",
                         "PostgreSQL");
static toSQL SQLListViewSapDb("toBrowser:ListView",
                            "SELECT tablename \"View_Name\"\n"
                            " FROM tables \n"
                            " WHERE tabletype = 'VIEW' and owner = upper(:f1<char[101]>)\n"
                            " ORDER by tablename",
                            "",
                            "",
                            "SapDB");
static toSQL SQLViewSQL("toBrowser:ViewSQL",	
			"SELECT Text SQL\n"
			"  FROM SYS.ALL_Views\n"
			" WHERE Owner = :f1<char[101]> AND View_Name = :f2<char[101]>",
			"Display SQL of a specified view");
static toSQL SQLViewSQLPgSQL("toBrowser:ViewSQL",	
			"SELECT pg_get_viewdef(c.relname)\n"
                        "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
                        " WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                        "   AND c.relkind = 'v' AND c.relname = :f2",
                        "",
                        "7.1",
                        "PostgreSQL");
static toSQL SQLViewSQLSapDb("toBrowser:ViewSQL",
                            "SELECT definition \"SQL\"\n"
                            " FROM viewdefs \n"
                            " WHERE  viewname = :f2<char[101]> and owner = upper(:f1<char[101]>)\n",
                            "",
                            "",
                            "SapDB");
static toSQL SQLListIndex("toBrowser:ListIndex",
			  "SELECT Index_Name,NULL \" Ignore\",NULL \" Ignore2\",Tablespace_name \" Ignore2\"\n"
			  "  FROM SYS.ALL_INDEXES\n"
			  " WHERE OWNER = :f1<char[101]>\n"
			  "   AND UPPER(INDEX_NAME) LIKE :f2<char[101]>\n"
			  " ORDER BY Index_Name\n",
			  "List the available indexes in a schema");
static toSQL SQLListIndexPgSQL("toBrowser:ListIndex",
			       "SELECT c.relname AS \"Index Name\"\n"
                               "FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
                               "WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
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
static toSQL SQLIndexCols("toBrowser:IndexCols",
			  "SELECT a.Table_Name,a.Column_Name,a.Column_Length,a.Descend,b.Column_Expression \" \"\n"
			  "  FROM sys.All_Ind_Columns a,sys.All_Ind_Expressions b\n"
			  " WHERE a.Index_Owner = :f1<char[101]> AND a.Index_Name = :f2<char[101]>\n"
			  "   AND a.Index_Owner = b.Index_Owner(+) AND a.Index_Name = b.Index_Name(+)\n"
			  "   AND a.column_Position = b.Column_Position(+)\n"
			  " ORDER BY a.Column_Position",
			  "Display columns on which an index is built",
			  "8.1");

static toSQL SQLIndexCols8("toBrowser:IndexCols",
			   "SELECT Table_Name,Column_Name,Column_Length,Descend\n"
			   "  FROM SYS.ALL_IND_COLUMNS\n"
			   " WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
			   " ORDER BY Column_Position",
			   "",
			   "8.0");
static toSQL SQLIndexCols7("toBrowser:IndexCols",
			   "SELECT Table_Name,Column_Name,Column_Length\n"
			   "  FROM SYS.ALL_IND_COLUMNS\n"
			   " WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
			   " ORDER BY Column_Position",
			   "",
			   "7.3");
// The following one for PostgreSQL needs verification
static toSQL SQLIndexColsPgSQL("toBrowser:IndexCols",
                               "SELECT a.attname,\n"
                               "       format_type(a.atttypid, a.atttypmod) as FORMAT,\n"
                               "       a.attnotnull,\n"
                               "       a.atthasdef\n"
                               "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid,\n"
                               "       pg_attribute a\n"
                               " WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                               "   AND a.attrelid = c.oid AND c.relname = :f2\n"
                               "   AND a.attnum > 0\n"
                               " ORDER BY a.attnum\n",
			       "",
			       "7.1",
                               "PostgreSQL");
static toSQL SQLIndexColsSapDb("toBrowser:IndexCols",
                            "SELECT tablename,columnno,columnname,len \"Length\",DataType,Sort \n"
                            " FROM indexcolumns \n"
                            " WHERE  owner = upper(:f1<char[101]>) and indexname = upper(:f2<char[101]>)\n"
                            " ORDER BY indexname,columnno",
                            "",
                            "",
                            "SapDB");
static toSQL SQLIndexInfo("toBrowser:IndexInformation",
			  "SELECT * FROM SYS.ALL_INDEXES\n"
			  " WHERE Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>",
			  "Display information about an index");
static toSQL SQLIndexInfoSapDb("toBrowser:IndexInformation",
                            "SELECT  INDEXNAME,TABLENAME, TYPE, CREATEDATE,CREATETIME,INDEX_USED, DISABLED \n"
                            " FROM indexes\n"
                            " WHERE  owner = upper(:f1<char[101]>) and indexname = :f2<char[101]>\n",
                            "",
                            "",
                            "SapDB");
static toSQL SQLIndexStatistic("toBrowser:IndexStatstics",
                            "SELECT description \"Description\", value(char_value,numeric_value) \"Value\" \n"
                            " FROM indexstatistics \n"
                            " WHERE owner = upper(:f1<char[101]>) and indexname = :f2<char[101]>",
                            "Index Statistics",
                            "",
                            "SapDB");
static toSQL SQLListSequence("toBrowser:ListSequence",
			     "SELECT Sequence_Name FROM SYS.ALL_SEQUENCES\n"
			     " WHERE SEQUENCE_OWNER = :f1<char[101]>\n"
			     "   AND UPPER(SEQUENCE_NAME) LIKE :f2<char[101]>\n"
			     " ORDER BY Sequence_Name",
			     "List the available sequences in a schema");
static toSQL SQLSequenceInfo("toBrowser:SequenceInformation",
			     "SELECT * FROM SYS.ALL_SEQUENCES\n"
			     " WHERE Sequence_Owner = :f1<char[101]>\n"
			     "   AND Sequence_Name = :f2<char[101]>",
			     "Display information about a sequence");

static toSQL SQLListSequencePgSQL("toBrowser:ListSequence",
                                  "SELECT c.relname AS \"Sequence Name\"\n"
                                  "  FROM pg_class c LEFT OUTER JOIN pg_user u ON c.relowner=u.usesysid\n"
                                  " WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                                  "   AND c.relkind = 'S'\n"
                                  " ORDER BY \"Sequence Name\"",
			          "",
			          "7.1",
                                  "PostgreSQL");
static toSQL SQLSequenceInfoPgSQL("toBrowser:SequenceInformation",
                                  "SELECT *, substr(:f1,1) as \"Owner\" FROM :f2<noquote>",
			          "",
			          "7.1",
                                  "PostgreSQL");
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
static toSQL SQLListSQL("toBrowser:ListCode",
			"SELECT Object_Name,Object_Type,Status Type FROM SYS.ALL_OBJECTS\n"
			" WHERE OWNER = :f1<char[101]>\n"
			"   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
			"                       'PROCEDURE','TYPE')\n"
			"   AND UPPER(OBJECT_NAME) LIKE :f2<char[101]>\n"
			" ORDER BY Object_Name",
			"List the available Code objects in a schema");
static toSQL SQLListSQLPgSQL("toBrowser:ListCode",
			     "SELECT p.proname AS Object_Name,\n"
                             "  CASE WHEN p.prorettype = 0 THEN 'PROCEDURE'\n"
                             "       ELSE 'FUNCTION'\n"
                             "   END AS Object_Type\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_user u ON p.proowner=u.usesysid\n"
                             "WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                             "ORDER BY Object_Name",
			     "",
			     "7.1",
                             "PostgreSQL");
static toSQL SQLListSQLShort("toBrowser:ListCodeShort",
			     "SELECT Object_Name Type FROM SYS.ALL_OBJECTS\n"
		 	     " WHERE OWNER = :f1<char[101]>\n"
			     "   AND Object_Type IN ('FUNCTION','PACKAGE',\n"
			     "                       'PROCEDURE','TYPE')\n"
			     "   AND UPPER(OBJECT_NAME) LIKE :f2<char[101]>\n"
			     " ORDER BY Object_Name",
			     "List the available Code objects in a schema, one column version");
static toSQL SQLListSQLShortPgSQL("toBrowser:ListCodeShort",
			     "SELECT p.proname AS Object_Name\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_user u ON p.proowner=u.usesysid\n"
                             "WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                             "ORDER BY Object_Name",
			     "",
			     "7.1",
                             "PostgreSQL");


static toSQL SQLSQLTemplate("toBrowser:CodeTemplate",
			    "SELECT Text FROM SYS.ALL_SOURCE\n"
			    " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
			    "   AND Type IN ('PACKAGE','PROCEDURE','FUNCTION','PACKAGE','TYPE')",
			    "Declaration of object displayed in template window");
static toSQL SQLSQLHead("toBrowser:CodeHead",
			"SELECT Text FROM SYS.ALL_SOURCE\n"
			" WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
			"   AND Type IN ('PACKAGE','TYPE')",
			"Declaration of object");
// PostgreSQL does not distinguish between Head and Body for Stored SQL
// package code will be returnd for both Head and Body
static toSQL SQLSQLHeadPgSQL("toBrowser:CodeHead",
			     "SELECT p.prosrc\n"
                             "FROM pg_proc p LEFT OUTER JOIN pg_user u ON p.proowner=u.usesysid\n"
                             "WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                             "  AND p.proname = :f2\n",
			     "",
			     "7.1",
                             "PostgreSQL");

static toSQL SQLSQLBody("toBrowser:CodeBody",
			"SELECT Text FROM SYS.ALL_SOURCE\n"
			" WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]>\n"
			"   AND Type IN ('PROCEDURE','FUNCTION','PACKAGE BODY','TYPE BODY')",
			"Implementation of object");

static toSQL SQLSQLBodyPgSQL("toBrowser:CodeBody",
			"SELECT p.prosrc\n"
                        "FROM pg_proc p LEFT OUTER JOIN pg_user u ON p.proowner=u.usesysid\n"
                        "WHERE (u.usename = :f1 OR u.usesysid IS NULL)\n"
                        "  AND p.proname = :f2\n",
			"",
			"7.1",
                        "PostgreSQL");

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
			    "8.1");
static toSQL SQLTriggerInfo8("toBrowser:TriggerInfo",
			     "SELECT Owner,Trigger_Name,\n"
			     "       Trigger_Type,Triggering_Event,\n"
			     "       Table_Owner,Table_Name,\n"
			     "       Referencing_Names,When_Clause,Status,\n"
			     "       Description\n"
			     "  FROM SYS.ALL_TRIGGERS\n"
			     "WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
			     "",
			     "8.0");
static toSQL SQLTriggerBody("toBrowser:TriggerBody",
			    "SELECT Trigger_Body FROM SYS.ALL_TRIGGERS\n"
			    " WHERE Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
			    "Implementation of trigger");
static toSQL SQLTriggerCols("toBrowser:TriggerCols",
			    "SELECT Column_Name,Column_List \"In Update\",Column_Usage Usage\n"
			    "  FROM SYS.ALL_TRIGGER_COLS\n"
			    " WHERE Trigger_Owner = :f1<char[101]> AND Trigger_Name = :f2<char[101]>",
			    "Columns used by trigger");

QString toBrowser::schema(void)
{
  try {
    QString ret=Schema->selected();
    if (ret==tr("No schemas"))
      return connection().database();
    return ret;
  } catch(...) {
    return QString::null;
  }
}

void toBrowser::setNewFilter(toBrowserFilter *filter)
{
  if (Filter) {
    delete Filter;
    Filter=NULL;
  }
  if (filter)
    Filter=filter;
  for(std::map<QCString,toResultView *>::iterator i=Map.begin();i!=Map.end();i++)
    (*i).second->setFilter(filter?filter->clone():NULL);
  refresh();
}

toBrowser::toBrowser(QWidget *parent,toConnection &connection)
  : toToolWidget(BrowserTool,"browser.html",parent,connection)
{
  Filter=new toBrowserFilter();

  QToolBar *toolbar=toAllocBar(this,tr("DB Browser"));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Update from DB"),
		  tr("Update from DB"),
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)filter_xpm),
		  tr("Define the object filter"),
		  tr("Define the object filter"),
		  this,SLOT(defineFilter(void)),
		  toolbar);
  new QToolButton(QPixmap((const char **)nofilter_xpm),
		  tr("Remove any object filter"),
		  tr("Remove any object filter"),
		  this,SLOT(clearFilter(void)),
		  toolbar);
  Schema=new toResultCombo(toolbar,TO_KDE_TOOLBAR_WIDGET);
  connect(Schema,SIGNAL(activated(int)),
	  this,SLOT(changeSchema(int)));
  Schema->setSQL(toSQL::sql(toSQL::TOSQL_USERLIST));
  if (toIsMySQL(connection))
    Schema->setSelected(connection.database());
  else if (toIsOracle(connection)||toIsSapDB(connection))
    Schema->setSelected(connection.user().upper());
  else
    Schema->setSelected(connection.user());
  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));
  new toChangeConnection(toolbar,TO_KDE_TOOLBAR_WIDGET);
  
  TopTab=new QTabWidget(this);
  QSplitter *splitter=new QSplitter(Horizontal,TopTab,TAB_TABLES);
  TopTab->addTab(splitter,tr("T&ables"));
  CurrentTop=splitter;
  toResultView *resultView=new toResultLong(true,false,toQuery::Background,splitter);
  resultView->setReadAll(true);
  resultView->setSQL(SQLListTables);
  resultView->resize(FIRST_WIDTH,resultView->height());
  resultView->setResizeMode(QListView::AllColumns);
  setFocusProxy(resultView);
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  connect(resultView,SIGNAL(done()),this,SLOT(firstDone()));
  FirstTab=resultView;
  Map[TAB_TABLES]=resultView;
  resultView->setTabWidget(TopTab);
  resultView->setSelectionMode(QListView::Single);
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));

  QVBox *box=new QVBox(splitter);
  splitter->setResizeMode(box,QSplitter::Stretch);

  toolbar=toAllocBar(box,tr("Table browser"));
  new QToolButton(QPixmap((const char **)addtable_xpm),
		  tr("Create new table"),
		  tr("Create new table"),
		  this,SLOT(addTable()),
		  toolbar);
  toolbar->addSeparator();
  new QToolButton(QPixmap((const char **)modtable_xpm),
		  tr("Modify table columns"),
		  tr("Modify table columns"),
		  this,SLOT(modifyTable()),
		  toolbar);
  new QToolButton(QPixmap((const char **)modconstraint_xpm),
		  tr("Modify constraints"),
		  tr("Modify constraints"),
		  this,SLOT(modifyTable()),
		  toolbar);
  new QToolButton(QPixmap((const char **)modindex_xpm),
		  tr("Modify indexes"),
		  tr("Modify indexes"),
		  this,SLOT(modifyTable()),
		  toolbar);
  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));

  QTabWidget *curr=new QTabWidget(box);
  toResultCols *resultCols=new toResultCols(curr,TAB_TABLE_COLUMNS);
  curr->addTab(resultCols,tr("&Columns"));
  SecondTab=resultCols;
  SecondMap[TAB_TABLES]=resultCols;
  SecondMap[TAB_TABLE_COLUMNS]=resultCols;

  resultView=new toResultIndexes(curr,TAB_TABLE_INDEXES);
  curr->addTab(resultView,tr("&Indexes"));
  SecondMap[TAB_TABLE_INDEXES]=resultView;

  toResultConstraint *resultConstraint=new toResultConstraint(curr,TAB_TABLE_CONS);
  curr->addTab(resultConstraint,tr("C&onstraints"));
  SecondMap[TAB_TABLE_CONS]=resultConstraint;

  toResultReferences *resultReferences=new toResultReferences(curr,TAB_TABLE_DEPEND);
  curr->addTab(resultReferences,tr("&References"));
  SecondMap[TAB_TABLE_DEPEND]=resultReferences;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_TABLE_GRANTS);
  resultView->setSQL(SQLAnyGrants);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("&Grants"));
  SecondMap[TAB_TABLE_GRANTS]=resultView;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_TABLE_TRIGGERS);
  resultView->setSQL(SQLTableTrigger);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("Triggers"));
  SecondMap[TAB_TABLE_TRIGGERS]=resultView;

  TableContent=new toResultContent(curr,TAB_TABLE_DATA);
  curr->addTab(TableContent,tr("&Data"));
  SecondMap[TAB_TABLE_DATA]=TableContent;

  toResultItem *resultInfo=new toResultItem(2,true,curr,TAB_TABLE_INFO);
  resultInfo->setSQL(SQLTableInfo);
  curr->addTab(resultInfo,tr("Information"));
  SecondMap[TAB_TABLE_INFO]=resultInfo;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_TABLE_STATISTIC);
  resultView->setSQL(SQLTableStatistic);
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("Statistic"));
  SecondMap[TAB_TABLE_STATISTIC]=resultView;

  toResultExtent *resultExtent=new toResultExtent(curr,TAB_TABLE_EXTENT);
  curr->addTab(resultExtent,tr("Extents"));
  SecondMap[TAB_TABLE_EXTENT]=resultExtent;
  toResultExtract *resultExtract=new toResultExtract(true,this,TAB_TABLE_EXTRACT);
  curr->addTab(resultExtract,tr("Script"));
  SecondMap[TAB_TABLE_EXTRACT]=resultExtract;

  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));

  splitter=new QSplitter(Horizontal,TopTab,TAB_VIEWS);
  TopTab->addTab(splitter,tr("&Views"));
  resultView=new toResultLong(true,false,toQuery::Background,splitter);
  resultView->setReadAll(true);
  connect(resultView,SIGNAL(done()),this,SLOT(firstDone()));
  Map[TAB_VIEWS]=resultView;
  resultView->setTabWidget(TopTab);
  resultView->setSQL(SQLListView);
  resultView->resize(FIRST_WIDTH,resultView->height());
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setSelectionMode(QListView::Single);
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);
  resultCols=new toResultCols(curr,TAB_VIEW_COLUMNS);
  curr->addTab(resultCols,tr("&Columns"));
  SecondMap[TAB_VIEWS]=resultCols;
  SecondMap[TAB_VIEW_COLUMNS]=resultCols;

  toResultField *resultField=new toResultField(curr,TAB_VIEW_SQL);
  resultField->setSQL(SQLViewSQL);
  curr->addTab(resultField,tr("SQL"));
  SecondMap[TAB_VIEW_SQL]=resultField;

  ViewContent=new toResultContent(curr,TAB_VIEW_DATA);
  ViewContent->useNoReturning(true);
  curr->addTab(ViewContent,tr("&Data"));
  SecondMap[TAB_VIEW_DATA]=ViewContent;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_VIEW_GRANTS);
  resultView->setSQL(SQLAnyGrants);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("&Grants"));
  SecondMap[TAB_VIEW_GRANTS]=resultView;

  toResultDepend *resultDepend=new toResultDepend(curr,TAB_VIEW_DEPEND);
  curr->addTab(resultDepend,tr("De&pendencies"));
  SecondMap[TAB_VIEW_DEPEND]=resultDepend;

  resultExtract=new toResultExtract(true,this,TAB_VIEW_EXTRACT);
  curr->addTab(resultExtract,tr("Script"));
  SecondMap[TAB_VIEW_EXTRACT]=resultExtract;
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));

  splitter=new QSplitter(Horizontal,TopTab,TAB_INDEX);
  TopTab->addTab(splitter,tr("Inde&xes"));
  resultView=new toResultLong(true,false,toQuery::Background,splitter);
  resultView->setReadAll(true);
  connect(resultView,SIGNAL(done()),this,SLOT(firstDone()));
  Map[TAB_INDEX]=resultView;
  resultView->setTabWidget(TopTab);
  resultView->setSQL(SQLListIndex);
  resultView->resize(FIRST_WIDTH,resultView->height());
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setSelectionMode(QListView::Single);
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_INDEX_COLS);
  resultView->setSQL(SQLIndexCols);
  connect(resultView,SIGNAL(done()),this,SLOT(fixIndexCols()));
  curr->addTab(resultView,tr("&Columns"));
  SecondMap[TAB_INDEX]=resultView;
  SecondMap[TAB_INDEX_COLS]=resultView;

  toResultItem *resultIdxInfo=new toResultItem(2,true,curr,TAB_INDEX_INFO);
  resultIdxInfo->setSQL(SQLIndexInfo);
  curr->addTab(resultIdxInfo,tr("Info"));
  SecondMap[TAB_INDEX_INFO]=resultIdxInfo;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_INDEX_STATISTIC);
  resultView->setSQL(SQLIndexStatistic);
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("&Statistic"));
  SecondMap[TAB_INDEX]=resultView;
  SecondMap[TAB_INDEX_COLS]=resultView;


  resultExtent=new toResultExtent(curr,TAB_INDEX_EXTENT);
  curr->addTab(resultExtent,tr("Extents"));
  SecondMap[TAB_INDEX_EXTENT]=resultExtent;

  resultExtract=new toResultExtract(true,this,TAB_INDEX_EXTRACT);
  curr->addTab(resultExtract,tr("Script"));
  SecondMap[TAB_INDEX_EXTRACT]=resultExtract;
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));

  splitter=new QSplitter(Horizontal,TopTab,TAB_SEQUENCES);
  TopTab->addTab(splitter,tr("Se&quences"));
  resultView=new toResultLong(true,false,toQuery::Background,splitter);
  resultView->setReadAll(true);
  resultView->setSQL(SQLListSequence);
  connect(resultView,SIGNAL(done()),this,SLOT(firstDone()));
  Map[TAB_SEQUENCES]=resultView;
  resultView->setTabWidget(TopTab);
  resultView->setSQL(SQLListSequence);
  resultView->resize(FIRST_WIDTH,resultView->height());
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setSelectionMode(QListView::Single);
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);

  toResultItem *resultSequences=new toResultItem(2,true,curr,TAB_SEQUENCES_INFO);
  resultSequences->setSQL(SQLSequenceInfo);
  curr->addTab(resultSequences,tr("Info"));
  SecondMap[TAB_SEQUENCES]=resultSequences;
  SecondMap[TAB_SEQUENCES_INFO]=resultSequences;
  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_SEQUENCES_GRANTS);
  resultView->setSQL(SQLAnyGrants);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("&Grants"));
  SecondMap[TAB_SEQUENCES_GRANTS]=resultView;

  resultExtract=new toResultExtract(true,this,TAB_SEQUENCES_EXTRACT);
  curr->addTab(resultExtract,tr("Script"));
  SecondMap[TAB_SEQUENCES_EXTRACT]=resultExtract;

  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));

  splitter=new QSplitter(Horizontal,TopTab,TAB_SYNONYM);
  TopTab->addTab(splitter,tr("S&ynonyms"));
  resultView=new toResultLong(true,false,toQuery::Background,splitter);
  resultView->setReadAll(true);
  connect(resultView,SIGNAL(done()),this,SLOT(firstDone()));
  Map[TAB_SYNONYM]=resultView;
  resultView->setTabWidget(TopTab);
  resultView->setSQL(SQLListSynonym);
  resultView->resize(FIRST_WIDTH,resultView->height());
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setSelectionMode(QListView::Single);
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);

  toResultItem *resultSynonym=new toResultItem(2,true,curr,TAB_SYNONYM_INFO);
  resultSynonym->setSQL(SQLSynonymInfo);
  curr->addTab(resultSynonym,tr("Info"));
  SecondMap[TAB_SYNONYM]=resultSynonym;
  SecondMap[TAB_SYNONYM_INFO]=resultSynonym;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_SYNONYM_GRANTS);
  resultView->setSQL(SQLAnyGrants);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("&Grants"));
  SecondMap[TAB_SYNONYM_GRANTS]=resultView;

  resultExtract=new toResultExtract(true,this,TAB_SYNONYM_EXTRACT);
  curr->addTab(resultExtract,tr("Script"));
  SecondMap[TAB_SYNONYM_EXTRACT]=resultExtract;

  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));

  splitter=new QSplitter(Horizontal,TopTab,TAB_PLSQL);
  TopTab->addTab(splitter,tr("Cod&e"));
  resultView=new toResultLong(true,false,toQuery::Background,splitter);
  resultView->setReadAll(true);
  connect(resultView,SIGNAL(done()),this,SLOT(firstDone()));
  Map[TAB_PLSQL]=resultView;
  resultView->setTabWidget(TopTab);
  resultView->setSQL(SQLListSQL);
  resultView->resize(FIRST_WIDTH*2,resultView->height());
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setSelectionMode(QListView::Single);
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);

  resultField=new toResultField(curr,TAB_PLSQL_SOURCE);
  resultField->setSQL(SQLSQLHead);
  curr->addTab(resultField,tr("&Declaration"));
  SecondMap[TAB_PLSQL]=resultField;
  SecondMap[TAB_PLSQL_SOURCE]=resultField;

  resultField=new toResultField(curr,TAB_PLSQL_BODY);
  resultField->setSQL(SQLSQLBody);
  curr->addTab(resultField,tr("B&ody"));
  SecondMap[TAB_PLSQL_BODY]=resultField;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_PLSQL_GRANTS);
  resultView->setReadAll(true);
  resultView->setSQL(SQLAnyGrants);
  curr->addTab(resultView,tr("&Grants"));
  SecondMap[TAB_PLSQL_GRANTS]=resultView;
  resultDepend=new toResultDepend(curr,TAB_PLSQL_DEPEND);
  curr->addTab(resultDepend,tr("De&pendencies"));
  SecondMap[TAB_PLSQL_DEPEND]=resultDepend;

  resultExtract=new toResultExtract(true,this,TAB_PLSQL_EXTRACT);
  curr->addTab(resultExtract,tr("Script"));
  SecondMap[TAB_PLSQL_EXTRACT]=resultExtract;
  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));

  splitter=new QSplitter(Horizontal,TopTab,TAB_TRIGGER);
  TopTab->addTab(splitter,tr("Tri&ggers"));
  resultView=new toResultLong(true,false,toQuery::Background,splitter);
  resultView->setReadAll(true);
  connect(resultView,SIGNAL(done()),this,SLOT(firstDone()));
  Map[TAB_TRIGGER]=resultView;
  resultView->setTabWidget(TopTab);
  resultView->setSQL(SQLListTrigger);
  resultView->resize(FIRST_WIDTH,resultView->height());
  resultView->setResizeMode(QListView::AllColumns);
  resultView->setSelectionMode(QListView::Single);
  connect(resultView,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));
  splitter->setResizeMode(resultView,QSplitter::KeepSize);
  curr=new QTabWidget(splitter);
  splitter->setResizeMode(curr,QSplitter::Stretch);

  toResultItem *resultTrigger=new toResultItem(2,true,curr,TAB_TRIGGER_INFO);
  resultTrigger->setSQL(SQLTriggerInfo);
  curr->addTab(resultTrigger,tr("Info"));
  SecondMap[TAB_TRIGGER]=resultTrigger;
  SecondMap[TAB_TRIGGER_INFO]=resultTrigger; 

  resultField=new toResultField(curr,TAB_TRIGGER_SOURCE);
  resultField->setSQL(SQLTriggerBody);
  curr->addTab(resultField,tr("C&ode"));
  SecondMap[TAB_TRIGGER_SOURCE]=resultField;

  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_TRIGGER_COLS);
  resultView->setSQL(SQLTriggerCols);
  curr->addTab(resultView,tr("&Columns"));
  SecondMap[TAB_TRIGGER_COLS]=resultView;


  resultView=new toResultLong(true,false,toQuery::Background,curr,TAB_TRIGGER_GRANTS);
  resultView->setSQL(SQLAnyGrants);
  resultView->setReadAll(true);
  curr->addTab(resultView,tr("&Grants"));
  SecondMap[TAB_TRIGGER_GRANTS]=resultView;

  resultDepend=new toResultDepend(curr,TAB_TRIGGER_DEPEND);
  curr->addTab(resultDepend,tr("De&pendencies"));
  SecondMap[TAB_TRIGGER_DEPEND]=resultDepend;

  resultExtract=new toResultExtract(true,this,TAB_TRIGGER_EXTRACT);
  curr->addTab(resultExtract,tr("Script"));
  SecondMap[TAB_TRIGGER_EXTRACT]=resultExtract;

  connect(curr,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeSecondTab(QWidget *)));

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  refresh();

  connect(TopTab,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeTab(QWidget *)));
  connect(this,SIGNAL(connectionChange()),this,SLOT(refresh()));
  Schema->setFocus();
  connect(&Poll,SIGNAL(timeout()),this,SLOT(changeSecond()));
}

void toBrowser::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),this,SLOT(refresh(void)),
			   Key_F5);
      ToolMenu->insertItem(tr("&Change Schema"),Schema,SLOT(setFocus(void)),
			   Key_S+ALT);
      ToolMenu->insertItem(tr("Change &Object"),this,SLOT(focusObject(void)),
			   Key_N+ALT);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)filter_xpm),tr("&Define filter..."),
			   this,SLOT(defineFilter(void)),CTRL+SHIFT+Key_G);
      ToolMenu->insertItem(QPixmap((const char **)nofilter_xpm),tr("&Clear filter"),this,SLOT(clearFilter(void)),
			   CTRL+SHIFT+Key_H);
      toMainWidget()->menuBar()->insertItem(tr("&Browser"),ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toBrowser::changeSchema(int)
{
  SecondText=QString::fromLatin1("");
  refresh();
}

toBrowser::~toBrowser()
{
  delete Filter;
}

void toBrowser::refresh(void)
{
  try {
    Schema->refresh();
    if (FirstTab) {
      toQList pars=FirstTab->params();
      if (pars.begin()==pars.end()) {
	updateTabs();
	return;
      }
      FirstTab->refresh();
    }
    if (SecondTab) {
      toQList pars=SecondTab->params();
      if (pars.begin()==pars.end()) {
	updateTabs();
	return;
      }
      SecondTab->refresh();
    }
  } TOCATCH
}

void toBrowser::focusObject(void)
{
  if (FirstTab)
    FirstTab->setFocus();
}

void toBrowser::updateTabs(void)
{
  try {
    if (!Schema->selected().isEmpty()&&FirstTab)
      FirstTab->changeParams(schema(),Filter?Filter->wildCard():QString::fromLatin1("%"));
    firstDone(); // In case it is ignored cause it is already done.
    if (SecondTab&&!SecondText.isEmpty())
      changeSecond();
  } TOCATCH
}

void toBrowser::firstDone(void)
{
  if (!SecondText.isEmpty()&&FirstTab) {
    for (QListViewItem *item=FirstTab->firstChild();item;item=item->nextSibling()) {
      if (item->text(0)==SecondText) {
	FirstTab->setSelected(item,true);
	FirstTab->setCurrentItem(item);
	FirstTab->ensureItemVisible(item);
	break;
      }
    }
  } else if (FirstTab->selectedItem())
    SecondText=FirstTab->selectedItem()->text(0);
}

void toBrowser::changeItem(QListViewItem *item)
{
  if (item) {
    SecondText=item->text(0);
    if (SecondTab&&!SecondText.isEmpty())
      Poll.start(250,true);
  }
}

void toBrowser::changeSecond(void)
{
  QWidget *tab=TopTab->currentPage();
  if (tab&&!strcmp(tab->name(),TAB_SYNONYM)) {
    QString owner;
    QString name;
    int pos=SecondText.find(QString::fromLatin1("."));
    if (pos>=0) {
      owner=SecondText.mid(0,pos);
      name=SecondText.mid(pos+1);
    } else {
      owner=QString::fromLatin1("PUBLIC");
      name=SecondText;
    }
    SecondTab->changeParams(owner,name);
  } else
    SecondTab->changeParams(schema(),
			    SecondText);
}

void toBrowser::changeSecondTab(QWidget *tab)
{
  for (QWidget *t=tab->parentWidget();t!=TopTab->currentPage();t=t->parentWidget())
    if (!t)
      return;

  if (tab) {
    toResult *newtab=SecondMap[tab->name()];
    if (newtab==SecondTab)
      return;

    // The change second tab can get called for other tabs than the current one. Ignore those
    // calls.
    QWidget *t=dynamic_cast<QWidget *>(newtab);
    while(t&&t!=CurrentTop)
      t=t->parentWidget();
    if (!t)
      return;

    SecondTab=newtab;
    SecondMap[TopTab->currentPage()->name()]=SecondTab;
    if (SecondTab&&!SecondText.isEmpty())
      changeSecond();
  }
}

void toBrowser::changeTab(QWidget *tab)
{
  if (tab&&this==toMainWidget()->workspace()->activeWindow()) {
    toResultView *newtab=Map[tab->name()];
    if (newtab==FirstTab)
      return;
    CurrentTop=tab;
    setFocusProxy(newtab);
    FirstTab=newtab;
    SecondTab=SecondMap[tab->name()];
    SecondText="";
    
    if (FirstTab&&SecondTab)
      updateTabs();
  }
}

void toBrowser::clearFilter(void)
{
  setNewFilter(NULL);
}

void toBrowser::defineFilter(void)
{
  if (Filter) {
    toBrowserFilterSetup filt(false,*Filter,this);
    if (filt.exec())
      setNewFilter(filt.getSetting());
  } else {
    toBrowserFilterSetup filt(false,this);
    if (filt.exec())
      setNewFilter(filt.getSetting());
  }
}

bool toBrowser::canHandle(toConnection &conn)
{
  return toIsOracle(conn)||toIsMySQL(conn)||toIsSapDB(conn);
}

void toBrowser::modifyTable(void)
{

}

void toBrowser::addTable(void)
{

}

void toBrowser::exportData(std::map<QCString,QString> &data,const QCString &prefix)
{
  data[prefix+":Schema"]=Schema->selected();
  data[prefix+":FirstTab"]=TopTab->currentPage()->name();
  data[prefix+":SecondText"]=SecondText;
  for(std::map<QCString,toResult *>::iterator i=SecondMap.begin();i!=SecondMap.end();i++) {
    if ((*i).second==SecondTab&&Map.find((*i).first)==Map.end()) {
      data[prefix+":SecondTab"]=(*i).first;
      break;
    }
  }
  ViewContent->exportData(data,prefix+":View");
  TableContent->exportData(data,prefix+":Table");
  
  toToolWidget::exportData(data,prefix);
  if (Filter)
    Filter->exportData(data,prefix+":Filter");

}

void toBrowser::importData(std::map<QCString,QString> &data,const QCString &prefix)
{
  disconnect(Schema,SIGNAL(activated(int)),
	     this,SLOT(changeSchema(int)));
  disconnect(TopTab,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeTab(QWidget *)));

  ViewContent->importData(data,prefix+":View");
  TableContent->importData(data,prefix+":Table");

  if (data.find(prefix+":Filter:Type")!=data.end()) {
    toBrowserFilter *filter=new toBrowserFilter;
    filter->importData(data,prefix+":Filter");
    setNewFilter(filter);
  } else
    setNewFilter(NULL);

  toToolWidget::importData(data,prefix);
  QString str=data[prefix+":Schema"];
  Schema->setSelected(str);
  for(int i=0;i<Schema->count();i++)
    if (Schema->text(i)==str)
      Schema->setCurrentItem(i);

  str=data[prefix+":FirstTab"];
  QWidget *chld=(QWidget *)child(str);
  if(chld&&str.length()) {
    SecondText=QString::null;
    TopTab->showPage(chld);
    str=data[prefix+":SecondTab"];
    chld=(QWidget *)child(str);
    if (chld&&str.length()) {
      QWidget *par=chld->parentWidget();
      while(par&&!par->isA("QTabWidget"))
	par=par->parentWidget();
      if (par)
	((QTabWidget *)par)->showPage(chld);
    }
    SecondText=data[prefix+":SecondText"];
  }

  connect(Schema,SIGNAL(activated(int)),
	  this,SLOT(changeSchema(int)));
  connect(TopTab,SIGNAL(currentChanged(QWidget *)),this,SLOT(changeTab(QWidget *)));
  refresh();
}

void toBrowser::fixIndexCols(void)
{
  toResultLong *tmp=dynamic_cast<toResultLong *>(SecondMap[TAB_INDEX_COLS]);
  if (tmp)
    for(QListViewItem *item=tmp->firstChild();item;item=item->nextSibling()) {
      if (!toUnnull(item->text(4)).isNull()) {
	toResultViewItem *resItem=dynamic_cast<toResultViewItem *>(item);
	if (resItem)
	  resItem->setText(1,item->text(4));
      }
    }
}

static toBrowseTemplate BrowseTemplate;

void toBrowseTemplate::removeDatabase(const QString &name)
{
  for(std::list<toTemplateItem *>::iterator i=Parents.begin();i!=Parents.end();i++) {
    for (QListViewItem *item=(*i)->firstChild();item;item=item->nextSibling())
      if (item->text(0)==name) {
	delete item;
	break;
      }
  }
}

void toBrowseTemplate::defineFilter(void)
{
  if (Filter) {
    toBrowserFilterSetup filt(true,*Filter,toMainWidget());
    if (filt.exec()) {
      delete Filter;
      Filter=filt.getSetting();
    }
  } else {
    toBrowserFilterSetup filt(true,toMainWidget());
    if (filt.exec())
      Filter=filt.getSetting();
  }
}

void toBrowseTemplate::clearFilter(void)
{
  delete Filter;
  Filter=NULL;
}

void toBrowseTemplate::removeItem(QListViewItem *item)
{
  for(std::list<toTemplateItem *>::iterator i=Parents.begin();i!=Parents.end();i++)
    if ((*i)==item) {
      Parents.erase(i);
      break;
    }
}

class toTemplateTableItem : public toTemplateItem {
  toConnection &Connection;
public:
  toTemplateTableItem(toConnection &conn,toTemplateItem *parent,
		       const QString &name)
    : toTemplateItem(parent,name),Connection(conn)
  {
  }
  virtual QWidget *selectedWidget(QWidget *par)
  {
    QString ptyp=parent()->parent()->text(0);
    QString object=parent()->text(0);
    QString typ=text(0);
    QString schema=parent()->parent()->parent()->text(0);
    if (ptyp=="Synonyms") {
      int pos=object.find(QString::fromLatin1("."));
      if (pos>=0) {
	schema=object.mid(0,pos);
	object=object.mid(pos+1);
      } else {
	schema=QString::fromLatin1("PUBLIC");
      }
    }

    if (schema==qApp->translate("toBrowser","No schemas"))
      schema=Connection.database();

    toResultView *res;

    toToolWidget *tool=new toToolWidget(BrowserTool,
					"",
					par,
					Connection);
    if (typ==qApp->translate("toBrowser","Data")) {
      toResultContent *cnt=new toResultContent(tool);
      cnt->changeParams(schema,object);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Indexes")) {
      res=new toResultIndexes(tool);
    } else if (typ==qApp->translate("toBrowser","Extents")) {
      new toResultExtent(tool);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Constraints")) {
      res=new toResultConstraint(tool);
    } else if (typ==qApp->translate("toBrowser","Triggers")) {
      res=new toResultLong(true,false,toQuery::Background,tool);
      res->setReadAll(true);
      res->setSQL(SQLTableTrigger);
    } else if (typ==qApp->translate("toBrowser","SQL")) {
      toResultField *sql=new toResultField(tool);
      sql->setSQL(SQLViewSQL);
      sql->changeParams(schema,object);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Script")) {
      toResultExtract *ext=new toResultExtract(true,tool);
      ext->changeParams(schema,object);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Information")) {
      toResultItem *inf=new toResultItem(2,true,tool);
      if (ptyp==qApp->translate("toBrowser","Tables")) {
	inf->setSQL(SQLTableInfo);
      } else if (ptyp==qApp->translate("toBrowser","Triggers")) {
	inf->setSQL(SQLTriggerInfo);
      } else if (ptyp==qApp->translate("toBrowser","Indexes")) {
	inf->setSQL(SQLIndexInfo);
      }
      inf->changeParams(schema,object);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Columns")) {
      res=new toResultLong(true,false,toQuery::Background,tool);
      res->setSQL(SQLTriggerCols);
    } else if (typ==qApp->translate("toBrowser","References")) {
      res=new toResultReferences(tool);
    } else if (typ==qApp->translate("toBrowser","Grants")) {
      res=new toResultLong(true,false,toQuery::Background,tool);
      res->setSQL(SQLAnyGrants);
    } else if (typ==qApp->translate("toBrowser","Dependencies")) {
      res=new toResultDepend(tool);
    } else {
      delete tool;
      return NULL;
    }
    res->changeParams(schema,object);
    return tool;
  }
};

class toTemplateSchemaItem : public toTemplateItem {
  toConnection &Connection;
public:
  toTemplateSchemaItem(toConnection &conn,toTemplateItem *parent,
		       const QString &name)
    : toTemplateItem(parent,name),Connection(conn)
  {
    QString typ=parent->text(0);
    if (typ==qApp->translate("toBrowser","Tables")) {
      QPixmap image((const char **)table_xpm);
      setPixmap(0,image);
      new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Indexes"));
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Constraints"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","References"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Grants"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Triggers"));
      }
      new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Data"));
      new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Information"));
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Extents"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Script"));
      }
    } else if (typ==qApp->translate("toBrowser","Views")) {
      QPixmap image((const char **)view_xpm);
      setPixmap(0,image);
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","SQL"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Grants"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Data"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Dependencies"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Script"));
      }
    } else if (typ==qApp->translate("toBrowser","Sequences")) {
      QPixmap image((const char **)sequence_xpm);
      setPixmap(0,image);
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Grants"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Script"));
      }
    } else if (typ==qApp->translate("toBrowser","Code")) {
      QPixmap image((const char **)function_xpm);
      setPixmap(0,image);
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Grants"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Dependencies"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Script"));
      }
    } else if (typ==qApp->translate("toBrowser","Triggers")) {
      QPixmap image((const char **)function_xpm);
      setPixmap(0,image);
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Information"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Columns"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Grants"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Dependencies"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Script"));
      }
    } else if (typ==qApp->translate("toBrowser","Indexes")) {
      QPixmap image((const char **)index_xpm);
      setPixmap(0,image);
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Information"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Extents"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Script"));
      }
    } else if (typ==qApp->translate("toBrowser","Synonyms")) {
      QPixmap image((const char **)synonym_xpm);
      setPixmap(0,image);
      if (toIsOracle(conn)) {
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Grants"));
	new toTemplateTableItem(conn,this,qApp->translate("toBrowser","Script"));
      }
    }
  }

  virtual QString allText(int col) const
  {
    QString txt=parent()->parent()->text(0);
    txt+=QString::fromLatin1(".");
    txt+=text(col);
    return txt;
  }

  virtual QWidget *selectedWidget(QWidget *par)
  {
    QString object=text(0);
    QString typ=parent()->text(0);
    QString schema=parent()->parent()->text(0);
    if (schema==qApp->translate("toBrowser","No schemas"))
      schema=Connection.database();

    toToolWidget *tool=new toToolWidget(BrowserTool,
					"",
					par,
					Connection);
    if (typ==qApp->translate("toBrowser","Code")||typ==qApp->translate("toBrowser","Triggers")) {
      toResultField *fld=new toResultField(tool);
      if(typ==qApp->translate("toBrowser","Code"))
	fld->setSQL(SQLSQLTemplate);
      else
	fld->setSQL(SQLTriggerBody);
      fld->changeParams(schema,object);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Tables")||typ==qApp->translate("toBrowser","Views")) {
      toResultCols *cols=new toResultCols(tool);
      cols->changeParams(schema,object);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Indexes")) {
      toResultView *resultView=new toResultLong(true,false,toQuery::Background,tool);
      resultView->setSQL(SQLIndexCols);
      resultView->changeParams(schema,object);
      return tool;
    } else if (typ==qApp->translate("toBrowser","Synonyms")||typ==qApp->translate("toBrowser","Sequences")) {
      toResultItem *resultItem=new toResultItem(2,true,tool);
      if (typ==qApp->translate("toBrowser","Synonyms")) {
	resultItem->setSQL(SQLSynonymInfo);
	int pos=object.find(QString::fromLatin1("."));
	if (pos>=0) {
	  schema=object.mid(0,pos);
	  object=object.mid(pos+1);
	} else {
	  schema=QString::fromLatin1("PUBLIC");
	}
      } else
	resultItem->setSQL(SQLSequenceInfo);
      resultItem->changeParams(schema,object);
      return tool;
    } else {
      delete tool;
      return NULL;
    }
  }
};

class toTemplateSchemaList : public toTemplateSQL {
public:
  toTemplateSchemaList(toConnection &conn,toTemplateItem *parent,
		       const QString &name,const QString &sql)
    : toTemplateSQL(conn,parent,name,sql)
  { }
  virtual toTemplateItem *createChild(const QString &name)
  {
    try {
      toBrowserFilter *filter=BrowseTemplate.filter();
      toTemplateItem *item=new toTemplateSchemaItem(connection(),this,name);
      if (filter&&!filter->check(item)) {
	delete item;
	return NULL;
      }
      return item;
    } catch(...) {
      return NULL;
    }
  }
  virtual toQList parameters(void)
  {
    toQList ret;
    ret.insert(ret.end(),parent()->text(0));
    toBrowserFilter *filter=BrowseTemplate.filter();
    if (filter)
      ret.insert(ret.end(),filter->wildCard());
    else
      ret.insert(ret.end(),toQValue(QString::fromLatin1("%")));
    return ret;
  }
};

class toTemplateDBItem : public toTemplateSQL {
public:
  toTemplateDBItem(toConnection &conn,toTemplateItem *parent,
		   const QString &name)
    : toTemplateSQL(conn,parent,name,toSQL::string(toSQL::TOSQL_USERLIST,conn))
  {
  }
  virtual ~toTemplateDBItem()
  {
    toBrowseTemplate *prov=dynamic_cast<toBrowseTemplate *>(&provider());
    if (prov)
      prov->removeItem(this);
  }
  virtual toTemplateItem *createChild(const QString &name)
  {
    try {
      toTemplateItem *item=new toTemplateItem(this,name);
      QPixmap image((const char **)schema_xpm);
      item->setPixmap(0,image);
      QPixmap table((const char **)table_xpm);
      QPixmap view((const char **)view_xpm);
      QPixmap sequence((const char **)sequence_xpm);
      QPixmap function((const char **)function_xpm);
      QPixmap index((const char **)index_xpm);
      QPixmap synonym((const char **)synonym_xpm);

      toBrowserFilter *filter=BrowseTemplate.filter();
      if (filter&&filter->onlyOwnSchema()&&
	  name.upper()!=connection().user().upper()) {
	delete item;
	return NULL;
      }

      (new toTemplateSchemaList(connection(),
				item,
				qApp->translate("toBrowser","Tables"),
				toSQL::string(SQLListTables,connection())))->setPixmap(0,table);
      if (connection().provider()=="Oracle") {
	(new toTemplateSchemaList(connection(),
				  item,
				  qApp->translate("toBrowser","Views"),
				  toSQL::string(SQLListView,connection())))->setPixmap(0,view);
	(new toTemplateSchemaList(connection(),
				  item,
				  qApp->translate("toBrowser","Indexes"),
				  toSQL::string(SQLListIndex,connection())))->setPixmap(0,index);
	(new toTemplateSchemaList(connection(),
				  item,
				  qApp->translate("toBrowser","Sequences"),
				  toSQL::string(SQLListSequence,connection())))->setPixmap(0,sequence);
	(new toTemplateSchemaList(connection(),
				  item,
				  qApp->translate("toBrowser","Synonyms"),
				  toSQL::string(SQLListSynonym,connection())))->setPixmap(0,synonym);
	(new toTemplateSchemaList(connection(),
				  item,
				  qApp->translate("toBrowser","Code"),
				  toSQL::string(SQLListSQLShort,connection())))->setPixmap(0,function);
	(new toTemplateSchemaList(connection(),
				  item,
				  qApp->translate("toBrowser","Triggers"),
				  toSQL::string(SQLListTrigger,connection())))->setPixmap(0,function);
      }
      return item;
    } catch(...) {
      return NULL;
    }
  }
};

class toBrowseTemplateItem : public toTemplateItem {
public:
  toBrowseTemplateItem(toTemplateProvider &prov,QListView *parent,const QString &name)
    : toTemplateItem(prov,parent,name)
  { }
  virtual ~toBrowseTemplateItem()
  {
    dynamic_cast<toBrowseTemplate &>(provider()).removeItem(this);
  }
};

void toBrowseTemplate::insertItems(QListView *parent,QToolBar *toolbar)
{
  if (!Registered) {
    connect(toMainWidget(),SIGNAL(addedConnection(const QString &)),
	    this,SLOT(addDatabase(const QString &)));
    connect(toMainWidget(),SIGNAL(removedConnection(const QString &)),
	    this,SLOT(removeDatabase(const QString &)));
  }
  toTemplateItem *dbitem=new toBrowseTemplateItem(*this,parent,qApp->translate("toBrowser","DB Browser"));
  std::list<QString> conn=toMainWidget()->connections();
  for (std::list<QString>::iterator i=conn.begin();i!=conn.end();i++) {
    toConnection &conn=toMainWidget()->connection(*i);
    new toTemplateDBItem(conn,dbitem,*i);
  }
  Parents.insert(Parents.end(),dbitem);

  new QToolButton(QPixmap((const char **)filter_xpm),
		  qApp->translate("toBrowser","Define the object filter for database browser"),
		  qApp->translate("toBrowser","Define the object filter for database browser"),
		  this,SLOT(defineFilter(void)),
		  toolbar);
  new QToolButton(QPixmap((const char **)nofilter_xpm),
		  qApp->translate("toBrowser","Remove any object filter for database browser"),
		  qApp->translate("toBrowser","Remove any object filter for database browser"),
		  this,SLOT(clearFilter(void)),
		  toolbar);
}

void toBrowseTemplate::addDatabase(const QString &name)
{
  try {
    for(std::list<toTemplateItem *>::iterator i=Parents.begin();i!=Parents.end();i++)
      new toTemplateDBItem(toMainWidget()->connection(name),*i,name);
  } TOCATCH
}

void toBrowseTemplate::importData(std::map<QCString,QString> &data,const QCString &prefix)
{
  if (data.find(prefix+":Filter:Type")!=data.end()) {
    Filter=new toBrowserFilter;
    Filter->importData(data,prefix+":Filter");
  }
}

void toBrowseTemplate::exportData(std::map<QCString,QString> &data,const QCString &prefix)
{
  if (Filter)
    Filter->exportData(data,prefix+":Filter");
}
