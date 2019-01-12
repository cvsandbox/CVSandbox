# MinGW makefile

include ../src.mk
include ../../../../../make/settings/mingw/compiler_cpp.mk

OUT = vs_screen_cap.dll
OUT_SUB_FOLDER = cvsplugins\vs_screen_cap

LIBDIR = -L../../../../../../build/$(TARGET)/$(BUILD_TYPE)/lib

LDFLAGS += -shared -fopenmp

include ../../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y "..\..\*.txt" $(OUT_FOLDER)
