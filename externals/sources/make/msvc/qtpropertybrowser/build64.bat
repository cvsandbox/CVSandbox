@echo off

set PRO_NAME=qtpropertybrowser.pro
set PRO_FOLDER=..\..\..\qtpropertybrowser\custom_build
set BUILD_FOLDER=..\..\..\..\..\build

if "%1"=="clean" (
    echo "Cleaning QtPropertyBrowser MSVC build ..."

    if exist Makefile64.Debug (
        nmake -f Makefile64.Debug clean
        del Makefile64.Debug
    )
    if exist Makefile64.Release (
        nmake -f Makefile64.Release clean
        del Makefile64.Release
    )

    del Makefile64
    del .qmake.stash
    
    rd /S /Q debug
    rd /S /Q release
    rd /S /Q %PRO_FOLDER%\lib64

    goto EOF
)

if "%QT_MSVC_BIN_64%"=="" (
    echo "Cannot build QtPropertyBrowser because Qt's MSVC 64-bit binary folder is not set (QT_MSVC_BIN_64)."
    goto EOF
)

%QT_MSVC_BIN_64%\qmake.exe -o Makefile64 %PRO_FOLDER%\%PRO_NAME%
nmake -f Makefile64.Debug
nmake -f Makefile64.Release

xcopy /Y "%PRO_FOLDER%\lib64\msvc\debug\*.dll" "%BUILD_FOLDER%\msvc\debug64\bin"
xcopy /Y "%PRO_FOLDER%\lib64\msvc\debug\*.lib" "%BUILD_FOLDER%\msvc\debug64\lib"

xcopy /Y "%PRO_FOLDER%\lib64\msvc\release\*.dll" "%BUILD_FOLDER%\msvc\release64\bin"
xcopy /Y "%PRO_FOLDER%\lib64\msvc\release\*.lib" "%BUILD_FOLDER%\msvc\release64\lib"

:EOF
