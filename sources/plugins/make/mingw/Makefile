# list of projects to build
BUILDS = image_processing\ip_stdimaging \
    image_processing\ip_tools \
    image_processing\ip_effects \
    image_processing\ip_blobs_processing \
    image_formats\fmt_jpeg \
    image_formats\fmt_png \
    computer_vision\cv_bar_codes \
    computer_vision\cv_glyphs \
    computer_vision\cv_motion \
    video_sources\vs_mjpeg \
    video_sources\vs_dshow \
    video_sources\vs_effects \
    video_sources\vs_ffmpeg \
    video_sources\vs_repeater \
    video_sources\vs_screen_cap \
    video_sources\vs_image_folder \
    video_processing\vp_ffmpeg_io \
    video_processing\vp_vcam_push \
    scripting_engine\se_lua \
    devices\dev_sysinfo \
    devices\dev_gamepad \
    devices\dev_raspberry \
    devices\dev_com

# current directory (supposed to be directory of the Makefile)
CD=$(CURDIR)

all:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make && cd $(CD) && echo."

clean:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make clean && cd $(CD) && echo."

debug:
	@cmd /C "FOR %B IN ($(BUILDS)) DO @cd ..\..\%B\make\mingw && make debug && cd $(CD) && echo."
