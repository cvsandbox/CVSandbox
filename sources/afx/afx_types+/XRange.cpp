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

#include "XRange.hpp"

namespace CVSandbox
{

XRange::XRange( int32_t min, int32_t max ) :
    mRange( )
{
    mRange.min = min;
    mRange.max = max;
}

XRange::XRange( const XRange& rhs ) :
    mRange( rhs.mRange )
{
}

XRange::XRange( const xrange& rhs ) :
    mRange( rhs )
{
}

XRange& XRange::operator=( const XRange& rhs )
{
    mRange = rhs.mRange;
    return *this;
}

XRange& XRange::operator=( const xrange& rhs )
{
    mRange = rhs;
    return *this;
}

// Check if two ranges are equal
bool XRange::operator==( const XRange& rhs ) const
{
    return ( ( mRange.min == rhs.mRange.min ) && ( mRange.max == rhs.mRange.max ) );
}

} // namespace CVSandbox
