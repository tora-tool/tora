//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 GlobeCom AB
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
 *      software in the executable aside from Oracle client libraries. You
 *      are also allowed to link this program with the Qt Non Commercial for
 *      Windows.
 *
 *      Specifically you are not permitted to link this program with the
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
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
  addColumn("Free fragments");
  setSQLName("toResultStorage");

  setColumnAlignment(5,AlignRight);
  setColumnAlignment(6,AlignRight);
  setColumnAlignment(7,AlignRight);
  setColumnAlignment(10,AlignRight);

  ShowCoalesced=false;
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
			      "	TO_CHAR(f.total_extents)\n"
			      "  FROM  sys.dba_tablespaces d,\n"
			      "	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from dba_data_files group by tablespace_name) a,\n"
			      "	(select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from dba_free_space_coalesced) f,\n"
			      "       (select :unit<int> unit from dual) b\n"
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
			      "	TO_CHAR(f.total_extents)\n"
			      "  FROM  sys.dba_tablespaces d,\n"
			      "	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from dba_temp_files group by tablespace_name) a,\n"
			      "	(select tablespace_name, sum(bytes_cached) bytes, count(1) total_extents from v$temp_extent_pool group by tablespace_name) f,\n"
			      "       (select :unit<int> unit from dual) b\n"
			      " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
			      "   AND  d.tablespace_name = f.tablespace_name(+)\n"
			      "   AND  d.extent_management = 'LOCAL'\n"
			      "   AND  d.contents = 'TEMPORARY'",
			      "Display storage usage of database. This includes the coalesced columns which may make the query sluggish on some DB:s. "
			      "All columns must be present in output (Should be 11)",
			      "8.1");

static toSQL SQLShowCoalesced8("toResultStorage:ShowCoalesced",
			       "SELECT  d.tablespace_name, \n"
			       "	d.status,\n"
			       "	' ',\n"
			       "	d.contents,\n"
			       "       d.logging,\n"
			       "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			       "	TO_CHAR(ROUND(NVL(a.bytes / b.unit, 0),2)),\n"
			       "	TO_CHAR(ROUND(NVL(f.bytes,0) / b.unit,2)), \n"
			       "	TO_CHAR(ROUND(NVL(f.bytes, 0) / a.bytes * 100, 2))||'%',\n"
			       "       TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
			       "	TO_CHAR(f.total_extents)\n"
			       "  FROM  sys.dba_tablespaces d,\n"
			       "	(select tablespace_name, sum(bytes) bytes from dba_data_files group by tablespace_name) a,\n"
			       "	(select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from dba_free_space_coalesced) f,\n"
			       "       (select :unit<int> unit from dual) b\n"
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
			       "       TO_CHAR(ROUND(f.percent_extents_coalesced,1))||'%',\n"
			       "	TO_CHAR(f.total_extents)\n"
			       "  FROM  sys.dba_tablespaces d,\n"
			       "	(select tablespace_name, sum(bytes) bytes from dba_data_files group by tablespace_name) a,\n"
			       "	(select tablespace_name, total_bytes bytes, total_extents, percent_extents_coalesced from dba_free_space_coalesced) f,\n"
			       "       (select :unit<int> unit from dual) b\n"
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
				"	'-',\n"
				"	TO_CHAR(f.total_extents)\n"
				"  FROM  sys.dba_tablespaces d,\n"
				"	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from dba_data_files group by tablespace_name) a,\n"
				"	(select tablespace_name, sum(bytes) bytes, count(1) total_extents from dba_free_space group by tablespace_name) f,\n"
				"       (select :unit<int> unit from dual) b\n"
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
				"	TO_CHAR(f.total_extents)\n"
				"  FROM  sys.dba_tablespaces d,\n"
				"	(select tablespace_name, sum(bytes) bytes, sum(user_bytes) user_bytes from dba_temp_files group by tablespace_name) a,\n"
				"	(select tablespace_name, sum(bytes_cached) bytes, count(1) total_extents from v$temp_extent_pool group by tablespace_name) f,\n"
				"       (select :unt<int> unit from dual) b\n"
				" WHERE  d.tablespace_name = a.tablespace_name(+)\n"
				"   AND  d.tablespace_name = f.tablespace_name(+)\n"
				"   AND  d.extent_management = 'LOCAL'\n"
				"   AND  d.contents = 'TEMPORARY'",
				"Display storage usage of database. This does not include the coalesced columns which may make the query sluggish on some DB:s. "
				"All columns must be present in output (Should be 11)",
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
				 "	'-',\n"
				 "	TO_CHAR(f.total_extents)\n"
				 "  FROM  sys.dba_tablespaces d,\n"
				 "	(select tablespace_name, sum(bytes) bytes from dba_data_files group by tablespace_name) a,\n"
				 "	(select tablespace_name, sum(bytes) bytes, count(1) total_extents from dba_free_space group by tablespace_name) f,\n"
				 "       (select :unt<int> unit from dual) b\n"
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
				 "	'-',\n"
				 "	TO_CHAR(f.total_extents)\n"
				 "  FROM  sys.dba_tablespaces d,\n"
				 "	(select tablespace_name, sum(bytes) bytes from dba_data_files group by tablespace_name) a,\n"
				 "	(select tablespace_name, sum(bytes) bytes, count(1) total_extents from dba_free_space group by tablespace_name) f,\n"
				 "       (select :unt<int> unit from dual) b\n"
				 " WHERE  d.tablespace_name = a.tablespace_name(+)\n"
				 "   AND  d.tablespace_name = f.tablespace_name(+)\n"
				 " ORDER BY d.tablespace_name",
				 "",
				 "7.3");

static toSQL SQLDatafile("toResultStorage:Datafile",
			 "SELECT  d.tablespace_name,\n"
			 "	v.name,\n"
			 "	v.status,\n"
			 "	v.enabled,\n"
			 "	' ',\n"
			 "	' ',\n"
			 "        to_char(round(d.bytes/b.unit,2)),\n"
			 "        to_char(round(d.user_bytes/b.unit,2)),\n"
			 "        to_char(round(s.bytes/b.unit,2)),\n"
			 "        to_char(round(s.bytes*100/d.user_bytes,2))||'%',\n"
			 "	' ',\n"
			 "	to_char(s.num)\n"
			 "  FROM  sys.dba_data_files d,\n"
			 "	v$datafile v,\n"
			 "	(SELECT file_id, NVL(SUM(bytes),0) bytes, COUNT(1) num FROM sys.dba_free_space  GROUP BY file_id) s,\n"
			 "        (select :unt<int> unit from dual) b\n"
			 " WHERE  (s.file_id (+)= d.file_id)\n"
			 "   AND  (d.file_name = v.name)\n"
			 " UNION  ALL\n"
			 "SELECT  d.tablespace_name,\n"
			 "      v.name,\n"
			 "	v.status,\n"
			 "	v.enabled,\n"
			 "	' ',\n"
			 "	' ',\n"
			 "        to_char(round(d.bytes/b.unit,2)),\n"
			 "        to_char(round(d.user_bytes/b.unit,2)),\n"
			 "        to_char(round((d.user_bytes-t.bytes_cached)/b.unit,2)),\n"
			 "        to_char(round((d.user_bytes-t.bytes_cached)*100/d.user_bytes,2))||'%',\n"
			 "	' ',\n"
			 "	'1'\n"
			 "  FROM  sys.dba_temp_files d,\n"
			 "	v$tempfile v,\n"
			 "	v$temp_extent_pool t,\n"
			 "        (select :unt<int> unit from dual) b\n"
			 " WHERE  (t.file_id (+)= d.file_id)\n"
			 "   AND  (d.file_name = v.file#)",
			 "Display information about a datafile in a tablespace. "
			 "All columns must be present in the output (Should be 12)",
			 "8.1");

static toSQL SQLDatafile8("toResultStorage:Datafile",
			  "SELECT  d.tablespace_name,\n"
			  "	v.name,\n"
			  "	v.status,\n"
			  "	v.enabled,\n"
			  "	' ',\n"
			  "	' ',\n"
			  "        to_char(round(d.bytes/b.unit,2)),\n"
			  "        to_char(round(d.bytes/b.unit,2)),\n"
			  "        to_char(round(s.bytes/b.unit,2)),\n"
			  "        to_char(round(s.bytes*100/d.bytes,2))||'%',\n"
			  "	' ',\n"
			  "	to_char(s.num)\n"
			  "  FROM  sys.dba_data_files d,\n"
			  "	v$datafile v,\n"
			  "	(SELECT file_id, NVL(SUM(bytes),0) bytes, COUNT(1) num FROM sys.dba_free_space  GROUP BY file_id) s,\n"
			  "        (select :unt<int> unit from dual) b\n"
			  " WHERE  (s.file_id (+)= d.file_id)\n"
			  "   AND  (d.file_name = v.name)",
			  "",
			  "8.0");

void toResultStorage::query(void)
{
  if (!handled())
    return;

  QListViewItem *item=selectedItem();
  QString currentSpace;
  QString currentFile;
  if (item) {
    if (item->parent()) {
      currentSpace=item->text(11);
      currentFile=item->text(0);
    } else
      currentSpace=item->text(0);
  }
  clear();

  try {
    toConnection &conn=connection();

    toQList args;
    toPush(args,toQValue(toSizeDecode(Unit)));

    toQuery tblspc(conn,ShowCoalesced?SQLShowCoalesced:SQLNoShowCoalesced,args);

    while(!tblspc.eof()) {
      QListViewItem *tablespace=new toResultStorageItem(this,NULL);
      for (int i=0;i<11;i++)
	tablespace->setText(i,tblspc.readValue());

      tablespace->setExpandable(true);
      if (currentSpace==tablespace->text(0)) {
	if (currentFile.isEmpty())
	  setSelected(tablespace,true);
      }
    }

    toQuery datfil(conn,SQLDatafile,args);
    while(!datfil.eof()) {
      QString name=datfil.readValue();
      QListViewItem *tablespace;
      for (tablespace=firstChild();tablespace&&tablespace->text(0)!=name;tablespace=tablespace->nextSibling())
	;
      if (!tablespace)
	throw QString("Couldn't find tablespace parent %1 for datafile").arg(name);
      QListViewItem *file=new toResultStorageItem(tablespace,NULL);
      for (int i=0;i<11;i++)
	file->setText(i,datfil.readValue());

      file->setText(11,name);
      if (currentSpace==file->text(11)&&
	  currentFile==file->text(0))
	setSelected(file,true);
    }
  } TOCATCH
  updateContents();
}

QString toResultStorage::currentTablespace(void)
{
  QListViewItem *item=selectedItem();
  if (!item)
    throw QString("No tablespace selected");
  QString name;
  if (item->parent())
    name=item->text(11);
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
