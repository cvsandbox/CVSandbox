# Building the source code

All official releases of Computer Vision Sandbox are built using MinGW tool set. However, the source code also comes with solution files to build it using MS Visual Studio (2015), which is a convenient option for debugging the code within IDE.

Since the project uses number of external open source components, those need to be built first. Normally this will need to be done once. After external components are all built, the project's code itself can be built as well.

Below are the instructions for building both external components and the project's code using MinGW and MS VS.

Just for reference, lets suppose Computer Vision Sandbox code is download into **CVSandbox** folder.

## Prerequisites

In order to build the project and external components, it is required to install number of different tools. Below is the list of all tools needed and their versions we use (for reference). You may upgrade to any version you like, however. Feel free to address any build issues you may get on your own in this case.

### Common tools

1. [MSYS toolset](http://www.mingw.org/wiki/MSYS) (1.0.11) need to be installed, which is used for building external components.

2. [NASM Assembler](https://www.nasm.us/) (2.14.02) - used for external components. Make sure its binaries folder is added into system's %PATH% environment variable.

3. [CMake](https://cmake.org/) (3.14.0) – used for external components. Make sure its binaries folder is added into system's %PATH% environment variable.

### Tools needed for MinGW builds

1. [MinGW tool set](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/4.9.2/threads-posix/dwarf/) (4.9.2 revision 4.7). Make sure its binaries folder is set into system's %MINGW_BIN% environment variable.

MSYS configuration need to be updated as well, so it is aware of MinGW installation. The /etc/fstab file within MSYS file system need to be edited for this, where MinGW's installation folder (not the binaries folder) is mounted as /mingw. For example:
```
#Win32_Path		Mount_Point
c:/mingw32/4.9.2	/mingw
```

2. [Qt Framework](https://download.qt.io/official_releases/qt/5.6/5.6.3/) (5.6.3, MinGW version). Make sure its binaries folder (where qmake.exe lives) is set into system's %QT_MINGW_BIN% environment variable.

### Tools needed for MS VS builds

1. [MS Visual Studio](https://visualstudio.microsoft.com/vs/older-downloads/) (2015). Make sure C++ compiler is installed.

2. [Qt Framework](https://download.qt.io/official_releases/qt/5.6/5.6.3/) (5.6.3, MSVC 32-bit version). Make sure its binaries folder (where qmake.exe lives) is set into system's %QT_MSVC_BIN% environment variable.

## MinGW build

### Building external components

Run MSYS environment. Change directory to CVSandbox/externals/sources/make/mingw. Run **./build** from there. On completion, the CVSandbox/build/mingw/release folder is populated with binaries, libraries and header files of the built external components. Exit MSYS, not needed any more.

### Building project's code

Open command prompt. Change directory to CVSandbox/sources/make/mingw. Run build.bat from there. On completion, the CVSandbox/build/mingw/release/bin folder is populated with all binaries, plug-ins and dependencies, which are required to run Computer Vision Sandbox. All done.

## MS VS build

### Building external components

Run MS VS Developer Command Prompt. Change directory to CVSandbox/externals/sources/make/msvc. Run build.bat from there. On completion, the CVSandbox/build/msvc/release folder is populated with binaries, libraries and header files of the built external components, except FFmpeg.

### Building FFmpeg components

Run MS VS Developer Command Prompt. From that run MSYS environment. Change directory to CVSandbox/externals/sources/make/msvc/ffmpeg. Run ./build from there. On completion, the CVSandbox/build/msvc/release folder is populated with binaries, libraries and header files of the FFmpeg component.

### Building project's code

Run MS VS Developer Command Prompt. Change directory to CVSandbox/sources/make/msvc. Run build.bat from there. On completion, the CVSandbox/build/msvc/release/bin folder is populated with all binaries, plug-ins and dependencies, which are required to run Computer Vision Sandbox. All done.

## Optional

### Building virtual camera’s COM DLL

Virtual camera's COM DLL is not included into MinGW build for now, but is done automatically in MS VS build. If this component is needed, make sure MS VS is installed and then build it separately. From MS VS Developer Command Prompt run build.bat located in CVSandbox/sources/apps/cvs_vcam/make/msvc. On build completion, register the COM DLL by running regsvr.bat from CVSandbox/build/msvc/release/bin/vcam folder.

### Development and debugging with IDEs

Although all the above mentioned build steps require working only with MSYS environment and command prompt, it is convenient to use MS Visual Studio and Qt Creator IDEs for development and debugging. Each component of Computer Vision Sandbox comes with its own project files, which can be opened in IDE and build from there. External components still need to be built using steps described above.
