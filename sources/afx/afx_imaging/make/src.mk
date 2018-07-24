# afx_imaging source files

# search path for source files
VPATH = ../../

# source files
SRC =  additive_noise.c alpha.c \
	binary_dilatation_3x3.c binary_erosion_3x3.c binary2grayscale.c blob_counter.c blur_image.c \
	canny_edge_detector.c color_conversion.c color_filtering.c color_maps.c color_remapping.c color2grayscale.c \
	contrast_stretching.c convolution.c \
	dilatation_3x3.c distance_transform.c drawing.c drawing_text.c \
	edge_detectors.c erosion_3x3.c error_diffusion_dithering.c extract_channel.c extract_channel_nrgb.c \
	gaussian.c gray_world.c grayscale2color.c \
	histogram_equalization.c hsl_color_filtering.c \
	image_statistics.c indexed2color.c invert.c \
	mean_3x3.c mean_shift.c mirror.c morphology.c \
	ordered_dithering.c otsu.c \
	pixellate.c \
	quadrilateral_transform.c \
	resize_bilinear.c resize_nearest_neightbor.c rotate_bilinear.c rotate_rgb.c rotate90.c \
	run_length_smoothing.c \
	salt_and_pepper_noise.c sepia.c set_hue.c shape_checker.c shift_image.c simple_posterization.c swap_rgb.c \
	threshold.c two_source_image_routines.c

# additional include folders
INCLUDES = -I../../../afx_types

CFLAGS += $(INCLUDES)
