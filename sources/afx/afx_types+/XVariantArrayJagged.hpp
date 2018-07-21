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
#ifndef CVS_XVARIANT_ARRAY_JAGGED_HPP
#define CVS_XVARIANT_ARRAY_JAGGED_HPP

#include <xtypes.h>
#include "XVariant.hpp"

namespace CVSandbox
{

class XVariantArray;

class XVariantArrayJagged
{
friend class XVariant;

public:
    XVariantArrayJagged( );
    XVariantArrayJagged( XVarType type, uint32_t length );

    ~XVariantArrayJagged( );

    // Copy constructors
    XVariantArrayJagged( const XVariantArrayJagged& rhs );
    XVariantArrayJagged( const xarrayJagged* rhs );

    // Move constructor
    XVariantArrayJagged( XVariantArrayJagged&& rhs );

    operator const xarrayJagged*( ) const { return array; }

    // Allocate specified sub-array of the jagged array
    XErrorCode AllocateSubArray( uint32_t outerIndex, uint32_t length );

    // Check if the array was successfully allocated
    bool IsAllocated( ) const;
    // Check if the sub-array was successfully allocated
    bool IsAllocated( uint32_t outerIndex ) const;
    // Get length of the array
    uint32_t Length( ) const;
    // Get length of the specified sub-array (return 0 if sub array is not allocated or the outer index is out of bounds)
    uint32_t Length( uint32_t outerIndex ) const;
    // Get array type
    XVarType Type( ) const;

    // Assignment operators
    XVariantArrayJagged& operator= ( const XVariantArrayJagged& rhs );
    XVariantArrayJagged& operator= ( const xarrayJagged* rhs );
    // Move assignment
    XVariantArrayJagged& operator= ( XVariantArrayJagged&& rhs );

    // Check if two arrays are equal
    bool operator==( const xarrayJagged* rhs ) const;
    bool operator==( const XVariantArrayJagged& rhs ) const;

    // Check if two arrays are NOT equal
    bool operator!=( const xarrayJagged* rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }
    bool operator!=( const XVariantArrayJagged& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    // Set element of the array
    XErrorCode Set( uint32_t outerIndex, uint32_t innerIndex, const XVariant& element );
    // Get element of the array
    XErrorCode Get( uint32_t outerIndex, uint32_t innerIndex, XVariant& element ) const;

    // Set specified row of the array
    XErrorCode Set( uint32_t outerIndex, const XVariantArray& subArray );
    // Get specified row of the array
    XErrorCode Get( uint32_t outerIndex, XVariantArray& subArray );

    // Set all elements of the array to the specified value
    XErrorCode SetAll( const XVariant& element );
    XErrorCode SetAll( const XVariantArray& subArray );

private:
    xarrayJagged* array;
};

} // namespace CVSandbox

#endif // CVS_XVARIANT_ARRAY_JAGGED_HPP
