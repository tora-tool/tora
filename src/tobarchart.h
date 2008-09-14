/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOBARCHART_H
#define TOBARCHART_H

#include "config.h"
#include "tolinechart.h"

/** A widget that displays a barchart. Each line is added onto the next one.
 */

class toBarChart : public toLineChart
{
    Q_OBJECT

protected:
    virtual void paintChart(QPainter *p, QRect &rect);
public:
    /** Create a new barchart.
     * @param parent Parent widget.
     * @param name Name of widget.
     * @param f Widget flags.
     */
    toBarChart(QWidget *parent = NULL, const char *name = NULL, Qt::WFlags f = 0);

    /** Create a new barchart by copying all the data from another barchart.
     * @param chart Chart to copy data from.
     * @param parent Parent widget.
     * @param name Name of widget.
     * @param f Widget flags.
     */
    toBarChart(toBarChart *chart, QWidget *parent = NULL, const char *name = NULL, Qt::WFlags f = 0);

    /** Open chart in new window.
     */
    virtual toLineChart *openCopy(QWidget *parent);
};

#endif
