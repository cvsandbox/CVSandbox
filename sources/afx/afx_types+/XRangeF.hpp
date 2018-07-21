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
#ifndef CVS_XRANGEF_HPP
#define CVS_XRANGEF_HPP

#include <xtypes.h>

namespace CVSandbox
{

// Floating point range class encapsulating min/max values
class XRangeF
{
public:
    // Constructors
    XRangeF( float min = 0, float max = 0 );
    XRangeF( const XRangeF& rhs );
    XRangeF( const xrangef& rhs );

    // Assignment operators
    XRangeF& operator= ( const XRangeF& rhs );
    XRangeF& operator= ( const xrangef& rhs );

    // Cast back to C range
    operator xrangef( ) const { return mRange; }

    // Check if two ranges are equal
    bool operator==( const XRangeF& rhs ) const;

    // Check if two ranges are NOT equal
    bool operator!=( const XRangeF& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    float Min( ) const { return mRange.min; }
    float Max( ) const { return mRange.max; }

    void SetMin( float min ) { mRange.min = min; }
    void SetMax( float max ) { mRange.max = max; }
    void Set( float min, float max ) { mRange.min = min; mRange.max = max; }

private:
    xrangef mRange;
};

} // namespace CVSandbox

#endif // CVS_XRANGEF_HPP
