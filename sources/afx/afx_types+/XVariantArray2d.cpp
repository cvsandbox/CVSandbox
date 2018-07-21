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

#include "XVariantArray2d.hpp"

using namespace std;

namespace CVSandbox
{

XVariantArray2d::XVariantArray2d( ) :
    array( nullptr )
{
}

XVariantArray2d::XVariantArray2d( XVarType type, uint32_t rows, uint32_t columns ) :
    array( nullptr )
{
    // ignore errors
    XArrayAllocate2d( &array, type, rows, columns );
}

XVariantArray2d::XVariantArray2d( XVarType type, uint32_t rows, uint32_t columns, const XVariant& element ) :
    array( nullptr )
{
    if ( XArrayAllocate2d( &array, type, rows, columns ) == SuccessCode )
    {
        XArraySetAll2d( array, element );
    }
}

XVariantArray2d::~XVariantArray2d( )
{
    XArrayFree2d( &array );
}

// Copy constructors
XVariantArray2d::XVariantArray2d( const XVariantArray2d& rhs ) :
    array( nullptr )
{
    // the call below does null checks, so don't bother
    XArrayCopy2d( rhs.array, &array );
}
XVariantArray2d::XVariantArray2d( const xarray2d* rhs ) :
    array( nullptr )
{
    // the call below does null check, so don't bother
    XArrayCopy2d( rhs, &array );
}

// Move constructor
XVariantArray2d::XVariantArray2d( XVariantArray2d&& rhs )
{
    array = rhs.array;
    rhs.array = nullptr;
}

// Check if the array was successfully allocated
bool XVariantArray2d::IsAllocated( ) const
{
    return ( array != nullptr );
}

// Get lngth of the array
uint32_t XVariantArray2d::Length( ) const
{
    return ( array == nullptr ) ? 0 : array->length;
}

// Get number of rows/columns
uint32_t XVariantArray2d::Rows( ) const
{
    return ( array == nullptr ) ? 0 : array->rows;
}
uint32_t XVariantArray2d::Columns( ) const
{
    return ( array == nullptr ) ? 0 : array->cols;
}

// Get array type
XVarType XVariantArray2d::Type( ) const
{
    return ( array == nullptr ) ? XVT_Empty : array->type;
}

// Assignment operators
XVariantArray2d& XVariantArray2d::operator = ( const XVariantArray2d& rhs )
{
    // copy would free old array before allocating new one, but we want to make
    // sure it is freed even if some errors happen
    XArrayFree2d( &array );
    XArrayCopy2d( rhs.array, &array );
    return *this;
}
XVariantArray2d& XVariantArray2d::operator = ( const xarray2d* rhs )
{
    // copy would free old array before allocating new one, but we want to make
    // sure it is freed even if some errors happen
    XArrayFree2d( &array );
    XArrayCopy2d( rhs, &array );
    return *this;
}

// Move assignment
XVariantArray2d& XVariantArray2d::operator= ( XVariantArray2d&& rhs )
{
    if ( this != &rhs )
    {
        XArrayFree2d( &array );
        array = rhs.array;
        rhs.array = nullptr;
    }
    return *this;
}

// Check if two arrays are equal
bool XVariantArray2d::operator==( const xarray2d* rhs ) const
{
    return ( XArrayAreEqual2d( array, rhs ) == SuccessCode );
}
bool XVariantArray2d::operator==( const XVariantArray2d& rhs ) const
{
    return ( XArrayAreEqual2d( array, rhs.array ) == SuccessCode );
}

// Set element of the array
XErrorCode XVariantArray2d::Set( uint32_t row, uint32_t column, const XVariant& element )
{
    return XArraySet2d( array, row, column, element );
}

// Get element of the array
XErrorCode XVariantArray2d::Get( uint32_t row, uint32_t column, XVariant& element ) const
{
    xvariant   var;
    XErrorCode ret;

    XVariantInit( &var );

    ret = XArrayGet2d( array, row, column, &var );
    if ( ret == SuccessCode )
    {
        element = var;
    }

    XVariantClear( &var );

    return ret;
}


// Set all elements of the array to the specified value
XErrorCode XVariantArray2d::SetAll( const XVariant& element )
{
    return XArraySetAll2d( array, element );
}

} // namespace CVSandbox
