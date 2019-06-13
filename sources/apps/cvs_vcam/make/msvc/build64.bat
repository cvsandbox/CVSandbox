@echo off

pushd ..\..

if "%1"=="clean" (
	msbuild cvs_vcam.sln /t:Clean /p:Configuration=Debug /p:Platform=x64
	msbuild cvs_vcam.sln /t:Clean /p:Configuration=Release /p:Platform=x64
) else (
	msbuild cvs_vcam.sln /p:Configuration=Debug /p:Platform=x64
	msbuild cvs_vcam.sln /p:Configuration=Release /p:Platform=x64
)

popd
