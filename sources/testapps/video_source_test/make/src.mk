# video_source_test test application's source files

# search path for source files
VPATH = ../../

# source files
SRC = video_source_test.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
	-I../../../../afx/afx_video+ \
	-I../../../../afx/video/afx_video_dshow+ \
	-I../../../../afx/video/afx_video_mjpeg+ \
	-I../../../../afx/video/afx_video_ffmpeg+

# libraries to use
LIBS = -lafx_video_dshow+ -lafx_video_mjpeg+ -lafx_video_ffmpeg+ -lafx_platform+ \
	-lafx_imaging_formats -lafx_types+ -lafx_types \
	-lole32 -loleaut32 -lstrmiids -luuid \
	-ljpeg -lcurldll -lws2_32 \
	-lavcodec -lavformat -lavutil -lswscale

CFLAGS += $(INCLUDES)
LDFLAGS += $(LIBS)
