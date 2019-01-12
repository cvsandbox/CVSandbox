# vs_image_folder plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = vs_image_folder.cpp \
    ImageFolderVideoSourcePlugin.cpp ImageFolderVideoSourcePluginDescriptor.cpp \
    ImageFolderWriterPlugin.cpp ImageFolderWriterPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_types+ \
    -I../../../../../afx/afx_imaging -I../../../../../afx/afx_imaging_formats \
	-I../../../../../afx/afx_video+ -I../../../../../afx/afx_platform+ \
	-I../../../../../afx/video/afx_video_mjpeg+ \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_imaging_formats -lafx_imaging \
	-lafx_platform+ -lafx_types+ -lafx_types -ljpeg -lexif -lpng
