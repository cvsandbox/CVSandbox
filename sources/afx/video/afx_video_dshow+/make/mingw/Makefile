# MinGW makefile

include ../src.mk
include ../../../../../make/settings/mingw/compiler_cpp.mk

OUT = libafx_video_dshow+.a

CFLAGS += -Wno-unknown-pragmas -Wno-reorder -mssse3

# extra include folders
INCLUDES += -I../../../../../../build/mingw/$(BUILD_TYPE)/include
  
include ../../../../../make/settings/mingw/build_lib.mk
