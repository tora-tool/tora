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

#include <qlineedit.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qsplitter.h>
#include <qworkspace.h>
#include <qpopupmenu.h>
#include <qmenubar.h>

#ifdef TO_KDE
#  include <kmenubar.h>
#endif

#include "totool.h"
#include "tohighlightedtext.h"
#include "toresultview.h"
#include "toresultextract.h"
#include "tosql.h"
#include "toconnection.h"
#include "toinvalid.h"
#include "tochangeconnection.h"
#include "toextract.h"

#include "toinvalid.moc"

#include "icons/refresh.xpm"
#include "icons/toinvalid.xpm"

static toSQL SQLListInvalid("toInvalid:ListInvalid",
			    "SELECT owner \"Owner\",object_name \"Object\",object_type \"Type\"\n"
			    "  FROM sys.all_objects WHERE status = 'INVALID'",
			    "Get invalid objects, must have same first three columns.");

static toSQL SQLListSource("toInvalid:ListSource",
			   "SELECT Text FROM SYS.ALL_SOURCE\n"
			   " WHERE Owner = :f1<char[101]> AND Name = :f2<char[101]> AND type = :f3<char[101]>",
			   "List source of an object.");

static toSQL SQLReadErrors("toInvalid:ReadErrors",
			   "SELECT Line-1,Text FROM sys.All_Errors\n"
			   " WHERE OWNER = :f1<char[101]>\n"
			   "   AND NAME = :f2<char[101]>\n"
			   "   AND TYPE = :f3<char[101]>",
			   " ORDER BY Type,Line",
			   "Get lines with errors in object (Observe first line 0)");

class toInvalidTool : public toTool {
  virtual char **pictureXPM(void)
  { return toinvalid_xpm; }
public:
  toInvalidTool()
    : toTool(202,"Invalid Objects")
  { }
  virtual const char *menuItem()
  { return "Invalid Objects"; }
  virtual QWidget *toolWindow(QWidget *parent,toConnection &connection)
  {
    return new toInvalid(parent,connection);
  }
};

static toInvalidTool InvalidTool;

toInvalid::toInvalid(QWidget *main,toConnection &connection)
  : toToolWidget(InvalidTool,"invalid.html",main,connection)
{
  QToolBar *toolbar=toAllocBar(this,"Invalid Objects",connection.description());

  new QToolButton(QPixmap((const char **)refresh_xpm),
		  "Refresh list",
		  "Refresh list",
		  this,SLOT(refresh()),
		  toolbar);

  toolbar->setStretchableWidget(new QLabel(toolbar));
  new toChangeConnection(toolbar);

  QSplitter *splitter=new QSplitter(Horizontal,this);

  Objects=new toResultView(false,false,splitter);
  Objects->setSQL(SQLListInvalid);

  connect(Objects,SIGNAL(selectionChanged()),this,SLOT(changeSelection()));

  Source=new toResultExtract(false,splitter);
  Source->setSQL(SQLListSource);

  connect(Source,SIGNAL(executed()),this,SLOT(refresh()));

  {
    QValueList<int> sizes=splitter->sizes();
    sizes[0]=300;
    splitter->setSizes(sizes);
    splitter->setResizeMode(Objects,QSplitter::KeepSize);
  }

  ToolMenu=NULL;
  connect(toMainWidget()->workspace(),SIGNAL(windowActivated(QWidget *)),
	  this,SLOT(windowActivated(QWidget *)));

  refresh();
}

void toInvalid::windowActivated(QWidget *widget)
{
  if (widget==this) {
    if (!ToolMenu) {
      ToolMenu=new QPopupMenu(this);
      ToolMenu->insertItem(QPixmap((const char **)refresh_xpm),"&Refresh",
			   this,SLOT(refresh(void)),Key_F5);

      toMainWidget()->menuBar()->insertItem("&Invalid",ToolMenu,-1,toToolMenuIndex());
    }
  } else {
    delete ToolMenu;
    ToolMenu=NULL;
  }
}

void toInvalid::refresh(void)
{
  QListViewItem *item=Objects->selectedItem();
  QString owner;
  QString object;
  QString type;
  if (item) {
    owner=item->text(0);
    object=item->text(1);
    type=item->text(2);
  }
  Objects->refresh();
  if (item) {
    for (item=Objects->firstChild();item;item=item->nextSibling()) {
      if (item->text(0)==owner&&item->text(1)==object&&item->text(2)==type) {
	Objects->setSelected(item,true);
	break;
      }
    }
    changeSelection();
  }
}

void toInvalid::changeSelection(void)
{
  QListViewItem *item=Objects->selectedItem();
  if (item) {
    Source->changeParams(item->text(0),item->text(1),item->text(2));
    std::map<int,QString> Errors;

    toQuery errors(connection(),SQLReadErrors,item->text(0),item->text(1),item->text(2));
    while(!errors.eof()) {
      int line=errors.readValue().toInt();
      Errors[line]+=" ";
      Errors[line]+=errors.readValue();
    }
    Source->editor()->setErrors(Errors);
  }
}
