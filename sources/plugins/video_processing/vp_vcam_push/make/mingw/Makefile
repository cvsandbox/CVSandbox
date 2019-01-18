# MinGW makefile

include ../src.mk
include ../../../../../make/settings/mingw/compiler_cpp.mk

OUT = vp_vcam_push.dll
OUT_SUB_FOLDER = cvsplugins\vp_vcam_push

LIBDIR = -L../../../../../../build/$(TARGET)/$(BUILD_TYPE)/lib

LDFLAGS += -shared -fopenmp

include ../../../../../make/settings/mingw/build_app.mk

post_build: $(OUT)
	xcopy /Y "..\..\*.txt" $(OUT_FOLDER)
