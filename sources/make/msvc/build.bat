@echo off

@rem  Build CVSandbox source code
set BUILD_FOLDERS=..\..\afx\make\msvc ^
                  ..\..\core\make\msvc ^
                  ..\..\plugins\make\msvc ^
                  ..\..\testapps\make\msvc ^
                  ..\..\apps\make\msvc

set MY_FOLDER=%cd%

FOR %%I IN (%BUILD_FOLDERS%) DO cd %%I && call build.bat && cd %MY_FOLDER%

@rem  Copy Qt libraries
set TO_COPY=Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll
for %%F in (%TO_COPY%) do (
    xcopy /Y "%QT_MSVC_BIN%\%%F" ..\..\..\build\msvc\release\bin\
)

@rem  Copy Qt platform plug-in
mkdir ..\..\..\build\msvc\release\bin\platforms
xcopy /Y "%QT_MSVC_BIN%\..\plugins\platforms\qwindows.dll" ..\..\..\build\msvc\release\bin\platforms\
