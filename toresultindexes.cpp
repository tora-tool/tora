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

#include "toconf.h"
#include "toconnection.h"
#include "tonoblockquery.h"
#include "toresultindexes.h"
#include "tosql.h"
#include "totool.h"
#include "toresultindexes.moc"

bool toResultIndexes::canHandle(toConnection &conn)
{
  return toIsOracle(conn)||toIsMySQL(conn)||toIsPostgreSQL(conn)|| toIsSapDB(conn);
}

toResultIndexes::toResultIndexes(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  setReadAll(true);
  addColumn(tr("Index Name"));
  addColumn(tr("Columns"));
  addColumn(tr("Type"));
  addColumn(tr("Unique"));
  setSQLName(QString::fromLatin1("toResultIndexes"));
  Query=NULL;
  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

toResultIndexes::~toResultIndexes()
{
  delete Query;
}

static toSQL SQLColumns("toResultIndexes:Columns",
			"SELECT b.Column_Expression,a.Column_Name\n"
			"  FROM sys.All_Ind_Columns a,\n"
			"       sys.All_Ind_Expressions b\n"
			" WHERE a.Index_Owner = b.Index_Owner(+)\n"
			"   AND a.Index_Name  = b.Index_Name(+)\n"
			"   AND a.Column_Position = b.Column_Position(+)\n"
			"   AND a.Index_Owner = :own<char[101]>\n"
			"   AND a.Index_Name = :nam<char[101]>\n"
			" ORDER BY a.Column_Position",
			"List columns an index is built on",
			"8.1");
static toSQL SQLColumns8("toResultIndexes:Columns",
			 "SELECT Column_Name,NULL FROM sys.All_Ind_Columns\n"
			 " WHERE Index_Owner = :f1<char[101]> AND Index_Name = :f2<char[101]>\n"
			 " ORDER BY Column_Position",
			 "",
			 "8.0");
static toSQL SQLColumnsPgSQL("toResultIndexes:Columns",
			     "SELECT a.attname, a.attname as x\n"
			     "  FROM pg_class c, pg_attribute a, pg_user u\n"
			     " WHERE c.relowner=u.usesysid AND u.usename = :f1\n"
			     "   AND a.attrelid = c.oid AND c.relname = :f2\n"
			     "   AND a.attnum > 0\n"
			     " ORDER BY a.attnum",
			     "",
			     "7.1",
			     "PostgreSQL");

static toSQL SQLColumnsSapDB("toResultIndexes:Columns",
			       "SELECT columnname,datatype\n"
			       "  FROM indexcolumns\n"
			       " WHERE owner = :f1<char101> and indexname = :f2<char[101]>\n"
			       " ORDER BY columnno\n",
			       "",
			       "",
			       "SapDB");

QString toResultIndexes::indexCols(const QString &indOwner,const QString &indName)
{
  toQuery query(connection(),SQLColumns,indOwner,indName);

  QString ret;
  while(!query.eof()) {
    if (!ret.isEmpty())
      ret.append(QString::fromLatin1(","));
    QString t=query.readValueNull();
    if (t.isEmpty())
      t=query.readValue();
    else
      query.readValue();
    ret.append(t);
  }
  return ret;
}

static toSQL SQLListIndex("toResultIndexes:ListIndex",
			  "SELECT Owner,\n"
			  "       Index_Name,\n"
			  "       Index_Type,\n"
			  "       Uniqueness\n"
			  "  FROM sys.All_Indexes\n"
			  " WHERE Table_Owner = :f1<char[101]>\n"
			  "   AND Table_Name = :f2<char[101]>\n"
			  " ORDER BY Index_Name",
			  "List the indexes available on a table",
			  "8.0");
static toSQL SQLListIndex7("toResultIndexes:ListIndex",
			   "SELECT Owner,\n"
			   "       Index_Name,\n"
			   "       'NORMAL',\n"
			   "       Uniqueness\n"
			   "  FROM sys.All_Indexes\n"
			   " WHERE Table_Owner = :f1<char[101]>\n"
			   "   AND Table_Name = :f2<char[101]>\n"
			   " ORDER BY Index_Name",
			   "",
			   "7.3");

static toSQL SQLListIndexMySQL("toResultIndexes:ListIndex",
			       "SHOW INDEX FROM :f1<noquote>.:tab<noquote>",
			       "",
			       "3.0",
			       "MySQL");
static toSQL SQLListIndexPgSQL("toResultIndexes:ListIndex",
                               "SELECT u.usename as Owner,\n"
                               "       c2.relname as Index_Name,\n"
                               "  CASE WHEN i.indisprimary = TRUE THEN 'PRIMARY'\n"
                               "       ELSE 'NORMAL'\n"
                               "  END AS Index_Type,\n"
                               "  CASE WHEN i.indisunique = TRUE THEN 'UNIQUE'\n"
                               "       ELSE 'NON UNIQUE'\n"
                               "  END AS non_unique\n"
                               "  FROM pg_class c, pg_class c2, pg_index i, pg_user u\n"
                               " WHERE c.relowner=u.usesysid and u.usename = :f1\n"
                               "   AND c.relname = :f2\n"
                               "   AND c.oid = i.indrelid\n"
                               "   AND i.indexrelid = c2.oid\n"
                               " ORDER BY c2.relname",
			       "",
			       "7.1",
			       "PostgreSQL");
static toSQL SQLListIndexSapDB("toResultIndexes:ListIndex",
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

void toResultIndexes::query(const QString &,const toQList &param)
{
  if (!handled())
    return;

  if (Query)
    delete Query;
  Query=NULL;

  try {
    toConnection &conn=connection();
    if(toIsOracle(conn))
      Type=Oracle;
    else if (toIsMySQL(conn))
      Type=MySQL;
    else if (toIsPostgreSQL(conn))
      Type=PostgreSQL;
    else if (toIsSapDB(conn))
      Type=SapDB;
    else
      return;
    
    toQList::iterator cp=((toQList &)param).begin();
    if (cp!=((toQList &)param).end())
      Owner=*cp;
    cp++;
    if (cp!=((toQList &)param).end())
      TableName=(*cp);

    RowNumber=0;

    clear();

    toQuery query(connection());
    toQList par;
    par.insert(par.end(),Owner);
    par.insert(par.end(),TableName);
    Last=NULL;

    Query=new toNoBlockQuery(connection(),toQuery::Background,
			     toSQL::string(SQLListIndex,conn),par);
    Poll.start(100);
  } TOCATCH
}

void toResultIndexes::poll(void)
{
  try {
    if (!toCheckModal(this))
      return;
    if (Query&&Query->poll()) {
      while(Query->poll()&&!Query->eof()) {
	if (Type==Oracle||Type==PostgreSQL||Type==SapDB) {
	  Last=new toResultViewItem(this,NULL);
	  
	  QString indexOwner(Query->readValue());
	  QString indexName(Query->readValue());
	  Last->setText(0,indexName);
	  Last->setText(1,indexCols(indexOwner,indexName));
	  Last->setText(2,Query->readValue());
	  Last->setText(3,Query->readValue());
	} else {
	  Query->readValue(); // Tablename
	  int unique=Query->readValue().toInt();
	  QString name=Query->readValue();
	  Query->readValue(); // SeqID
	  QString col=Query->readValue();
	  Query->readValue();
	  Query->readValue();
	  Query->readValue();
	  Query->readValue();
	  Query->readValue();
	  if (Last&&Last->text(0)==name)
	    Last->setText(1,Last->text(1)+QString::fromLatin1(",")+col);
	  else {
	    Last=new toResultViewItem(this,NULL);
	    Last->setText(0,name);
	    Last->setText(1,col);
	    Last->setText(2,QString::fromLatin1((name==QString::fromLatin1("PRIMARY"))?"PRIMARY":"INDEX"));
	    Last->setText(3,QString::fromLatin1(unique?"NONUNIQUE":"UNIQUE"));
	  }
	}
      }
      if (Query->eof()) {
	delete Query;
	Query=NULL;
	Poll.stop();
      }
    }
  } catch(const QString &exc) {
    delete Query;
    Query=NULL;
    Poll.stop();
    toStatusMessage(exc);
  }
}
