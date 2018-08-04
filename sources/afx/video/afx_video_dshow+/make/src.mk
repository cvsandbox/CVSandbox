# afx_video_dshow+ source files

# search path for source files
VPATH = ../../

# source files
SRC = XDeviceCapabilities.cpp XDeviceName.cpp XDevicePinInfo.cpp XLocalVideoDevice.cpp

# additional include folders
INCLUDES = -I../../../../afx_types -I../../../../afx_types+ \
	-I../../../../afx_video+ -I../../../../afx_platform+
