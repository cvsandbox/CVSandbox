# libafx_video_ffmpeg+ source files

# search path for source files
VPATH = ../../

# source files
SRC =  XFFmpegNetworkStream.cpp XFFmpegVideoFileReader.cpp XFFmpegVideoFileWriter.cpp

# additional include folders
INCLUDES = -I../../../../afx_types -I../../../../afx_types+ \
	-I../../../../afx_video+ -I../../../../afx_platform+
