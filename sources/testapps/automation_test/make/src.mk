# automation_test test application's source files

# search path for source files
VPATH = ../../

# source files
SRC = automation_test.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
    -I../../../../afx/afx_platform+ \
    -I../../../../core/iplugin -I../../../../core/pluginmgr \
    -I../../../../core/automationserver

# libraries to use
LIBS = -lautomationserver -lpluginmgr -liplugin -lafx_platform+ -lafx_types+ -lafx_types
