@echo off

if "%1"=="clean" (
	msbuild cvssr.sln /t:Clean /p:Configuration=Debug
	msbuild cvssr.sln /t:Clean /p:Configuration=Release
) else (
	msbuild cvssr.sln /p:Configuration=Debug
	msbuild cvssr.sln /p:Configuration=Release
)
