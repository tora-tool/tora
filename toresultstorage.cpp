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

#include <qpainter.h>
#include <qheader.h>

#include "toresultstorage.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"
#include "toconnection.h"
#include "tonoblockquery.h"

#include "toresultstorage.moc"

class toResultStorageItem : public toResultViewItem {
public:
  toResultStorageItem(QListView *parent,QListViewItem *after,const QString &buf=QString::null)
    : toResultViewItem(parent,after,buf)
  { }
  toResultStorageItem(QListViewItem *parent,QListViewItem *after,const QString &buf=QString::null)
    : toResultViewItem(parent,after,buf)
  { }
  virtual void paintCell(QPainter * p,const QColorGroup & cg,int column,int width,int align)
  {
    if (column==8||column==9) {
      QString ct=text(column);
      if (ct.isEmpty()) {
	QListViewItem::paintCell(p,cg,column,width,align);
	return;
      }
      ct=ct.left(ct.length()-1); // Strip last %
      double val=ct.toDouble();

      p->fillRect(0,0,int(val*width/100),height(),QBrush(blue));
      p->fillRect(int(val*width/100),0,width,height(),
		  QBrush(isSelected()?cg.highlight():cg.base()));

      QPen pen(isSelected()?cg.highlightedText():cg.foreground());
      p->setPen(pen);
      p->drawText(0,0,width,height(),AlignCenter,text(column));
    } else {
      QListViewItem::paintCell(p,cg,column,width,align);
    }
  }
};

toResultStorage::toResultStorage(QWidget *parent,const char *name)
  : toResultView(false,false,parent,name)
{
  Unit=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
  setAllColumnsShowFocus(true);
  setSorting(0);
  setRootIsDecorated(true);
  addColumn("Name");
  addColumn("Status");
  addColumn("Enabled");
  addColumn("Contents");
  addColumn("Logging");
  addColumn(QString("Size (%1)").arg(Unit));
  addColumn(QString("User (%1)").arg(Unit));
  addColumn(QString("Free (%1)").arg(Unit));
  addColumn("Available");
  addColumn("Coalesced");
  addColumn(QString("Max free (%1)").arg(Unit));
  addColumn("Free fragments");
  setSQLName("toResultStorage");

  setColumnAlignment(5,AlignRight);
  setColumnAlignment(6,AlignRight);
  setColumnAlignment(7,AlignRight);
  setColumnAlignment(8,AlignCenter);
  setColumnAlignment(9,AlignCenter);
  setColumnAlignment(10,AlignRight);
  setColumnAlignment(11,AlignRight);

  ShowCoalesced=false;

  Tablespaces=Files=NULL;

  connect(&Poll,SIGNAL(timeout()),this,SLOT(poll()));
}

toResultStorage::~toResultStorage()
{
  delete Tablespaces;
  delete Files;
}

static toSQL SQLShowCoalesced("toResultStorage:ShowCoalesced",
			      "SELECT  d.tablespace_name, \n"
			      "	d.status,\n"
			      "	' ',\n"
			      "	d.contents,\n"
			      "       d.logging,\n"
			      "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			      "	TO_CHAR(ROUND(NVL(a.user_bytes / b.unit, 0),2)),\n"
			      "	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)), \n"
			      "	TO_CHAR(ROUND(NVL(f.bytes, 0) / a.bytes * 100, 2))||'%',\n"
			      "       TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
			      " '-',\n"
			      "	TO_CHAR(f.total_extents)\n"
			      "  FROM  sys.dba_tablespaces d,\n"
			      "	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from sys.dba_data_files group by tablespace_name) a,\n"
			      "	(select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from sys.dba_free_space_coalesced) f,\n"
			      "       (select :unt<int> unit from sys.dual) b\n"
			      " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
			      "   AND  d.tablespace_name = f.tablespace_name(+)\n"
			      "   AND  NOT (d.extent_management like 'LOCAL' AND d.contents like 'TEMPORARY')\n"
			      " UNION  ALL\n"
			      "SELECT  d.tablespace_name, \n"
			      "	d.status,\n"
			      "	' ',\n"
			      "	d.contents,\n"
			      "       d.logging,\n"
			      "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			      "	TO_CHAR(ROUND(NVL(a.user_bytes / b.unit, 0),2)),\n"
			      "	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)),\n"
			      "	TO_CHAR(ROUND((NVL(a.bytes,0) - NVL(f.bytes, 0)) / a.bytes * 100, 2))||'%',\n"
			      "	'-',\n"
			      "	TO_CHAR(ROUND(NVL(f.maxbytes,0) / b.unit,2)), \n"
			      "	TO_CHAR(f.total_extents)\n"
			      "  FROM  sys.dba_tablespaces d,\n"
			      "	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from sys.dba_temp_files group by tablespace_name) a,\n"
			      "	(select tablespace_name, sum(bytes_cached) bytes, count(1) total_extents,max(bytes_cached) maxbytes from v$temp_extent_pool group by tablespace_name) f,\n"
			      "       (select :unt<int> unit from sys.dual) b\n"
			      " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
			      "   AND  d.tablespace_name = f.tablespace_name(+)\n"
			      "   AND  d.extent_management = 'LOCAL'\n"
			      "   AND  d.contents = 'TEMPORARY'",
			      "Display storage usage of database. This includes the coalesced columns which may make the query sluggish on some DB:s. "
			      "All columns must be present in output (Should be 12)",
			      "8.1");

static toSQL SQLShowCoalesced8("toResultStorage:ShowCoalesced",
			       "SELECT  d.tablespace_name, \n"
			       "	d.status,\n"
			       "	' ',\n"
			       "	d.contents,\n"
			       "        d.logging,\n"
			       "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			       "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			       "	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)), \n"
			       "	TO_CHAR(ROUND(NVL(f.bytes, 0) / a.bytes * 100, 2))||'%',\n"
			       "        TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
			       "        '-',\n"
			       "	TO_CHAR(f.total_extents)\n"
			       "  FROM  sys.dba_tablespaces d,\n"
			       "	(select tablespace_name, sum(bytes) bytes from sys.dba_data_files group by tablespace_name) a,\n"
			       "	(select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from sys.dba_free_space_coalesced) f,\n"
			       "       (select :unt<int> unit from sys.dual) b\n"
			       " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
			       "   AND  d.tablespace_name = f.tablespace_name(+)\n"
			       " ORDER  BY d.tablespace_name",
			       "",
			       "8.0");

static toSQL SQLShowCoalesced7("toResultStorage:ShowCoalesced",
			       "SELECT  d.tablespace_name, \n"
			       "	d.status,\n"
			       "	' ',\n"
			       "	d.contents,\n"
			       "        'N/A',\n"
			       "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			       "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			       "	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)), \n"
			       "	TO_CHAR(ROUND(NVL(f.bytes, 0) / a.bytes * 100, 2))||'%',\n"
			       "        TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
			       "        '-',\n"
			       "	TO_CHAR(f.total_extents)\n"
			       "  FROM  sys.dba_tablespaces d,\n"
			       "	(select tablespace_name, sum(bytes) bytes from sys.dba_data_files group by tablespace_name) a,\n"
			       "	(select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from sys.dba_free_space_coalesced) f,\n"
			       "       (select :unt<int> unit from sys.dual) b\n"
			       " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
			       "   AND  d.tablespace_name = f.tablespace_name(+)\n"
			       " ORDER  BY d.tablespace_name",
			       "",
			       "7.3");

static toSQL SQLNoShowCoalesced("toResultStorage:NoCoalesced",
				"SELECT  d.tablespace_name, \n"
				"	d.status,\n"
				"	' ',\n"
				"	d.contents,\n"
				"       d.logging,\n"
				"	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
				"	TO_CHAR(ROUND(NVL(a.user_bytes / b.unit, 0),2)),\n"
				"	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)), \n"
				"	TO_CHAR(ROUND(NVL(f.bytes, 0) / a.bytes * 100, 2))||'%',\n"
				"       '-',\n"
				"	TO_CHAR(ROUND(NVL(f.maxbytes,0) / b.unit,2)), \n"
				"	TO_CHAR(f.total_extents)\n"
				"  FROM  sys.dba_tablespaces d,\n"
				"	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from sys.dba_data_files group by tablespace_name) a,\n"
				"	(select tablespace_name, sum(bytes) bytes, count(1) total_extents, max(bytes) maxbytes from sys.dba_free_space group by tablespace_name) f,\n"
				"       (select :unt<int> unit from sys.dual) b\n"
				" WHERE  d.tablespace_name = a.tablespace_name(+)\n"
				"   AND  d.tablespace_name = f.tablespace_name(+)\n"
				"   AND  NOT (d.extent_management like 'LOCAL' AND d.contents like 'TEMPORARY')\n"
				" UNION  ALL\n"
				"SELECT  d.tablespace_name, \n"
				"	d.status,\n"
				"	' ',\n"
				"	d.contents,\n"
				"       d.logging,\n"
				"	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
				"	TO_CHAR(ROUND(NVL(a.user_bytes / b.unit, 0),2)),\n"
				"	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)),\n"
				"	TO_CHAR(ROUND((NVL(a.bytes,0) - NVL(f.bytes, 0)) / a.bytes * 100, 2))||'%',\n"
				"       '-',\n"
				"	TO_CHAR(ROUND(NVL(f.maxbytes,0) / b.unit,2)), \n"
				"	TO_CHAR(f.total_extents)\n"
				"  FROM  sys.dba_tablespaces d,\n"
				"	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from sys.dba_temp_files group by tablespace_name) a,\n"
				"	(select tablespace_name, sum(bytes_cached) bytes, count(1) total_extents, max(bytes_cached) maxbytes from v$temp_extent_pool group by tablespace_name) f,\n"
				"       (select :unt<int> unit from sys.dual) b\n"
				" WHERE  d.tablespace_name = a.tablespace_name(+)\n"
				"   AND  d.tablespace_name = f.tablespace_name(+)\n"
				"   AND  d.extent_management = 'LOCAL'\n"
				"   AND  d.contents = 'TEMPORARY'",
				"Display storage usage of database. This does not include the coalesced columns which may make the query sluggish on some DB:s. "
				"All columns must be present in output (Should be 12)",
				"8.1");

static toSQL SQLNoShowCoalesced8("toResultStorage:NoCoalesced",
				 "SELECT  d.tablespace_name, \n"
				 "	d.status,\n"
				 "	' ',\n"
				 "	d.contents,\n"
				 "       d.logging,\n"
				 "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
				 "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
				 "	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)), \n"
				 "	TO_CHAR(ROUND(NVL(f.bytes, 0) / a.bytes * 100, 2))||'%',\n"
				 "      '-',\n"
				 "	TO_CHAR(ROUND(NVL(f.maxbytes,0) / b.unit,2)), \n"
				 "	TO_CHAR(f.total_extents)\n"
				 "  FROM  sys.dba_tablespaces d,\n"
				 "	(select tablespace_name, sum(bytes) bytes from sys.dba_data_files group by tablespace_name) a,\n"
				 "	(select tablespace_name, sum(bytes) bytes, count(1) total_extents, max(bytes) maxbytes from sys.dba_free_space group by tablespace_name) f,\n"
				 "       (select :unt<int> unit from sys.dual) b\n"
				 " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
				 "   AND  d.tablespace_name = f.tablespace_name(+)\n"
				 " ORDER BY d.tablespace_name",
				 "",
				 "8.0");

static toSQL SQLNoShowCoalesced7("toResultStorage:NoCoalesced",
				 "SELECT  d.tablespace_name, \n"
				 "	d.status,\n"
				 "	' ',\n"
				 "	d.contents,\n"
				 "      'N/A',\n"
				 "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
				 "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
				 "	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)), \n"
				 "	TO_CHAR(ROUND(NVL(f.bytes, 0) / a.bytes * 100, 2))||'%',\n"
				 "      '-',\n"
				 "	TO_CHAR(ROUND(NVL(f.maxbytes,0) / b.unit,2)), \n"
				 "	TO_CHAR(f.total_extents)\n"
				 "  FROM  sys.dba_tablespaces d,\n"
				 "	(select tablespace_name, sum(bytes) bytes from sys.dba_data_files group by tablespace_name) a,\n"
				 "	(select tablespace_name, sum(bytes) bytes, count(1) total_extents, max(bytes) maxbytes from sys.dba_free_space group by tablespace_name) f,\n"
				 "       (select :unt<int> unit from sys.dual) b\n"
				 " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
				 "   AND  d.tablespace_name = f.tablespace_name(+)\n"
				 " ORDER BY d.tablespace_name",
				 "",
				 "7.3");

static toSQL SQLDatafile("toResultStorage:Datafile",
			 "SELECT d.tablespace_name,\n"
			 "       v.name,\n"
			 "       v.status,\n"
			 "       v.enabled,\n"
			 "	 ' ',\n"
			 "	 ' ',\n"
			 "       to_char(round(d.bytes/b.unit,2)),\n"
			 "       to_char(round(d.user_bytes/b.unit,2)),\n"
			 "       to_char(round(s.bytes/b.unit,2)),\n"
			 "       to_char(round(s.bytes*100/d.bytes,2))||'%',\n"
			 "       ' ',\n"
			 "       to_char(round(s.maxbytes/b.unit,2)),\n"
			 "       to_char(s.num),\n"
			 "       v.file#\n"
			 "  FROM sys.dba_data_files d,\n"
			 "       v$datafile v,\n"
			 "       (SELECT file_id, NVL(SUM(bytes),0) bytes, COUNT(1) num, NVL(MAX(bytes),0) maxbytes FROM sys.dba_free_space  GROUP BY file_id) s,\n"
			 "       (select :unt<int> unit from sys.dual) b\n"
			 " WHERE (s.file_id (+)= d.file_id)\n"
			 "   AND (d.file_name = v.name)\n"
			 " UNION ALL\n"
			 "SELECT d.tablespace_name,\n"
			 "       v.name,\n"
			 "       v.status,\n"
			 "       v.enabled,\n"
			 "       ' ',\n"
			 "       ' ',\n"
			 "       to_char(round(d.bytes/b.unit,2)),\n"
			 "       to_char(round(d.user_bytes/b.unit,2)),\n"
			 "       to_char(round((d.user_bytes-t.bytes_cached)/b.unit,2)),\n"
			 "       to_char(round((d.bytes-t.bytes_cached)*100/d.bytes,2))||'%',\n"
			 "       ' ',\n"
			 "       ' ',\n"
			 "       '1',\n"
			 "       v.file#\n" // Needed by toStorage to work
			 "  FROM sys.dba_temp_files d,\n"
			 "       v$tempfile v,\n"
			 "       v$temp_extent_pool t,\n"
			 "       (select :unt<int> unit from sys.dual) b\n"
			 " WHERE (t.file_id (+)= d.file_id)\n"
			 "   AND (d.file_id = v.file#)",
			 "Display information about a datafile in a tablespace. "
			 "All columns must be present in the output (Should be 13)",
			 "8.1");

static toSQL SQLDatafile8("toResultStorage:Datafile",
			  "SELECT  d.tablespace_name,\n"
			  "	   v.name,\n"
			  "	   v.status,\n"
			  "	   v.enabled,\n"
			  "	   ' ',\n"
			  "	   ' ',\n"
			  "        to_char(round(d.bytes/b.unit,2)),\n"
			  "        to_char(round(d.bytes/b.unit,2)),\n"
			  "        to_char(round(s.bytes/b.unit,2)),\n"
			  "        to_char(round(s.bytes*100/d.bytes,2))||'%',\n"
			  "	   ' ',\n"
			  "        to_char(round(s.maxbytes/b.unit,2)),\n"
			  "	   to_char(s.num),\n"
			  "        v.file#\n"
			  "  FROM  sys.dba_data_files d,\n"
			  "	   v$datafile v,\n"
			  "	   (SELECT file_id, NVL(SUM(bytes),0) bytes, COUNT(1) num,NVL(MAX(bytes),0) maxbytes FROM sys.dba_free_space  GROUP BY file_id) s,\n"
			  "        (select :unt<int> unit from sys.dual) b\n"
			  " WHERE  (s.file_id (+)= d.file_id)\n"
			  "   AND  (d.file_name = v.name)",
			  "",
			  "8.0");

void toResultStorage::query(void)
{
  if (!handled()||Tablespaces||Files)
    return;

  QListViewItem *item=selectedItem();
  if (item) {
    if (item->parent()) {
      CurrentSpace=item->text(13);
      CurrentFile=item->text(0);
    } else
      CurrentSpace=item->text(0);
  }
  clear();

  toConnection &conn=connection();

  toQList args;
  toPush(args,toQValue(toSizeDecode(Unit)));

  try {
    Tablespaces=new toNoBlockQuery(conn,toQuery::Background,
				   toSQL::string(ShowCoalesced?SQLShowCoalesced:SQLNoShowCoalesced,connection()),args);
    Files=NULL;
    Files=new toNoBlockQuery(conn,toQuery::Background,
			     toSQL::string(SQLDatafile,connection()),args);
  
    Poll.start(100);
  } TOCATCH
}

void toResultStorage::poll(void)
{
  try {
    if (Tablespaces&&Tablespaces->poll()) {
      int cols=Tablespaces->describe().size();
      while(Tablespaces->poll()&&!Tablespaces->eof()) {
	for (int i=0;i<cols&&!Tablespaces->eof();i++)
	  toPush(TablespaceValues,QString(Tablespaces->readValue()));
      }
      if (Tablespaces->eof()) {
	delete Tablespaces;
	Tablespaces=NULL;

	while(TablespaceValues.size()>0) {
	  QListViewItem *tablespace=new toResultStorageItem(this,NULL);
	  for (int i=0;i<cols;i++)
	    tablespace->setText(i,toShift(TablespaceValues));

	  if (CurrentSpace==tablespace->text(0)) {
	    if (CurrentFile.isEmpty())
	      setSelected(tablespace,true);
	  }
	}
      }
    }

    if (Files&&Files->poll()) {
      int cols=Files->describe().size();
       while(Files->poll()&&!Files->eof()) {
	for (int i=0;i<cols&&!Files->eof();i++)
	  toPush(FileValues,QString(Files->readValue()));
      }
      if (Files->eof()) {
	delete Files;
	Files=NULL;
      }
    }

    if (Tablespaces==NULL&&Files==NULL) {
      while(FileValues.size()>0) {
	QString name=toShift(FileValues);
	QListViewItem *tablespace;
	for (tablespace=firstChild();tablespace&&tablespace->text(0)!=name;tablespace=tablespace->nextSibling())
	  ;
	if (!tablespace)
	  throw QString("Couldn't find tablespace parent %1 for datafile").arg(name);
	QListViewItem *file=new toResultStorageItem(tablespace,NULL);
	for (int i=0;i<13;i++)
	  file->setText(i,toShift(FileValues));

	file->setText(13,name);
	if (CurrentSpace==file->text(13)&&
	    CurrentFile==file->text(0)) {
	  tablespace->setOpen(true);
	  setSelected(file,true);
	}
      }
      Poll.stop();
    }
  } catch(const QString &exc) {
    delete Tablespaces;
    Tablespaces=NULL;
    delete Files;
    Files=NULL;
    Poll.stop();
    toStatusMessage(exc);
  }
}

QString toResultStorage::currentTablespace(void)
{
  QListViewItem *item=selectedItem();
  if (!item)
    throw QString("No tablespace selected");
  QString name;
  if (item->parent())
    name=item->text(13);
  else
    name=item->text(0);
  if (name.isEmpty())
    throw QString("Weird, empty tablespace name");
  return name;
}

QString toResultStorage::currentFilename(void)
{
  QListViewItem *item=selectedItem();
  if (!item||!item->parent())
    throw QString("No file selected");
  QString name=item->text(0);
  return name;
}
