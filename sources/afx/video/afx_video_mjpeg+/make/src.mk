# afx_video_mjpeg+ source files

# search path for source files
VPATH = ../../

# source files
SRC = XHttpCommandsThread.cpp XJpegHttpStream.cpp XMjpegHttpStream.cpp base64.cpp tools.cpp

# additional include folders
INCLUDES = -I../../../../afx_types -I../../../../afx_types+ \
	-I../../../../afx_video+ -I../../../../afx_platform+ \
    -I../../../../afx_imaging_formats
