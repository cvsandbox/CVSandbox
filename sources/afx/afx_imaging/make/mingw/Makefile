# MinGW makefile

include ../src.mk
include ../../../../make/settings/mingw/compiler_c.mk

OUT = libafx_imaging.a

CFLAGS += -fopenmp -mssse3

# MinGW bug when using OpenMP with SSE 
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=48659
CFLAGS += -mstackrealign

include ../../../../make/settings/mingw/build_lib.mk
