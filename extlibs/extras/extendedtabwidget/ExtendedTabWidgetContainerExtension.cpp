#include "ExtendedTabWidgetContainerExtension.h"
#include "ExtendedTabWidget.h"
 
ExtendedTabWidgetContainerExtension::ExtendedTabWidgetContainerExtension(ExtendedTabWidget *widget, QObject *parent) : QObject(parent)
{
    myWidget = widget;
}

void ExtendedTabWidgetContainerExtension::addWidget(QWidget *widget)
{
    myWidget->addPage(widget);
}

int ExtendedTabWidgetContainerExtension::count() const
{
    return myWidget->count();
}

int ExtendedTabWidgetContainerExtension::currentIndex() const
{
    return myWidget->currentIndex();
}

void ExtendedTabWidgetContainerExtension::insertWidget(int index, QWidget *widget)
{
    myWidget->insertPage(index, widget);
}

void ExtendedTabWidgetContainerExtension::remove(int index)
{
    myWidget->removePage(index);
}

void ExtendedTabWidgetContainerExtension::setCurrentIndex(int index)
{
    myWidget->setCurrentIndex(index);
}

QWidget* ExtendedTabWidgetContainerExtension::widget(int index) const
{
    return myWidget->widget(index);
}
