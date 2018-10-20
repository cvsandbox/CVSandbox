# ip_tools plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = ip_tools.cpp \
	BlendImagesPlugin.cpp BlendImagesPluginDescriptor.cpp \
	FadeImagesPlugin.cpp FadeImagesPluginDescriptor.cpp \
	GradientReColoringPlugin.cpp GradientReColoringPluginDescriptor.cpp \
	GradientReColoring2Plugin.cpp GradientReColoring2PluginDescriptor.cpp \
	GradientReColoring4Plugin.cpp GradientReColoring4PluginDescriptor.cpp \
	HeatGradientPlugin.cpp HeatGradientPluginDescriptor.cpp \
	ImageDrawingPlugin.cpp ImageDrawingPluginDescriptor.cpp \
	MoveTowardsImagesPlugin.cpp MoveTowardsImagesPluginDescriptor.cpp \
	PutTextPlugin.cpp PutTextPluginDescriptor.cpp \
	SaltAndPepperNoisePlugin.cpp SaltAndPepperNoisePluginDescriptor.cpp \
	SimplePosterizationPlugin.cpp SimplePosterizationPluginDescriptor.cpp \
	UniformAdditiveNoisePlugin.cpp UniformAdditiveNoisePluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging \
	-I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_imaging -lafx_types

CFLAGS += $(INCLUDES)
LDFLAGS += $(LIBS)
