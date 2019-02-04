TARGET = cvsandboxtools
TEMPLATE = lib

CONFIG += qt
greaterThan(QT_MAJOR_VERSION, 4): QT *= widgets

CONFIG += embed_manifest_dll
DEFINES += CVSANDBOXTOOLS_LIBRARY
DEFINES += QT_QTPROPERTYBROWSER_IMPORT

INCLUDEPATH += ../../afx/afx_types/ \
               ../../afx/afx_types+/ \
               ../../afx/afx_imaging/ \
               ../../core/iplugin/ \
               ../../core/pluginmgr/ \
               ../../../externals/sources/qtpropertybrowser/src \
               ../../../externals/sources/scintilla/qt/ScintillaEdit/ \
               ../../../externals/sources/scintilla/qt/ScintillaEditBase/ \
               ../../../externals/sources/scintilla/src/ \
               ../../../externals/sources/scintilla/include/

message( $$MAKEFILE_GENERATOR )

contains(MAKEFILE_GENERATOR, "MSBUILD") || contains(MAKEFILE_GENERATOR, "MSVC.NET") {
    CONFIG (debug, debug|release) {
        message( "MSVC Debug build" )

        MSVC_LIBS_DIR = $$PWD/../../../build/msvc/debug/lib
    } else {
        message( "MSVC Release build" )

        MSVC_LIBS_DIR = $$PWD/../../../build/msvc/release/lib
    }

    DESTDIR = $$MSVC_LIBS_DIR
    DLLDESTDIR = $${MSVC_LIBS_DIR}/../bin

    LIBS += $$MSVC_LIBS_DIR/pluginmgr.lib \
            $$MSVC_LIBS_DIR/iplugin.lib \
            $$MSVC_LIBS_DIR/afx_platform+.lib \
            $$MSVC_LIBS_DIR/afx_imaging.lib \
            $$MSVC_LIBS_DIR/afx_types+.lib \
            $$MSVC_LIBS_DIR/afx_types.lib \
            $$MSVC_LIBS_DIR/qtpropertybrowser.lib \
            $$MSVC_LIBS_DIR/ScintillaEdit3.lib

    CONFIG += embed_manifest_dll
    QMAKE_LFLAGS += /INCREMENTAL:NO
}

contains(MAKEFILE_GENERATOR, "MINGW") {
    CONFIG (debug, debug|release) {
        message( "MinGW Debug build" )

        MINGW_LIBS_DIR = $$PWD/../../../build/mingw/debug/lib
    } else {
        message( "MinGW Release build" )

        MINGW_LIBS_DIR = $$PWD/../../../build/mingw/release/lib
    }

    DESTDIR = $$MINGW_LIBS_DIR
    DLLDESTDIR = $${MINGW_LIBS_DIR}/../bin

    LIBS += -L$${MINGW_LIBS_DIR} \
            -lqtpropertybrowser \
            -lScintillaEdit3 \
            -lpluginmgr \
            -liplugin \
            -lafx_platform+ \
            -lafx_imaging \
            -lafx_types+ \
            -lafx_types

    QMAKE_CXXFLAGS += -std=c++0x
    LIBS += -fopenmp
}

contains(MAKEFILE_GENERATOR, "MSBUILD") || contains(MAKEFILE_GENERATOR, "MSVC.NET") {
    QMAKE_CXXFLAGS += /FS
}

SOURCES += \
    PluginsListFrame.cpp \
    XVariantConverter.cpp \
    VariantPropertyManagerEx.cpp \
    PluginPropertyEditorFrame.cpp \
    ConfigurePluginPropertiesDialog.cpp \
    UIPersistenceService.cpp \
    ImageFileService.cpp \
    ApplicationSettingsService.cpp \
    UITools.cpp \
    ErrorProviderHelper.cpp \
    HistogramWidget.cpp \
    DraggableTreeWidget.cpp \
    XImageInterface.cpp \
    DraggableListWidget.cpp \
    CustomPropertyManagers.cpp \
    PluginDescriptionDialog.cpp \
    FavouritePluginsManager.cpp \
    GlobalServiceManager.cpp \
    ScriptEditorDialog.cpp \
    HelpService.cpp \
    GoToLineDialog.cpp \
    HuePickerDialog.cpp \
    HuePickerWidget.cpp \
    FindTextDialog.cpp \
    HistogramInfoFrame.cpp \
    MorphologyStructuringElementDialog.cpp \
    ConvolutionKernelDialog.cpp

HEADERS +=\
    cvsandboxtools_global.h \
    PluginsListFrame.hpp \
    ConfigurePluginPropertiesDialog.hpp \
    PluginPropertyEditorFrame.hpp \
    VariantPropertyManagerEx.hpp \
    XVariantConverter.hpp \
    UIPersistenceService.hpp \
    IUIPersistenceService.hpp \
    ImageFileService.hpp \
    IImageFileService.hpp \
    ApplicationSettingsService.hpp \
    IApplicationSettingsService.hpp \
    UITools.hpp \
    IPersistentUIContent.hpp \
    UserCommand.hpp \
    ErrorProviderHelper.hpp \
    HistogramWidget.hpp \
    DraggableTreeWidget.hpp \
    XImageInterface.hpp \
    DraggableListWidget.hpp \
    CustomPropertyManagers.hpp \
    PluginDescriptionDialog.hpp \
    FavouritePluginsManager.hpp \
    GlobalServiceManager.hpp \
    ScriptEditorDialog.hpp \
    IHelpService.hpp \
    HelpService.hpp \
    GoToLineDialog.hpp \
    HuePickerDialog.hpp \
    HuePickerWidget.hpp \
    FindTextDialog.hpp \
    HistogramInfoFrame.hpp \
    MorphologyStructuringElementDialog.hpp \
    ConvolutionKernelDialog.hpp

FORMS += \
    PluginsListFrame.ui \
    PluginPropertyEditorFrame.ui \
    ConfigurePluginPropertiesDialog.ui \
    PluginDescriptionDialog.ui \
    ScriptEditorDialog.ui \
    GoToLineDialog.ui \
    HuePickerDialog.ui \
    FindTextDialog.ui \
    HistogramInfoFrame.ui \
    MorphologyStructuringElementDialog.ui \
    ConvolutionKernelDialog.ui

RESOURCES += \
    resources.qrc
