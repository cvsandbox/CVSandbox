# dev_gamepad plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = dev_gamepad.cpp \
    GamepadPlugin.cpp GamepadPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -lwinmm -liplugin -lafx_types
