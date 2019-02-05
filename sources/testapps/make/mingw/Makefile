# list of projects to build
BUILDS = automation_test \
    plugins_memory_test \
    scripting_test \
    video_read_test \
    video_source_test \
    video_write_test

# current directory (supposed to be directory of the Makefile)
CD=$(CURDIR)

all:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make && cd $(CD) && echo."

clean:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make clean && cd $(CD) && echo."

debug:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make debug && cd $(CD) && echo."
