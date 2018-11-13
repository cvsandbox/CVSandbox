# MinGW compiler common settings

TARGET = mingw
SRC_FILE_EXT = %.cpp

OBJ = $(SRC:.cpp=.o)

# compiler
COMPILER = $(MINGW_BIN)/g++.exe
# lib archiver
ARCHIVER = $(MINGW_BIN)/ar.exe

CFLAGS += -Wall -std=gnu++0x -fno-rtti

# compiler options
ifneq "$(findstring debug, $(MAKECMDGOALS))" ""
# "Debug" build - no optimization, and debugging symbols 
CFLAGS += -O0 -g
BUILD_TYPE = debug
else 
# "Release" build - optimization, and no debug symbols 
CFLAGS += -O3 -s -DNDEBUG
BUILD_TYPE = release
endif 
