# dev_raspberry plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = dev_raspberry.cpp \
	RaspberryPiCameraPlugin.cpp RaspberryPiCameraPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
	-I../../../../../afx/afx_video+ -I../../../../../afx/afx_platform+ \
	-I../../../../../afx/video/afx_video_mjpeg+ \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS =  -liplugin -lafx_video_mjpeg+ -lafx_imaging_formats -lafx_imaging \
	-lafx_platform+ -lafx_types+ -lafx_types \
	-ljpeg -lexif -lcurldll -lws2_32
