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

#include <assert.h>
#include "XPluginDescriptor.hpp"
#include "XPluginWrapperFactory.hpp"

using namespace std;
using namespace CVSandbox;

XPluginDescriptor::XPluginDescriptor( PluginDescriptor* desc ) :
    mDescriptor( desc ), mProperties( ), mFunctions( )
{
    // collect properties
    if ( ( mDescriptor->PropertiesCount != 0 ) && ( mDescriptor->Properties != 0 ) )
    {
        mProperties.reserve( desc->PropertiesCount );

        for ( int32_t i = 0; i < mDescriptor->PropertiesCount; i++ )
        {
            mProperties.push_back( XPropertyDescriptor::Create( i, desc->Properties[i] ) );
        }
    }

    // collect functions
    if ( ( mDescriptor->FunctionsCount != 0 ) && ( mDescriptor->Functions != 0 ) )
    {
        mFunctions.reserve( desc->FunctionsCount );

        for ( int32_t i = 0; i < mDescriptor->FunctionsCount; i++ )
        {
            mFunctions.push_back( XFunctionDescriptor::Create( i, desc->Functions[i] ) );
        }
    }
}

XPluginDescriptor::~XPluginDescriptor( )
{
    FreePluginDescriptor( &mDescriptor );
}

shared_ptr<XPluginDescriptor> XPluginDescriptor::Create( PluginDescriptor* desc )
{
    assert( desc );
    return shared_ptr<XPluginDescriptor>( ( desc == 0 ) ? 0 : new XPluginDescriptor( desc ) );
}

shared_ptr<XPluginDescriptor> XPluginDescriptor::Clone( ) const
{
    return shared_ptr<XPluginDescriptor>( new XPluginDescriptor( CopyPluginDescriptor( mDescriptor ) ) );
}

// Plug-in ID
XGuid XPluginDescriptor::ID( ) const
{
    return XGuid( mDescriptor->ID );
}

// Plugin's family ID
XGuid XPluginDescriptor::FamilyID( ) const
{
    return XGuid( mDescriptor->Family );
}

XVersion XPluginDescriptor::Version( ) const
{
    return XVersion( mDescriptor->Version );
}

string XPluginDescriptor::ShortName( ) const
{
    return std::string( mDescriptor->ShortName );
}

string XPluginDescriptor::Name( ) const
{
    return std::string( mDescriptor->Name );
}

string XPluginDescriptor::Description( ) const
{
    return std::string( mDescriptor->Description );
}

string XPluginDescriptor::Help( ) const
{
    return std::string( mDescriptor->Help );
}

shared_ptr<const XImage> XPluginDescriptor::Icon( bool getSmall ) const
{
    return XImage::Create( const_cast<const ximage*>(
        ( getSmall ) ? mDescriptor->SmallIcon : mDescriptor->Icon ) );
}

// Number of properties which are not read-only
int32_t XPluginDescriptor::ConfigurablePropertiesCount( ) const
{
    int32_t count = 0;

    for ( int i = 0, n = mDescriptor->PropertiesCount; i < n; i++ )
    {
        if ( ( !mProperties[i]->IsReadOnly( ) ) &&
             ( !mProperties[i]->IsHidden( ) ) )
        {
            count++;
        }
    }

    return count;
}

// Number of properties which can be configured while plug-in is running (video source or device)
int32_t XPluginDescriptor::DeviceRuntimeConfigurablePropertiesCount( ) const
{
    int32_t count = 0;

    for ( int i = 0, n = mDescriptor->PropertiesCount; i < n; i++ )
    {
        if ( ( !mProperties[i]->IsReadOnly( ) ) &&
             ( !mProperties[i]->IsHidden( ) ) &&
             (  mProperties[i]->IsDeviceRuntimeConfiguration( ) ) )
        {
            count++;
        }
    }

    return count;
}

// Get descriptor of the property with the specified index
const shared_ptr<const XPropertyDescriptor> XPluginDescriptor::GetPropertyDescriptor( int32_t id ) const
{
    shared_ptr<const XPropertyDescriptor> prop;

    if ( ( id >= 0 ) && ( id < mDescriptor->PropertiesCount ) )
    {
        if ( mDescriptor->PropertyUpdater != 0 )
        {
            mDescriptor->PropertyUpdater( mDescriptor );
        }

        prop = mProperties[id];
    }

    return prop;
}

// Get descriptor of the property with the specified short name
const shared_ptr<const XPropertyDescriptor> XPluginDescriptor::GetPropertyDescriptor( const std::string& shortName ) const
{
    return GetPropertyDescriptor( GetPropertyIndexByName( shortName ) );
}

// Get index of the property with the given short name
int32_t XPluginDescriptor::GetPropertyIndexByName( const std::string& shortName ) const
{
    int32_t index = -1;

    for ( int32_t i = 0, n = mDescriptor->PropertiesCount; i < n; i++ )
    {
        if ( mProperties[i]->ShortName( ) == shortName )
        {
            index = i;
            break;
        }
    }

    return index;
}

// Get descriptor of a function with the specified index
const shared_ptr<const XFunctionDescriptor> XPluginDescriptor::GetFunctionDescriptor( int32_t id ) const
{
    shared_ptr<const XFunctionDescriptor> func;

    if ( ( id >= 0 ) && ( id < mDescriptor->FunctionsCount ) )
    {
        func = mFunctions[id];
    }

    return func;
}

// Get descriptor of a function with the specified name
const shared_ptr<const XFunctionDescriptor> XPluginDescriptor::GetFunctionDescriptor( const string& name ) const
{
    return GetFunctionDescriptor( GetFunctionIndexByName( name) );
}

// Get index of a function with the given name
int32_t XPluginDescriptor::GetFunctionIndexByName( const string& name ) const
{
    int32_t index = -1;

    for ( int32_t i = 0, n = mDescriptor->FunctionsCount; i < n; i++ )
    {
        if ( mFunctions[i]->Name( ) == name )
        {
            index = i;
            break;
        }
    }

    return index;

}

// Create instance of the plug-in
const shared_ptr<XPlugin> XPluginDescriptor::CreateInstance( ) const
{
    return XPluginWrapperFactory::CreateWrapper( mDescriptor->Creator( ), mDescriptor->Type );
}

// Helper function to get plug-in's configuration
static map<string, XVariant> GetPluginConfigurationHelper( const PluginDescriptor* descriptor, const shared_ptr<const XPlugin> plugin,
                                                           bool deviceRuntimeOnly = false )
{
    map<string, XVariant> properties;

    if ( plugin )
    {
        for ( int32_t i = 0, n = descriptor->PropertiesCount; i < n; i++ )
        {
            if ( ( ( descriptor->Properties[i]->Flags & PropertyFlag_ReadOnly ) == 0 ) &&
                 ( ( !deviceRuntimeOnly ) ||
                   ( ( deviceRuntimeOnly ) && ( ( descriptor->Properties[i]->Flags & PropertyFlag_DeviceRuntimeConfiguration ) != 0 ) )
                 )
               )
            {
                XVariant value;

                if ( plugin->GetProperty( i, value ) == SuccessCode )
                {
                    properties.insert( pair<string, XVariant>( descriptor->Properties[i]->ShortName, value ) );
                }
            }
        }
    }

    return properties;
}

// Get plug-in configuration as a map of properties' name-value
map<string, XVariant> XPluginDescriptor::GetPluginConfiguration( const shared_ptr<const XPlugin> plugin ) const
{
    return GetPluginConfigurationHelper( mDescriptor, plugin );
}

// Get plug-in device run time configuration as a map of properties' name-value
map<string, XVariant> XPluginDescriptor::GetPluginDeviceRuntimeConfiguration( const shared_ptr<const XPlugin> plugin ) const
{
    return GetPluginConfigurationHelper( mDescriptor, plugin, true );
}

// Set plug-in configuration
XErrorCode XPluginDescriptor::SetPluginConfiguration( const shared_ptr<XPlugin> plugin, const map<string, XVariant>& config ) const
{
    XErrorCode ret = SuccessCode;

    for ( map<string, XVariant>::const_iterator i = config.begin( ); i != config.end( ); i++ )
    {
        int propertyIndex = GetPropertyIndexByName( i->first );

        if ( propertyIndex != -1 )
        {
            plugin->SetProperty( propertyIndex, i->second );
        }
    }

    return ret;
}

// Get copy of the wrapped C plug-in descriptor
PluginDescriptor* XPluginDescriptor::GetPluginDescriptorCopy( ) const
{
    return CopyPluginDescriptor( mDescriptor );
}
