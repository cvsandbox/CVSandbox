@echo off

set BUILD_FOLDER=out_make64
set PRO_FOLDER=..\..
set PRO_NAME=cvsandboxtools.pro

if "%1"=="clean" (
    echo "Cleaning cvsandboxtools.dll build ..."
    
    rd /S /Q %BUILD_FOLDER%

) else (

    if "%QT_MSVC_BIN_64%"=="" (
        echo "Cannot build cvsandboxtools.dll because Qt's MSVC 64-bit binary folder is not set (QT_MSVC_BIN_64)."
        goto EOF
    )

    mkdir %BUILD_FOLDER%
    cd %BUILD_FOLDER%
        
    %QT_MSVC_BIN_64%\qmake.exe -o Makefile64 ..\%PRO_FOLDER%\%PRO_NAME%
    
    if "%1"=="debug" (
        nmake -f Makefile64.Debug
    ) else (
        nmake -f Makefile64.Release
    )
    
    cd ..
)

:EOF
