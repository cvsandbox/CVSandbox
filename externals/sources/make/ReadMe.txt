Avoid building both MinGW and MSVC builds at the same time
because it may cause build errors. Also don't build MSVC build
without cleaning MinGW build, because it may leave some
configuration files causing MSVC build to fail.
