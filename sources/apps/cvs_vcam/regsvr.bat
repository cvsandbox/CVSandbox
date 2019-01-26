@echo OFF

reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT

@rem - register 32-bit version of the COM DLL
regsvr32.exe cvs_vcam.dll

@rem - register 64-bit version of the COM DLL
if %OS%==64BIT regsvr32.exe cvs_vcam64.dll
