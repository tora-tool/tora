/****************************************************************************
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000 GlobeCom AB
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
 *      Qt/UNIX or Qt/Windows products of TrollTech. And you are not
 *      permitted to distribute binaries compiled against these libraries
 *      without written consent from GlobeCom AB. Observe that this does not
 *      disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

TO_NAMESPACE;

#include "toresultstorage.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"
#include "tosql.h"

class toResultStorageItem : public QListViewItem {
public:
  toResultStorageItem(QListView *parent,QListViewItem *after,const char *buffer=NULL)
    : QListViewItem(parent,after,buffer)
  { }
  toResultStorageItem(QListViewItem *parent,QListViewItem *after,const char *buffer=NULL)
    : QListViewItem(parent,after,buffer)
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
      p->fillRect(int(val*width/100),0,width,height(),QBrush(isSelected()?cg.highlight():cg.base()));

      QPen pen(isSelected()?cg.highlightedText():cg.foreground());
      p->setPen(pen);
      p->drawText(0,0,width,height(),AlignCenter,text(column));
    } else {
      QListViewItem::paintCell(p,cg,column,width,align);
    }
  }
};

toResultStorage::toResultStorage(toConnection &conn,QWidget *parent,const char *name)
  : toResultView(false,false,conn,parent,name)
{
  setAllColumnsShowFocus(true);
  setSorting(-1);
  setRootIsDecorated(true);
  addColumn("Name");
  addColumn("Status");
  addColumn("Enabled");
  addColumn("Contents");
  addColumn("Logging");
  addColumn("Size (MB)");
  addColumn("User (MB)");
  addColumn("Free (MB)");
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
			      "select a.tablespace_name,\n"
			      "       a.status,\n"
			      "       ' ',\n"
			      "       a.contents,\n"
			      "       a.logging,\n"
			      "       to_char(round(sum(c.bytes)/1024/1024,2)),\n"
			      "       to_char(round(sum(c.user_bytes)/1024/1024,2)),\n"
			      "       to_char(round(b.total_bytes/1024/1023,2)),\n"
			      "       to_char(round(b.total_bytes*100/sum(c.user_bytes),2))||'%',\n"
			      "       to_char(round(b.percent_extents_coalesced,1))||'%',\n"
			      "       to_char(b.total_extents)\n"
			      "  from dba_tablespaces a,\n"
			      "       (select * from dba_temp_files union select * from dba_data_files) c,\n"
			      "       dba_free_space_coalesced b\n"
			      " where a.tablespace_name = b.tablespace_name\n"
			      "   and c.tablespace_name = a.tablespace_name\n"
			      " group by a.tablespace_name,a.status,a.contents,a.logging,a.allocation_type,b.percent_extents_coalesced,b.total_extents,b.total_bytes\n"
			      " order by a.tablespace_name",
			      "Display storage usage of database. This includes the coalesced columns which may make the query sluggish on some DB:s. "
			      "All columns must be present in output (Should be 11)",
			      "8.1");

static toSQL SQLShowCoalesced8("toResultStorage:ShowCoalesced",
			       "select a.tablespace_name,\n"
			       "       a.status,\n"
			       "       ' ',\n"
			       "       a.contents,\n"
			       "       a.logging,\n"
			       "       to_char(round(sum(c.bytes)/1024/1024,2)),\n"
			       "       to_char(round(sum(c.bytes)/1024/1024,2)),\n"
			       "       to_char(round(b.total_bytes/1024/1023,2)),\n"
			       "       to_char(round(b.total_bytes*100/sum(c.bytes),2))||'%',\n"
			       "       to_char(round(b.percent_extents_coalesced,1))||'%',\n"
			       "       to_char(b.total_extents)\n"
			       "  from dba_tablespaces a,\n"
			       "       dba_data_files c,\n"
			       "       dba_free_space_coalesced b\n"
			       " where a.tablespace_name = b.tablespace_name\n"
			       "   and c.tablespace_name = a.tablespace_name\n"
			       " group by a.tablespace_name,a.status,a.contents,a.logging,a.allocation_type,b.percent_extents_coalesced,b.total_extents,b.total_bytes\n"
			       " order by a.tablespace_name",
			       "",
			       "8.0");

static toSQL SQLNoShowCoalesced("toResultStorage:NoCoalesced",
				"select a.tablespace_name,\n"
				"       a.status,\n"
				"       ' ',\n"
				"       a.contents,\n"
				"       a.logging,\n"
				"       to_char(round(sum(c.bytes)/1024/1024,2)),\n"
				"       to_char(round(sum(c.user_bytes)/1024/1024,2)),\n"
				"       to_char(round(b.total_bytes/1024/1023,2)),\n"
				"       to_char(round(b.total_bytes*100/sum(c.user_bytes),2))||'%',\n"
				"       '-',\n"
				"       to_char(b.total_extents)\n"
				"  from dba_tablespaces a,\n"
				"       (select * from dba_temp_files union select * from dba_data_files) c,\n"
				"       (select tablespace_name,sum(bytes) total_bytes,count(1) total_extents from dba_free_space group by tablespace_name) b\n"
				" where a.tablespace_name = b.tablespace_name\n"
				"   and c.tablespace_name = a.tablespace_name\n"
				" group by a.tablespace_name,a.status,a.contents,a.logging,a.allocation_type,b.total_extents,b.total_bytes\n"
				" order by a.tablespace_name",
				"Display storage usage of database. This does not include the coalesced columns which may make the query sluggish on some DB:s. "
				"All columns must be present in output (Should be 11)",
				"8.1");

static toSQL SQLNoShowCoalesced8("toResultStorage:NoCoalesced",
				 "select a.tablespace_name,\n"
				 "       a.status,\n"
				 "       ' ',\n"
				 "       a.contents,\n"
				 "       a.logging,\n"
				 "       to_char(round(sum(c.bytes)/1024/1024,2)),\n"
				 "       to_char(round(sum(c.bytes)/1024/1024,2)),\n"
				 "       to_char(round(b.total_bytes/1024/1023,2)),\n"
				 "       to_char(round(b.total_bytes*100/sum(c.bytes),2))||'%',\n"
				 "       '-',\n"
				 "       to_char(b.total_extents)\n"
				 "  from dba_tablespaces a,\n"
				 "       dba_data_files c,\n"
				 "       (select tablespace_name,sum(bytes) total_bytes,count(1) total_extents from dba_free_space group by tablespace_name) b\n"
				 " where a.tablespace_name = b.tablespace_name\n"
				 "   and c.tablespace_name = a.tablespace_name\n"
				 " group by a.tablespace_name,a.status,a.contents,a.logging,a.allocation_type,b.total_extents,b.total_bytes\n"
				 " order by a.tablespace_name",
				 "",
				 "8.0");

static toSQL SQLDatafile("toResultStorage:Datafile",
			 "select b.name,\n"
			 "       b.status,\n"
			 "       b.enabled,\n"
			 "       ' ',\n"
			 "       ' ',\n"
			 "       to_char(round(c.bytes/1024/1024,2)),\n"
			 "       to_char(round(c.user_bytes/1024/1024,2)),\n"
			 "       to_char(round(sum(a.bytes)/1024/1023,2)),\n"
			 "       to_char(round(sum(a.bytes)*100/c.user_bytes,2))||'%',\n"
			 "       ' ',\n"
			 "       to_char(count(1)),\n"
			 "       a.tablespace_name\n"
			 "  from dba_free_space a,\n"
			 "       v$datafile b,\n"
			 "       (select * from dba_temp_files union select * from dba_data_files) c\n"
			 " where a.file_id=b.file#\n"
			 "   and a.file_id=c.file_id\n"
			 "   and a.tablespace_name = :f1<char[31]>\n"
			 " group by a.tablespace_name,b.status,b.enabled,b.name,c.user_bytes,c.bytes,b.checkpoint_time,b.creation_time\n"
			 " order by a.tablespace_name,b.name",
			 "Display information about a datafile in a tablespace. "
			 "All columns must be present in the output (Should be 12)",
			 "8.1");

static toSQL SQLDatafile8("toResultStorage:Datafile",
			  "select b.name,\n"
			  "       b.status,\n"
			  "       b.enabled,\n"
			  "       ' ',\n"
			  "       ' ',\n"
			  "       to_char(round(c.bytes/1024/1024,2)),\n"
			  "       to_char(round(c.bytes/1024/1024,2)),\n"
			  "       to_char(round(sum(a.bytes)/1024/1023,2)),\n"
			  "       to_char(round(sum(a.bytes)*100/c.bytes,2))||'%',\n"
			  "       ' ',\n"
			  "       to_char(count(1)),\n"
			  "       a.tablespace_name\n"
			  "  from dba_free_space a,\n"
			  "       v$datafile b,\n"
			  "       dba_data_files c\n"
			  " where a.file_id=b.file#\n"
			  "   and a.file_id=c.file_id\n"
			  "   and a.tablespace_name = :f1<char[31]>\n"
			  " group by a.tablespace_name,b.status,b.enabled,b.name,c.user_bytes,c.bytes,b.checkpoint_time,b.creation_time\n"
			  " order by a.tablespace_name,b.name",
			  "",
			  "8.0");

void toResultStorage::query(void)
{
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
    char buffer[1000];

    QCString sql;

    if (ShowCoalesced)
      sql=toSQL::sql(SQLShowCoalesced,Connection);
    else
      sql=toSQL::sql(SQLNoShowCoalesced,Connection);

    otl_stream tblspc(1,sql,Connection.connection());

    otl_stream datfil(1,
		      SQLDatafile(Connection),
		      Connection.connection());
    QListViewItem *lastTablespace=NULL;
    while(!tblspc.eof()) {
      QListViewItem *tablespace=new toResultStorageItem(this,lastTablespace);
      for (int i=0;i<11;i++) {
	tblspc>>buffer;
	tablespace->setText(i,QString::fromUtf8(buffer));
      }
      datfil<<tablespace->text(0).utf8();
      QListViewItem *lastFile=NULL;
      while(!datfil.eof()) {
	QListViewItem *file=new toResultStorageItem(tablespace,lastFile);
	for (int i=0;i<12;i++) {
	  datfil>>buffer;
	  file->setText(i,QString::fromUtf8(buffer));
	}
	lastFile=file;
	if (currentSpace==file->text(11)&&
	    currentFile==file->text(0))
	  setSelected(file,true);
      }
      lastTablespace=tablespace;
      if (currentSpace==tablespace->text(0)) {
	if (currentFile.isEmpty())
	  setSelected(tablespace,true);
	else
	  setOpen(tablespace,true);
      }
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
