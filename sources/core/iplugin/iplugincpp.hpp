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
#ifndef CVS_IPLUGINCPP_HPP
#define CVS_IPLUGINCPP_HPP

#include "iplugin.h"
#include "iplugintypescpp.hpp"

#include "Wrapper_ImageProcessingFilter.hpp"
#include "Wrapper_ImageProcessingFilter2.hpp"
#include "Wrapper_ImageImporter.hpp"
#include "Wrapper_ImageExporter.hpp"
#include "Wrapper_ImageGenerator.hpp"
#include "Wrapper_VideoSource.hpp"
#include "Wrapper_Device.hpp"
#include "Wrapper_CommunicationDevice.hpp"
#include "Wrapper_VideoProcessing.hpp"
#include "Wrapper_ScriptingEngine.hpp"
#include "Wrapper_ScriptingApi.hpp"
#include "Wrapper_ImageProcessing.hpp"

// Helper class to automate plugins registraton in C++ by defining a global variable
class PluginRegister
{
private:
    PluginCleanupHandler cleanupHandler;

public:
    PluginRegister( xguid id, xguid family, PluginType type, xversion version,
        const char* name, const char* shortName, const char* description, const char* help,
        CreatePluginFunc creator,
        const ximage* smallIcon = 0, const ximage* icon = 0,
        int32_t propsCount = 0, PropertyDescriptor** props = 0,
        PluginInitializationHandler init = 0, PluginCleanupHandler cleanup = 0,
        PropertyDescriptorUpdater updater = 0,
        int32_t functionsCount = 0, FunctionDescriptor** funcs = 0 ) : cleanupHandler( cleanup )
    {
        if ( init != 0 )
        {
            // call initialization handler if it was provided
            init( );
        }

        PluginDescriptor desc =
        {
            { id.part1, id.part2, id.part3, id.part4 },
            { family.part1, family.part2, family.part3, family.part4 },
            type,
            { version.major, version.minor, version.revision },
            name,
            shortName,
            description,
            help,
            (ximage*) smallIcon,
            (ximage*) icon,
            creator,
            propsCount,
            props,
            updater,
            functionsCount,
            funcs
        };

        RegisterPlugin( &desc );
    }

    ~PluginRegister( )
    {
        if ( cleanupHandler != 0 )
        {
            // let plugin perform any cleanup routine
            cleanupHandler( );
        }
    }
};

// Macros to generate variable name based on current line number
#define __VARNAME( name, line ) name##line
#define _VARNAME( name, line ) __VARNAME(name, line)
#define VARNAME( base ) _VARNAME(base, __LINE__)

// Plugin registration macro without properties
#define REGISTER_CPP_PLUGIN( id, family, type, version, name, shortName, desc, help, smallIcon, icon, className ) \
    static void* VARNAME(_creator_)( void ) { return _VARNAME(PluginRegister_,type)::CreateWrapper( new className( ) ); } \
    static PluginRegister_##type VARNAME(_pr_)( id, family, type, version, name, shortName, desc, help, VARNAME(_creator_), smallIcon, icon );

// Plugin registration macro with properties
#define REGISTER_CPP_PLUGIN_WITH_PROPS( id, family, type, version, name, shortName, desc, help, smallIcon, icon, className, propsCount, props, init, cleanup, updater ) \
    static void* VARNAME(_creator_)( void ) { return _VARNAME(PluginRegister_,type)::CreateWrapper( new className( ) ); } \
    static PluginRegister_##type VARNAME(_pr_)( id, family, type, version, name, shortName, desc, help, VARNAME(_creator_), smallIcon, icon, propsCount, props, init, cleanup, updater );

// Plugin registration macro with properties and functions
#define REGISTER_CPP_PLUGIN_WITH_PROPS_AND_FUNCS( id, family, type, version, name, shortName, desc, help, smallIcon, icon, className, propsCount, props, init, cleanup, updater, funcCount, functions ) \
    static void* VARNAME(_creator_)( void ) { return _VARNAME(PluginRegister_,type)::CreateWrapper( new className( ) ); } \
    static PluginRegister_##type VARNAME(_pr_)( id, family, type, version, name, shortName, desc, help, VARNAME(_creator_), smallIcon, icon, propsCount, props, init, cleanup, updater, funcCount, functions );

#endif // CVS_IPLUGINCPP_HPP
