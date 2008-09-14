/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOVISUALIZE_H
#define TOVISUALIZE_H

#include "config.h"

#include <QWidget>
#include <algorithm>

class QComboBox;
class QToolButton;
class toResultTableView;

/** This widget is used to visualize the data in a toListView into charts.
 */
class toVisualize : public QWidget
{
    Q_OBJECT;

    QComboBox *Type;
    QComboBox *Title;

    QWidget           *Result;
    toResultTableView *Source;

    QToolButton *Legend;
    QToolButton *Grid;
    QToolButton *Axis;

public:
    /** Create visualize widget.
     * @param source Source list view to get data from.
     */
    toVisualize(toResultTableView *Source,
                QWidget *parent,
                const char *name = NULL);

public slots:
    /** Refresh result
     */
    void display(void);
};

#endif
