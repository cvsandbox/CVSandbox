# MinGW makefile

include ../src.mk
include ../../../../make/settings/mingw/compiler_cpp.mk

OUT = cvssr.exe

LIBDIR = -L../../../../../build/$(TARGET)/$(BUILD_TYPE)/lib

LDFLAGS += $(LIBDIR)

include ../../../../make/settings/mingw/build_app.mk

post_build:
	xcopy /Y ..\..\*.lua $(OUT_FOLDER)
