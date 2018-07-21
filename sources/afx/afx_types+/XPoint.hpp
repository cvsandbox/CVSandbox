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
#ifndef CVS_XPOINT_HPP
#define CVS_XPOINT_HPP

#include <xtypes.h>

namespace CVSandbox
{

// Integer point class encapsulating X/Y values
class XPoint
{
public:
    // Constructors
    XPoint( int32_t x = 0, int32_t y = 0 );
    XPoint( const XPoint& rhs );
    XPoint( const xpoint& rhs );

    // Assignment operators
    XPoint& operator= ( const XPoint& rhs );
    XPoint& operator= ( const xpoint& rhs );

    // Cast back to C point
    operator xpoint( ) const { return mPoint; }

    // Check if two points are equal
    bool operator==( const XPoint& rhs ) const;

    // Check if two points are NOT equal
    bool operator!=( const XPoint& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    int32_t X( ) const { return mPoint.x; }
    int32_t Y( ) const { return mPoint.y; }

    void SetX( int32_t x ) { mPoint.x = x; }
    void SetY( int32_t y ) { mPoint.y = y; }
    void Set( int32_t x, int32_t y ) { mPoint.x = x; mPoint.y = y; }

private:
    xpoint mPoint;
};

} // namespace CVSandbox

#endif // CVS_XPOINT_HPP
