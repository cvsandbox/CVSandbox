# afx_imaging_effects source files

# search path for source files
VPATH = ../../

# source files
SRC = colorize.c \
    drop_light.c \
    emboss.c \
	frames_and_borders.c \
	jitter.c \
	oil_painting.c \
	perlin_noise_textures.c \
    rotate_hue.c \
	saturation.c vignetting.c \
	textures.c

# additional include folders
INCLUDES = -I../../../afx_types -I../../../afx_imaging
