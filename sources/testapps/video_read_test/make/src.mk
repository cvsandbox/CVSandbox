# video_read_test test application's source files

# search path for source files
VPATH = ../../

# source files
SRC = video_read_test.cpp

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_types+ \
	-I../../../../afx/afx_imaging_formats \
	-I../../../../afx/afx_video+ \
	-I../../../../afx/video/afx_video_ffmpeg+    

# libraries to use
LIBS = -lafx_video_dshow+ -lafx_video_mjpeg+ -lafx_video_ffmpeg+ -lafx_platform+ \
	-lafx_imaging_formats -lafx_types+ -lafx_types \
	-lavcodec -lavformat -lavutil -lswscale -lpng -lz
