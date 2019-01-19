TARGET = qtpropertybrowser
TEMPLATE = lib

CONFIG += qt
QT *= widgets

CONFIG += embed_manifest_dll
DEFINES += QT_QTPROPERTYBROWSER_EXPORT
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

CONFIG (debug, debug|release) {
    QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/unknow/debug

    msvc:QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/msvc/debug
    mingw:QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/mingw/debug

} else {
    QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/unknow/release

    msvc:QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/msvc/release
    mingw:QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/mingw/release
}

msvc:QMAKE_CXXFLAGS += /FS

DESTDIR = $$QTPROPERTYBROWSER_LIBDIR
DLLDESTDIR = $$QTPROPERTYBROWSER_LIBDIR/bin

INCLUDEPATH += ../src
DEPENDPATH += ../src

SOURCES += ../src/qtpropertybrowser.cpp \
        ../src/qtpropertymanager.cpp \
        ../src/qteditorfactory.cpp \
        ../src/qtvariantproperty.cpp \
        ../src/qttreepropertybrowser.cpp \
        ../src/qtbuttonpropertybrowser.cpp \
        ../src/qtgroupboxpropertybrowser.cpp \
        ../src/qtpropertybrowserutils.cpp

HEADERS += ../src/qtpropertybrowser.h \
        ../src/qtpropertymanager.h \
        ../src/qteditorfactory.h \
        ../src/qtvariantproperty.h \
        ../src/qttreepropertybrowser.h \
        ../src/qtbuttonpropertybrowser.h \
        ../src/qtgroupboxpropertybrowser.h \
        ../src/qtpropertybrowserutils_p.h

RESOURCES += ../src/qtpropertybrowser.qrc
