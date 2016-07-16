#include <QtDesigner/QExtensionFactory>
#include <QtDesigner/QExtensionManager>
#include <QtDesigner/QDesignerFormEditorInterface>
#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerPropertySheetExtension>

#include <QIcon>
#include <QtPlugin>

#include "ExtendedTabWidget.h"
#include "ExtendedTabWidgetPlugin.h"
#include "ExtendedTabWidgetExtensionFactory.h"

ExtendedTabWidgetPlugin::ExtendedTabWidgetPlugin(QObject *parent) : QObject(parent)
{
    initialized = false;
}

QString ExtendedTabWidgetPlugin::name() const
{
    return QLatin1String("ExtendedTabWidget");
}

QString ExtendedTabWidgetPlugin::group() const
{
    return QLatin1String("Containers");
}

QString ExtendedTabWidgetPlugin::toolTip() const
{
    return QLatin1String("Modified QTabWidget with tabs on the right");
}

QString ExtendedTabWidgetPlugin::whatsThis() const
{
    return QLatin1String("Modified QTabWidget with tabs on the right");
}

QString ExtendedTabWidgetPlugin::includeFile() const
{
    return QLatin1String("ExtendedTabWidget.h");
}

QIcon ExtendedTabWidgetPlugin::icon() const
{
    return QIcon("designericon.png");
}

bool ExtendedTabWidgetPlugin::isContainer() const
{
    return true;
}

QWidget *ExtendedTabWidgetPlugin::createWidget(QWidget *parent)
{
    ExtendedTabWidget *widget = new ExtendedTabWidget(parent);
    connect(widget, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCurrentIndexChanged(int)));
    connect(widget, SIGNAL(pageTitleChanged(QString)), this, SLOT(slotPageTitleChanged(QString)));
    connect(widget, SIGNAL(pageIconChanged(QIcon)), this, SLOT(slotPageIconChanged(QIcon)));
    return widget;
}

bool ExtendedTabWidgetPlugin::isInitialized() const
{
    return initialized;
}

void ExtendedTabWidgetPlugin::initialize(QDesignerFormEditorInterface *formEditor)
{
    if (initialized) return;

    QExtensionManager *manager = formEditor->extensionManager();
    QExtensionFactory *factory = new ExtendedTabWidgetExtensionFactory(manager);

    Q_ASSERT(manager != 0);
    manager->registerExtensions(factory, Q_TYPEID(QDesignerContainerExtension));
    //manager->registerExtensions(factory, Q_TYPEID(QDesignerPropertySheetExtension));

    initialized = true;
}

QString ExtendedTabWidgetPlugin::domXml() const
{
    return QLatin1String("\
		<ui language=\"c++\">\
			<widget class=\"ExtendedTabWidget\" name=\"ExtendedTabWidget\">\
				<widget class=\"QWidget\" name=\"page\" />\
			</widget>\
			<customwidgets>\
				<customwidget>\
					<class>ExtendedTabWidget</class>\
                    <propertyspecifications>\
                    <stringpropertyspecification name=\"pageTitle\" notr=\"false\" type=\"singleline\"/>\
                    <stringpropertyspecification name=\"pageTitleList\" notr=\"false\" type=\"singleline\"/>\
                    </propertyspecifications>\
					<extends>QWidget</extends>\
					<addpagemethod>addPage</addpagemethod>\
				</customwidget>\
			</customwidgets>\
		</ui>");
}

void ExtendedTabWidgetPlugin::slotCurrentIndexChanged(int index)
{
    Q_UNUSED(index);
    ExtendedTabWidget *widget = qobject_cast<ExtendedTabWidget*>(sender());

    if (widget)
    {
        QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(widget);
        if (form)
            form->emitSelectionChanged();
    }
}

void ExtendedTabWidgetPlugin::slotPageTitleChanged(const QString &title)
{
    Q_UNUSED(title);
    ExtendedTabWidget *widget = qobject_cast<ExtendedTabWidget*>(sender());

    if (widget)
    {
        QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(widget);

        if (form)
        {
            QDesignerFormEditorInterface *editor = form->core();
            QExtensionManager *manager = editor->extensionManager();

            QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(manager, widget);
            const int propertyIndex1 = sheet->indexOf(QLatin1String("pageTitle"));
            sheet->setChanged(propertyIndex1, true);
            const int propertyIndex2 = sheet->indexOf(QLatin1String("pageTitleList"));
            sheet->setChanged(propertyIndex2, true);   
        }
    }
}

void ExtendedTabWidgetPlugin::slotPageIconChanged(const QIcon &icon)
{
    Q_UNUSED(icon);
    ExtendedTabWidget *widget = qobject_cast<ExtendedTabWidget*>(sender());

    if (widget)
    {
        QDesignerFormWindowInterface *form = QDesignerFormWindowInterface::findFormWindow(widget);

        if (form)
        {
            QDesignerFormEditorInterface *editor = form->core();
            QExtensionManager *manager = editor->extensionManager();

            QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(manager, widget);
            const int propertyIndex1 = sheet->indexOf(QLatin1String("pageIcon"));
            sheet->setChanged(propertyIndex1, true);
            const int propertyIndex2 = sheet->indexOf(QLatin1String("pageIconList"));
            sheet->setChanged(propertyIndex2, true);
        }
    }
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(extendedtabwidget, ExtendedTabWidgetPlugin)
#endif
