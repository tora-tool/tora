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
 *      with the Qt and Oracle Client libraries and distribute executables,
 *      as long as you follow the requirements of the GNU GPL in regard to
 *      all of the software in the executable aside from Qt and Oracle client
 *      libraries.
 *
 ****************************************************************************/



#include "toresultstorage.h"
#include "tomain.h"
#include "toconf.h"
#include "totool.h"

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

  setColumnAlignment(5,AlignRight);
  setColumnAlignment(6,AlignRight);
  setColumnAlignment(7,AlignRight);
  setColumnAlignment(10,AlignRight);
}

QString toResultStorage::query()
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

    otl_stream tblspc(1,
		      "select a.tablespace_name,"
		      "       a.status,"
		      "       ' ',"
		      "       a.contents,"
		      "       a.logging,"
		      "       to_char(round(sum(c.bytes)/1024/1024,2)),"
		      "       to_char(round(sum(c.user_bytes)/1024/1024,2)),"
		      "       to_char(round(b.total_bytes/1024/1023,2)),"
		      "       to_char(round(b.total_bytes*100/sum(c.user_bytes),2))||'%',"
		      "       to_char(round(b.percent_extents_coalesced,1))||'%',"
		      "       to_char(b.total_extents)"
		      "  from dba_tablespaces a,"
		      "       dba_data_files c,"
		      "       dba_free_space_coalesced b"
		      " where a.tablespace_name = b.tablespace_name"
		      "   and c.tablespace_name = a.tablespace_name"
		      " group by a.tablespace_name,a.status,a.contents,a.logging,a.allocation_type,b.percent_extents_coalesced,b.total_extents,b.total_bytes"
		      " order by a.tablespace_name",
		      Connection.connection());

    otl_stream datfil(1,
		      "select b.name,"
		      "       b.status,"
		      "       b.enabled,"
		      "       ' ',"
		      "       ' ',"
		      "       to_char(round(c.bytes/1024/1024,2)),"
		      "       to_char(round(c.user_bytes/1024/1024,2)),"
		      "       to_char(round(sum(a.bytes)/1024/1023,2)),"
		      "       to_char(round(sum(a.bytes)*100/c.user_bytes,2))||'%',"
		      "       ' ',"
		      "       to_char(count(1)),"
		      "       a.tablespace_name"
		      "  from dba_free_space a,"
		      "       v$datafile b,"
		      "       dba_data_files c"
		      " where a.file_id=b.file#"
		      "   and a.file_id=c.file_id"
		      "   and a.tablespace_name = :f1<char[31]>"
		      " group by a.tablespace_name,b.status,b.enabled,b.name,c.user_bytes,c.bytes,b.checkpoint_time,b.creation_time"
		      " order by a.tablespace_name,b.name",
		      Connection.connection());
    QListViewItem *lastTablespace=NULL;
    while(!tblspc.eof()) {
      QListViewItem *tablespace=new toResultStorageItem(this,lastTablespace);
      for (int i=0;i<11;i++) {
	tblspc>>buffer;
	tablespace->setText(i,buffer);
      }
      datfil<<(const char *)tablespace->text(0);
      QListViewItem *lastFile=NULL;
      while(!datfil.eof()) {
	QListViewItem *file=new toResultStorageItem(tablespace,lastFile);
	for (int i=0;i<12;i++) {
	  datfil>>buffer;
	  file->setText(i,buffer);
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
  } catch (const otl_exception &exc) {
    toStatusMessage((const char *)exc.msg);
    return QString((const char *)exc.msg);
  } catch (const QString &str) {
    toStatusMessage((const char *)str);
    return str;
  }
  updateContents();
  return "";
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

