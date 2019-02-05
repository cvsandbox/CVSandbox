@echo off

set BUILD_FOLDERS=..\..\afx\make\mingw ^
                  ..\..\core\make\mingw ^
                  ..\..\plugins\make\mingw ^
                  ..\..\testapps\make\mingw ^
                  ..\..\apps\make\mingw

set MY_FOLDER=%cd%

FOR %%I IN (%BUILD_FOLDERS%) DO cd %%I && call build.bat && cd %MY_FOLDER%
