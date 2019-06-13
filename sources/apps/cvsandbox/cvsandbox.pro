TARGET = cvsandbox
TEMPLATE = app

CONFIG += qt
greaterThan(QT_MAJOR_VERSION, 4): QT *= widgets

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0

INCLUDEPATH += ../../afx/afx_types/ \
               ../../afx/afx_types+/ \
               ../../afx/afx_imaging/ \
               ../../afx/afx_video+/ \
               ../../core/iplugin/ \
               ../../core/pluginmgr/ \
               ../../core/automationserver/ \
               ../../../externals/sources/qtpropertybrowser/src \
               ../cvsandboxtools

message( $$MAKEFILE_GENERATOR )

contains(QMAKE_TARGET.arch, x86_64) {
    message("64-bit build")
    CONFIG (debug, debug|release) {
        message( "Debug build" )
        OUTDIR = debug64
    } else {
        message( "Release build" )
        OUTDIR = release64
    }
} else {
    message("32-bit build")
    CONFIG (debug, debug|release) {
        message( "Debug build" )
        OUTDIR = debug
    } else {
        message( "Release build" )
        OUTDIR = release
    }
}

contains(MAKEFILE_GENERATOR, "MSBUILD") || contains(MAKEFILE_GENERATOR, "MSVC.NET") {
    message( "MSVC build" )
    
    MSVC_LIBS_DIR = $$PWD/../../../build/msvc/$$OUTDIR/lib

    DESTDIR = $${MSVC_LIBS_DIR}/../bin

    LIBS += $$MSVC_LIBS_DIR/automationserver.lib \
            $$MSVC_LIBS_DIR/cvsandboxtools.lib \
            $$MSVC_LIBS_DIR/pluginmgr.lib \
            $$MSVC_LIBS_DIR/iplugin.lib \
            $$MSVC_LIBS_DIR/afx_platform+.lib \
            $$MSVC_LIBS_DIR/afx_imaging.lib \
            $$MSVC_LIBS_DIR/afx_types+.lib \
            $$MSVC_LIBS_DIR/afx_types.lib

    QMAKE_CXXFLAGS += /D "_CRT_SECURE_NO_WARNINGS"
    QMAKE_LFLAGS += /INCREMENTAL:NO

    LIBS += -lPdh
    LIBS += -lWinmm
}

contains(MAKEFILE_GENERATOR, "MINGW") {
    message( "MinGW build" )

    MINGW_LIBS_DIR = $$PWD/../../../build/mingw/$$OUTDIR/lib

    DESTDIR = $${MINGW_LIBS_DIR}/../bin

    LIBS += -L$${MINGW_LIBS_DIR} \
            -lautomationserver \
            -lcvsandboxtools \
            -lpluginmgr \
            -liplugin \
            -lafx_platform+ \
            -lafx_imaging \
            -lafx_types+ \
            -lafx_types

    QMAKE_CXXFLAGS += -std=c++0x
    LIBS += -fopenmp

    LIBS += -lPdh
    LIBS += -lWinmm
}

contains(MAKEFILE_GENERATOR, "MSBUILD") || contains(MAKEFILE_GENERATOR, "MSVC.NET") {
    QMAKE_CXXFLAGS += /FS
}

message( "Out: " $$DESTDIR )

SOURCES += main.cpp\
    MainWindow.cpp \
    ServiceManager.cpp \
    UserCommands.cpp \
    WizardDialog.cpp \
    AddCameraDialog.cpp \
    SelectPluginPage.cpp \
    ConfigurePluginPage.cpp \
    VideoSourcePlayer.cpp \
    VideoSourceInPlugin.cpp \
    SingleCameraView.cpp \
    XGuidGenerator.cpp \
    ProjectTreeFrame.cpp \
    ProjectManager.cpp \
    ProjectObject.cpp \
    EditFolderDialog.cpp \
    FolderProjectObject.cpp \
    ProjectObjectFactory.cpp \
    CameraProjectObject.cpp \
    PropertyPagesDialog.cpp \
    EditCameraPropertiesDialog.cpp \
    VideoSourcePlayerQt.cpp \
    VideoSourcePlayerFactory.cpp \
    AddSandboxDialog.cpp \
    SelectDevicesPage.cpp \
    ProjectObjectNamePage.cpp \
    ProjectObjectIcon.cpp \
    ConfigureCamerasViewPage.cpp \
    GridWidget.cpp \
    CamerasViewConfiguration.cpp \
    SandboxProjectObject.cpp \
    EditSandboxPropertiesDialog.cpp \
    VideoSourceInAutomationServer.cpp \
    ProjectObjectOpener.cpp \
    SandboxView.cpp \
    MultiCameraView.cpp \
    GridButton.cpp \
    ProjectObjectViewFrame.cpp \
    MainViewFrame.cpp \
    AboutDialog.cpp \
    SandboxWizardDialog.cpp \
    VideoProcessingWizardPage.cpp \
    ProjectObjectSerializationHelper.cpp \
    SandboxSettingsPage.cpp \
    SandboxSettings.cpp \
    CameraInfoDialog.cpp \
    PerformanceMonitorService.cpp \
    VideoProcessingInfoDialog.cpp \
    IProjectManager.cpp \
    version.cpp \
    RenameProcessingStepDialog.cpp \
    VideoSnapshotDialog.cpp \
    EditVideoSourceRunTimeProperties.cpp \
    ScriptingThreadsPage.cpp \
    EditScriptingThreadDialog.cpp \
    SandboxVariablesMonitorFrame.cpp

HEADERS  += MainWindow.hpp \
    ServiceManager.hpp \
    UserCommands.hpp \
    IMainWindowService.hpp \
    WizardDialog.hpp \
    WizardPageFrame.hpp \
    AddCameraDialog.hpp \
    SelectPluginPage.hpp \
    ConfigurePluginPage.hpp \
    VideoSourcePlayer.hpp \
    VideoSourceInPlugin.hpp \
    SingleCameraView.hpp \
    XGuidGenerator.hpp \
    ProjectTreeFrame.hpp \
    IProjectTreeUI.hpp \
    IProjectManager.hpp \
    ProjectManager.hpp \
    ProjectObject.hpp \
    EditFolderDialog.hpp \
    FolderProjectObject.hpp \
    ProjectObjectFactory.hpp \
    CameraProjectObject.hpp \
    PropertyPagesDialog.hpp \
    EditCameraPropertiesDialog.hpp \
    VideoSourcePlayerQt.hpp \
    VideoSourcePlayerFactory.hpp \
    AddSandboxDialog.hpp \
    SelectDevicesPage.hpp \
    ProjectObjectNamePage.hpp \
    ProjectObjectIcon.hpp \
    ConfigureCamerasViewPage.hpp \
    GridWidget.hpp \
    CamerasViewConfiguration.hpp \
    SandboxProjectObject.hpp \
    EditSandboxPropertiesDialog.hpp \
    VideoSourceInAutomationServer.hpp \
    ProjectObjectOpener.hpp \
    SandboxView.hpp \
    MultiCameraView.hpp \
    GridButton.hpp \
    ProjectObjectViewFrame.hpp \
    MainViewFrame.hpp \
    AboutDialog.hpp \
    SandboxWizardDialog.hpp \
    VideoProcessingWizardPage.hpp \
    ProjectObjectSerializationHelper.hpp \
    SandboxSettingsPage.hpp \
    SandboxSettings.hpp \
    CameraInfoDialog.hpp \
    IPerformanceMonitorService.hpp \
    PerformanceMonitorService.hpp \
    VideoProcessingInfoDialog.hpp \
    DeviceListWidgetItem.hpp \
    version.hpp \
    RenameProcessingStepDialog.hpp \
    VideoSnapshotDialog.hpp \
    EditVideoSourceRunTimeProperties.hpp \
    ScriptingThreadsPage.hpp \
    EditScriptingThreadDialog.hpp \
    ScriptingThreadDesc.hpp \
    SandboxVariablesMonitorFrame.hpp

FORMS    += MainWindow.ui \
    WizardDialog.ui \
    SelectPluginPage.ui \
    ConfigurePluginPage.ui \
    SingleCameraView.ui \
    ProjectTreeFrame.ui \
    EditFolderDialog.ui \
    PropertyPagesDialog.ui \
    SelectDevicesPage.ui \
    ProjectObjectNamePage.ui \
    ConfigureCamerasViewPage.ui \
    SandboxView.ui \
    MultiCameraView.ui \
    GridButton.ui \
    MainViewFrame.ui \
    AboutDialog.ui \
    VideoProcessingWizardPage.ui \
    SandboxSettingsPage.ui \
    CameraInfoDialog.ui \
    VideoProcessingInfoDialog.ui \
    RenameProcessingStepDialog.ui \
    VideoSnapshotDialog.ui \
    ScriptingThreadsPage.ui \
    EditScriptingThreadDialog.ui \
    SandboxVariablesMonitorFrame.ui

RESOURCES += \
    resources.qrc

win32:RC_FILE += resources_win.rc
