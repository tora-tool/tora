
/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * TOra - An Oracle Toolkit for DBA's and developers
 *
 * Shared/mixed copyright is held throughout files in this product
 *
 * Portions Copyright (C) 2000-2001 Underscore AB
 * Portions Copyright (C) 2003-2005 Quest Software, Inc.
 * Portions Copyright (C) 2004-2013 Numerous Other Contributors
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
 * along with this program as the file COPYING.txt; if not, please see
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt.
 *
 *      As a special exception, you have permission to link this program
 *      with the Oracle Client libraries and distribute executables, as long
 *      as you follow the requirements of the GNU GPL in regard to all of the
 *      software in the executable aside from Oracle client libraries.
 *
 * All trademarks belong to their respective owners.
 *
 * END_COMMON_COPYRIGHT_HEADER */

#ifndef TOLINECHART_H
#define TOLINECHART_H

#include <QWidget>
#include <QtCore/QString>
#include <QtGui/QPaintEvent>
#include <QtGui/QMouseEvent>
#include <QtCore/QRect>
#include <QtCore/QPoint>

#include <list>
#include <map>
#include <algorithm>

#include "core/utils.h"

class QMenu;
class QScrollBar;

/**
 * A widget that displays a linechart with optional background
 * throbber (Not implemented yet).
 *
 */
class toLineChart : public QWidget
{
        Q_OBJECT;

        QMenu *Menu;

        QScrollBar *Horizontal;
        QScrollBar *Vertical;

    protected:
        std::list<std::list<double> > Values;
        std::list<QString> XValues;
        std::list<QString> Labels;
        std::list<bool> Enabled;
        bool Legend;
        bool Last;
        int Grid;
        bool AxisText;
        double MinValue;
        bool MinAuto;
        double MaxValue;
        bool MaxAuto;
        QString YPostfix;
        int Samples;
        QString Title;

        QRect Chart;
        QPoint MousePoint[2];
        int SkipSamples;
        int UseSamples;
        int DisplaySamples;
        bool Zooming;
        double zMinValue;
        double zMaxValue;

        static double round(double round, bool up);
        QRect fixRect(QPoint p1, QPoint p2);
        virtual void mouseReleaseEvent(QMouseEvent *e);
        virtual void mouseMoveEvent(QMouseEvent *e);
        virtual void mouseDoubleClickEvent(QMouseEvent *e);
        virtual void mousePressEvent(QMouseEvent *e);

        int countSamples(void);
        void clearZoom(void);

        virtual void paintLegend(QPainter *p, QRect &rect);
        virtual void paintTitle(QPainter *p, QRect &rect);
        virtual void paintAxis(QPainter *p, QRect &rect);
        virtual void paintChart(QPainter *p, QRect &rect);
    public:
        /** Create a new linechart.
         * @param parent Parent widget.
         * @param name Name of widget.
         * @param f Widget flags.
         */
        toLineChart(QWidget *parent = NULL, const char *name = NULL, toWFlags f = 0);

        /** Create a new chart by copying all the data from another chart.
         * @param chart Chart to copy data from.
         * @param parent Parent widget.
         * @param name Name of widget.
         * @param f Widget flags.
         */
        toLineChart(toLineChart *chart, QWidget *parent = NULL, const char *name = NULL, toWFlags f = 0);

        /** Destroy chart
         */
        ~toLineChart();

        /** Specify if legend should be displayed to the right of the graph, default is on.
         * @param on Whether to display legend or not.
         */
        void showLegend(bool on)
        {
            Legend = on;
            update();
        }
        /** Check if legend is displayed or not.
         * @return If legend is displayed or not.
         */
        bool legend(void) const
        {
            return Legend;
        }

        /** Show most recent value on top of graph
         * @param on Whether to display or not.
         */
        void showLast(bool on)
        {
            Last = on;
            update();
        }
        /** Check if last value is displayed or not.
         * @return If value is displayed or not.
         */
        bool last(void) const
        {
            return Last;
        }

        /** Set title of the chart. Set to empty string to not display title.
         * @param title Title of chart.
         */
        void setTitle(const QString &title = QString::null)
        {
            Title = title;
            setWindowTitle(title);
            update();
        }
        /** Get title of chart.
         * @return Title of chart.
         */
        const QString &title(void)
        {
            return Title;
        }

        /** Specify if a grid should be displayed in the graph, default is on.
         * @param div Number of parts to divide grid into.
         */
        void showGrid(int div = 0)
        {
            Grid = div;
            update();
        }
        /** Check if grid is displayed or not.
         * @return Number of parts to divide grid into.
         */
        int grid(void) const
        {
            return Grid;
        }

        /** Specify if a y-axis legend should be displayed in the graph, default is on.
         * @param on Whether to display legend or not.
         */
        void showAxisLegend(bool on)
        {
            AxisText = on;
            update();
        }
        /** Check if y-axis legend is displayed or not.
         * @return If legend is displayed or not.
         */
        bool axisLegend(void) const
        {
            return AxisText;
        }

        /** Set y postfix value. This will be appended to the values in the axis legend.
         * @param postfix The string to append.
         */
        void setYPostfix(const QString &postfix)
        {
            YPostfix = postfix;
            update();
        }
        /** Set max value on y-axis to auto.
         */
        void setMaxValueAuto(void)
        {
            MaxAuto = true;
            update();
        }
        /** Set min value on y-axis to auto.
         */
        void setMinValueAuto(void)
        {
            MinAuto = true;
            update();
        }
        /** Set max value on y-axis.
         * @param val Max value on y-axis.
         */
        void setMaxValue(double maxVal)
        {
            MaxAuto = false;
            MaxValue = maxVal;
            update();
        }
        /** Set min value on y-axis.
         * @param val Min value on y-axis.
         */
        void setMinValue(double minVal)
        {
            MinAuto = false;
            MinValue = minVal;
            update();
        }
        /** Get minimum value on y-axis. Will not return the automatically determinned minimum value.
         * @return Minimum value on y-axis.
         */
        double minValue(void) const
        {
            return MinValue;
        }
        /** Get maximum value on y-axis. Will not return the automatically determinned maximum value.
         * @return Maximum value on y-axis.
         */
        double maxValue(void) const
        {
            return MaxValue;
        }

        /** Set the number of samples on the x-axis. Setting samples to -1 will keep all entries.
         * @param samples Number of samples.
         */
        void setSamples(int samples = -1);
        /** Get the maximum number of samples on the x-axis.
         * @return Max number of samples.
         */
        int samples(void) const
        {
            return Samples;
        }

        /** Set the labels on the chart lines.
         * @param labels Labels of the lines. Empty labels will not show up in the legend.
         */
        void setLabels(const std::list<QString> &labels)
        {
            Labels = labels;
            update();
        }
        /** Get the labels of the chart lines.
         * @return List of labels.
         */
        std::list<QString> &labels(void)
        {
            return Labels;
        }

        /** Add a new value set to the chart.
         * @param value New values for charts (One for each line).
         * @param label X-value on these values.
         */
        virtual void addValues(std::list<double> &value, const QString &xValues);

        /** Get list of labels
         * @return List of labels
         */
        std::list<QString> &xValues(void)
        {
            return XValues;
        }

        /** Get list of values.
         * @return Values in piechart.
         */
        std::list<std::list<double> > &values(void)
        {
            return Values;
        }

        /** Export chart to a map.
         * @param data A map that can be used to recreate the data of a chart.
         * @param prefix Prefix to add to the map.
         */
        virtual void exportData(std::map<QString, QString> &data, const QString &prefix);
        /** Import data
         * @param data Data to read from a map.
         * @param prefix Prefix to read data from.
         */
        virtual void importData(std::map<QString, QString> &data, const QString &prefix);

        /** Get enabled datavalues. Values in this list with false are not drawn in the chart.
         * Could be an empty list if everything is enabled.
         */
        std::list<bool> enabledCharts(void)
        {
            return Enabled;
        }
        /** Set enabled datavalues. Values in this list with false are not drawn in the chart.
         */
        void setEnabledCharts(std::list<bool> &enabled)
        {
            Enabled = enabled;
            update();
        }

#ifdef TORA3_CHART
        /** Open chart in new window.
         */
        virtual toLineChart *openCopy(QWidget *parent);
#endif

    signals:
        /** A new value set was added to the chart.
         * @param value New values for charts (One for each line).
         * @param label X-value on these values.
         */
        void valueAdded(std::list<double> &value, const QString &xValues);
    public slots:
        /** Clear the values from the chart.
         */
        virtual void clear(void)
        {
            Values.clear();
            XValues.clear();
            update();
        }

        /** Setup values of charts.
         */
        virtual void setup(void);

        /** Print the chart.
         */
        virtual void editPrint(void);
#ifdef TORA3_CHART
        void openCopy(void)
        {
            openCopy(NULL);
        }
#endif
    protected:
        /** Reimplemented for internal reasons.
         */
        virtual void paintEvent(QPaintEvent *e);
        /** Reimplemented for internal reasons.
         */
        virtual void addMenues(QMenu *)
        { }
    private slots:
        void horizontalChange(int);
        void verticalChange(int);
};

#endif
