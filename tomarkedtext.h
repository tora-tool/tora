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

#ifndef __TOMARKEDTEXT_H
#define __TOMARKEDTEXT_H

#include <qglobal.h>

#include "tomain.h"
#include "toeditwidget.h"

#if QT_VERSION < 300
#include <qmultilineedit.h>
#include "tomarkedtext.2.h"
#else
#include "qtlegacy/qttableview.h"
#include "qtlegacy/qtmultilineedit.h"
#include "tomarkedtext.3.h"
#endif
#include <qstring.h>

class TOPrinter;

/** This is the enhanced editor used in TOra. It mainly offers integration in the TOra
 * menues and printsupport in addition to normal QMultiLineEdit.
 */

class toMarkedText : public toMultiLineEdit, public toEditWidget {
  Q_OBJECT

  /** Filename of the file in this buffer.
   */
  QString Filename;
  /** Print one page to printer.
   * @param printer Printer to print to.
   * @param painter Painter to print to.
   * @param line Line at top of page.
   * @param offset Where one the drawn result this line starts.
   * @param pageNo Pagenumber.
   * @param paint Wether to paint or just test.
   */
  virtual int printPage(TOPrinter *printer,QPainter *painter,int line,int &offset,
			int pageNo,bool paint=true);
protected:
  /** Reimplemented for internal reasons.
   */
  virtual void keyPressEvent(QKeyEvent *e);
public:
  /** Create an editor.
   * @param parent Parent of this widget.
   * @param name Name of this widget.
   */
  toMarkedText(QWidget *parent,const char *name=NULL);

  /** Get selected text. This function is now public.
   * @return The selected text.
   */
  QString markedText()
  { return toMultiLineEdit::markedText(); }
  /** Check if selection is available. This function is now public.
   * @return True if selection is available.
   */
  bool hasMarkedText()
  { return toMultiLineEdit::hasMarkedText(); }
  /** Erase the contents of the editor.
   */
  void clear(void)
  { Filename=""; redoEnabled(false); undoEnabled(false); setEdit(); toMultiLineEdit::clear(); setEdited(false); }

  /** Get location of the current selection. This function is now public. See the
   * Qt documentation for more information.
   */
  bool getMarkedRegion (int * line1,int * col1,int * line2,int * col2) const
  { return toMultiLineEdit::getMarkedRegion(line1,col1,line2,col2); }

  /** Get filename of current file in editor.
   * @return Filename of editor.
   */
  virtual QString filename(void) const
  { return Filename; }
  /** Set the current filename of the file in editor.
   * @param str String containing filename.
   */
  virtual void setFilename(const QString &str)
  { Filename=str; }
  /** Update user interface with availability of copy/paste etc.
   */
  virtual void setEdit(void);
  /** Reimplemented for internal reasons.
   */
  virtual void focusInEvent (QFocusEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void paintEvent(QPaintEvent *pe);
  /** Print this editor.
   */
  virtual void editPrint(void);
  /** Reimplemented for internal reasons.
   */
  virtual void editOpen(void);
  /** Reimplemented for internal reasons.
   */
  virtual void editSave(bool ask);
  /** Reimplemented for internal reasons.
   */
  virtual void editUndo(void)
  { undo(); }
  /** Reimplemented for internal reasons.
   */
  virtual void editRedo(void)
  { redo(); }
  /** Reimplemented for internal reasons.
   */
  virtual void editCut(void)
  { cut(); }
  /** Reimplemented for internal reasons.
   */
  virtual void editCopy(void)
  { copy(); }
  /** Reimplemented for internal reasons.
   */
  virtual void editPaste(void)
  { paste(); }
  /** Reimplemented for internal reasons.
   */
  virtual void editSearch(toSearchReplace *search);
  /** Reimplemented for internal reasons.
   */
  virtual void editSelectAll(void)
  { selectAll(); }

protected:
  virtual void newLine(void);
  virtual void dropEvent(QDropEvent *);
private slots:
  void setRedoAvailable(bool avail)
  { redoEnabled(avail); }
  void setUndoAvailable(bool avail)
  { undoEnabled(avail); }
  void setCopyAvailable(bool avail)
  { setEdit(); }
};

#endif
