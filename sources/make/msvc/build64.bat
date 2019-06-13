@echo off

@rem  Build CVSandbox source code
set BUILD_FOLDERS=..\..\afx\make\msvc ^
                  ..\..\core\make\msvc ^
                  ..\..\plugins\make\msvc ^
                  ..\..\testapps\make\msvc ^
                  ..\..\apps\make\msvc

set MY_FOLDER=%cd%

FOR %%I IN (%BUILD_FOLDERS%) DO cd %%I && call build64.bat && cd %MY_FOLDER%

@rem  Copy Qt release libraries
set TO_COPY=Qt5Core.dll Qt5Gui.dll Qt5Widgets.dll
for %%F in (%TO_COPY%) do (
    xcopy /Y "%QT_MSVC_BIN_64%\%%F" ..\..\..\build\msvc\release64\bin\
)

@rem  Copy Qt release platform plug-in
mkdir ..\..\..\build\msvc\release\bin\platforms
xcopy /Y "%QT_MSVC_BIN_64%\..\plugins\platforms\qwindows.dll" ..\..\..\build\msvc\release64\bin\platforms\

@rem  Copy Qt debug libraries
set TO_COPY=Qt5Cored.dll Qt5Guid.dll Qt5Widgetsd.dll
for %%F in (%TO_COPY%) do (
    xcopy /Y "%QT_MSVC_BIN_64%\%%F" ..\..\..\build\msvc\debug64\bin\
)

@rem  Copy Qt debug platform plug-in
mkdir ..\..\..\build\msvc\debug\bin\platforms
xcopy /Y "%QT_MSVC_BIN_64%\..\plugins\platforms\qwindowsd.dll" ..\..\..\build\msvc\debug64\bin\platforms\
