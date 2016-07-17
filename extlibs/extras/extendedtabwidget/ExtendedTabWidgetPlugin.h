#pragma once

#include <QtDesigner/QDesignerCustomWidgetInterface>

QT_BEGIN_NAMESPACE
class QIcon;
class QWidget;
QT_END_NAMESPACE

class ExtendedTabWidgetPlugin: public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetInterface)

#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.ExtendedTabWidgetPlugin")
#endif

public:
    ExtendedTabWidgetPlugin(QObject *parent = 0);

    QString name() const;
    QString group() const;
    QString toolTip() const;
    QString whatsThis() const;
    QString includeFile() const;
    QIcon icon() const;
    bool isContainer() const;
    QWidget *createWidget(QWidget *parent);
    bool isInitialized() const;
    void initialize(QDesignerFormEditorInterface *formEditor);
    QString domXml() const;

private slots:
    void slotCurrentIndexChanged(int index);
    void slotPageTitleChanged(const QString &title);
    void slotPageIconChanged(const QIcon &icon);

private:
    bool initialized;
};
