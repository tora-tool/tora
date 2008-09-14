/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOSGASTATEMENT_H
#define TOSGASTATEMENT_H

#include "config.h"

#include <qtabwidget.h>

class QComboBox;
class QTabWidget;
class toResultField;
class toResultItem;
class toResultPlan;
class toResultView;

/** This widget displays information about a statement in the Oracle SGA. To get an
 * address use the @ref toSQLToAddress function.
 */

class toSGAStatement : public QTabWidget
{
    Q_OBJECT
    /** The SQL run.
     */
    toResultField *SQLText;
    /** Tab widget
     */
    QWidget *CurrentTab;
    /** Resources used by the statement.
     */
    toResultItem *Resources;
    /** Address of the statement.
     */
    QString Address;
    /** Execution plan of the statement.
     */
    toResultPlan *Plan;

    /** Display resources of statement.
     */
    void viewResources(void);
private slots:
    /** Change the displayed tab.
     */
    void changeTab(int);
public:
    /** Create widget.
     * @param parent Parent widget.
     */
    toSGAStatement(QWidget *parent);
public slots:
    /** Display another statement.
     * @param address Address of the statement to display.
     */
    virtual void changeAddress(const QString &address);
    /** Update the contents of this widget.
     */
    void refresh(void)
    {
        changeTab(QTabWidget::indexOf(CurrentTab));
    }
};

#endif
