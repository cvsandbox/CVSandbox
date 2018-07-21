/*
    Core C++ types library of Computer Vision Sandbox

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
#ifndef CVS_XVARIANT_ARRAY_HPP
#define CVS_XVARIANT_ARRAY_HPP

#include <xtypes.h>
#include "XVariant.hpp"

namespace CVSandbox
{

class XVariantArray
{
friend class XVariant;

public:
    XVariantArray( );
    XVariantArray( XVarType type, uint32_t length );
    XVariantArray( XVarType type, uint32_t length, const XVariant& element );

    ~XVariantArray( );

    // Copy constructors
    XVariantArray( const XVariantArray& rhs );
    XVariantArray( const xarray* rhs );

    // Move constructor
    XVariantArray( XVariantArray&& rhs );

    operator const xarray*( ) const { return array; }

    // Check if the array was successfully allocated
    bool IsAllocated( ) const;
    // Get length of the array
    uint32_t Length( ) const;
    // Get array type
    XVarType Type( ) const;

    // Assignment operators
    XVariantArray& operator= ( const XVariantArray& rhs );
    XVariantArray& operator= ( const xarray* rhs );
    // Move assignment
    XVariantArray& operator= ( XVariantArray&& rhs );

    // Check if two arrays are equal
    bool operator==( const xarray* rhs ) const;
    bool operator==( const XVariantArray& rhs ) const;

    // Check if two arrays are NOT equal
    bool operator!=( const xarray* rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }
    bool operator!=( const XVariantArray& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    // Set element of the array
    XErrorCode Set( uint32_t index, const XVariant& element );
    // Get element of the array
    XErrorCode Get( uint32_t index, XVariant& element ) const;

    // Set all elements of the array to the specified value
    XErrorCode SetAll( const XVariant& element );

private:
    xarray* array;
};

} // namespace CVSandbox

#endif // CVS_XVARIANT_ARRAY_HPP
