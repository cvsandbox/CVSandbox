# vs_ffmpeg plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = vs_ffmpeg.cpp \
    FileVideoSourcePlugin.cpp FileVideoSourcePluginDescriptor.cpp \
	NetworkStreamVideoSourcePlugin.cpp NetworkStreamVideoSourcePluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
	-I../../../../../afx/afx_video+ -I../../../../../afx/afx_platform+ \
	-I../../../../../afx/video/afx_video_ffmpeg+ \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS =  -liplugin -lafx_video_ffmpeg+ -lafx_imaging_formats \
	-lafx_platform+ -lafx_types+ -lafx_types \
    -lavcodec -lavformat -lavutil -lswscale
