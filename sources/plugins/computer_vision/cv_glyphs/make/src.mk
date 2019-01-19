# cv_glyphs plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = cv_glyphs.cpp \
    GlyphDetectorPlugin.cpp GlyphDetectorPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging \
	-I../../../../../afx/afx_vision \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_vision -lafx_imaging -lafx_types
