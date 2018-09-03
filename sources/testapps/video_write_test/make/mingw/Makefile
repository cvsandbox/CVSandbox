# MinGW makefile

include ../src.mk
include ../../../../make/settings/mingw/compiler_cpp.mk

OUT = video_write_test.exe

LIBDIR = -L..\..\..\..\..\build\$(TARGET)\$(BUILD_TYPE)\lib

LDFLAGS += $(LIBDIR) -fopenmp

include ../../../../make/settings/mingw/build_app.mk
