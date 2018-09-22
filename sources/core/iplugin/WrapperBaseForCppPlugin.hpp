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
#ifndef CVS_WRAPPER_BASE_FOR_CPP_PLUGIN_HPP
#define CVS_WRAPPER_BASE_FOR_CPP_PLUGIN_HPP

#include "iplugintypes.h"

typedef void (*PluginInitializationHandler)( );
typedef void (*PluginCleanupHandler)( );

class PluginRegisterAndWrapper
{
private:
    PluginCleanupHandler cleanupHandler;

public:
    PluginRegisterAndWrapper( xguid id, xguid family,
        PluginType type, xversion version,
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

    virtual ~PluginRegisterAndWrapper( )
    {
        if ( cleanupHandler != 0 )
        {
            // let plugin perform any cleanup routine
            cleanupHandler( );
        }
    }
};

#endif // CVS_WRAPPER_BASE_FOR_CPP_PLUGIN_HPP
