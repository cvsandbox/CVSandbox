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

#include <xlist.h>
#include "iplugin.h"

static xlist* pluginStore;

// Register plugin with the provided description
void RegisterPlugin( const PluginDescriptor* desc )
{
    PluginDescriptor* descToStore = 0;

    if ( pluginStore == 0 )
    {
        pluginStore = XListCreate( );
    }

    if ( pluginStore != 0 )
    {
        descToStore = CopyPluginDescriptor( desc );

        if ( descToStore != 0 )
        {
            XListAddTail( pluginStore, descToStore );
        }
    }
}

// Unregister all plugins and free resources used by the registry
void UnregisterAllPlugins( )
{
    if ( pluginStore != 0 )
    {
        xlistnode* node;

        for ( node = pluginStore->head; node != 0; node = node->next )
        {
            FreePluginDescriptor( (PluginDescriptor**) &node->data );
        }

        XListFree( pluginStore );
        pluginStore = 0;
    }
}

// Get number of registered plugins
int32_t GetPluginsCount( )
{
    return (int32_t) XListCount( pluginStore );
}

// Get descriptor for the specified plugin's index
PluginDescriptor* GetPluginDescriptor( int32_t plugin )
{
    PluginDescriptor* desc = 0;

    if  ( ( pluginStore != 0 ) && ( plugin >= 0 ) )
    {
        xlistnode* node = pluginStore->head;

        while ( ( plugin != 0 ) && ( node != 0 ) )
        {
            plugin--;
            node = node->next;
        }

        if ( ( plugin == 0 ) && ( node != 0 ) )
        {
            desc = CopyPluginDescriptor( (PluginDescriptor*) node->data );
        }
    }
    return desc;
}
