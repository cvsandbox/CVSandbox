@echo off

if "%1"=="clean" (
    msbuild build.sln /t:Clean /p:Configuration=Debug
    msbuild build.sln /t:Clean /p:Configuration=Release

    RD /Q /S Debug
    RD /Q /S Release

    pushd libjpeg-turbo
    call build clean
    popd

    for /D %%f in (*) do (
        echo %%f
        RD /Q /S %%f\Debug
        RD /Q /S %%f\Release
        RD /Q /S %%f\build
    )
) else (
    msbuild build.sln /p:Configuration=Debug
    msbuild build.sln /p:Configuration=Release

    pushd libjpeg-turbo
    call build
    popd
)