# Static library build steps valid for MS Windows + MinGW environment

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

# compiler options
ifneq "$(findstring debug, $(MAKECMDGOALS))" ""
# "Debug" build - no optimization, and debugging symbols 
OUT_FOLDER = "$(SELF_DIR)..\..\..\..\build\$(TARGET)\debug\lib\"
else 
# "Release" build - optimization, and no debug symbols 
OUT_FOLDER = "$(SELF_DIR)..\..\..\..\build\$(TARGET)\release\lib\"
endif 

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
