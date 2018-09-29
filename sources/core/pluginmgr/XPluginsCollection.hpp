/*
    Plug-ins' management library of Computer Vision Sandbox

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
#ifndef CVS_XPLUGINS_COLLECTION_HPP
#define CVS_XPLUGINS_COLLECTION_HPP

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <XInterfaces.hpp>
#include <XGuid.hpp>
#include <imodule.h>

#include "XPluginDescriptor.hpp"
#include "XMapValuesConstIterator.hpp"

class XPluginsCollection : private CVSandbox::Uncopyable
{
private:
    XPluginsCollection( );

public:
    ~XPluginsCollection( );

    // Create empty collection
    static const std::shared_ptr<XPluginsCollection> Create( );
    // Collect plug-ins given a pointer to a function which provides plug-in descriptors
    int32_t CollectPlugins( GetDescriptorFunc pluginsNest, int32_t count, PluginType typesToCollec = PluginType_All );
    // Create copy of the collection
    const std::shared_ptr<XPluginsCollection> Copy( ) const;

    // Clear collection
    void Clear( );
    // Add new plug-in descriptor to the collection
    void Add( const std::shared_ptr<const XPluginDescriptor>& plugin );
    // Copy plug-in descriptors from the specified collection into this collection
    void Add( const std::shared_ptr<const XPluginsCollection>& plugins );
    // Remove plug-in descriptor from the collection
    void Remove( const CVSandbox::XGuid& id );
    // Total count of descriptors in this collection
    int32_t Count( ) const { return mPlugins.size( ); }
    // Number of plug-in descriptors of the specified type
    int32_t CountType( PluginType typeMask ) const;

    // Get plug-in descriptor by its short name
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const std::string& shortName ) const;
    // Get plug-in descriptor by its ID
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const CVSandbox::XGuid& id ) const;

    // Get collection of plug-in descriptors of the specified types only
    const std::shared_ptr<const XPluginsCollection> GetPluginsOfType( PluginType typesMask ) const;

    // Enumeration API
    typedef XMapValuesConstIterator<CVSandbox::XGuid, std::shared_ptr<const XPluginDescriptor> > ConstIterator;
    ConstIterator begin( ) const { return ConstIterator( mPlugins.begin( ) ); }
    ConstIterator end( ) const { return ConstIterator( mPlugins.end( ) ); }

private:
    std::map<CVSandbox::XGuid, std::shared_ptr<const XPluginDescriptor> > mPlugins;
};

#endif // CVS_XPLUGINS_COLLECTION_HPP
