# vs_repeater plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = vs_repeater.cpp \
	VideoRepeaterPlugin.cpp VideoRepeaterPluginDescriptor.cpp \
	VideoRepeaterPushPlugin.cpp VideoRepeaterPushPluginDescriptor.cpp \
    VideoRepeaterRegistry.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
	-I../../../../../afx/afx_video+ -I../../../../../afx/afx_platform+ \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS =  -liplugin -lafx_platform+ -lafx_types+ -lafx_types
