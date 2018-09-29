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
#ifndef CVS_XPROPERTY_DESCRIPTOR_HPP
#define CVS_XPROPERTY_DESCRIPTOR_HPP

#include <string>
#include <memory>
#include <XVariant.hpp>
#include <XInterfaces.hpp>
#include <iproperty.h>

// Class which wraps description of a property
class XPropertyDescriptor : private CVSandbox::Uncopyable
{
private:
    XPropertyDescriptor( int32_t id, PropertyDescriptor* desc );

public:
    ~XPropertyDescriptor( );

    static const std::shared_ptr<XPropertyDescriptor> Create( int32_t id, PropertyDescriptor* desc );

    // Property ID
    inline int32_t ID( ) const { return mID; }

    // Description of the property
    const std::string Name( ) const;
    const std::string ShortName( ) const;
    const std::string Description( ) const;

    // Type of the property
    XVarType Type( ) const
    {
        return mDescriptor->Type;
    }
    // Check if the property is indexed (of array type)
    bool IsIndexed( ) const
    {
        return ( ( mDescriptor->Type & XVT_Array ) == XVT_Array );
    }

    // Check if the property is read only or can be configured
    bool IsReadOnly( ) const;
    // Check if the property is currently disabled (cannot be modified)
    bool IsDisabled( ) const;
    // Check if the property must stay hidden from user
    bool IsHidden( ) const;

    // Check if the property is enum like (selection) property (by index or by value)
    bool IsSelection( ) const;
    bool IsSelectionByIndex( ) const;
    bool IsSelectionByValue( ) const;
    // Check if the property's configuration is only known at run time
    bool IsRuntimeConfiguration( ) const;
    // Check if the property's configuration is only known at run time of a device (or video source)
    bool IsDeviceRuntimeConfiguration( ) const;
    // Check if the property depends on some other's properties value
    bool IsDependent( ) const;
    // Get ID of the preferred editor for the property
    uint32_t PreferredEditor( ) const;

    // Get default/min/max values of the property
    const CVSandbox::XVariant GetDefaultValue( ) const;
    const CVSandbox::XVariant GetMinValue( ) const;
    const CVSandbox::XVariant GetMaxValue( ) const;

    // Get number of possible choices for a enum-like property
    // (when choice must be done from a set of possible values)
    int32_t ChoicesCount( ) const
    {
        return mDescriptor->ChoicesCount;
    }

    // Get possible choice with the specified index
    const CVSandbox::XVariant GetPossibleChoice( int32_t index ) const;
    // Get index of possible choice by its value
    int32_t GetChoiceIndex( const CVSandbox::XVariant& choice ) const;

    // Get ID of the dependent property
    int32_t GetParentPropertyID( ) const;
    // Update dependent property's info
    bool Update( const CVSandbox::XVariant& parentValue ) const;

private:
    int32_t             mID;
    PropertyDescriptor* mDescriptor;
};

#endif // CVS_XPROPERTY_DESCRIPTOR_HPP
