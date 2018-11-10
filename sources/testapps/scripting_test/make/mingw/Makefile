# MinGW makefile

include ../src.mk
include ../../../../make/settings/mingw/compiler_cpp.mk

OUT = scripting_test.exe

LIBDIR = -L..\..\..\..\..\build\$(TARGET)\$(BUILD_TYPE)\lib

LDFLAGS += $(LIBDIR)

include ../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y ..\..\image1.jpg $(OUT_FOLDER)
	xcopy /Y ..\..\test1.lua $(OUT_FOLDER)
	xcopy /Y ..\..\test2.lua $(OUT_FOLDER)
