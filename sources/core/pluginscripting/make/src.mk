# libpluginscripting source files

# search path for source files
VPATH = ../../

# source files
SRC = XDefaultScriptingHost.cpp XLuaPluginScripting.cpp XLuaPluginScripting_UserTypes.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
	-I../../../../afx/afx_platform+ \
	-I../../../iplugin -I../../../pluginmgr
