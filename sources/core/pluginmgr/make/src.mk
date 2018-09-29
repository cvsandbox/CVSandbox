# pluginmgr source files

# search path for source files
VPATH = ../../

# source files
SRC = XDevicePlugin.cpp \
	XFamiliesCollection.cpp XFamily.cpp XFunctionDescriptor.cpp \
	XImageExportingPlugin.cpp XImageGenerationPlugin.cpp \
	XImageImportingPlugin.cpp \
	XImageProcessingFilterPlugin.cpp XImageProcessingFilterPlugin2.cpp \
	XImageProcessingPlugin.cpp \
	XModulesCollection.cpp XPlugin.cpp XPluginWrapperFactory.cpp XPluginDescriptor.cpp \
	XPluginsCollection.cpp XPluginsEngine.cpp XPluginsModule.cpp \
	XPropertyDescriptor.cpp \
	XScriptingEnginePlugin.cpp \
	XVideoProcessingPlugin.cpp XVideoSourcePlugin.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
	-I../../../../afx/afx_imaging -I../../../iplugin \
	-I../../../../afx/afx_platform+ 
