# MinGW makefile

include ../src.mk
include ../../../../../make/settings/mingw/compiler_cpp.mk

OUT = vs_dshow.dll
OUT_SUB_FOLDER = cvsplugins\vs_dshow

LIBDIR = -L..\..\..\..\..\..\build\$(TARGET)\$(BUILD_TYPE)\lib

LDFLAGS += -shared

include ../../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y "..\..\*.txt" $(OUT_FOLDER)
