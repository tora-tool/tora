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

#include "toeditwidget.h"
#include "tomain.h"

std::list<toEditWidget::editHandler *> toEditWidget::Handlers;

void toEditWidget::setMainSettings(void)
{
  toMain::editEnable(this);
}

toEditWidget::toEditWidget()
{
  Open=Save=Print=Undo=Redo=Cut=Copy=Paste=Search=SelectAll=ReadAll=false;
}

toEditWidget::toEditWidget(bool open,bool save,bool print,
			   bool undo,bool redo,
			   bool cut,bool copy,bool paste,
			   bool search,
			   bool selectAll,bool readAll)
{
  Open=open;
  Save=save;
  Print=print;
  Undo=undo;
  Redo=redo;
  Cut=cut;
  Copy=copy;
  Paste=paste;
  Search=search;
  SelectAll=selectAll;
  ReadAll=readAll;
}

void toEditWidget::setEdit(bool open,bool save,bool print,
			   bool undo,bool redo,
			   bool cut,bool copy,bool paste,
			   bool search,
			   bool selectAll,bool readAll)
{
  Open=open;
  Save=save;
  Print=print;
  Undo=undo;
  Redo=redo;
  Cut=cut;
  Copy=copy;
  Paste=paste;
  Search=search;
  SelectAll=selectAll;
  ReadAll=readAll;

  setMainSettings();
}

toEditWidget::~toEditWidget()
{
  toMain::editDisable(this);
}

void toEditWidget::lostFocus(void)
{
  for(std::list<editHandler *>::iterator i=Handlers.begin();i!=Handlers.end();i++) {
    (*i)->lostFocus(dynamic_cast<QWidget *>(this));
  }
}

void toEditWidget::receivedFocus(void)
{
  toMain::setEditWidget(this);
  for(std::list<editHandler *>::iterator i=Handlers.begin();i!=Handlers.end();i++) {
    (*i)->receivedFocus(dynamic_cast<QWidget *>(this));
  }
}
