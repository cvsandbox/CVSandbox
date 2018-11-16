# MinGW makefile

include ../src.mk
include ../../../../make/settings/mingw/compiler_cpp.mk

OUT = plugins_memory_test.exe

LIBDIR = -L../../../../../build/$(TARGET)/$(BUILD_TYPE)/lib

LDFLAGS += $(LIBDIR)

include ../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y ..\..\test_images\*.* $(OUT_FOLDER)test_images\*
