@echo off

if "%1"=="clean" (
	msbuild build.sln /t:Clean /p:Configuration=Debug
	msbuild build.sln /t:Clean /p:Configuration=Release
) else (
	msbuild build.sln /p:Configuration=Debug
	msbuild build.sln /p:Configuration=Release
)
