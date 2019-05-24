To do MSVC build of FFMPEG library it is required to have Visual Studio 2012 or newer (tested with 2013 Express):
1) Run MSVC command prompt so proper environment is set.

32-bit build
2) From there run msys.bat to get into MSYS shell.
3) Run ./build to get both debug and release build. Or ./build-ffmpeg with release/debug/clean options.

64-bit build
2) From the MSVC prompt run "VC\vcvarsall.bat amd64".
3) From there run msys.bat to get into MSYS shell.
4) Run ./build64 to get both debug and release build. Or ./build-ffmpeg with release/debug/clean options and "64" as the second option.
