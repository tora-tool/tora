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

#ifndef TOMARKEDTEXT_H
#define TOMARKEDTEXT_H

#include "toeditwidget.h"

#if QT_VERSION < 0x030000
#include "tomarkedtext.2.h"
#else
#include "qtlegacy/qttableview.h"
#include "qtlegacy/qtmultilineedit.h"
#include "tomarkedtext.3.h"
#endif

#include <map>

#include <qglobal.h>
#include <qstring.h>
#include <qtimer.h>

class TOPrinter;

/** This is the enhanced editor used in TOra. It mainly offers integration in the TOra
 * menues and printsupport in addition to normal QMultiLineEdit.
 */

class toMarkedText : public toMultiLineEdit, public toEditWidget {
  Q_OBJECT

  /** Filename of the file in this buffer.
   */
  QString Filename;

  bool Search;
  bool SearchFailed;
  bool SearchForward;
  QString SearchString;
  QString LastSearch;

  int CursorTimerID;

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

  void searchFound(int line,int col);
  void incrementalSearch(bool forward,bool next);
protected:
  /** Reimplemented for internal reasons.
   */
  virtual void mousePressEvent(QMouseEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void keyPressEvent(QKeyEvent *e);
public:
  /** Create an editor.
   * @param parent Parent of this widget.
   * @param name Name of this widget.
   */
  toMarkedText(QWidget *parent,const char *name=NULL);
  /** Insert text and optionallly mark inserted text.
   * @param str String to insert.
   * @param mark True if mark inserted as selected.
   */
  virtual void insert(const QString &str,bool mark)
  { toMultiLineEdit::insert(str,mark); }
  /** Insert text.
   * @param str String to insert.
   */
  virtual void insert(const QString &str)
  { toMultiLineEdit::insert(str); }
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
  /** Open a file for editing.
   * @param file File to open for editing.
   */
  virtual void openFilename(const QString &file);
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
  virtual void focusOutEvent (QFocusEvent *e);
  /** Reimplemented for internal reasons.
   */
  virtual void paintEvent(QPaintEvent *pe);
  /** Print this editor.
   */
  virtual void editPrint(void);
  /** Reimplemented for internal reasons.
   */
  virtual bool editOpen(QString file=QString::null);
  /** Reimplemented for internal reasons.
   */
  virtual bool editSave(bool ask);
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
  virtual void editSelectAll(void)
  { selectAll(); }

  /** Move to top of data
   */
  virtual void searchTop(void)
  { setCursorPosition(0,0,false); }
  /** Search for next entry
   * @return True if found, should select the found text.
   */
  virtual bool searchNext(toSearchReplace *search);
  /** Replace entry with new data
   */
  virtual void searchReplace(const QString &newData);
  /** Check if data can be modified by search
   * @param all If true can replace all, otherwise can replace right now.
   */
  virtual bool searchCanReplace(bool all);

  /** Start incremental search mode, or search next if started.
   */
  void incrementalSearch(bool forward);

  /** Export data to a map.
   * @param data A map that can be used to recreate the data of a chart.
   * @param prefix Prefix to add to the map.
   */
  virtual void exportData(std::map<QCString,QString> &data,const QCString &prefix);
  /** Import data
   * @param data Data to read from a map.
   * @param prefix Prefix to read data from.
   */
  virtual void importData(std::map<QCString,QString> &data,const QCString &prefix);
  /** Find the line and column of an index into the string of the entire editor.
   * @param index Index to get line and col from.
   * @param line Will be filled out with the line.
   * @param col Will be filled out with the col.
   */
  void findPosition(int index,int &line,int &col);
protected:
  virtual void newLine(void);
  virtual void dropEvent(QDropEvent *);
  virtual void timerEvent(QTimerEvent *);
private slots:
  void setRedoAvailable(bool avail)
  { redoEnabled(avail); }
  void setUndoAvailable(bool avail)
  { undoEnabled(avail); }
  void setCopyAvailable(bool)
  { setEdit(); }
};

#endif
