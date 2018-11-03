# fmt_jpeg plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = fmt_jpeg.cpp \
	JpegExporterPlugin.cpp JpegExporterPluginDescriptor.cpp \
	JpegImporterPlugin.cpp JpegImporterPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging_formats \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_imaging_formats -lafx_imaging -lafx_types -ljpeg -lexif

CFLAGS += $(INCLUDES)
LDFLAGS += $(LIBS)
