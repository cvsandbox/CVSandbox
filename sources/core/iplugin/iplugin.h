/*
    Plug-ins' interface library of Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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
#ifndef CVS_IPLUGIN_H
#define CVS_IPLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <xtypes.h>
#include <ximage.h>
#include "iproperty.h"
#include "ifunction.h"
#include "ifamily.h"

// Supported plug-in types
typedef uint32_t PluginType;

static const PluginType PluginType_Unknown                  = 0x0000;
static const PluginType PluginType_ImageProcessingFilter    = 0x0001;
static const PluginType PluginType_ImageImporter            = 0x0002;
static const PluginType PluginType_ImageExporter            = 0x0004;
static const PluginType PluginType_VideoSource              = 0x0008;
static const PluginType PluginType_ImageGenerator           = 0x0010;
static const PluginType PluginType_ImageProcessingFilter2   = 0x0020;
static const PluginType PluginType_Device                   = 0x0040;
static const PluginType PluginType_VideoProcessing          = 0x0080;
static const PluginType PluginType_ScriptingEngine          = 0x0100;
static const PluginType PluginType_ScriptingApi             = 0x0200;
static const PluginType PluginType_ImageProcessing          = 0x0400;
static const PluginType PluginType_CommunicationDevice      = 0x0800;
static const PluginType PluginType_Detection                = 0x1000;
static const PluginType PluginType_All                      = 0xFFFFFFFF;


struct _PluginDescriptor;

// Plug-in's factory functions
typedef void* (*CreatePluginFunc)( void );
// Property descriptor dynamic update
typedef XErrorCode ( *PropertyDescriptorUpdater )( struct _PluginDescriptor* desc );

// Structure providing description of a plug-in
typedef struct _PluginDescriptor
{
    xguid                       ID;
    xguid                       Family;
    PluginType                  Type;
    xversion                    Version;
    xstring                     Name;
    xstring                     ShortName;
    xstring                     Description;
    xstring                     Help;
    ximage*                     SmallIcon;
    ximage*                     Icon;

    CreatePluginFunc            Creator;

    int32_t                     PropertiesCount;
    PropertyDescriptor**        Properties;
    PropertyDescriptorUpdater   PropertyUpdater;

    int32_t                     FunctionsCount;
    FunctionDescriptor**        Functions;
}
PluginDescriptor;

// Register plugin with the provided description
void RegisterPlugin( const PluginDescriptor* desc );

// Unregister all plugins and free resources used by the registry
void UnregisterAllPlugins( );

// Get number of registered plugins
int32_t GetPluginsCount( );

// Get descriptor for the specified plugin's index
PluginDescriptor* GetPluginDescriptor( int32_t plugin );

// Free memory taken by the specified plugin descriptor
void FreePluginDescriptor( PluginDescriptor** pDescriptor );
// Create a copy of the specified plugin descriptor
PluginDescriptor* CopyPluginDescriptor( const PluginDescriptor* src );

#ifdef __cplusplus
}
#endif

#endif // CVS_IPLUGIN_H
