# scripting_test test application's source files

# search path for source files
VPATH = ../../

# source files
SRC = scripting_test.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
	-I../../../../core/iplugin -I../../../../core/pluginmgr \
    -I../../../../core/pluginscripting

# libraries to use
LIBS = -lpluginscripting -llua -lpluginmgr -liplugin -lafx_platform+ -lafx_types+ -lafx_types
