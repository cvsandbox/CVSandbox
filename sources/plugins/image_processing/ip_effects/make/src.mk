# ip_effects plug-in source files

# search path for source files
VPATH = ../../

# source files
SRC = ip_effects.cpp \
    ApplyTexturePlugin.cpp ApplyTexturePluginDescriptor.cpp \
    ColorizePlugin.cpp ColorizePluginDescriptor.cpp \
    DropLightPlugin.cpp DropLightPluginDescriptor.cpp \
    EmbossPlugin.cpp EmbossPluginDescriptor.cpp \
    FuzzyBorderPlugin.cpp FuzzyBorderPluginDescriptor.cpp \
    GenerateCloudsTexturePlugin.cpp GenerateCloudsTexturePluginDescriptor.cpp \
    GenerateFuzzyBorderTexturePlugin.cpp GenerateFuzzyBorderTexturePluginDescriptor.cpp \
    GenerateGrainTexturePlugin.cpp GenerateGrainTexturePluginDescriptor.cpp \
    GenerateMarbleTexturePlugin.cpp GenerateMarbleTexturePluginDescriptor.cpp \
    GenerateRoundedBorderTexturePlugin.cpp GenerateRoundedBorderTexturePluginDescriptor.cpp \
    GenerateTextileTexturePlugin.cpp GenerateTextileTexturePluginDescriptor.cpp \
    GrainPlugin.cpp GrainPluginDescriptor.cpp \
    JitterPlugin.cpp JitterPluginDescriptor.cpp \
    OilPaintingPlugin.cpp OilPaintingPluginDescriptor.cpp \
    RotateHuePlugin.cpp RotateHuePluginDescriptor.cpp \
    RotateRgbChannelsPlugin.cpp RotateRgbChannelsPluginDescriptor.cpp \
    RoundedBorderPlugin.cpp RoundedBorderPluginDescriptor.cpp \
    SaturatePlugin.cpp SaturatePluginDescriptor.cpp \
    SepiaPlugin.cpp SepiaPluginDescriptor.cpp \
    SetHuePlugin.cpp SetHuePluginDescriptor.cpp \
    TextileTexturePlugin.cpp TextileTexturePluginDescriptor.cpp \
    VignettingPlugin.cpp VignettingPluginDescriptor.cpp

# additional include folders
INCLUDES = -I../../../../../afx/afx_types -I../../../../../afx/afx_imaging \
	-I../../../../../afx/afx_imaging_effects -I../../../../../core/iplugin -I../../../../../images

# libraries to use
LIBS = -liplugin -lafx_imaging_effects -lafx_imaging -lafx_types
