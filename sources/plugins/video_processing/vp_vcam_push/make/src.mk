# vp_vcam_push plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = vp_vcam_push.cpp \
	VCamPushPlugin.cpp VCamPushPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
	-I../../../../../afx/afx_imaging \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS =  -liplugin -lafx_imaging -lafx_types+ -lafx_types
