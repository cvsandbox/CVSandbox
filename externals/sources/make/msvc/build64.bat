@echo off

if "%1"=="clean" (
    msbuild build.sln /t:Clean /p:Configuration=Debug /p:Platform=x64
    msbuild build.sln /t:Clean /p:Configuration=Release /p:Platform=x64

    RD /Q /S Debug
    RD /Q /S Release

    pushd libjpeg-turbo
    call build64 clean
    popd

    pushd qtpropertybrowser
    call build64 clean
    popd

    pushd scintilla
    call build64 clean
    popd

    for /D %%f in (*) do (
        echo %%f
        RD /Q /S %%f\Debug
        RD /Q /S %%f\Release
        RD /Q /S %%f\build
    )
) else (
    msbuild build.sln /p:Configuration=Debug /p:Platform=x64
    msbuild build.sln /p:Configuration=Release /p:Platform=x64

    pushd libjpeg-turbo
    call build64
    popd

    pushd qtpropertybrowser
    call build64
    popd

    pushd scintilla
    call build64
    popd

    xcopy /Y "..\..\directshow\*.h" "..\..\..\..\build\msvc\debug64\include\"
    xcopy /Y "..\..\directshow\*.h" "..\..\..\..\build\msvc\release64\include\"
)
