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


#ifndef __TOMARKEDTEXT_H
#define __TOMARKEDTEXT_H

#include <qmultilineedit.h>
#include <qstring.h>

class toMarkedText : public QMultiLineEdit {
  Q_OBJECT

  bool RedoAvailable;
  bool UndoAvailable;

  QString Filename;
public:
  toMarkedText(QWidget *parent,const char *name=NULL);
  ~toMarkedText();

  QString markedText()
  { return QMultiLineEdit::markedText(); }
  bool hasMarkedText()
  { return QMultiLineEdit::hasMarkedText(); }
  virtual void keyPressEvent(QKeyEvent *e);
  void clear(void)
  { Filename=""; RedoAvailable=false; UndoAvailable=false; setEdit(); QMultiLineEdit::clear(); setEdited(false); }

  bool getRedoAvailable(void)
  { return RedoAvailable; }
  bool getUndoAvailable(void)
  { return UndoAvailable; }
  bool getMarkedRegion (int * line1,int * col1,int * line2,int * col2) const
  { return QMultiLineEdit::getMarkedRegion(line1,col1,line2,col2); }

  virtual const QString &filename(void) const
  { return Filename; }
  virtual void setFilename(const QString &str)
  { Filename=str; }
  virtual void setEdit(void);
  virtual void focusInEvent (QFocusEvent *e);
  virtual void focusOutEvent (QFocusEvent *e); 

signals:
  void execute(void);

private slots:
  void setRedoAvailable(bool avail)
  { RedoAvailable=avail; setEdit(); }
  void setUndoAvailable(bool avail)
  { UndoAvailable=avail; setEdit(); }
  void setCopyAvailable(bool avail)
  { setEdit(); }
};

#endif
