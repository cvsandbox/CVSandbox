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

#include "XVariant.hpp"
#include "XVariantArray.hpp"
#include "XVariantArray2d.hpp"
#include "XVariantArrayJagged.hpp"

using namespace std;

namespace CVSandbox
{

// Initialize empty variant class
XVariant::XVariant( )
{
    XVariantInit( &mVariant );
}

XVariant::~XVariant( )
{
    XVariantClear( &mVariant );
}

// Copy constructor
XVariant::XVariant( const XVariant& rhs )
{
    XVariantInit( &mVariant );
    XVariantCopy( &rhs.mVariant, &mVariant );
}

// Initialize variant from xvariant structure
XVariant::XVariant( const xvariant& rhs )
{
    XVariantInit( &mVariant );
    XVariantCopy( &rhs, &mVariant );
}

// Initialize variant from boolean
XVariant::XVariant( bool rhs )
{
    mVariant.type = XVT_Bool;
    mVariant.value.boolVal = rhs;
}

// Intialize variant from zero terminated C string
XVariant::XVariant( const char* rhs )
{
    xstring xstr = XStringAlloc( rhs );

    if ( xstr != 0 )
    {
        mVariant.type = XVT_String;
        mVariant.value.strVal = xstr;
    }
    else
    {
        XVariantInit( &mVariant );
    }
}

// Initialize variant from string
XVariant::XVariant( const string& rhs )
{
    xstring xstr = XStringAlloc( rhs.data( ) );

    if ( xstr != 0 )
    {
        mVariant.type = XVT_String;
        mVariant.value.strVal = xstr;
    }
    else
    {
        XVariantInit( &mVariant );
    }
}

// Initialize variant from integer range
XVariant::XVariant( const XRange& rhs )
{
    mVariant.type = XVT_Range;
    mVariant.value.rangeVal = rhs;
}

// Initialize variant from floating point range
XVariant::XVariant( const XRangeF& rhs )
{
    mVariant.type = XVT_RangeF;
    mVariant.value.frangeVal = rhs;
}

// Initialize variant from integer point
XVariant::XVariant( const XPoint& rhs )
{
    mVariant.type = XVT_Point;
    mVariant.value.pointVal = rhs;
}

// Initialize variant from floating point
XVariant::XVariant( const XPointF& rhs )
{
    mVariant.type = XVT_PointF;
    mVariant.value.fpointVal = rhs;
}

// Initialize variant from integer size
XVariant::XVariant( const XSize& rhs )
{
    mVariant.type = XVT_Size;
    mVariant.value.sizeVal = rhs;
}

// Initialize variant from color value
XVariant::XVariant( const XColor& rhs )
{
    mVariant.type = XVT_ARGB;
    mVariant.value.argbVal = rhs;
}

// Initialize variant from array value
XVariant::XVariant( const XVariantArray& rhs )
{
    XVariantInit( &mVariant );

    if ( rhs.IsAllocated( ) )
    {
        mVariant.type = XVT_Array | rhs.Type( );
        XArrayCopy( rhs, &( mVariant.value.arrayVal ) );
    }
}

// Initialize variant from 2d array value
XVariant::XVariant( const XVariantArray2d& rhs )
{
    XVariantInit( &mVariant );

    if ( rhs.IsAllocated( ) )
    {
        mVariant.type = XVT_Array2d | rhs.Type( );
        XArrayCopy2d( rhs, &( mVariant.value.array2Val ) );
    }
}

// Initialize variant from jagged array value
XVariant::XVariant( const XVariantArrayJagged& rhs )
{
    XVariantInit( &mVariant );

    if ( rhs.IsAllocated( ) )
    {
        mVariant.type = XVT_ArrayJagged | rhs.Type( );
        XArrayCopyJagged( rhs, &( mVariant.value.arrayJaggedVal ) );
    }
}

// Initilize variant from image (image data must stay alive)
XVariant::XVariant( const shared_ptr<XImage>& rhs )
{
    ximage* image = rhs->ImageData( );

    XVariantInit( &mVariant );

    if ( XImageCreate( image->data, image->width, image->height, image->stride, image->format, &mVariant.value.imageVal ) == SuccessCode )
    {
        mVariant.type = XVT_Image;
    }
}

// Initialize variant from double value
XVariant::XVariant( double rhs )
{
    mVariant.type = XVT_R8;
    mVariant.value.dVal = rhs;
}

// Initialize variant from float value
XVariant::XVariant( float rhs )
{
    mVariant.type = XVT_R4;
    mVariant.value.fVal = rhs;
}

// Initialize variant from 32 bit unsigned integer value
XVariant::XVariant( uint32_t rhs )
{
    mVariant.type = XVT_U4;
    mVariant.value.uiVal = rhs;
}

// Initialize variant from 16 bit unsigned integer value
XVariant::XVariant( uint16_t rhs )
{
    mVariant.type = XVT_U2;
    mVariant.value.usVal = rhs;
}

// Initialize variant from 8 bit unsigned integer value
XVariant::XVariant( uint8_t rhs )
{
    mVariant.type = XVT_U1;
    mVariant.value.ubVal = rhs;
}

// Initialize variant from 32 bit signed integer value
XVariant::XVariant( int32_t rhs )
{
    mVariant.type = XVT_I4;
    mVariant.value.iVal = rhs;
}

// Initialize variant from 16 bit signed integer value
XVariant::XVariant( int16_t rhs )
{
    mVariant.type = XVT_I2;
    mVariant.value.sVal = rhs;
}

// Initialize variant from 8 bit signed integer value
XVariant::XVariant( int8_t rhs )
{
    mVariant.type = XVT_I1;
    mVariant.value.bVal = rhs;
}

// Clear variants
void XVariant::SetEmpty( )
{
    XVariantClear( &mVariant );
}
void XVariant::SetNull( )
{
    XVariantClear( &mVariant );
    mVariant.type = XVT_Null;
}

// Assignment operator making copy of another XVariant object
XVariant& XVariant::operator=( const XVariant& rhs )
{
    XVariantCopy( &rhs.mVariant, &mVariant );
    return *this;
}

// Assignment operator making copy of xvariant structure
XVariant& XVariant::operator=( const xvariant& rhs )
{
    XVariantCopy( &rhs, &mVariant );
    return *this;
}

// Check if two variants are equal
bool XVariant::operator==( const xvariant& rhs ) const
{
    return ( XVariantAreEqual( &mVariant, &rhs ) == SuccessCode );
}
bool XVariant::operator==( const XVariant& rhs ) const
{
    return ( XVariantAreEqual( &mVariant, &rhs.mVariant ) == SuccessCode );
}

// Convert variant to string
const string XVariant::ToString( XErrorCode* errorCode ) const
{
    string  ret;
    xstring xstr;

    XErrorCode code = XVariantToString( &mVariant, &xstr );

    if ( code == SuccessCode )
    {
        ret.assign( xstr );
        XStringFree( &xstr );
    }

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to boolean
bool XVariant::ToBool( XErrorCode* errorCode ) const
{
    bool       ret  = false;
    XErrorCode code = XVariantToBool( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to integer range
const XRange XVariant::ToRange( XErrorCode* errorCode ) const
{
    XRange ret;
    xrange range;

    XErrorCode code = XVariantToRange( &mVariant, &range );

    if ( code == SuccessCode )
    {
        ret = range;
    }

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to floating point range
const XRangeF XVariant::ToRangeF( XErrorCode* errorCode ) const
{
    XRangeF ret;
    xrangef range;

    XErrorCode code = XVariantToRangeF( &mVariant, &range );

    if ( code == SuccessCode )
    {
        ret = range;
    }

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to integer point
const XPoint XVariant::ToPoint( XErrorCode* errorCode ) const
{
    XPoint ret;
    xpoint point;

    XErrorCode code = XVariantToPoint( &mVariant, &point );

    if ( code == SuccessCode )
    {
        ret = point;
    }

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to floating point
const XPointF XVariant::ToPointF( XErrorCode* errorCode ) const
{
    XPointF ret;
    xpointf point;

    XErrorCode code = XVariantToPointF( &mVariant, &point );

    if ( code == SuccessCode )
    {
        ret = point;
    }

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to integer size
const XSize XVariant::ToSize( XErrorCode* errorCode ) const
{
    XSize ret;
    xsize size;

    XErrorCode code = XVariantToSize( &mVariant, &size );

    if ( code == SuccessCode )
    {
        ret = size;
    }

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to color value
const XColor XVariant::ToColor( XErrorCode* errorCode ) const
{
    XColor ret;
    xargb  argb;

    XErrorCode code = XVariantToArgb( &mVariant, &argb );

    if ( code == SuccessCode )
    {
        ret = argb;
    }

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to double
double XVariant::ToDouble( XErrorCode* errorCode ) const
{
    double     ret  = 0.0;
    XErrorCode code = XVariantToDouble( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to float
float XVariant::ToFloat( XErrorCode* errorCode ) const
{
    float      ret  = 0.0f;
    XErrorCode code = XVariantToFloat( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to 32 bit unsigned integer
uint32_t XVariant::ToUInt( XErrorCode* errorCode ) const
{
    uint32_t   ret  = 0;
    XErrorCode code = XVariantToUInt( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to 16 bit unsigned integer
uint16_t XVariant::ToUShort( XErrorCode* errorCode ) const
{
    uint16_t   ret  = 0;
    XErrorCode code = XVariantToUShort( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to 8 bit unsigned integer
uint8_t XVariant::ToUByte( XErrorCode* errorCode ) const
{
    uint8_t    ret  = 0;
    XErrorCode code = XVariantToUByte( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to 32 bit signed integer
int32_t XVariant::ToInt( XErrorCode* errorCode ) const
{
    int32_t    ret  = 0;
    XErrorCode code = XVariantToInt( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to 16 bit signed integer
int16_t XVariant::ToShort( XErrorCode* errorCode ) const
{
    int16_t    ret  = 0;
    XErrorCode code = XVariantToShort( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to 8 bit signed integer
int8_t XVariant::ToByte( XErrorCode* errorCode ) const
{
    int8_t     ret  = 0;
    XErrorCode code = XVariantToByte( &mVariant, &ret );

    if ( errorCode != nullptr )
    {
        *errorCode = code;
    }

    return ret;
}

// Convert variant to an array
XVariantArray XVariant::ToArray( XErrorCode* errorCode ) const
{
    XVariantArray array;
    XErrorCode    ec = ErrorIncompatibleTypes;

    if ( ( mVariant.type & XVT_Array ) == XVT_Array )
    {
        ec = XArrayCopy( mVariant.value.arrayVal, &array.array );
    }

    if ( errorCode != nullptr )
    {
        *errorCode = ec;
    }

    return array;
}

// Convert variant to a 2D array
XVariantArray2d XVariant::ToArray2d( XErrorCode* errorCode ) const
{
    XVariantArray2d array;
    XErrorCode      ec = ErrorIncompatibleTypes;

    if ( ( mVariant.type & XVT_Array2d ) == XVT_Array2d )
    {
        ec = XArrayCopy2d( mVariant.value.array2Val, &array.array );
    }

    if ( errorCode != nullptr )
    {
        *errorCode = ec;
    }

    return array;
}

// Convert variant to a jagged array
XVariantArrayJagged XVariant::ToArrayJagged( XErrorCode* errorCode ) const
{
    XVariantArrayJagged array;
    XErrorCode          ec = ErrorIncompatibleTypes;

    if ( ( mVariant.type & XVT_ArrayJagged ) == XVT_ArrayJagged )
    {
        ec = XArrayCopyJagged( mVariant.value.arrayJaggedVal, &array.array );
    }

    if ( errorCode != nullptr )
    {
        *errorCode = ec;
    }

    return array;
}

// Change to type of the variant to the specified one
const XVariant XVariant::ChangeType( XVarType type, XErrorCode* errorCode ) const
{
    XVariant ret;
    xvariant temp;

    XVariantInit( &temp );

    XErrorCode error = XVariantChangeType( &mVariant, &temp, type );

    // provide error code if user wants it
    if ( errorCode != nullptr )
    {
        *errorCode = error;
    }

    // provide new variant if conversion succeeded
    if ( error == SuccessCode )
    {
        ret = temp;
    }

    XVariantClear( &temp );

    return ret;
}

// Make sure numeric variant value is in the specified range
void XVariant::CheckInRange( const XVariant& min, const XVariant& max )
{
    XVariantCheckInRange( &mVariant, &min.mVariant, &max.mVariant );
}

} // namespace CVSandbox
