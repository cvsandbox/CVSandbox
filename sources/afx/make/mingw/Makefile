# list of projects to build
BUILDS = afx_types \
         afx_types+ \
         afx_platform+ \
         afx_imaging \
         afx_imaging_effects \
         afx_imaging_formats \
         afx_video+ \
         video/afx_video_mjpeg+ \
         video/afx_video_dshow+ \
         video/afx_video_ffmpeg+

# current directory (supposed to be directory of the Makefile)
CD=$(CURDIR)

all:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make && cd $(CD) && echo."

clean:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make clean && cd $(CD) && echo."

debug:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make debug && cd $(CD) && echo."
