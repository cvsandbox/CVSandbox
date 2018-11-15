# list of projects to build
BUILDS = iplugin pluginmgr pluginscripting automationserver

# current directory (supposed to be directory of the Makefile)
CD=$(CURDIR)

all:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make && cd $(CD) && echo."

clean:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make clean && cd $(CD) && echo."

debug:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make debug && cd $(CD) && echo."
