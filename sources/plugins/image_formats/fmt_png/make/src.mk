# fmt_png plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = fmt_png.cpp \
    PngExporterPlugin.cpp PngExporterPluginDescriptor.cpp \
	PngImporterPlugin.cpp PngImporterPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging_formats \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_imaging_formats -lafx_types -lpng
