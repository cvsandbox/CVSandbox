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
#ifndef CVS_XVARIANT_HPP
#define CVS_XVARIANT_HPP

#include <xtypes.h>
#include <string>
#include "XRange.hpp"
#include "XRangeF.hpp"
#include "XPoint.hpp"
#include "XPointF.hpp"
#include "XSize.hpp"
#include "XColor.hpp"
#include "XImage.hpp"

namespace CVSandbox
{

class XVariantArray;
class XVariantArray2d;
class XVariantArrayJagged;

class XVariant
{
public:
    XVariant( );
    ~XVariant( );

    // Copy constructors
    XVariant( const XVariant& rhs );
    XVariant( const xvariant& rhs );

    // Set of constructors from different types
    XVariant( const char* rhs );
    XVariant( const std::string& rhs );
    explicit XVariant( bool rhs );
    explicit XVariant( const XRange& rhs );
    explicit XVariant( const XRangeF& rhs );
    explicit XVariant( const XPoint& rhs );
    explicit XVariant( const XPointF& rhs );
    explicit XVariant( const XSize& rhs );
    explicit XVariant( const XColor& rhs );
    explicit XVariant( const XVariantArray& rhs );
    explicit XVariant( const XVariantArray2d& rhs );
    explicit XVariant( const XVariantArrayJagged& rhs );
    explicit XVariant( double rhs );
    explicit XVariant( float rhs );
    explicit XVariant( uint32_t rhs );
    explicit XVariant( uint16_t rhs );
    explicit XVariant( uint8_t rhs );
    explicit XVariant( int32_t rhs );
    explicit XVariant( int16_t rhs );
    explicit XVariant( int8_t rhs );
    // XVariant does not have means to handle XImage's properly. It can only be used to pass ximage to functions expecting
    // it through xvariant interface. However it does not provide support for functions, which may want to return ximage
    // through xvariant interface (not through C++ XVariant). So this constructor is a one way hack only.
    // NOTE: Image data must stay alive for the life time of the XVariant object.
    explicit XVariant( const std::shared_ptr<XImage>& rhs );

    // Clear variants
    void SetEmpty( );
    void SetNull( );

    // Assignment operators
    XVariant& operator= ( const XVariant& rhs );
    XVariant& operator= ( const xvariant& rhs );

    operator const xvariant*( ) const { return &mVariant; }

    // Get native type of the value wrapped by variant
    XVarType Type( )      const { return mVariant.type; }
    bool IsArray( )       const { return ( ( mVariant.type & XVT_Array )       == XVT_Array ); }
    bool IsArray2d( )     const { return ( ( mVariant.type & XVT_Array2d )     == XVT_Array2d ); }
    bool IsArrayJagged( ) const { return ( ( mVariant.type & XVT_ArrayJagged ) == XVT_ArrayJagged ); }

    // Check if variant is empty or null
    bool IsEmpty( )       const { return ( mVariant.type == XVT_Empty ); }
    bool IsNull( )        const { return ( mVariant.type == XVT_Null ); }
    bool IsNullOrEmpty( ) const { return ( mVariant.type == XVT_Null ) || ( mVariant.type == XVT_Empty ); }

    // Check if two variants are equal
    bool operator==( const xvariant& rhs ) const;
    bool operator==( const XVariant& rhs ) const;

    // Check if two variants are NOT equal
    bool operator!=( const xvariant& rhs  ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }
    bool operator!=( const XVariant& rhs  ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }

    // Set of type convertors
    const std::string ToString( XErrorCode* errorCode = nullptr ) const;
    const XRange      ToRange ( XErrorCode* errorCode = nullptr ) const;
    const XRangeF     ToRangeF( XErrorCode* errorCode = nullptr ) const;
    const XPoint      ToPoint ( XErrorCode* errorCode = nullptr ) const;
    const XPointF     ToPointF( XErrorCode* errorCode = nullptr ) const;
    const XSize       ToSize  ( XErrorCode* errorCode = nullptr ) const;
    const XColor      ToColor ( XErrorCode* errorCode = nullptr ) const;
    bool              ToBool  ( XErrorCode* errorCode = nullptr ) const;
    double            ToDouble( XErrorCode* errorCode = nullptr ) const;
    float             ToFloat ( XErrorCode* errorCode = nullptr ) const;
    uint32_t          ToUInt  ( XErrorCode* errorCode = nullptr ) const;
    uint16_t          ToUShort( XErrorCode* errorCode = nullptr ) const;
    uint8_t           ToUByte ( XErrorCode* errorCode = nullptr ) const;
    int32_t           ToInt   ( XErrorCode* errorCode = nullptr ) const;
    int16_t           ToShort ( XErrorCode* errorCode = nullptr ) const;
    int8_t            ToByte  ( XErrorCode* errorCode = nullptr ) const;

    XVariantArray       ToArray      ( XErrorCode* errorCode = nullptr ) const;
    XVariantArray2d     ToArray2d    ( XErrorCode* errorCode = nullptr ) const;
    XVariantArrayJagged ToArrayJagged( XErrorCode* errorCode = nullptr ) const;

    // Change to type of the variant to the specified one
    const XVariant ChangeType( XVarType type, XErrorCode* errorCode = nullptr ) const;

    // Make sure numeric variant value is in the specified range
    void CheckInRange( const XVariant& min, const XVariant& max );

private:
    xvariant mVariant;
};

} // namespace CVSandbox

#endif // CVS_XVARIANT_HPP
