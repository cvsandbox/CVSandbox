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
#ifndef CVS_XPOINTF_HPP
#define CVS_XPOINTF_HPP

#include <xtypes.h>

namespace CVSandbox
{

// Floating point class encapsulating X/Y values
class XPointF
{
public:
    // Constructors
    XPointF( float x = 0, float y = 0 );
    XPointF( const XPointF& rhs );
    XPointF( const xpointf& rhs );

    // Assignment operators
    XPointF& operator= ( const XPointF& rhs );
    XPointF& operator= ( const xpointf& rhs );

    // Cast back to C point
    operator xpointf( ) const { return mPoint; }

    // Check if two points are equal
    bool operator==( const XPointF& rhs ) const;

    // Check if two points are NOT equal
    bool operator!=( const XPointF& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    float X( ) const { return mPoint.x; }
    float Y( ) const { return mPoint.y; }

    void SetX( float x ) { mPoint.x = x; }
    void SetY( float y ) { mPoint.y = y; }
    void Set( float x, float y ) { mPoint.x = x; mPoint.y = y; }

private:
    xpointf mPoint;
};

} // namespace CVSandbox

#endif // CVS_XPOINTF_HPP
