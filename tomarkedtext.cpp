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



#include "tomarkedtext.h"
#include "tomain.h"
#include "totool.h"
#include "toconf.h"

#include "tomarkedtext.moc"

toMarkedText::toMarkedText(QWidget *parent,const char *name)
: QMultiLineEdit(parent,name)
{
  QString font=toTool::globalConfig(CONF_TEXT,"");
  if (!font.isEmpty()) {
    QFont fnt;
    fnt.setRawName(font);
    setFont(fnt);
  } else {
    setFont( QFont( "Courier", 12, QFont::Bold ) );
  }
  UndoAvailable=false;
  RedoAvailable=false;
  connect(this,SIGNAL(redoAvailable(bool)),this,SLOT(setRedoAvailable(bool)));
  connect(this,SIGNAL(undoAvailable(bool)),this,SLOT(setUndoAvailable(bool)));
  connect(this,SIGNAL(copyAvailable(bool)),this,SLOT(setCopyAvailable(bool)));
}

void toMarkedText::keyPressEvent(QKeyEvent *e)
{
  if (e->key()==Key_Return&&e->state()==ControlButton) {
    emit execute();
    e->accept();
  } else
    QMultiLineEdit::keyPressEvent(e);
}

void toMarkedText::setEdit(void)
{
  if (isReadOnly()) {
    toMain::editEnable(false,true,
		       false,false,
		       hasMarkedText(),hasMarkedText(),false);
  } else {
    toMain::editEnable(true,true,
		       getUndoAvailable(),getRedoAvailable(),
		       hasMarkedText(),hasMarkedText(),true);
  }
}

void toMarkedText::focusInEvent (QFocusEvent *e)
{
  setEdit();
  QMultiLineEdit::focusInEvent(e);
}

void toMarkedText::focusOutEvent (QFocusEvent *e)
{
  toMain::editDisable();
  QMultiLineEdit::focusOutEvent(e);
}

toMarkedText::~toMarkedText()
{
  if (qApp->focusWidget()==this)
    toMain::editDisable();
}
