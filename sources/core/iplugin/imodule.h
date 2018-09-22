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

#pragma once
#ifndef CVS_IMODULE_H
#define CVS_IMODULE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "iplugin.h"

// Structure providing description for plug-ins' module
typedef struct _ModuleDescriptor
{
    xguid       ID;
    xversion    Version;
    xstring     Name;
    xstring     ShortName;
    xstring     Description;
    xstring     Vendor;
    xstring     Copyright;
    xstring     Website;
    ximage*     SmallIcon;
    ximage*     Icon;
    int32_t     PluginsCount;
}
ModuleDescriptor;

// Free memory taken by the specified module descriptor
void FreeModuleDescriptor( ModuleDescriptor** pDescriptor );
// Create a copy of the specified module descriptor
ModuleDescriptor* CopyModuleDescriptor( const ModuleDescriptor* src );


// --- Types and names of 3 functions which need to be exported by any module providing plugins ---

// Function to initialize a module and provide it's descriptor
typedef ModuleDescriptor* (*ModuleInitializeFunc)( );
extern const char* ModuleInitializeFuncName;

// Function to provide plugin descriptors
typedef PluginDescriptor* (*GetDescriptorFunc)( int32_t plugin );
extern const char* GetDescriptorFuncName;

// Function to clean-up module
typedef void (*ModuleCleanupFunc)( );
extern const char* ModuleCleanupFuncName;

// --- Define shared module export attributes
#if defined _WIN32 || defined __CYGWIN__
    #ifdef __GNUC__
        #define MODULE_PUBLIC __attribute__ ((dllexport))
    #else
        #define MODULE_PUBLIC __declspec(dllexport)
    #endif
#else
    #if __GNUC__ >= 4
        #define MODULE_PUBLIC __attribute__ ((visibility ("default")))
    #else
        #define MODULE_PUBLIC
    #endif
#endif

#ifdef __cplusplus
}
#endif

#endif // CVS_IMODULE_H
