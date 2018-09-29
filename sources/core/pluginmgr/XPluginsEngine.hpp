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
#ifndef CVS_XPLUGINS_ENGINE_HPP
#define CVS_XPLUGINS_ENGINE_HPP

#include <stdint.h>
#include <string>
#include <memory>
#include <XInterfaces.hpp>

#include "XModulesCollection.hpp"
#include "XFamiliesCollection.hpp"

class XPluginsEngine : private CVSandbox::Uncopyable
{
private:
    XPluginsEngine( );

public:
    ~XPluginsEngine( );

    static const std::shared_ptr<XPluginsEngine> Create( );

    // Collect modules containing plug-ins of the specified types
    int32_t CollectModules( const std::string& path, PluginType typesToCollect = PluginType_All );

    // Get collection of loaded module
    const std::shared_ptr<const XModulesCollection> GetModules( ) const
    {
        return mModules;
    }

    // Get collection of available plug-in families
    const std::shared_ptr<const XFamiliesCollection> GetFamilies( ) const
    {
        return mFamilies;
    }

    // Get all plug-ins of the specified types
    const std::shared_ptr<const XPluginsCollection> GetPluginsOfType( PluginType typesMask ) const;

    // Get plug-in descriptor by its short name
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const std::string& shortName ) const;

    // Get plug-in descriptor by its ID
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const CVSandbox::XGuid& id ) const;

    // Get plug-in descriptor by its ID. Also provides module descriptor for it.
    const std::shared_ptr<const XPluginDescriptor> GetPlugin( const CVSandbox::XGuid& id, std::shared_ptr<const XPluginsModule>& moduleDesc ) const;

    // Get module descriptor by its short name
    const std::shared_ptr<const XPluginsModule> GetModule( const std::string& shortName ) const;

    // Get module descriptor by its ID
    const std::shared_ptr<const XPluginsModule> GetModule( const CVSandbox::XGuid& id ) const;

private:
    // Load module with the specified file name
    void LoadMoadule( const std::string& fileName, PluginType typesToCollect );

private:
    std::shared_ptr<XFamiliesCollection> mFamilies;
    std::shared_ptr<XModulesCollection>  mModules;
};

#endif // CVS_XPLUGINS_ENGINE_HPP
