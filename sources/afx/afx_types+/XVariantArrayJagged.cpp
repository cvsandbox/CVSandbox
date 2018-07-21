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

#include "XVariantArrayJagged.hpp"
#include "XVariantArray.hpp"

using namespace std;

namespace CVSandbox
{

XVariantArrayJagged::XVariantArrayJagged( ) :
    array( nullptr )
{
}

XVariantArrayJagged::XVariantArrayJagged( XVarType type, uint32_t length ) :
    array( nullptr )
{
    // ignore errors
    XArrayAllocateJagged( &array, type, length );
}

XVariantArrayJagged::~XVariantArrayJagged( )
{
    XArrayFreeJagged( &array );
}

// Copy constructors
XVariantArrayJagged::XVariantArrayJagged( const XVariantArrayJagged& rhs ) :
    array( nullptr )
{
    // the call below does null checks, so don't bother
    XArrayCopyJagged( rhs.array, &array );
}
XVariantArrayJagged::XVariantArrayJagged( const xarrayJagged* rhs ) :
    array( nullptr )
{
    // the call below does null check, so don't bother
    XArrayCopyJagged( rhs, &array );
}

// Move constructor
XVariantArrayJagged::XVariantArrayJagged( XVariantArrayJagged&& rhs )
{
    array = rhs.array;
    rhs.array = nullptr;
}

// Allocate specified sub-array of the jagged array
XErrorCode XVariantArrayJagged::AllocateSubArray( uint32_t outerIndex, uint32_t length )
{
    return XArrayAllocateJaggedSub( array, outerIndex, length );
}

// Check if the array was successfully allocated
bool XVariantArrayJagged::IsAllocated( ) const
{
    return ( array != nullptr );
}

// Check if the sub-array was successfully allocated
bool XVariantArrayJagged::IsAllocated( uint32_t outerIndex ) const
{
    bool allocated = false;

    if ( ( array != nullptr ) && ( outerIndex < array->length ) )
    {
        if ( ( ( array->elements[outerIndex].type & XVT_Array ) == XVT_Array ) &&
             ( array->elements[outerIndex].value.arrayVal != nullptr ) )
        {
            allocated = true;
        }
    }

    return allocated;
}

// Get length of the array
uint32_t XVariantArrayJagged::Length( ) const
{
    return ( array == nullptr ) ? 0 : array->length;
}

// Get length of the specified sub-array
uint32_t XVariantArrayJagged::Length( uint32_t outerIndex ) const
{
    uint32_t subLength = 0;

    if ( ( array != nullptr ) && ( outerIndex < array->length ) )
    {
        if ( ( ( array->elements[outerIndex].type & XVT_Array ) == XVT_Array ) &&
             ( array->elements[outerIndex].value.arrayVal != nullptr ) )
        {
            subLength = array->elements[outerIndex].value.arrayVal->length;
        }
    }

    return subLength;
}

// Get type of array elements
XVarType XVariantArrayJagged::Type( ) const
{
    return ( array == nullptr ) ? XVT_Empty : ( array->type & XVT_Any );
}

// Assignment operators
XVariantArrayJagged& XVariantArrayJagged::operator = ( const XVariantArrayJagged& rhs )
{
    // copy would free old array before allocating new one, but we want to make
    // sure it is freed even if some errors happen
    XArrayFreeJagged( &array );
    XArrayCopyJagged( rhs.array, &array );
    return *this;
}
XVariantArrayJagged& XVariantArrayJagged::operator = ( const xarrayJagged* rhs )
{
    // copy would free old array before allocating new one, but we want to make
    // sure it is freed even if some errors happen
    XArrayFreeJagged( &array );
    XArrayCopyJagged( rhs, &array );
    return *this;
}

// Move assignment
XVariantArrayJagged& XVariantArrayJagged::operator= ( XVariantArrayJagged&& rhs )
{
    if ( this != &rhs )
    {
        XArrayFreeJagged( &array );
        array = rhs.array;
        rhs.array = nullptr;
    }
    return *this;
}

// Check if two arrays are equal
bool XVariantArrayJagged::operator==( const xarrayJagged* rhs ) const
{
    return ( XArrayAreEqualJagged( array, rhs ) == SuccessCode );
}
bool XVariantArrayJagged::operator==( const XVariantArrayJagged& rhs ) const
{
    return ( XArrayAreEqualJagged( array, rhs.array ) == SuccessCode );
}

// Set element of the array
XErrorCode XVariantArrayJagged::Set( uint32_t outerIndex, uint32_t innerIndex, const XVariant& element )
{
    return XArraySetJagged( array, outerIndex, innerIndex, element );
}

// Get element of the array
XErrorCode XVariantArrayJagged::Get( uint32_t outerIndex, uint32_t innerIndex, XVariant& element ) const
{
    xvariant   var;
    XErrorCode ret;

    XVariantInit( &var );

    ret = XArrayGetJagged( array, outerIndex, innerIndex, &var );
    if ( ret == SuccessCode )
    {
        element = var;
    }

    XVariantClear( &var );

    return ret;
}

// Set specified row the array
XErrorCode XVariantArrayJagged::Set( uint32_t outerIndex, const XVariantArray& subArray )
{
    xvariant   var;

    var.type           = XVT_Array | subArray.Type( );
    var.value.arrayVal = const_cast<xarray*>( static_cast<const xarray*>( subArray ) );

    return XArraySet( (xarray*) array, outerIndex, &var );
}

// Get specified row of the array
XErrorCode XVariantArrayJagged::Get( uint32_t outerIndex, XVariantArray& subArray )
{
    xvariant   var;
    XErrorCode ret;

    XVariantInit( &var );

    ret = XArrayGet( (xarray*) array, outerIndex, &var );
    if ( ret == SuccessCode )
    {
        if ( ( ( var.type & XVT_Array ) == XVT_Array ) || ( var.type == XVT_Empty ) )
        {
            subArray = var.value.arrayVal;
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
    }

    XVariantClear( &var );

    return ret;
}


// Set all elements of the array to the specified value
XErrorCode XVariantArrayJagged::SetAll( const XVariant& element )
{
    XErrorCode ret = ErrorIncompatibleTypes;

    if ( element.IsArray( ) )
    {
        ret = XArraySetAll( (xarray*) array, element );
    }
    else if ( !IS_ARRAY_TYPE( element.Type( ) ) )
    {
        ret = XArraySetAllJagged( array, element );
    }

    return ret;
}
XErrorCode XVariantArrayJagged::SetAll( const XVariantArray& subArray )
{
    xvariant   var;

    var.type           = XVT_Array | subArray.Type( );
    var.value.arrayVal = const_cast<xarray*>( static_cast<const xarray*>( subArray ) );

    return XArraySetAll( (xarray*) array, &var );
}

} // namespace CVSandbox
