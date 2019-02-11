@echo off

@rem  Build CVSandbox source code
set BUILD_FOLDERS=..\..\afx\make\mingw ^
                  ..\..\core\make\mingw ^
                  ..\..\plugins\make\mingw ^
                  ..\..\testapps\make\mingw ^
                  ..\..\apps\make\mingw

set MY_FOLDER=%cd%

FOR %%I IN (%BUILD_FOLDERS%) DO cd %%I && call build.bat && cd %MY_FOLDER%

@rem  Copy MinGW libraries
set TO_COPY=libgcc_s_dw2-1.dll libstdc++-6.dll libwinpthread-1.dll libgomp-1.dll
for %%F in (%TO_COPY%) do (
    xcopy /Y "%MINGW_BIN%\%%F" ..\..\..\build\mingw\release\bin\
)

@rem  Copy Qt libraries
set TO_COPY=Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll
for %%F in (%TO_COPY%) do (
    xcopy /Y "%QT_MINGW_BIN%\%%F" ..\..\..\build\mingw\release\bin\
)

@rem  Copy Qt platform plug-in
mkdir ..\..\..\build\mingw\release\bin\platforms
xcopy /Y "%QT_MINGW_BIN%\..\plugins\platforms\qwindows.dll" ..\..\..\build\mingw\release\bin\platforms\
