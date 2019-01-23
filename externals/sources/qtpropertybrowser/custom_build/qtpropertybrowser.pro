TARGET = qtpropertybrowser
TEMPLATE = lib

CONFIG += qt
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += embed_manifest_dll
DEFINES += QT_QTPROPERTYBROWSER_EXPORT
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

message( $$MAKEFILE_GENERATOR )

CONFIG (debug, debug|release) {
    message( "Debug build" )

    QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/unknown/debug

    contains(MAKEFILE_GENERATOR, "MSBUILD") || contains(MAKEFILE_GENERATOR, "MSVC.NET") {
        QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/msvc/debug
    }
    contains(MAKEFILE_GENERATOR, "MINGW") {
        QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/mingw/debug
    }
} else {
    message( "Release build" )

    QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/unknown/release

    contains(MAKEFILE_GENERATOR, "MSBUILD") || contains(MAKEFILE_GENERATOR, "MSVC.NET") {
        QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/msvc/release
    }
    contains(MAKEFILE_GENERATOR, "MINGW") {
        QTPROPERTYBROWSER_LIBDIR = $$PWD/lib/mingw/release
    }
}

contains(MAKEFILE_GENERATOR, "MSBUILD") || contains(MAKEFILE_GENERATOR, "MSVC.NET") {
    QMAKE_CXXFLAGS += /FS
}

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
