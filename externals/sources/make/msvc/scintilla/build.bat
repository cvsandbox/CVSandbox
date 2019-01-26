@echo off

set PRO_NAME=ScintillaEdit.pro
set PRO_FOLDER=..\..\..\scintilla\qt\ScintillaEdit
set BUILD_FOLDER=..\..\..\..\..\build

if "%1"=="clean" (
    echo "Cleaning ScintillaEdit MSVC build ..."

    pushd %PRO_FOLDER%

    if exist Makefile.Debug (
        nmake -f Makefile.Debug clean
        del Makefile.Debug
    )
    if exist Makefile.Release (
        nmake -f Makefile.Release clean
        del Makefile.Release
    )

    del Makefile
    del *.pro.user
    del ScintillaEdit_resource.rc

    rd /S /Q debug
    rd /S /Q release
    rd /S /Q %PRO_FOLDER%\lib

    popd

    goto EOF
)

if "%QT_MSVC_BIN%"=="" (
    echo "Cannot build Scintilla because Qt's MSVC binary folder is not set (QT_MSVC_BIN)."
    goto EOF
)

pushd %PRO_FOLDER%

%QT_MSVC_BIN%\qmake.exe %PRO_NAME%
nmake -f Makefile.Debug
nmake -f Makefile.Release

popd

xcopy /Y "%PRO_FOLDER%\lib\msvc\debug\*.dll" "%BUILD_FOLDER%\msvc\debug\bin"
xcopy /Y "%PRO_FOLDER%\lib\msvc\debug\*.lib" "%BUILD_FOLDER%\msvc\debug\lib"

xcopy /Y "%PRO_FOLDER%\lib\msvc\release\*.dll" "%BUILD_FOLDER%\msvc\release\bin"
xcopy /Y "%PRO_FOLDER%\lib\msvc\release\*.lib" "%BUILD_FOLDER%\msvc\release\lib"

:EOF
