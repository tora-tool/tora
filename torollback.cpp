//***************************************************************************
/*
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

TO_NAMESPACE;

#include <list>

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qregexp.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qsplitter.h>
#include <qtabwidget.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qpainter.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qvariant.h>
#include <qwhatsthis.h>
#include <qwidget.h>
#include <qworkspace.h>
#include <qheader.h>

#include "tomain.h"
#include "totool.h"
#include "torollback.h"
#include "toresultview.h"
#include "toresultitem.h"
#include "toconf.h"
#include "tosql.h"
#include "tostoragedefinition.h"
#include "tosgastatement.h"
#include "tohelp.h"

#include "torollback.moc"

#include "icons/torollback.xpm"
#include "icons/trash.xpm"
#include "icons/addrollback.xpm"
#include "icons/online.xpm"
#include "icons/offline.xpm"
#include "icons/refresh.xpm"

#define CONF_NO_EXEC    "NoExec"
#define CONF_NEED_READ  "NeedRead"
#define CONF_NEED_TWO   "NeedTwo"
#define CONF_ALIGN_LEFT "AlignLeft"

class toRollbackPrefs : public QFrame,public toSettingTab
{ 
  QGroupBox *GroupBox1;
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
  : QFrame(parent,name),toSettingTab("unknown.html"),Tool(tool)
{
  GroupBox1=new QGroupBox(this,"GroupBox1");
  GroupBox1->setGeometry(QRect(10,10,380,380)); 
  GroupBox1->setTitle(tr("Rollback Tool" ));
  
  NoExec=new QCheckBox(GroupBox1,"NoCopy");
  NoExec->setGeometry(QRect(20,30,340,20)); 
  NoExec->setText(tr("&Restart reexecuted statements" ));
  QToolTip::add(NoExec,tr("Start statements again that have been reexecuted."));
  
  NeedRead=new QCheckBox(GroupBox1,"Needread");
  NeedRead->setGeometry(QRect(20,70,340,20)); 
  NeedRead->setText(tr("&Must read buffers" ));
  QToolTip::add(NeedRead,tr("Don't display statements that have not read buffers."));
  
  NeedTwo=new QCheckBox(GroupBox1,"NeedTwo");
  NeedTwo->setGeometry(QRect(20,110,340,20)); 
  NeedTwo->setText(tr("&Exclude first appearance" ));
  QToolTip::add(NeedTwo,tr("A statement must be visible at least two consecutive polls to be displayed."));
  
  AlignLeft=new QCheckBox(GroupBox1,"AlignLeft");
  AlignLeft->setGeometry(QRect(20,150,340,20)); 
  AlignLeft->setText(tr("&Disregard start extent." ));
  QToolTip::add(AlignLeft,tr("Always start from the left border when displaying extent usage."));
  
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
}

class toRollbackTool : public toTool {
protected:
  virtual char **pictureXPM(void)
  { return torollback_xpm; }
public:
  toRollbackTool()
    : toTool(102,"Rollback Segments")
  { }
  virtual const char *menuItem()
  { return "Rollback Segments"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    QWidget *window=new toRollback(parent,connection);
    window->setIcon(*toolbarImage());
    return window;
  }
  virtual QWidget *configurationTab(QWidget *parent)
  {
    return new toRollbackPrefs(this,parent);
  }
};

static toRollbackTool RollbackTool;

static toSQL SQLTablespace("toRollbackDialog:TablespaceName",
			   "select tablespace_name from dba_tablespaces order by tablespace_name",
			   "Get a list of tablespace names, should only have one column and same binds");

toRollbackDialog::toRollbackDialog(toConnection &Connection,QWidget* parent,const char* name)
  : QDialog(parent,name,true)
{
  toHelp::connectDialog(this);
  if (!name)
    setName("Form1");
  resize(470,500); 
  setMinimumSize(QSize(470,500));
  setMaximumSize(QSize(470,500));
  setCaption(tr("Form1"));
  
  DialogTabs=new QTabWidget(this,"DialogTabs");
  DialogTabs->setGeometry(QRect(10,10,330,480)); 

  RollbackInfo=new QWidget(DialogTabs,"RollbackInfo");
  
  Name=new QLineEdit(RollbackInfo,"Name");
  Name->setGeometry(QRect(10,30,310,23)); 
  Name->setFocus();
  connect(Name,SIGNAL(textChanged(const QString &)),
	  this,SLOT(valueChanged(const QString &)));
  
  TextLabel1=new QLabel(RollbackInfo,"TextLabel1");
  TextLabel1->setGeometry(QRect(10,10,310,16)); 
  TextLabel1->setText(tr("&Name"));
  QToolTip::add(TextLabel1,tr("Name of this rollback segment."));
  
  Tablespace=new QComboBox(false,RollbackInfo,"Tablespace");
  Tablespace->setGeometry(QRect(10,90,310,27)); 
  Tablespace->setEditable(false);
  DialogTabs->addTab(RollbackInfo,tr("Rollback Segment"));

  try {
    otl_stream q(1,
		 SQLTablespace(Connection),
		 Connection.connection());
    char buf[100];
    while(!q.eof()) {
      q>>buf;
      Tablespace->insertItem(QString::fromUtf8(buf));
    }
  } catch (...) {

  }
  connect(Tablespace,SIGNAL(textChanged(const QString &)),
	  this,SLOT(valueChanged(const QString &)));
  
  Public=new QCheckBox(RollbackInfo,"CheckBox1");
  Public->setGeometry(QRect(10,130,310,19)); 
  Public->setText(tr("&Public"));
  Public->setChecked(true);
  QToolTip::add(Public,tr("If this should be a publicly available rollback segment."));
  
  TextLabel1_2=new QLabel(RollbackInfo,"TextLabel1_2");
  TextLabel1_2->setGeometry(QRect(10,70,310,16)); 
  TextLabel1_2->setText(tr("&Tablespace"));
  QToolTip::add(TextLabel1_2,tr("Name of this rollback segment."));
  
  // buddies
  TextLabel1->setBuddy(Name);
  TextLabel1_2->setBuddy(Tablespace);

  Storage=new toStorageDefinition(DialogTabs);
  Storage->forRollback(true);
  DialogTabs->addTab(Storage,"Storage");

  OkButton=new QPushButton(this,"OkButton");
  OkButton->move(350,40); 
  OkButton->setText(tr("&OK"));
  OkButton->setDefault(true);

  QPushButton *CancelButton=new QPushButton(this,"CancelButton");
  CancelButton->move(350,90); 
  CancelButton->setText(tr("Cancel" ));
  CancelButton->setDefault(false);
  
  connect(OkButton,SIGNAL(clicked()),this,SLOT(accept()));
  connect(CancelButton,SIGNAL(clicked()),this,SLOT(reject()));

  OkButton->setEnabled(false);
}

QString toRollbackDialog::getSQL(void)
{
  QString str("CREATE ");
  if (Public->isChecked())
    str.append("PUBLIC ");
  str.append("ROLLBACK SEGMENT \"");
  str.append(Name->text());
  str.append("\" TABLESPACE \"");
  str.append(Tablespace->currentText());
  str.append("\" ");
  str.append(Storage->getSQL());
  return str;
}

void toRollbackDialog::valueChanged(const QString &str)
{
  if (Name->text().isEmpty()||
      Tablespace->currentText().isEmpty())
    OkButton->setEnabled(false);
  else
    OkButton->setEnabled(true);
}

#define MIN_HEIGHT 4
#define TRANSCOL 16

static bool BarsAlignLeft=true;

static void PaintBars(QListViewItem *item,QPainter *p,const QColorGroup & cg,
		      int width,list<double> &val,list<double> &maxExt,list<double> &curExt)
{
  if (val.begin()==val.end()) {
    p->fillRect(0,0,width,item->height(),
		QBrush(item->isSelected()?cg.highlight():cg.base()));
  } else {
    int num=0;
    int lastHeight=0;

    list<double>::iterator j=curExt.begin();
    list<double>::iterator k=maxExt.begin();
    for(list<double>::iterator i=val.begin();
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
			 "select a.segment_name \"Segment\",\n"
			 "       a.owner \"Owner\",\n"
			 "       a.tablespace_name \"Tablespace\",\n"
			 "       a.status \"Status\",\n"
			 "       b.xacts \"-Transactions\",\n"
			 "       ROUND(a.initial_extent/unit,3) \"-Initial (MB)\",\n"
			 "       ROUND(a.next_extent/unit,3) \"-Next (MB)\",\n"
			 "       a.pct_increase \"-PCT Increase\",\n"
			 "       ROUND(b.rssize/unit,3) \"-Current (MB)\",\n"
			 "       ROUND(b.optsize/unit,3) \"-Optimal (MB)\",\n"
			 "       ROUND(b.aveactive/unit,3) \"-Used (MB)\",\n"
			 "       b.Extents \"-Extents\",\n"
			 "       b.CurExt \"-Current\",\n"
			 "       b.CurBlk \"-Block\",\n"
			 "       c.Blocks \"-Blocks\",\n"
			 "       a.segment_id \" USN\"\n"
			 "  from dba_rollback_segs a,\n"
			 "       v$rollstat b,\n"
			 "       dba_extents c,\n"
			 "       (select :unit<char[100]> unit from dual) d\n"
			 " where a.segment_id = b.usn(+)\n"
			 "   and a.owner = c.owner\n"
			 "   and a.segment_name = c.segment_name\n"
			 "   and c.segment_type = 'ROLLBACK'\n"
			 "   and (c.extent_id = b.CurExt or (b.curext is null and c.extent_id = 0))\n"
			 " order by a.segment_name",
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
	list<double> items;
	list<double> curExt;
	list<double> maxExt;
	for (int i=TRANSCOL;!text(i).isEmpty();i++) {
	  items.insert(items.end(),text(i).toDouble());
	  curExt.insert(curExt.end(),text(12).toDouble()+text(13).toDouble()/text(14).toDouble());
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

  toRollbackView(toConnection &conn,QWidget *parent)
    : toResultView(false,false,conn,parent)
  {
    setSQL(SQLRollback);
  }
  virtual void query(const QString &sql,const list<QString> &param)
  {
    QString unit=toTool::globalConfig(CONF_SIZE_UNIT,DEFAULT_SIZE_UNIT);
    
    list<QString> par;
    par.insert(par.end(),QString::number(toSizeDecode(unit)));
    toResultView::query(sql,par);
    QRegExp repl("(MB)");
    QString res="(";
    res+=unit;
    res+=")";
    for (int i=0;i<columns();i++) {
      QString str=header()->label(i);
      str.replace(repl,res);
      header()->setLabel(i,str);
    }
    try {
      otl_stream trx(1,
		     SQLStartExt(Connection),
		     Connection.connection());
      for(QListViewItem *i=firstChild();i;i=i->nextSibling()) {
	trx<<i->text(TRANSCOL-1).utf8();
	for (int j=TRANSCOL;!trx.eof();j++) {
	  char buffer[50];
	  trx>>buffer;
	  i->setText(j,QString::fromUtf8(buffer));
	}
      }
    } TOCATCH
  }
};

static toSQL SQLStatementInfo("toRollback:StatementInfo",
			      "SELECT TO_CHAR(SYSDATE),\n"
			      "       a.User_Name,\n"
			      "       a.SQL_Text,\n"
			      "       a.Address||':'||a.Hash_Value,\n"
			      "       TO_CHAR(b.Executions),\n"
			      "       TO_CHAR(b.Buffer_Gets)\n"
			      "  FROM v$open_cursor a,v$sql b\n"
			      " WHERE a.Address = b.Address AND a.Hash_Value = b.Hash_Value",	
			      "Get information about statements in SGA. All columns must "
			      "be in exactly the same order.");
static toSQL SQLCurrentExtent("toRollback:CurrentExtent",
			      "select b.Extents,\n"
			      "       b.CurExt+b.CurBlk/c.Blocks\n"
			      "  from dba_rollback_segs a,v$rollstat b,dba_extents c\n"
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
    list<double> OpenExt;
    QString Opened;
    int Executed;
    int BufferGets;
    int Shown;
  };

  typedef map<QString,statementData> statements;
  statements Statements;
  int NumExtents;
  list<double> CurExt;
  list<double> MaxExt;
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
	list<double> &StartExt=parent()->Statements[address].OpenExt;
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

  friend openItem;

  virtual QListViewItem *createItem(QListViewItem *last,const QString &str)
  { return new openItem(this,last,str); }

  toRollbackOpen(toConnection &conn,QWidget *parent)
    : toResultView(false,false,conn,parent)
  {
    addColumn("Started");
    addColumn("User");
    addColumn("Snapshot");
    addColumn("SQL");
    setSorting(0);
    NumExtents=0;
    setSQLName("toRollbackOpen");
  }
  virtual void query(const QString &sql,const list<QString> &param)
  {
    try {
      clear();
      otl_stream sql(1,
		     SQLStatementInfo(Connection),
		     Connection.connection());
      QListViewItem *last=NULL;
      while(!sql.eof()) {
	char buffer[1024];
	QListViewItem *item=createItem(last,NULL);
	last=item;
	sql>>buffer;
	item->setText(0,QString::fromUtf8(buffer));
	sql>>buffer;
	item->setText(1,QString::fromUtf8(buffer));
	sql>>buffer;
	item->setText(3,QString::fromUtf8(buffer));
	sql>>buffer;
	item->setText(4,QString::fromUtf8(buffer));
	sql>>buffer;
	item->setText(5,QString::fromUtf8(buffer));
	sql>>buffer;
	item->setText(6,QString::fromUtf8(buffer));
      }

      otl_stream rlb(1,
		     SQLCurrentExtent(Connection),
		     Connection.connection());

      CurExt.clear();
      MaxExt.clear();

      int num=0;
      while(!rlb.eof()) {
	double ext;
	rlb>>ext;
	MaxExt.insert(MaxExt.end(),ext);
	rlb>>ext;
	CurExt.insert(CurExt.end(),ext);
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

      map<QString,int> Exists;
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

static QPixmap *toRefreshPixmap;
static QPixmap *toAddPixmap;
static QPixmap *toTrashPixmap;
static QPixmap *toOnlinePixmap;
static QPixmap *toOfflinePixmap;

toRollback::toRollback(QWidget *main,toConnection &connection)
  : toToolWidget("rollback.html",main,connection)
{
  if (!toRefreshPixmap)
    toRefreshPixmap=new QPixmap((const char **)refresh_xpm);
  if (!toAddPixmap)
    toAddPixmap=new QPixmap((const char **)addrollback_xpm);
  if (!toTrashPixmap)
    toTrashPixmap=new QPixmap((const char **)trash_xpm);
  if (!toOnlinePixmap)
    toOnlinePixmap=new QPixmap((const char **)online_xpm);
  if (!toOfflinePixmap)
    toOfflinePixmap=new QPixmap((const char **)offline_xpm);

  QToolBar *toolbar=toAllocBar(this,"Rollback analyzer",connection.connectString());

  new QToolButton(*toRefreshPixmap,
		  "Update segment list",
		  "Update segment list",
		  this,SLOT(refresh(void)),
		  toolbar);
  toolbar->addSeparator();

  OnlineButton=new QToolButton(*toOnlinePixmap,
			       "Take segment online",
			       "Take segment online",
			       this,SLOT(online(void)),
			       toolbar);
  OnlineButton->setEnabled(false);
  OfflineButton=new QToolButton(*toOfflinePixmap,
				"Take segment offline",
				"Take segment offline",
				this,SLOT(offline(void)),
				toolbar);
  OfflineButton->setEnabled(false);
  toolbar->addSeparator();

  new QToolButton(*toAddPixmap,
		  "Create new rollback segment",
		  "Create new rollback segment",
		  this,SLOT(addSegment(void)),
		  toolbar);
  DropButton=new QToolButton(*toTrashPixmap,
			     "Drop segment",
			     "Drop segment",
			     this,SLOT(dropSegment(void)),
			     toolbar);
  DropButton->setEnabled(false);
  toolbar->addSeparator();

  new QLabel("Refresh",toolbar);
  connect(toRefreshCreate(toolbar),SIGNAL(activated(const QString &)),this,SLOT(changeRefresh(const QString &)));

  toolbar->setStretchableWidget(new QLabel("",toolbar));

  QSplitter *splitter=new QSplitter(Vertical,this);
  Segments=new toRollbackView(connection,splitter);
  connect(Segments,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeItem(QListViewItem *)));

  QTabWidget *tab=new QTabWidget(splitter,"TabWidget");
  QSplitter *horsplit=new QSplitter(Horizontal,splitter);
  tab->addTab(horsplit,"Open Cursors");
  
  Statements=new toRollbackOpen(connection,horsplit);
  connect(Statements,SIGNAL(selectionChanged(QListViewItem *)),
	  this,SLOT(changeStatement(QListViewItem *)));
  CurrentStatement=new toSGAStatement(horsplit,connection);

  Timer=new QTimer(this);
  connect(Timer,SIGNAL(timeout(void)),this,SLOT(refresh(void)));
  toRefreshParse(Timer,toTool::globalConfig(CONF_REFRESH,DEFAULT_REFRESH));

  refresh();
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

void toRollback::changeStatement(QListViewItem *item)
{
  if (item)
    CurrentStatement->changeAddress(item->text(4));
}

void toRollback::changeItem(QListViewItem *item)
{
  if (item) {
    if (item->text(3)=="OFFLINE")
      OfflineButton->setEnabled(false);
    else
      OfflineButton->setEnabled(true);
    if (item->text(3)=="ONLINE") {
      OnlineButton->setEnabled(false);
      DropButton->setEnabled(false);
    } else {
      OnlineButton->setEnabled(true);
      DropButton->setEnabled(true);
    }
  } else {
    OnlineButton->setEnabled(false);
    OfflineButton->setEnabled(false);
  }
  DropButton->setEnabled(item);
}

void toRollback::changeRefresh(const QString &str)
{
  toRefreshParse(Timer,str);
}

QString toRollback::currentSegment(void)
{
  QListViewItem *item=Segments->selectedItem();
  if (!item)
    throw QString("No segment selected");
  return item->text(0);
}

void toRollback::addSegment(void)
{
  toRollbackDialog newSegment(connection(),this);
  if (newSegment.exec()) {
    try {
      otl_cursor::direct_exec(otlConnect(),
			      newSegment.getSQL().utf8());
      refresh();
    } TOCATCH
  }
}

void toRollback::offline(void)
{
  try {
    QString str;
    str="ALTER ROLLBACK SEGMENT \"";
    str.append(currentSegment());
    str.append("\" OFFLINE");
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
    refresh();
  } TOCATCH
}

void toRollback::dropSegment(void)
{
  try {
    QString str;
    str="DROP ROLLBACK SEGMENT \"";
    str.append(currentSegment());
    str.append("\"");
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
    refresh();
  } TOCATCH
}

void toRollback::online(void)
{
  try {
    QString str;
    str="ALTER ROLLBACK SEGMENT \"";
    str.append(currentSegment());
    str.append("\" ONLINE");
    otl_cursor::direct_exec(otlConnect(),
			    str.utf8());
    refresh();
  } TOCATCH
}
