# MinGW makefile

include ../src.mk
include ../../../../../make/settings/mingw/compiler_cpp.mk

OUT = fmt_png.dll
OUT_SUB_FOLDER = cvsplugins\fmt_png\

LIBDIR = -L..\..\..\..\..\..\build\$(TARGET)\$(BUILD_TYPE)\lib

LDFLAGS += -shared -fopenmp

include ../../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y "..\..\*.txt" $(OUT_FOLDER)
