# MinGW makefile

include ../src.mk
include ../../../../../make/settings/mingw/compiler_cpp.mk

OUT = vs_effects.dll
OUT_SUB_FOLDER = cvsplugins\vs_effects

LIBDIR = -L../../../../../../build/$(TARGET)/$(BUILD_TYPE)/lib

LDFLAGS += -fopenmp -shared

include ../../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y "..\..\*.txt" $(OUT_FOLDER)
