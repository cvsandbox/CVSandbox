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

#include "XVariantArray.hpp"

using namespace std;

namespace CVSandbox
{

XVariantArray::XVariantArray( ) :
    array( nullptr )
{
}

XVariantArray::XVariantArray( XVarType type, uint32_t length ) :
    array( nullptr )
{
    // ignore errors
    XArrayAllocate( &array, type, length );
}

XVariantArray::XVariantArray( XVarType type, uint32_t length, const XVariant& element ) :
    array( nullptr )
{
    if ( XArrayAllocate( &array, type, length ) == SuccessCode )
    {
        XArraySetAll( array, element );
    }
}

XVariantArray::~XVariantArray( )
{
    XArrayFree( &array );
}

// Copy constructors
XVariantArray::XVariantArray( const XVariantArray& rhs ) :
    array( nullptr )
{
    // the call below does null checks, so don't bother
    XArrayCopy( rhs.array, &array );
}
XVariantArray::XVariantArray( const xarray* rhs ) :
    array( nullptr )
{
    // the call below does null check, so don't bother
    XArrayCopy( rhs, &array );
}

// Move constructor
XVariantArray::XVariantArray( XVariantArray&& rhs )
{
    array = rhs.array;
    rhs.array = nullptr;
}

// Check if the array was successfully allocated
bool XVariantArray::IsAllocated( ) const
{
    return ( array != nullptr );
}

// Get lngth of the array
uint32_t XVariantArray::Length( ) const
{
    return ( array == nullptr ) ? 0 : array->length;
}

// Get array type
XVarType XVariantArray::Type( ) const
{
    return ( array == nullptr ) ? XVT_Empty : array->type;
}

// Assignment operators
XVariantArray& XVariantArray::operator = ( const XVariantArray& rhs )
{
    // copy would free old array before allocating new one, but we want to make
    // sure it is freed even if some errors happen
    XArrayFree( &array );
    XArrayCopy( rhs.array, &array );
    return *this;
}
XVariantArray& XVariantArray::operator = ( const xarray* rhs )
{
    // copy would free old array before allocating new one, but we want to make
    // sure it is freed even if some errors happen
    XArrayFree( &array );
    XArrayCopy( rhs, &array );
    return *this;
}

// Move assignment
XVariantArray& XVariantArray::operator= ( XVariantArray&& rhs )
{
    if ( this != &rhs )
    {
        XArrayFree( &array );
        array = rhs.array;
        rhs.array = nullptr;
    }
    return *this;
}

// Check if two arrays are equal
bool XVariantArray::operator==( const xarray* rhs ) const
{
    return ( XArrayAreEqual( array, rhs ) == SuccessCode );
}
bool XVariantArray::operator==( const XVariantArray& rhs ) const
{
    return ( XArrayAreEqual( array, rhs.array ) == SuccessCode );
}

// Set element of the array
XErrorCode XVariantArray::Set( uint32_t index, const XVariant& element )
{
    return XArraySet( array, index, element );
}

// Get element of the array
XErrorCode XVariantArray::Get( uint32_t index, XVariant& element ) const
{
    xvariant   var;
    XErrorCode ret;

    XVariantInit( &var );

    ret = XArrayGet( array, index, &var );
    if ( ret == SuccessCode )
    {
        element = var;
    }

    XVariantClear( &var );

    return ret;
}

// Set all elements of the array to the specified value
XErrorCode XVariantArray::SetAll( const XVariant& element )
{
    return XArraySetAll( array, element );
}

} // namespace CVSandbox
