# vs_screen_cap plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = vs_screen_cap.cpp \
	ScreenCapturePlugin.cpp ScreenCapturePluginDescriptor.cpp 

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
    -I../../../../../afx/afx_imaging \
	-I../../../../../afx/afx_video+ -I../../../../../afx/afx_platform+ \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS =  -liplugin -lafx_platform+ -lafx_imaging -lafx_types+ -lafx_types -lGdi32
