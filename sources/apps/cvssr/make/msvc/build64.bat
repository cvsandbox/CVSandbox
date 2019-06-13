@echo off

if "%1"=="clean" (
	msbuild cvssr.sln /t:Clean /p:Configuration=Debug /p:Platform=x64
	msbuild cvssr.sln /t:Clean /p:Configuration=Release /p:Platform=x64
) else (
	msbuild cvssr.sln /p:Configuration=Debug /p:Platform=x64
	msbuild cvssr.sln /p:Configuration=Release /p:Platform=x64
)
