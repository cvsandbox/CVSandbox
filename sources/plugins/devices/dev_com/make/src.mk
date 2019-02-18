# dev_com plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = dev_com.cpp \
    SerialPortPlugin.cpp SerialPortPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_types
