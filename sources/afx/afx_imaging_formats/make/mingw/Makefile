# MinGW makefile

include ../src.mk
include ../../../../make/settings/mingw/compiler_c.mk

OUT = libafx_imaging_formats.a

CFLAGS += -fopenmp
CFLAGS += -DAFX_CORRECT_ORIENTATION

# extra include folder
INCLUDES += -I../../../../../build/mingw/$(BUILD_TYPE)/include

include ../../../../make/settings/mingw/build_lib.mk
