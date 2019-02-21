@echo off

set BUILD_FOLDERS=..\..\cvsandboxtools\make\mingw ^
                  ..\..\cvsandbox\make\mingw ^
                  ..\..\cvssr\make\mingw

set MY_FOLDER=%cd%

FOR %%I IN (%BUILD_FOLDERS%) DO cd %%I && call build.bat && cd %MY_FOLDER%
