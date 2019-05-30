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
#ifndef CVS_XPLUGINS_MODULE_HPP
#define CVS_XPLUGINS_MODULE_HPP

#include <string>
#include <memory>
#include <xtypes.h>
#include <XInterfaces.hpp>
#include <XGuid.hpp>
#include <XVersion.hpp>
#include <XImage.hpp>
#include <imodule.h>
#include <xmodule.h>

#include "XPluginsCollection.hpp"
#include "XPluginDescriptor.hpp"

// Class providing description of module containing plug-ins
class XPluginsModule : private CVSandbox::Uncopyable
{
private:
    XPluginsModule( const std::string& fileName );

public:
    ~XPluginsModule( );

    // Create empty module descriptor without loading the specified module
    static const std::shared_ptr<XPluginsModule> Create( const std::string& fileName );

    // Load the module
    XErrorCode Load( PluginType typesToCollect = PluginType_All );
    // Unload the module
    void Unload( );
    // Check if the module is loaded
    bool IsLoaded( ) const { return ( mModule != 0 ); }

    // Description of the module
    const CVSandbox::XGuid ID( ) const;
    const CVSandbox::XVersion Version( ) const;
    const std::string ShortName( ) const;
    const std::string Name( ) const;
    const std::string Description( ) const;
    const std::string Vendor( ) const;
    const std::string Copyright( ) const;
    const std::string Website( ) const;
    const std::shared_ptr<const CVSandbox::XImage> Icon( bool getSmall = true ) const;

    // Get plug-in descriptor by its short name
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const std::string& shortName ) const;

    // Get plug-in descriptor by its ID
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const CVSandbox::XGuid& id ) const;

    // Get collection of this module's plug-ins
    const std::shared_ptr<const XPluginsCollection> GetPlugins( ) const;

    // Get collection of plug-ins of the specified type
    const std::shared_ptr<const XPluginsCollection> GetPluginsOfType( PluginType typesMask ) const;

    // Number of plug-ins loaded for the module (the module may have more than that)
    size_t Count( ) const;

    // Number of plug-in descriptors of the specified type
    size_t CountType( PluginType typeMask ) const;

private:
    static const ModuleDescriptor       BlankModuleDescriptor;

    xmodule                             mModule;
    const std::string                   mFileName;
    std::shared_ptr<XPluginsCollection> mPlugins;
    ModuleDescriptor*                   mDescriptor;
};


#endif // CVS_XPLUGINS_MODULE_HPP
