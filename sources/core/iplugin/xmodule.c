/*
    Plug-ins' interface library of Computer Vision Sandbox

    Copyright (C) 2011-2018, cvsandbox
    http://www.cvsandbox.com/contacts.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <stdlib.h>
#include <string.h>

#ifdef WIN32
    #include <direct.h>
    #include <windows.h>
#else
    #include <sys/types.h>
    #include <dlfcn.h>
#endif

#include "xmodule.h"

#ifdef WIN32
    static const char*    module_ext   = ".dll";
    static const wchar_t* module_ext_w = L".dll";
#else
    static const char*    module_ext   = ".so";
    static const wchar_t* module_ext_w = L".so";
#endif

// Get extension of shared module valid for the platform
const char* XModuleDefaultExtension( )
{
    return module_ext;
}
const wchar_t* XModuleDefaultExtensionW( )
{
    return module_ext_w;
}

// Load specified module (file name with extension must be provided,
// since the function does not append it automatically)
xmodule XModuleLoad( const char* fileName )
{
    xmodule module = 0;

    #ifdef WIN32
        int      bytesRequired = MultiByteToWideChar( CP_UTF8, 0, fileName, -1, NULL, 0 );
        wchar_t* wFileName     = (wchar_t*) malloc( sizeof( wchar_t ) * bytesRequired );

        if ( fileName != 0 )
        {
            if ( MultiByteToWideChar( CP_UTF8, 0, fileName, -1, wFileName, bytesRequired ) > 0 )
            {
                module = (xmodule) LoadLibraryW( wFileName );
            }

            free( wFileName );
        }
    #else
        module = (xmodule) dlopen( fileName, RTLD_LAZY );
    #endif

    return module;
}

// Unload the specified module
void XModuleUnload( xmodule module )
{
    if ( module != 0 )
    {
        #ifdef WIN32
            FreeLibrary( (HMODULE) module );
        #else
            dlclose( (void*) module );
        #endif
    }
}

// Get exported symbol from the specified module
ProcAddress XModuleGetSymbol( xmodule module, const char* name )
{
    ProcAddress address = 0;

    if ( module != 0 )
    {
        #ifdef WIN32
            address = GetProcAddress( (HMODULE) module, name );
        #else
            address = dlsym( (void*) module , name );
        #endif
    }

    return address;
}
