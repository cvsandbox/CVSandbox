# MinGW makefile

include ../src.mk
include ../../../../../make/settings/mingw/compiler_cpp.mk

OUT = dev_gamepad.dll
OUT_SUB_FOLDER = cvsplugins\dev_gamepad

LIBDIR = -L../../../../../../build/$(TARGET)/$(BUILD_TYPE)/lib

LDFLAGS += -shared

include ../../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y "..\..\*.txt" $(OUT_FOLDER)
	xcopy /S /I /Y "..\..\examples" $(OUT_FOLDER)examples\
