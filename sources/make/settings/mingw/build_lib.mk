# Static library build steps valid for MS Windows + MinGW environment

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

OUT_FOLDER = "$(SELF_DIR)..\..\..\..\build\$(TARGET)\$(BUILD_TYPE)\lib\"

CFLAGS += $(INCLUDES)

all: build

debug: build
 
%.o: $(SRC_FILE_EXT)
	$(COMPILER) $(CFLAGS) -c $^ -o $@

$(OUT): $(OBJ)
	$(ARCHIVER) rcs $(OUT) $(OBJ)

build: $(OUT)
	xcopy /Y $(OUT) $(OUT_FOLDER)

clean:
	del $(OBJ) $(OUT)
