@echo off

set BUILD_FOLDERS=..\..\cvsandboxtools\make\msvc ^
                  ..\..\cvsandbox\make\msvc ^
                  ..\..\cvssr\make\msvc ^
                  ..\..\cvs_vcam\make\msvc

set MY_FOLDER=%cd%

FOR %%I IN (%BUILD_FOLDERS%) DO cd %%I && call build64.bat && cd %MY_FOLDER%
