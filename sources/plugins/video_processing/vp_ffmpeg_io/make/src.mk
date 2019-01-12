# vp_ffmpeg_io plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = vp_ffmpeg_io.cpp \
	VideoFileWriterPlugin.cpp VideoFileWriterPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
	-I../../../../../afx/afx_platform+ \
	-I../../../../../afx/video/afx_video_ffmpeg+ \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS =  -liplugin -lafx_video_ffmpeg+ -lafx_imaging_formats \
	-lafx_platform+ -lafx_types+ -lafx_types \
    -lavcodec -lavformat -lavutil -lswscale
