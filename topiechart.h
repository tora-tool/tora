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

#ifndef __TOPIECHART_H
#define __TOPIECHART_H

#include <list>
#include <qwidget.h>

/** A widget that displays a piechart.
 */

class toPieChart : public QWidget {
  list<double> Values;
  list<QString> Labels;
  QString Postfix;
  bool Legend;
public:
  /** Create a new piechart.
   * @param parent Parent widget.
   * @param name Name of widget.
   * @param f Widget flags.
   */
  toPieChart(QWidget *parent=NULL,const char *name=NULL,WFlags f=0);

  /** Set the postfix text to append the current values when they are displayed in the
   * pie.
   * @param post Postfix string, no space will be added after the value before the string
   *             so if you want the space you need to specify it first in this string.
   */
  void setPostfix(const QString &post)
  { Postfix=post; update(); }
  /** Get the postfix string.
   * @return Current postfix string.
   */
  const QString &postfix(void) const
  { return Postfix; }

  /** Specify if legend should be displayed to the right of the graph, default is on.
   * @param on Whether to display graph or not.
   */
  void showLegend(bool on)
  { Legend=on; update(); }
  /** Check if legend is displayed or not.
   * @return If legend is displayed or not.
   */
  bool legend(void) const
  { return Legend; }

  /** Set value list of piechart.
   * @param values List of values to display.
   * @param labels List of labels, if label is empty it will not appear in legend.
   */
  void setValues(list<double> &values,list<QString> &labels)
  { Values=values; Labels=labels; update(); }
  /** Add a value to the piechart.
   * @param value New value to add.
   * @param label Label of this new value.
   */
  void addValue(double value,const QString &label)
  { Values.insert(Values.end(),value); Labels.insert(Labels.end(),label); update(); }
  /** Get list of values.
   * @return Values in piechart.
   */
  list<double> &values(void)
  { return Values; }
  /** Get labels of piechart.
   * @return List of labels.
   */
  list<QString> &labels(void)
  { return Labels; }

protected:
  /** Reimplemented for internal reasons.
   */
  virtual void paintEvent(QPaintEvent *e);
};

#endif
