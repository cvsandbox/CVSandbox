@echo off

@rem  1 - Delete old files first
rmdir /S /Q Files
mkdir Files

@rem  2 - Copy main binaries
set TO_COPY=cvsandbox.exe cvsandboxtools.dll ^
            libcurl.dll libexif-12.dll libjpeg-8.dll libpng16-16.dll zlib1.dll ^
            avcodec-57.dll avfilter-6.dll avformat-57.dll avutil-55.dll ^
            swresample-2.dll swscale-4.dll ^
            qtpropertybrowser.dll ScintillaEdit3.dll
for %%F in (%TO_COPY%) do (
    xcopy "..\..\build\mingw\release\bin\%%F" .\Files\
)

@rem  2a - Copy MinGW libraries
set TO_COPY=libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll libgomp-1.dll
for %%F in (%TO_COPY%) do (
    xcopy "%MINGW_BIN%\%%F" .\Files\
)

@rem  2b - Copy Qt libraries
set TO_COPY=Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll
for %%F in (%TO_COPY%) do (
    xcopy "%QT_MINGW_BIN%\%%F" .\Files\
)

@rem  2c - Copy Qt platform plug-in
mkdir .\Files\platforms
xcopy "%QT_MINGW_BIN%\..\plugins\platforms\qwindows.dll" .\Files\platforms

@rem  3 - Copy main plug-ins
set TO_COPY=cv_bar_codes cv_glyphs dev_sysinfo fmt_jpeg fmt_png ip_blobs_processing ^
            ip_effects ip_stdimaging ip_tools vp_ffmpeg_io vs_dshow vs_ffmpeg ^
            vs_image_folder vs_mjpeg vs_repeater vs_screen_cap
mkdir .\Files\cvsplugins
for %%F in (%TO_COPY%) do (
    mkdir ".\Files\cvsplugins\%%F"
    xcopy "..\..\build\mingw\release\bin\cvsplugins\%%F\*" ".\Files\cvsplugins\%%F\"
)

@rem  4 - Copy Lua scripting engine plug-in
set TO_COPY=se_lua
mkdir .\Files\Lua\cvsplugins
for %%F in (%TO_COPY%) do (
    mkdir ".\Files\Lua\cvsplugins\%%F"
    xcopy "..\..\build\mingw\release\bin\cvsplugins\%%F\*" ".\Files\Lua\cvsplugins\%%F\"
)

@rem  5a - Copy virtual camera binaries
mkdir .\Files\CVSCamera\vcam
xcopy "..\..\build\msvc\release\bin\vcam\*.*" .\Files\CVSCamera\vcam\

@rem  5b - Copy virtual camera plug-ins
set TO_COPY=vp_vcam_push
mkdir .\Files\CVSCamera\cvsplugins
for %%F in (%TO_COPY%) do (
    mkdir ".\Files\CVSCamera\cvsplugins\%%F"
    xcopy "..\..\build\mingw\release\bin\cvsplugins\%%F\*" ".\Files\CVSCamera\cvsplugins\%%F\"
)

@rem  6 - Copy Raspberry Pi plug-in
set TO_COPY=dev_raspberry
mkdir .\Files\Raspberry\cvsplugins
for %%F in (%TO_COPY%) do (
    mkdir ".\Files\Raspberry\cvsplugins\%%F"
    xcopy "..\..\build\mingw\release\bin\cvsplugins\%%F\*" ".\Files\Raspberry\cvsplugins\%%F\"
)

@rem  7 - Copy Gamepad plug-in
set TO_COPY=dev_gamepad
mkdir .\Files\Gamepad\cvsplugins
for %%F in (%TO_COPY%) do (
    mkdir ".\Files\Gamepad\cvsplugins\%%F"
    xcopy "..\..\build\mingw\release\bin\cvsplugins\%%F\*" ".\Files\Gamepad\cvsplugins\%%F\"
)

@rem  9 - Copy sample files
mkdir .\Files\scripting_samples
xcopy /s ..\apps\scripting_samples .\Files\scripting_samples

@rem  10 - Copy release note, license, etc.
xcopy "..\..\Release notes.txt" .\Files\
xcopy ..\..\LICENSE .\Files\
xcopy eula.txt .\Files\
