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
#ifndef CVS_XRANGE_HPP
#define CVS_XRANGE_HPP

#include <xtypes.h>

namespace CVSandbox
{

// Integer range class encapsulating min/max values
class XRange
{
public:
    // Constructors
    XRange( int32_t min = 0, int32_t max = 0 );
    XRange( const XRange& rhs );
    XRange( const xrange& rhs );

    // Assignment operators
    XRange& operator= ( const XRange& rhs );
    XRange& operator= ( const xrange& rhs );

    // Cast back to C range
    operator xrange( ) const { return mRange; }

    // Check if two ranges are equal
    bool operator==( const XRange& rhs ) const;

    // Check if two ranges are NOT equal
    bool operator!=( const XRange& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    int32_t Min( ) const { return mRange.min; }
    int32_t Max( ) const { return mRange.max; }

    void SetMin( int32_t min ) { mRange.min = min; }
    void SetMax( int32_t max ) { mRange.max = max; }
    void Set( int32_t min, int32_t max ) { mRange.min = min; mRange.max = max; }

private:
    xrange mRange;
};

} // namespace CVSandbox

#endif // CVS_XRANGE_HPP
