@echo off

set OUT_FOLDER=..\..\..\..\..\build\msvc
set BUILD_FOLDER=build
set SRC_FOLDER=..\..\..\libjpeg-turbo

if "%1"=="clean" (
    rd /S /Q %BUILD_FOLDER%
) else (
    set MY_PATH=%cd%

    mkdir %BUILD_FOLDER%
    pushd %BUILD_FOLDER%

    cmake -G "Visual Studio 12 2013" -D ENABLE_SHARED:BOOL=ON ^
        -D WITH_ARITH_DEC:BOOL=ON ^
        -D WITH_ARITH_ENC:BOOL=ON ^
        -D WITH_JPEG7:BOOL=ON ^
        -D WITH_JPEG8:BOOL=ON ^
        -D WITH_MEM_SRCDST:BOOL=ON ^
        -D WITH_SIMD:BOOL=ON ^
        -D WITH_TURBOJPEG:BOOL=ON ^
        %MY_PATH%\%SRC_FOLDER%

    msbuild libjpeg-turbo.sln /p:Configuration=Debug
    msbuild libjpeg-turbo.sln /p:Configuration=Release

    popd

    set OUT_REL=%OUT_FOLDER%\release
    set OUT_DBG=%OUT_FOLDER%\debug

    xcopy /Y "%BUILD_FOLDER%\Release\jpeg8.dll" "%OUT_REL%\bin\"
    xcopy /Y "%BUILD_FOLDER%\Release\jpeg.lib" "%OUT_REL%\lib\"

    xcopy /Y "%BUILD_FOLDER%\Debug\jpeg8.dll" "%OUT_DBG%\bin\"
    xcopy /Y "%BUILD_FOLDER%\Debug\jpeg.lib" "%OUT_DBG%\lib\"

    xcopy /Y "%BUILD_FOLDER%\jconfig.h" "%OUT_REL%\include\"
    xcopy /Y "%SRC_FOLDER%\jpeglib.h" "%OUT_REL%\include\"
    xcopy /Y "%SRC_FOLDER%\jmorecfg.h" "%OUT_REL%\include\"
    xcopy /Y "%SRC_FOLDER%\jerror.h" "%OUT_REL%\include\"

    xcopy /Y "%BUILD_FOLDER%\jconfig.h" "%OUT_DBG%\include\"
    xcopy /Y "%SRC_FOLDER%\jpeglib.h" "%OUT_DBG%\include\"
    xcopy /Y "%SRC_FOLDER%\jmorecfg.h" "%OUT_DBG%\include\"
    xcopy /Y "%SRC_FOLDER%\jerror.h" "%OUT_DBG%\include\"
)