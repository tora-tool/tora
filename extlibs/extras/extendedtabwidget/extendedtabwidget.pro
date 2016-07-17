TEMPLATE = lib
TARGET   = $$qtLibraryTarget($$TARGET)
CONFIG  += plugin

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += designer
} else {
    CONFIG += designer
}

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/designer

HEADERS += ExtendedTabWidget.h \
           ExtendedTabWidgetPlugin.h \ 
           ExtendedTabWidgetContainerExtension.h \
#           ExtendedTabWidgetPropertySheetExtension.h \
           ExtendedTabWidgetExtensionFactory.h 

SOURCES += ExtendedTabWidget.cpp \
           ExtendedTabWidgetPlugin.cpp \
           ExtendedTabWidgetContainerExtension.cpp \
#           ExtendedTabWidgetPropertySheetExtension.cpp \
           ExtendedTabWidgetExtensionFactory.cpp 

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
#sources.files = $$SOURCES $$HEADERS *.pro
#sources.path = $$[QT_INSTALL_EXAMPLES]/designer/extendedtabwidget
INSTALLS += target
