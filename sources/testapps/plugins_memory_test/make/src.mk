# plugins_memory_test test application's source files

# search path for source files
VPATH = ../../

# source files
SRC = plugins_memory_test.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
    -I../../../../afx/afx_platform+ \
    -I../../../../core/iplugin -I../../../../core/pluginmgr

# libraries to use
LIBS = -lpluginmgr -liplugin -lafx_platform+ -lafx_types+ -lafx_types
