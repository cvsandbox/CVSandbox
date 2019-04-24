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
#ifndef CVS_XMODULES_COLLECTION_HPP
#define CVS_XMODULES_COLLECTION_HPP

#include <stdint.h>
#include <string>
#include <memory>
#include <map>
#include <XInterfaces.hpp>
#include <XGuid.hpp>

#include "XPluginsModule.hpp"
#include "XMapValuesConstIterator.hpp"

class XModulesCollection : private CVSandbox::Uncopyable
{
private:
    XModulesCollection( );

public:
    ~XModulesCollection( );

    // Create empty collection
    static const std::shared_ptr<XModulesCollection> Create( );
    // Create copy of the collection
    const std::shared_ptr<XModulesCollection> Copy( ) const;

    // Clear collection
    void Clear( );
    // Add new module to the collection
    void Add( const std::shared_ptr<const XPluginsModule>& module );
    // Get modules count in the collection
    int32_t Count( ) const { return mModules.size( ); }

    // Get plug-in descriptor by its short name
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const std::string& shortName ) const;
    // Get plug-in descriptor by its ID
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const CVSandbox::XGuid& id ) const;
    // Get plug-in descriptor by its ID. Also provides module descriptor for it.
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const CVSandbox::XGuid& id, std::shared_ptr<const XPluginsModule>& moduleDesc ) const;
    // Get module descriptor which contains the plug-in
    const std::shared_ptr<const XPluginsModule> GetPluginModule( const CVSandbox::XGuid& id ) const;

    // Get module descriptor by its short name
    const std::shared_ptr<const XPluginsModule> GetModule( const std::string& shortName ) const;
    // Get module descriptor by its ID
    const std::shared_ptr<const XPluginsModule> GetModule( const CVSandbox::XGuid& id ) const;

    // Get collection of plug-in descriptors of the specified types only from ALL modules
    const std::shared_ptr<const XPluginsCollection> GetPluginsOfType( PluginType typesMask ) const;

    // Enumeration API
    typedef XMapValuesConstIterator<CVSandbox::XGuid, std::shared_ptr<const XPluginsModule> > ConstIterator;
    ConstIterator begin( ) const { return ConstIterator( mModules.begin( ) ); }
    ConstIterator end( ) const { return ConstIterator( mModules.end( ) ); }

private:
    std::map<CVSandbox::XGuid, std::shared_ptr<const XPluginsModule> > mModules;
};

#endif // CVS_XMODULES_COLLECTION_HPP
