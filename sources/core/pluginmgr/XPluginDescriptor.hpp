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
#ifndef CVS_XPLUGIN_DESCRIPTOR_HPP
#define CVS_XPLUGIN_DESCRIPTOR_HPP

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <XInterfaces.hpp>
#include <XGuid.hpp>
#include <XVersion.hpp>
#include <XImage.hpp>
#include <iplugin.h>

#include "XPlugin.hpp"
#include "XPropertyDescriptor.hpp"
#include "XFunctionDescriptor.hpp"

// Class which wraps description of a plug-in
class XPluginDescriptor : private CVSandbox::Uncopyable
{
friend class XPlugin;

private:
    XPluginDescriptor( PluginDescriptor* desc );

public:
    ~XPluginDescriptor( );

    static std::shared_ptr<XPluginDescriptor> Create( PluginDescriptor* desc );
    std::shared_ptr<XPluginDescriptor> Clone( ) const;

    // Description of the plug-in
    CVSandbox::XGuid ID( ) const;
    CVSandbox::XGuid FamilyID( ) const;
    CVSandbox::XVersion Version( ) const;
    std::string ShortName( ) const;
    std::string Name( ) const;
    std::string Description( ) const;
    std::string Help( ) const;
    std::shared_ptr<const CVSandbox::XImage> Icon( bool getSmall = true ) const;

    PluginType Type( ) const
    {
        return mDescriptor->Type;
    }

    // Total number of properties
    int32_t PropertiesCount( ) const
    {
        return mDescriptor->PropertiesCount;
    }

    // Total number of functions
    int32_t FunctionsCount( ) const
    {
        return mDescriptor->FunctionsCount;
    }

    // Number of properties which are not read-only
    int32_t ConfigurablePropertiesCount( ) const;
    // Number of properties which can be configured while plug-in is running (video source or device)
    int32_t DeviceRuntimeConfigurablePropertiesCount( ) const;

    // Get descriptor of a property with the specified index
    const std::shared_ptr<const XPropertyDescriptor> GetPropertyDescriptor( int32_t id ) const;
    // Get descriptor of a property with the specified short name
    const std::shared_ptr<const XPropertyDescriptor> GetPropertyDescriptor( const std::string& shortName ) const;
    // Get index of a property with the given short name
    int32_t GetPropertyIndexByName( const std::string& shortName ) const;

    // Get descriptor of a function with the specified index
    const std::shared_ptr<const XFunctionDescriptor> GetFunctionDescriptor( int32_t id ) const;
    // Get descriptor of a function with the specified name
    const std::shared_ptr<const XFunctionDescriptor> GetFunctionDescriptor( const std::string& name ) const;
    // Get index of a function with the given name
    int32_t GetFunctionIndexByName( const std::string& name ) const;

    // Create instance of the plug-in
    const std::shared_ptr<XPlugin> CreateInstance( ) const;

    // Get plug-in configuration as a map of properties' name-value
    std::map<std::string, CVSandbox::XVariant> GetPluginConfiguration( const std::shared_ptr<const XPlugin> plugin ) const;
    // Get plug-in device run time configuration as a map of properties' name-value
    std::map<std::string, CVSandbox::XVariant> GetPluginDeviceRuntimeConfiguration( const std::shared_ptr<const XPlugin> plugin ) const;
    // Set plug-in configuration
    XErrorCode SetPluginConfiguration( const std::shared_ptr<XPlugin> plugin, const std::map<std::string, CVSandbox::XVariant>& config ) const;

    // Get copy of the wrapped C plug-in descriptor
    PluginDescriptor* GetPluginDescriptorCopy( ) const;

private:
    PluginDescriptor*                                        mDescriptor;
    std::vector<std::shared_ptr<const XPropertyDescriptor> > mProperties;
    std::vector<std::shared_ptr<const XFunctionDescriptor> > mFunctions;
};

#endif // CVS_XPLUGIN_DESCRIPTOR_HPP
