/* BEGIN_COMMON_COPYRIGHT_HEADER 
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
