/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#ifndef TOTABWIDGET_H
#define TOTABWIDGET_H

#include "config.h"

#include <QTabWidget>
#include <QMap>
#include <QPointer>
#include <QString>


/** An extension of the regular tab widget with posibility to hide
 * tabs.
 */
class toTabWidget: public QTabWidget
{
    Q_OBJECT;

    struct page
    {
        QString label;
        int     index;
    };

    typedef QMap<QWidget *, struct page> WidgetMap;
    WidgetMap tabs;

public:
    /** Constructor.
     */
    toTabWidget(QWidget *parent = 0);

    /** Specify if a tab is shown.
     * @param w Widget to change visibility.
     * @param shown Show tab if true, hide if false.
     */
    virtual void setTabShown(QWidget *w, bool value);

    /** Specify that a tab is shown.
     * @param w Tab to show.
     */
    virtual void showTab(QWidget *w)
    {
        setTabShown(w, true);
    }
    /** Specify that a tab is hidden.
     * @param w Tab to hide.
     */
    virtual void hideTab(QWidget *w)
    {
        setTabShown(w, false);
    }

    // ---------------------------------------- overrides QTabWidget

    virtual int addTab(QWidget *widget, const QString &);
    virtual int addTab(QWidget *widget,
                       const QIcon& icon,
                       const QString &label);

private:
    /* this should not be used  for simplicity */
    int insertTab(int index, QWidget *widget, const QString &);
    int insertTab(int index,
                  QWidget *widget,
                  const QIcon& icon,
                  const QString &label);

    void removeTab(int index);

private slots:
    void tabChanged(int index);

signals:
    void currentTabChanged(QWidget *);
};

#endif
