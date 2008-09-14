/* BEGIN_COMMON_COPYRIGHT_HEADER 
* END_COMMON_COPYRIGHT_HEADER */

#include <list>

#include "totabwidget.h"

#include <qobject.h>
#include <QTabWidget>

#define CONNECT_CHANGED                         \
    connect(this,                               \
            SIGNAL(currentChanged(int)),        \
            this,                               \
            SLOT(tabChanged(int)));

#define DISCONNECT_CHANGED                      \
    disconnect(this,                            \
               SIGNAL(currentChanged(int)),     \
               this,                            \
               SLOT(tabChanged(int)));

toTabWidget::toTabWidget(QWidget *parent)
        : QTabWidget(parent)
{

    CONNECT_CHANGED;
}


void toTabWidget::setTabShown(QWidget *w, bool show)
{
    int      pos;
    QWidget *parent = w;

    if (!show)
    {
        while ((pos = indexOf(parent)) < 0 && parent && parent != this)
            parent = parent->parentWidget();

        if (pos < 0 || parent == this)
            return;                 // not found

        DISCONNECT_CHANGED;
        QTabWidget::removeTab(pos);
        CONNECT_CHANGED;
    }
    else
    {
        while (!tabs.contains(parent) && parent && parent != this)
            parent = parent->parentWidget();

        if (!tabs.contains(parent) || parent == this || !parent)
            return;                 // not found

        struct page r = tabs[parent];

        DISCONNECT_CHANGED;
        QTabWidget::insertTab(r.index, parent, r.label);
        CONNECT_CHANGED;
    }
}


int toTabWidget::addTab(QWidget *widget, const QString &label)
{
    struct page p;
    p.label = label;
    p.index = QTabWidget::addTab(widget, label);
    tabs[widget] = p;
    return p.index;
}


int toTabWidget::addTab(QWidget *widget, const QIcon& icon, const QString &label)
{
    struct page p;
    p.label = label;
    p.index = QTabWidget::addTab(widget, icon, label);
    tabs[widget] = p;
    return p.index;
}


void toTabWidget::tabChanged(int index)
{
    QWidget *widget = QTabWidget::widget(index);
    if (widget)
        emit currentTabChanged(widget);
}


// ---------------------------------------- unused

int toTabWidget::insertTab(int index, QWidget *widget, const QString &label)
{
    return QTabWidget::insertTab(index, widget, label);
}


int toTabWidget::insertTab(int index,
                           QWidget *widget,
                           const QIcon& icon,
                           const QString &label)
{
    return QTabWidget::insertTab(index, widget, icon, label);
}


void toTabWidget::removeTab(int index)
{
    QTabWidget::removeTab(index);
}
