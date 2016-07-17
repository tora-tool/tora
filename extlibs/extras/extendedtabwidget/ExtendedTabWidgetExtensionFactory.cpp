#include "ExtendedTabWidgetExtensionFactory.h"
#include "ExtendedTabWidgetContainerExtension.h"
//#include "ExtendedTabWidgetPropertySheetExtension.h"
#include "ExtendedTabWidget.h"

ExtendedTabWidgetExtensionFactory::ExtendedTabWidgetExtensionFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{}

QObject *ExtendedTabWidgetExtensionFactory::createExtension(QObject *object,
                                                            const QString &iid,
                                                            QObject *parent) const
{
    ExtendedTabWidget *widget = qobject_cast<ExtendedTabWidget*>(object);

    if (widget && (iid == Q_TYPEID(QDesignerContainerExtension)))
        return new ExtendedTabWidgetContainerExtension(widget, parent);
//    else if (widget && (iid == Q_TYPEID(QDesignerPropertySheetExtension)))
//        return new ExtendedTabWidgetPropertySheetExtension(widget, parent);
    else
        return 0;
}
