//***************************************************************************
/*
 * TOra - An Oracle Toolkit for DBA's and developers
 * Copyright (C) 2000-2001,2001 Underscore AB
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
 *      these libraries without written consent from Underscore AB. Observe
 *      that this does not disallow linking to the Qt Free Edition.
 *
 * All trademarks belong to their respective owners.
 *
 ****************************************************************************/

#ifndef TOPIECHART_H
#define TOPIECHART_H

#include <list>
#include <qwidget.h>

class QPopupMenu;

/** A widget that displays a piechart.
 */

class toPieChart : public QWidget {
  Q_OBJECT

  std::list<double> Values;
  std::list<QString> Labels;
  QString Postfix;
  bool Legend;
  bool DisplayPercent;
  QString Title;
  QPopupMenu *Menu;

  QRect ChartRect;
  std::list<int> Angels;

protected:
  virtual void mouseDoubleClickEvent(QMouseEvent *e);
  virtual void mousePressEvent(QMouseEvent *e);
public:
  /** Create a new piechart.
   * @param parent Parent widget.
   * @param name Name of widget.
   * @param f Widget flags.
   */
  toPieChart(QWidget *parent=NULL,const char *name=NULL,WFlags f=0);

  /** Create a copy of a piechart.
   * @param pie Pie chart to copy.
   * @param parent Parent widget.
   * @param name Name of widget.
   * @param f Widget flags.
   */
  toPieChart(toPieChart *pie,QWidget *parent=NULL,const char *name=NULL,WFlags f=0);

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

  /** Set title of the chart. Set to empty string to not display title.
   * @param title Title of chart.
   */
  void setTitle(const QString &title=QString::null)
  { Title=title; update(); }
  /** Get title of chart.
   * @return Title of chart.
   */
  const QString &title(void)
  { return Title; }

  /** Display piecharts in percent instead of actual values
   * @param pct Wether or not to display percent only.
   */
  void setDisplayPercent(bool pct)
  { DisplayPercent=pct; update(); }
  /** Check if only percent is displayed
   * @return True if only percent is displayed.
   */
  bool displayPercent(void) const
  { return DisplayPercent; }

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
  void setValues(std::list<double> &values,std::list<QString> &labels);
  /** Add a value to the piechart.
   * @param value New value to add.
   * @param label Label of this new value.
   */
  void addValue(double value,const QString &label)
  { Values.insert(Values.end(),value); Labels.insert(Labels.end(),label); update(); }
  /** Get list of values.
   * @return Values in piechart.
   */
  std::list<double> &values(void)
  { return Values; }
  /** Get labels of piechart.
   * @return List of labels.
   */
  std::list<QString> &labels(void)
  { return Labels; }
  /** Find the label if any of a point in the chart.
   */
  QString findLabel(QPoint p);
  /** Get rectangle that the chart is contained in.
   */
  QRect chartRectangle()
  { return ChartRect; }
signals:
  void newValues(std::list<double> &values,std::list<QString> &labels);
public slots:
  /** Print the chart
   */
  void editPrint(void);
  /** Open in new window
   */
  void openCopy(void);
protected:
  /** Paint chart in a given rectangle.
   */
  virtual void paintChart(QPainter *p,QRect rect);
  /** Reimplemented for internal reasons.
   */
  virtual void paintEvent(QPaintEvent *e);
};

#endif
