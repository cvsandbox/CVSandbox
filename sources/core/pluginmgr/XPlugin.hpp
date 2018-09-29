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
#ifndef CVS_XPLUGIN_HPP
#define CVS_XPLUGIN_HPP

#include <memory>
#include <XVariant.hpp>
#include <XInterfaces.hpp>
#include <iplugintypescpp.hpp>

class XPluginDescriptor;

class XPlugin : private CVSandbox::Uncopyable
{
protected:
    XPlugin( void* plugin, PluginType type, bool ownIt );

public:
    virtual ~XPlugin( );

    // Create plug-in wrapper
    static const std::shared_ptr<XPlugin> Create( void* plugin, PluginType type, bool ownIt = true );

    // Underlying object of the plug-in
    void* PluginObject( ) const;

    // Type of the plug-in
    PluginType Type( ) const { return mType; }

    // Get/Set value of the property with the specified index
    XErrorCode GetProperty( int32_t id, CVSandbox::XVariant& value ) const;
    XErrorCode SetProperty( int32_t id, const CVSandbox::XVariant& value );

    // Get/Set indexed property of the plug-in - an element of array property
    XErrorCode GetIndexedProperty( int32_t id, uint32_t index, CVSandbox::XVariant& value ) const;
    XErrorCode SetIndexedProperty( int32_t id, uint32_t index, const CVSandbox::XVariant& value );

    // Call function of the plug-in
    XErrorCode CallFunction( int32_t id, CVSandbox::XVariant* returnValue, const xarray* arguments );

    // Update description of device run time configuration properties
    XErrorCode UpdateDescription( std::shared_ptr<XPluginDescriptor> descriptor );

protected:
    void*               mPlugin;
    const PluginType    mType;
    bool                mOwnIt;
};

#endif // CVS_XPLUGIN_HPP
