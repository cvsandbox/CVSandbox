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
#include "XPropertyDescriptor.hpp"

using namespace std;
using namespace CVSandbox;

XPropertyDescriptor::XPropertyDescriptor( int32_t id, PropertyDescriptor* desc ) :
    mID( id ),
    mDescriptor( desc )
{
}

XPropertyDescriptor::~XPropertyDescriptor( )
{
}

const shared_ptr<XPropertyDescriptor> XPropertyDescriptor::Create( int32_t id, PropertyDescriptor* desc )
{
    assert( desc );
    return shared_ptr<XPropertyDescriptor>( ( desc == 0 ) ? 0 : new XPropertyDescriptor( id, desc ) );
}

const string XPropertyDescriptor::Name( ) const
{
    return std::string( mDescriptor->Name );
}

const string XPropertyDescriptor::ShortName( ) const
{
    return std::string( mDescriptor->ShortName );
}

const string XPropertyDescriptor::Description( ) const
{
    return std::string( mDescriptor->Description );
}

// Check if the property is read only or can be configured
bool XPropertyDescriptor::IsReadOnly( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_ReadOnly ) != 0 );
}

// Check if the property is currently disabled (cannot be modified)
bool XPropertyDescriptor::IsDisabled( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_Disabled ) != 0 );
}

// Check if the property must stay hidden from user
bool XPropertyDescriptor::IsHidden( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_Hidden ) != 0 );
}

// Check if the property is enum like (selection) property (by index or by value)
bool XPropertyDescriptor::IsSelection( ) const
{
    return ( ( IsSelectionByIndex( ) ) || ( IsSelectionByValue( ) ) );
}
bool XPropertyDescriptor::IsSelectionByIndex( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_SelectionByIndex ) != 0 );
}
bool XPropertyDescriptor::IsSelectionByValue( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_SelectionByValue ) != 0 );
}

// Check if the property's configuration is only known at run time
bool XPropertyDescriptor::IsRuntimeConfiguration( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_RuntimeConfiguration ) != 0 );
}

// Check if the property's configuration is only known at run time of a device (or video source)
bool XPropertyDescriptor::IsDeviceRuntimeConfiguration( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_DeviceRuntimeConfiguration ) != 0 );
}

// Check if the property depends on some other's properties value
bool XPropertyDescriptor::IsDependent( ) const
{
    return ( ( mDescriptor->Flags & PropertyFlag_Dependent ) != 0 );
}

// Get ID of the preferred editor for the property
uint32_t XPropertyDescriptor::PreferredEditor( ) const
{
    return ( mDescriptor->Flags & PropertyFlag_PreferredEditor_Mask );
}

// Default value of the property
const XVariant XPropertyDescriptor::GetDefaultValue( ) const
{
    return XVariant( mDescriptor->DefaultValue );
}

// Minimum allowed value of the property
const XVariant XPropertyDescriptor::GetMinValue( ) const
{
    return XVariant( mDescriptor->MinValue );
}

// Maximum allowed value of the property
const XVariant XPropertyDescriptor::GetMaxValue( ) const
{
    return XVariant( mDescriptor->MaxValue );
}

// Get possible choice with the specified index
const XVariant XPropertyDescriptor::GetPossibleChoice( int32_t index ) const
{
    XVariant ret;

    if ( ( index >= 0 ) && ( index < mDescriptor->ChoicesCount ) )
    {
        ret = mDescriptor->Choices[index];
    }

    return ret;
}

// Get index of possible choice by its value
int32_t XPropertyDescriptor::GetChoiceIndex( const CVSandbox::XVariant& choice ) const
{
    int32_t ret = -1;

    for ( int32_t i = 0; i < mDescriptor->ChoicesCount; i++ )
    {
        if ( choice == mDescriptor->Choices[i] )
        {
            ret = i;
            break;
        }
    }

    return ret;
}

// Get ID of the dependent property
int32_t XPropertyDescriptor::GetParentPropertyID( ) const
{
    return ( IsDependent( ) ) ? mDescriptor->ParentProperty : 0;
}

// Update dependent property's info
bool XPropertyDescriptor::Update( const XVariant& parentValue ) const
{
    bool ret = false;

    if ( ( IsDependent( ) ) && ( mDescriptor->Updater != NULL ) )
    {
        if ( mDescriptor->Updater( mDescriptor, parentValue ) == SuccessCode )
        {
            ret = true;
        }
    }

    return ret;
}
