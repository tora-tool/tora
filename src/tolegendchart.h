
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 * 
 * Shared/mixed copyright is held throughout files in this product
 * 
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2009 Numerous Other Contributors
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
 *      these libraries. 
 * 
 *      You may link this product with any GPL'd Qt library.
 * 
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOLEGENDCHART_H
#define TOLEGENDCHART_H

#include "config.h"

#include <list>

#include <qwidget.h>
//Added by qt3to4:
#include <QPaintEvent>

/** A widget that displays a legend to other charts.
 */

class toLegendChart : public QWidget
{
    Q_OBJECT

    int Columns;
    QString Title;

    std::list<QString> Labels;
    std::list<int> sizeHint(int &height, int &items);
public:
    /** Create a new legend.
     * @param columns Columns.
     * @param parent Parent widget.
     * @param name Name of widget.
     * @param f Widget flags.
     */
    toLegendChart(int columns, QWidget *parent = NULL, const char *name = NULL, Qt::WFlags f = 0);

    /** Set title of the chart. Set to empty string to not display title.
     * @param title Title of chart.
     */
    void setTitle(const QString &title = QString::null)
    {
        Title = title;
        update();
    }
    /** Get title of chart.
     * @return Title of chart.
     */
    const QString &title(void)
    {
        return Title;
    }

    /** Set list of chart labels.
     * @param labels List of labels, if label is empty it will not appear in legend.
     */
    void setLabels(std::list<QString> &labels);
    /** Add a value to the chart.
     * @param value New value to add.
     * @param label Label of this new value.
     */
    void addLabel(const QString &label);
    /** Get labels of chart.
     * @return List of labels.
     */
    std::list<QString> &labels(void)
    {
        return Labels;
    }
    /** Reimplemented for internal reasons.
     */
    virtual QSize sizeHint();
protected:
    /** Reimplemented for internal reasons.
     */
    virtual void paintEvent(QPaintEvent *e);
};

#endif
