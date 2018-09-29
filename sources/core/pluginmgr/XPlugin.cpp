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

#include "XPlugin.hpp"
#include "XPluginDescriptor.hpp"

using namespace std;
using namespace CVSandbox;

XPlugin::XPlugin( void* plugin, PluginType type, bool ownIt ) :
    mPlugin( plugin ), mType( type ), mOwnIt( ownIt )
{
}

XPlugin::~XPlugin( )
{
    if ( mOwnIt )
    {
        ( (SPluginBase*) mPlugin )->Dispose( (SPluginBase*) mPlugin );
    }
}

// Create plug-in wrapper
const shared_ptr<XPlugin> XPlugin::Create( void* plugin, PluginType type, bool ownIt )
{
    return shared_ptr<XPlugin>( new XPlugin( plugin, type, ownIt ) );
}

// Underlying object of the plug-in
void* XPlugin::PluginObject( ) const
{
    return mPlugin;
}

// Get value of the property with the specified index
XErrorCode XPlugin::GetProperty( int32_t id, XVariant& value ) const
{
    xvariant var;
    XVariantInit( &var );

    SPluginBase* plugin = static_cast<SPluginBase*>( mPlugin );
    XErrorCode ret = plugin->GetProperty( plugin, id, &var );

    if ( ret == SuccessCode )
    {
        value = var;
    }

    XVariantClear( &var );

    return ret;
}

// Set value of the property with the specified index
XErrorCode XPlugin::SetProperty( int32_t id, const XVariant& value )
{
    SPluginBase* plugin = static_cast<SPluginBase*>( mPlugin );
    return plugin->SetProperty( plugin, id, value );
}

// Get indexed property of the plug-in - an element of array property
XErrorCode XPlugin::GetIndexedProperty( int32_t id, uint32_t index, XVariant& value ) const
{
    xvariant var;
    XVariantInit( &var );

    SPluginBase* plugin = static_cast<SPluginBase*>( mPlugin );
    XErrorCode ret = plugin->GetIndexedProperty( plugin, id, index, &var );

    if ( ret == SuccessCode )
    {
        value = var;
    }

    XVariantClear( &var );

    return ret;
}

// Set indexed property of the plug-in - an element of array property
XErrorCode XPlugin::SetIndexedProperty( int32_t id, uint32_t index, const XVariant& value )
{
    SPluginBase* plugin = static_cast<SPluginBase*>( mPlugin );
    return plugin->SetIndexedProperty( plugin, id, index, value );
}

// Call function of the plug-in
XErrorCode XPlugin::CallFunction( int32_t id, XVariant* returnValue, const xarray* arguments )
{
    SPluginBase* plugin = static_cast<SPluginBase*>( mPlugin );
    xvariant     fret;

    XVariantInit( &fret );

    XErrorCode ret = plugin->CallFunction( plugin, id, &fret, arguments );

    if ( ( ret == SuccessCode ) && ( returnValue != nullptr ) )
    {
        *returnValue = fret;
    }

    XVariantClear( &fret );

    return ret;
}

// Update description of device run time configuration properties
XErrorCode XPlugin::UpdateDescription( shared_ptr<XPluginDescriptor> descriptor )
{
    SPluginBase* plugin = static_cast<SPluginBase*>( mPlugin );

    return plugin->UpdateDescription( plugin, descriptor->mDescriptor );
}
