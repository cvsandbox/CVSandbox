# iplugin source files

# search path for source files
VPATH = ../../

# source files
SRC =  ifamily.c ifunction.c imodule.c iplugintypes.c iproperty.c \
	registry.c tools.c xmodule.c

# additional include folders
INCLUDES = -I../../../../afx/afx_types -I../../../../afx/afx_imaging \
	-I../../../../images
