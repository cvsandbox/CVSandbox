# dev_sysinfo plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = dev_sysinfo.cpp \
    LedKeysPlugin.cpp LedKeysPluginDescriptor.cpp \
    PerformanceInfoPlugin.cpp PerformanceInfoPluginDescriptor.cpp \
    PowerInfoPlugin.cpp PowerInfoPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -lwinmm -lpdh -liplugin -lafx_types
