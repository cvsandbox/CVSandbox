# se_lua plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = se_lua.cpp \
	LuaScriptingEnginePlugin.cpp LuaScriptingEnginePluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
	-I../../../../../core/iplugin -I../../../../../core/pluginmgr \
	-I../../../../../core/pluginscripting \
    -I../../../../../images

# libraries to use
LIBS = -lpluginscripting -llua -lpluginmgr -liplugin -lafx_types+ -lafx_types
