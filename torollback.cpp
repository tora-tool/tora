//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2003 Quest Software, Inc
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
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#include "utils.h"

#include "tochangeconnection.h"
#include "toconf.h"
#include "toconnection.h"
#include "tohelp.h"
#include "tomain.h"
#include "tomemoeditor.h"
#include "toresultitem.h"
#include "toresultlong.h"
#include "toresultview.h"
#include "torollback.h"
#include "tosgastatement.h"
#include "tosql.h"
#include "tostoragedefinition.h"
#include "totool.h"

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include <list>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qheader.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qregexp.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qworkspace.h>

#include "torollback.moc"
#include "torollbackdialogui.moc"

#include "icons/addrollback.xpm"
#include "icons/offline.xpm"
#include "icons/online.xpm"
#include "icons/refresh.xpm"
#include "icons/torollback.xpm"
#include "icons/trash.xpm"

#define CONF_NO_EXEC    "NoExec"
#define CONF_NEED_READ  "NeedRead"
#define CONF_NEED_TWO   "NeedTwo"
#define CONF_ALIGN_LEFT "AlignLeft"
#define CONF_OLD_ENABLE "OldEnable"

class toRollbackPrefs : public QGroupBox,public toSettingTab
{ 
  QCheckBox *OldEnable;
  QCheckBox *NoExec;
  QCheckBox *NeedRead;
  QCheckBox *NeedTwo;
  QCheckBox *AlignLeft;

  toTool *Tool;

public:
  toRollbackPrefs(toTool *tool,QWidget* parent=0,const char* name=0);
  virtual void saveSetting(void);
};

toRollbackPrefs::toRollbackPrefs(toTool *tool,QWidget* parent,const char* name)
  : QGroupBox(1,Horizontal,parent,name),toSettingTab("rollback.html#options"),Tool(tool)
{
  setTitle(qApp->translate("toRollbackPrefs","Rollback Tool" ));

  OldEnable=new QCheckBox(this,"OldEnable");
  OldEnable->setText(qApp->translate("toRollbackPrefs","&Enable snapshot too old detection." ));
  QToolTip::add(OldEnable,qApp->translate("toRollbackPrefs","Enable snapshot too old detection, will put load on large databases."));
  
  AlignLeft=new QCheckBox(this,"AlignLeft");
  AlignLeft->setText(qApp->translate("toRollbackPrefs","&Disregard start extent." ));
  AlignLeft->setEnabled(false);
  connect(OldEnable,SIGNAL(toggled(bool)),AlignLeft,SLOT(setEnabled(bool)));
  QToolTip::add(AlignLeft,qApp->translate("toRollbackPrefs","Always start from the left border when displaying extent usage."));
  
  NoExec=new QCheckBox(this,"NoCopy");
  NoExec->setText(qApp->translate("toRollbackPrefs","&Restart reexecuted statements" ));
  NoExec->setEnabled(false);
  connect(OldEnable,SIGNAL(toggled(bool)),NoExec,SLOT(setEnabled(bool)));
  QToolTip::add(NoExec,qApp->translate("toRollbackPrefs","Start statements again that have been reexecuted."));
  
  NeedRead=new QCheckBox(this,"Needread");
  NeedRead->setText(qApp->translate("toRollbackPrefs","&Must read buffers" ));
  NeedRead->setEnabled(false);
  connect(OldEnable,SIGNAL(toggled(bool)),NeedRead,SLOT(setEnabled(bool)));
  QToolTip::add(NeedRead,qApp->translate("toRollbackPrefs","Don't display statements that have not read buffers."));
  
  NeedTwo=new QCheckBox(this,"NeedTwo");
  NeedTwo->setText(qApp->translate("toRollbackPrefs","&Exclude first appearance" ));
  NeedTwo->setEnabled(false);
  connect(OldEnable,SIGNAL(toggled(bool)),NeedTwo,SLOT(setEnabled(bool)));
  QToolTip::add(NeedTwo,qApp->translate("toRollbackPrefs","A statement must be visible at least two consecutive polls to be displayed."));
  
  if (!tool->config(CONF_OLD_ENABLE,"").isEmpty())
    OldEnable->setChecked(true);
  if (!tool->config(CONF_NO_EXEC,"Yes").isEmpty())
    NoExec->setChecked(true);
  if (!tool->config(CONF_NEED_READ,"Yes").isEmpty())
    NeedRead->setChecked(true);
  if (!tool->config(CONF_NEED_TWO,"Yes").isEmpty())
    NeedTwo->setChecked(true);
  if (!tool->config(CONF_ALIGN_LEFT,"Yes").isEmpty())
    AlignLeft->setChecked(true);
}

void toRollbackPrefs::saveSetting(void)
{
  Tool->setConfig(CONF_NO_EXEC,NoExec->isChecked()?"Yes":"");
  Tool->setConfig(CONF_NEED_READ,NeedRead->isChecked()?"Yes":"");
  Tool->setConfig(CONF_NEED_TWO,NeedTwo->isChecked()?"Yes":"");
  Tool->setConfig(CONF_ALIGN_LEFT,AlignLeft->isChecked()?"Yes":"");
  Tool->setConfig(CONF_OLD_ENABLE,OldEnable->isChecked()?"Yes":"");
}

class toRollbackTool : public toTool {
protected:
  virtual const char **pictureXPM(void)
  { return torollback_xpm; }
public:
  toRollbackTool()
    : toTool(220,"Rollback Segments")
  { }
  virtual const char *menuItem()
  { return "Rollback Segments"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toRollback(parent,connection);
  }
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toRollbackPrefs(this,parent);
  }
};

static toRollbackTool RollbackTool;

static toSQL SQLTablespace("toRollbackDialog:TablespaceName",
			   "select tablespace_name from sys.dba_tablespaces order by tablespace_name",
			   "Get a list of tablespace names, should only have one column and same binds");

toRollbackDialog::toRollbackDialog(toConnection &Connection,QWidget* parent,const char* name)
  : toRollbackDialogUI(parent,name,true)
{
  toHelp::connectDialog(this);

  try {
    toQuery q(Connection,SQLTablespace);
    while(!q.eof())
      Tablespace->insertItem(q.readValue());
  } catch (...) {

  }
  connect(Tablespace,SIGNAL(textChanged(const QString &)),
	  this,SLOT(valueChanged(const QString &)));

  Storage=new toStorageDefinition(DialogTabs);
  Storage->forRollback(true);
  DialogTabs->addTab(Storage,tr("Storage"));

  OkButton->setEnabled(false);
}

std::list<QString> toRollbackDialog::sql(void)
{
  QString str(QString::fromLatin1("CREATE "));
  if (Public->isChecked())
    str.append(QString::fromLatin1("PUBLIC "));
  str.append(QString::fromLatin1("ROLLBACK SEGMENT \""));
  str.append(Name->text());
  str.append(QString::fromLatin1("\" TABLESPACE \""));
  str.append(Tablespace->currentText());
  str.append(QString::fromLatin1("\""));
  std::list<QString> lst=Storage->sql();
  for(std::list<QString>::iterator i=lst.begin();i!=lst.end();i++) {
    str+=QString::fromLatin1(" ");
    str+=*i;
  }
  std::list<QString> ret;
  toPush(ret,str);
  return ret;
}

void toRollbackDialog::displaySQL(void)
{
  std::list<QString> lines=sql();
  QString res;
  for(std::list<QString>::iterator i=lines.begin();i!=lines.end();i++) {
    res+=*i;
    res+=QString::fromLatin1(";\n");
  }
  if (res.length()>0) {
    toMemoEditor memo(this,res,-1,-1,true,true);
    memo.exec();
  } else
    toStatusMessage(tr("No changes made"),false,false);
}

void toRollbackDialog::valueChanged(const QString &)
{
  if (Name->text().isEmpty()||
      Tablespace->currentText().isEmpty())
    OkButton->setEnabled(false);
  else
    OkButton->setEnabled(true);
}

#define MIN_HEIGHT 4
#define TRANSCOL 17

static bool BarsAlignLeft=true;

static void PaintBars(QListViewItem *item,QPainter *p,const QColorGroup & cg,
		      int width,std::list<double> &val,std::list<double> &maxExt,std::list<double> &curExt)
{
  if (val.begin()==val.end()) {
    p->fillRect(0,0,width,item->height(),
		QBrush(item->isSelected()?cg.highlight():cg.base()));
  } else {
    int num=0;
    int lastHeight=0;

    std::list<double>::iterator j=curExt.begin();
    std::list<double>::iterator k=maxExt.begin();
    for(std::list<double>::iterator i=val.begin();
	i!=val.end()&&j!=curExt.end()&&k!=maxExt.end();
	i++,j++,k++) {
      num++;

      QBrush bg(item->isSelected()?cg.highlight():cg.base());
      QBrush fg(num%2?Qt::blue:Qt::red);

      double start=(*i);
      double end=(*j);

      if (BarsAlignLeft) {
	end-=start;
	start=0;
	if (end<0)
	  end+=(*k);
      }

      int height=item->height()*num/val.size();
      int pos=int(start*width/(*k));
      int posEnd=int(end*width/(*k));

      if (start>end) {
	p->fillRect(0,lastHeight,posEnd,height,fg);
	p->fillRect(posEnd,lastHeight,pos,height,bg);
	p->fillRect(pos,lastHeight,width,height,fg);
      } else {
	p->fillRect(0,lastHeight,pos,height,bg);
	p->fillRect(pos,lastHeight,posEnd,height,fg);
	p->fillRect(posEnd,lastHeight,width,height,bg);
      }
      lastHeight=height;
    }
  }
}

static toSQL SQLRollback("toRollback:Information",
			 "SELECT a.segment_name \"Segment\",\n"
			 "       a.owner \"Owner\",\n"
			 "       a.tablespace_name \"Tablespace\",\n"
			 "       a.status \"Status\",\n"
			 "       b.xacts \"-Transactions\",\n"
			 "       ROUND ( a.initial_extent / :unit<char[100]>,\n"
			 "	       3 ) \"-Initial (MB)\",\n"
			 "       ROUND ( a.next_extent / :unit<char[100]>,\n"
			 "	       3 ) \"-Next (MB)\",\n"
			 "       a.pct_increase \"-PCT Increase\",\n"
			 "       ROUND ( b.rssize / :unit<char[100]>,\n"
			 "	       3 ) \"-Current (MB)\",\n"
			 "       ROUND ( b.optsize / :unit<char[100]>,\n"
			 "	       3 ) \"-Optimal (MB)\",\n"
			 "       ROUND ( b.aveactive / :unit<char[100]>,\n"
			 "	       3 ) \"-Used (MB)\",\n"
			 "       b.EXTENTS \"-Extents\",\n"
			 "       b.CurExt \"-Current\",\n"
			 "       b.CurBlk \"-Block\",\n"
			 "       b.gets \"-Reads\",\n"
			 "       ROUND ( ( b.gets - b.waits ) * 100 / b.gets,\n"
			 "	       2 ) || '%' \"-Hitrate\",\n"
			 "       a.segment_id \" USN\"\n"
			 "  FROM sys.dba_rollback_segs a,\n"
			 "       v$rollstat b\n"
			 " WHERE a.segment_id = b.usn ( + )\n"
			 " ORDER BY a.segment_name",
			 "Get information about rollback segments.");

static toSQL SQLStartExt("toRollback:StartExtent",
			 "select to_char(b.start_uext)\n"
			 "  from v$transaction b\n"
			 " where b.xidusn = :f1<char[40]>",
			 "Get information about current extent in rollback of transactions");

class toRollbackView : public toResultView {
public:
  class rollbackItem : public toResultViewItem {
  public:
    rollbackItem(QListView *parent,QListViewItem *after,const QString &buf=QString::null)
      : toResultViewItem(parent,after,buf)
    { }
    virtual void paintCell (QPainter *pnt,const QColorGroup & cg,
			    int column,int width,int alignment)
    {
      if (column==4) {
	std::list<double> items;
	std::list<double> curExt;
	std::list<double> maxExt;
	for (int i=TRANSCOL;!text(i).isEmpty();i++) {
	  items.insert(items.end(),text(i).toDouble());
	  curExt.insert(curExt.end(),text(12).toDouble());
	  maxExt.insert(maxExt.end(),text(11).toDouble());
	}
	PaintBars(this,pnt,cg,width,items,
		  maxExt,curExt);
	QPen pen(isSelected()?cg.highlightedText():cg.foreground());
	pnt->setPen(pen);
	pnt->drawText(0,0,width,height(),AlignRight,text(column));
      } else
	toResultViewItem::paintCell(pnt,cg,column,width,alignment);
    }

    virtual void setup(void)
    {
      toResultViewItem::setup();

      int i;
      for (i=TRANSCOL;!text(i).isEmpty();i++)
	;
      setHeight(max(toResultViewItem::height(),MIN_HEIGHT*(i-TRANSCOL)));
    }
  };

  virtual QListViewItem *createItem(QListViewItem *last,const QString &str)
  { return new rollbackItem(this,last,str); }

  toRollbackView(QWidget *parent)
    : toResultView(false,false,parent)
  {
    setSQL(SQLRollback);
  }
  virtual void query(const QString &sql,const toQList &)
  {
    QString unit=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
    
    toQList par;
    par.insert(par.end(),QString::number(toSizeDecode(unit)));
    toResultView::query(sql,(const toQList &)par);
    QRegExp repl(QString::fromLatin1("(MB)"));
    QString res=QString::fromLatin1("(");
    res+=unit;
    res+=QString::fromLatin1(")");
    for (int i=0;i<columns();i++) {
      QString str=header()->label(i);
      str.replace(repl,res);
      header()->setLabel(i,str);
    }
    try {
      toQuery trx(connection());
      for(QListViewItem *i=firstChild();i;i=i->nextSibling()) {
	toQList args;
	toPush(args,toQValue(QString(i->text(TRANSCOL-1))));
	trx.execute(SQLStartExt,args);
	for (int j=TRANSCOL;!trx.eof();j++)
	  i->setText(j,trx.readValueNull());
      }
    } TOCATCH
  }
};

static toSQL SQLStatementInfo("toRollback:StatementInfo",
			      "SELECT TO_CHAR(SYSDATE),\n"
			      "       a.User_Name,\n"
			      "       a.SQL_Text,\n"
			      "       a.Address||':'||a.Hash_Value,\n"
			      "       TO_CHAR(SUM(b.Executions)),\n"
			      "       TO_CHAR(SUM(b.Buffer_Gets))\n"
			      "  FROM v$open_cursor a,v$sql b\n"
			      " WHERE a.Address = b.Address AND a.Hash_Value = b.Hash_Value\n"
			      " GROUP BY TO_CHAR(SYSDATE),a.user_name,a.sql_text,a.address||':'||a.hash_value",
			      "Get information about statements in SGA. All columns must "
			      "be in exactly the same order.");
static toSQL SQLCurrentExtent("toRollback:CurrentExtent",
			      "select b.Extents,\n"
			      "       b.CurExt+b.CurBlk/c.Blocks\n"
			      "  from sys.dba_rollback_segs a,v$rollstat b,sys.dba_extents c\n"
			      " where a.segment_id = b.usn\n"
			      "   and a.owner = c.owner\n"
			      "   and a.segment_name = c.segment_name\n"
			      "   and c.segment_type = 'ROLLBACK'\n"
			      "   and b.curext = c.extent_id\n"
			      " order by a.segment_name",
			      "Get current extent (And fraction of) of rollback segments, "
			      "columns must be in exactly the same order");

class toRollbackOpen : public toResultView {
  struct statementData {
    std::list<double> OpenExt;
    QString Opened;
    int Executed;
    int BufferGets;
    int Shown;
  };

  typedef std::map<QString,statementData> statements;
  statements Statements;
  int NumExtents;
  std::list<double> CurExt;
  std::list<double> MaxExt;
public:
  class openItem : public toResultViewItem {
    toRollbackOpen *parent(void)
    { return (toRollbackOpen *)listView(); }
  public:
    openItem(QListView *parent,QListViewItem *after,const QString &buf=QString::null)
      : toResultViewItem(parent,after,buf)
    { }
    virtual void paintCell (QPainter *pnt,const QColorGroup & cg,
			    int column,int width,int alignment)
    {
      if (column==2) {
	QString address=text(4);
	std::list<double> &StartExt=parent()->Statements[address].OpenExt;
	PaintBars(this,pnt,cg,width,StartExt,parent()->MaxExt,parent()->CurExt);
      } else
	toResultViewItem::paintCell(pnt,cg,column,width,alignment);
    }

    virtual int width(const QFontMetrics &fm,const QListView *top,int column) const
    {
      if (column==2)
	return 100;
      else
	return toResultViewItem::width(fm,top,column);
    }

    virtual void setup(void)
    {
      QString address=text(3);
      statementData &data=parent()->Statements[address];
      toResultViewItem::setup();
      setHeight(max(toResultViewItem::height(),int(MIN_HEIGHT*data.OpenExt.size())));
    }
  };

  friend class openItem;

  virtual QListViewItem *createItem(QListViewItem *last,const QString &str)
  { return new openItem(this,last,str); }

  toRollbackOpen(QWidget *parent)
    : toResultView(false,false,parent)
  {
    addColumn(qApp->translate("toRollbackOpen","Started"));
    addColumn(qApp->translate("toRollbackOpen","User"));
    addColumn(qApp->translate("toRollbackOpen","Snapshot"));
    addColumn(qApp->translate("toRollbackOpen","SQL"));
    setSorting(0);
    NumExtents=0;
    setSQLName(QString::fromLatin1("toRollbackOpen"));
  }
  virtual void query(const QString &,const toQList &)
  {
    try {
      clear();
      toConnection &conn=connection();
      toBusy busy;
      toQuery sql(conn,SQLStatementInfo);
      QListViewItem *last=NULL;
      while(!sql.eof()) {
	QListViewItem *item=createItem(last,QString::null);
	last=item;
	item->setText(0,sql.readValue());
	item->setText(1,sql.readValue());
	item->setText(3,sql.readValue());
	item->setText(4,sql.readValue());
	item->setText(5,sql.readValue());
	item->setText(6,sql.readValue());
      }

      toQuery rlb(conn,SQLCurrentExtent);

      CurExt.clear();
      MaxExt.clear();

      int num=0;
      while(!rlb.eof()) {
	MaxExt.insert(MaxExt.end(),rlb.readValue().toDouble());
	CurExt.insert(CurExt.end(),rlb.readValue().toDouble());
	num++;
      }
      statementData data;
      data.Shown=0;
      data.OpenExt=CurExt;
      if (num!=NumExtents) {
	Statements.clear();
	NumExtents=num;
      }

      bool noExec=true;
      bool needRead=true;
      bool needTwo=true;
      if (RollbackTool.config(CONF_NO_EXEC,"Yes").isEmpty())
	noExec=false;
      if (RollbackTool.config(CONF_NEED_READ,"Yes").isEmpty())
	needRead=false;
      if (RollbackTool.config(CONF_NEED_TWO,"Yes").isEmpty())
	needTwo=false;

      std::map<QString,int> Exists;
      for(QListViewItem *i=firstChild();i;) {
	QString address=i->text(4);
	Exists[address]=1;
	statements::iterator j=Statements.find(address);
	int exec=i->text(5).toInt();
	int bufget=i->text(6).toInt();
	if (j==Statements.end()||
	    ((*j).second.Executed!=exec&&noExec)) {
	  data.Opened=i->text(0);
	  data.Executed=exec;
	  data.BufferGets=bufget;
	  Statements[address]=data;
	  QListViewItem *next=i->nextSibling();
	  if (needTwo)
	    delete i;
	  i=next;
	} else if ((*j).second.BufferGets==bufget&&needRead) { // Don't reset, just don't show
	  QListViewItem *next=i->nextSibling();
	  delete i;
	  i=next;
	} else {
	  i->setText(0,(*j).second.Opened);
	  i=i->nextSibling();
	  (*j).second.BufferGets=bufget;
	}
      }
      sort();

      // Erase unused

      for (statements::iterator j=Statements.begin();j!=Statements.end();j++) {
	if (Exists.find((*j).first)==Exists.end()) {
	  Statements.erase((*j).first);
	  j=Statements.begin();
	}
      }
    } TOCATCH
  }
};

toSQL SQLTransactionUsers("toRollback:TransactionUsers",
			  "select r.name \"Rollback Seg\",\n"
			  "       s.sid \"Session ID\",\n"
			  "       s.osuser \"Os User\",\n"
			  "       s.username \"Oracle User\"\n"
			  "from v$rollname r, v$session s, v$lock l\n"
			  "where l.type = 'TX'\n"
			  "and s.sid = l.sid\n"
			  "and r.usn = trunc(l.id1/65536)",
			  "Get users currently having open transactions");

toRollback::toRollback(QWidget *main,toConnection &connection)
  : toToolWidget(RollbackTool,"rollback.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,tr("Rollback analyzer"));

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  tr("Update segment list"),
		  tr("Update segment list"),
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();

  QToolButton *enableOld=new QToolButton(toolbar);
  enableOld->setToggleButton(true);
  enableOld->setIconSet(QIconSet(QPixmap((const char **)torollback_xpm)));
  connect(enableOld,SIGNAL(toggled(bool)),this,SLOT(enableOld(bool)));
  QToolTip::add(enableOld,tr("Enable snapshot too old detection."));
  toolbar->addSeparator();

  OnlineButton=new QToolButton(QPixmap((const char **)online_xpm),
			       tr("Take segment online"),
			       tr("Take segment online"),
			       this,SLOT(online(void)),
			       toolbar);
  OnlineButton->setEnabled(false);
  OfflineButton=new QToolButton(QPixmap((const char **)offline_xpm),
				tr("Take segment offline"),
				tr("Take segment offline"),
				this,SLOT(offline(void)),
				toolbar);
  OfflineButton->setEnabled(false);
  toolbar->addSeparator();

  new QToolButton(QPixmap((const char **)addrollback_xpm),
		  tr("Create new rollback segment"),
		  tr("Create new rollback segment"),
		  this,SLOT(addSegment(void)),
		  toolbar);
  DropButton=new QToolButton(QPixmap((const char **)trash_xpm),
			     tr("Drop segment"),
			     tr("Drop segment"),
			     this,SLOT(dropSegment(void)),
			     toolbar);
  DropButton->setEnabled(false);
  toolbar->addSeparator();

  new QLabel(tr("Refresh")+" ",toolbar,TO_KDE_TOOLBAR_WIDGET);
  connect(Refresh=toRefreshCreate(toolbar,TO_KDE_TOOLBAR_WIDGET),
	  SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));

  toolbar->setStretchableWidget(new QLabel(toolbar,TO_KDE_TOOLBAR_WIDGET));
  new toChangeConnection(toolbar,TO_KDE_TOOLBAR_WIDGET);

  QSplitter *splitter=new QSplitter(Vertical,this);
  Segments=new toRollbackView(splitter);
  Segments->setSelectionMode(QListView::Single);
  connect(Segments,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));

  QTabWidget *tab=new QTabWidget(splitter,"TabWidget");
  TransactionUsers=new toResultLong(false,false,toQuery::Background,tab);
  tab->addTab(TransactionUsers,tr("Transaction Users"));
  TransactionUsers->setSQL(SQLTransactionUsers);

  QSplitter *horsplit=new QSplitter(Horizontal,splitter);
  tab->addTab(horsplit,tr("Open Cursors"));
  
  Statements=new toRollbackOpen(horsplit);
  Statements->setSelectionMode(QListView::Single);
  connect(Statements,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeStatement(QListViewItem *)));
  CurrentStatement=new toSGAStatement(horsplit);

  try {
    connect(timer(),SIGNAL(timeout(void)),this,SLOT(refresh(void)));
    toRefreshParse(timer(),toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH));
  } TOCATCH

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  if (!RollbackTool.config(CONF_OLD_ENABLE,"").isEmpty())
    enableOld->setOn(true);
  else
    Statements->setEnabled(false);

  refresh();

  setFocusProxy(Segments);
}

#define TO_ID_ONLINE		(toMain::TO_TOOL_MENU_ID+ 0)
#define TO_ID_OFFLINE		(toMain::TO_TOOL_MENU_ID+ 1)
#define TO_ID_CREATE		(toMain::TO_TOOL_MENU_ID+ 2)
#define TO_ID_DROP		(toMain::TO_TOOL_MENU_ID+ 3)

void toRollback::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),tr("&Refresh"),
			   this,SLOT(refresh(void)),
			   toKeySequence(tr("F5", "Rollback|Refresh")));
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)online_xpm),tr("Online"),
			   this,SLOT(online(void)),0,TO_ID_ONLINE);
      ToolMenu->insertItem(QPixmap((const char **)offline_xpm),tr("Offline"),
			   this,SLOT(offline(void)),0,TO_ID_OFFLINE);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(QPixmap((const char **)addrollback_xpm),tr("Create segment..."),
			   this,SLOT(addSegment(void)),0,TO_ID_CREATE);
      ToolMenu->insertItem(QPixmap((const char **)trash_xpm),tr("Drop segment"),
			   this,SLOT(dropSegment(void)),0,TO_ID_DROP);
      ToolMenu->insertSeparator();
      ToolMenu->insertItem(tr("&Change Refresh"),Refresh,SLOT(setFocus(void)),
			   toKeySequence(tr("Alt+R", "Rollback|Change refresh")));
      toMainWidget()->menuBar()->insertItem(tr("&Rollback"),ToolMenu,-1,toToolMenuIndex());
      ToolMenu->setItemEnabled(TO_ID_ONLINE,OnlineButton->isEnabled());
      ToolMenu->setItemEnabled(TO_ID_OFFLINE,OfflineButton->isEnabled());
      ToolMenu->setItemEnabled(TO_ID_DROP,DropButton->isEnabled());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toRollback::refresh(void)
{
  BarsAlignLeft=!RollbackTool.config(CONF_ALIGN_LEFT,"Yes").isEmpty();

  QListViewItem *item=Segments->selectedItem();
  QString current;
  if (item)
    current=item->text(TRANSCOL-1);
  Segments->refresh();
  if (!current.isEmpty())
    for (item=Segments->firstChild();item;item=item->nextSibling())
      if (item->text(TRANSCOL-1)==current) {
	Segments->setSelected(item,true);
	break;
      }
  if (Statements->isEnabled()) {
    item=Statements->selectedItem();
    if (item)
      current=item->text(4);
    else
      current="";
    Statements->refresh();
    if (!current.isEmpty())
      for (item=Statements->firstChild();item;item=item->nextSibling())
	if (item->text(4)==current) {
	  Statements->setSelected(item,true);
	  break;
	}
  }
  TransactionUsers->refresh();
}

void toRollback::changeStatement(QListViewItem *item)
{
  if (item)
    CurrentStatement->changeAddress(item->text(4));
}

void toRollback::changeItem(QListViewItem *item)
{
  if (item) {
    if (item->text(3)==QString::fromLatin1("OFFLINE"))
      OfflineButton->setEnabled(false);
    else
      OfflineButton->setEnabled(true);
    if (item->text(3)==QString::fromLatin1("ONLINE"))
      OnlineButton->setEnabled(false);
    else
      OnlineButton->setEnabled(true);
  } else {
    OnlineButton->setEnabled(false);
    OfflineButton->setEnabled(false);
  }
  DropButton->setEnabled(item);
  ToolMenu->setItemEnabled(TO_ID_ONLINE,OnlineButton->isEnabled());
  ToolMenu->setItemEnabled(TO_ID_OFFLINE,OfflineButton->isEnabled());
  ToolMenu->setItemEnabled(TO_ID_DROP,DropButton->isEnabled());
}

void toRollback::changeRefresh(const QString &str)
{
  try {
    toRefreshParse(timer(),str);
  } TOCATCH
}

QString toRollback::currentSegment(void)
{
  QListViewItem *item=Segments->selectedItem();
  if (!item)
    throw tr("No segment selected");
  return item->text(0);
}

void toRollback::addSegment(void)
{
  try {
    toRollbackDialog newSegment(connection(),this);
    if (newSegment.exec()) {
      std::list<QString> sql=newSegment.sql();
      for(std::list<QString>::iterator i=sql.begin();i!=sql.end();i++)
	connection().execute(*i);
      refresh();
    }
  } TOCATCH
}

void toRollback::offline(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER ROLLBACK SEGMENT \"");
    str.append(currentSegment());
    str.append(QString::fromLatin1("\" OFFLINE"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toRollback::dropSegment(void)
{
  try {
    if (TOMessageBox::warning(this,
			      tr("Drop rollback segment"),
			      tr("Are you sure you want to drop the segment %1.").arg(currentSegment()),
			      tr("&Drop"),
			      tr("Cancel"))==0) {
      QString str;
      str=QString::fromLatin1("DROP ROLLBACK SEGMENT \"");
      str.append(currentSegment());
      str.append(QString::fromLatin1("\""));
      connection().execute(str);
      refresh();
    }
  } TOCATCH
}

void toRollback::online(void)
{
  try {
    QString str;
    str=QString::fromLatin1("ALTER ROLLBACK SEGMENT \"");
    str.append(currentSegment());
    str.append(QString::fromLatin1("\" ONLINE"));
    connection().execute(str);
    refresh();
  } TOCATCH
}

void toRollback::enableOld(bool ena)
{
  Statements->setEnabled(ena);
}
