# ip_blobs_processing plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = ip_blobs_processing.cpp \
    FillHolesPlugin.cpp FillHolesPluginDescriptor.cpp \
    FilterBlobsBySizePlugin.cpp FilterBlobsBySizePluginDescriptor.cpp \
    FilterCircleBlobsPlugin.cpp FilterCircleBlobsPluginDescriptor.cpp \
    FilterQuadrilateralBlobsPlugin.cpp FilterQuadrilateralBlobsPluginDescriptor.cpp \
    FindBiggestBlobPlugin.cpp FindBiggestBlobPluginDescriptor.cpp \
    FindBlobsBySizePlugin.cpp FindBlobsBySizePluginDescriptor.cpp \
    KeepBiggestBlobPlugin.cpp KeepBiggestBlobPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_imaging -lafx_types

CFLAGS += $(INCLUDES)
LDFLAGS += $(LIBS)
