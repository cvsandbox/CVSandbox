# vs_dshow plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = vs_dshow.cpp \
	DirectShowVideoSourcePlugin.cpp DirectShowVideoSourcePluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
	-I../../../../../afx/afx_video+ -I../../../../../afx/afx_platform+ \
	-I../../../../../afx/video/afx_video_dshow+ \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_video_dshow+ -lafx_platform+ -lafx_types+ -lafx_types \
	-lole32 -loleaut32 -lstrmiids -luuid

CFLAGS += $(INCLUDES)
LDFLAGS += $(LIBS)
