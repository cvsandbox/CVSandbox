# cv_bar_codes plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = cv_bar_codes.cpp \
    BarCodeDetectorPlugin.cpp BarCodeDetectorPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging \
	-I../../../../../afx/afx_vision \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_vision -lafx_imaging -lafx_types
