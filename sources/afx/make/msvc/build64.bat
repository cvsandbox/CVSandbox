@echo off

if "%1"=="clean" (
	msbuild build.sln /t:Clean /p:Configuration=Debug /p:Platform=x64
	msbuild build.sln /t:Clean /p:Configuration=Release /p:Platform=x64
) else (
	msbuild build.sln /p:Configuration=Debug /p:Platform=x64
	msbuild build.sln /p:Configuration=Release /p:Platform=x64
)
