#pragma once

#include <QtDesigner/QExtensionFactory>

QT_BEGIN_NAMESPACE
class QExtensionManager;
QT_END_NAMESPACE

class ExtendedTabWidgetExtensionFactory: public QExtensionFactory
{
    Q_OBJECT

public:
    ExtendedTabWidgetExtensionFactory(QExtensionManager *parent = 0);

protected:
    QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};