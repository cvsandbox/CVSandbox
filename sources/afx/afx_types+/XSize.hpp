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
#ifndef CVS_XSIZE_HPP
#define CVS_XSIZE_HPP

#include <xtypes.h>

namespace CVSandbox
{

// Integer size class encapsulating Width/Height values
class XSize
{
public:
    // Constructors
    XSize( int32_t width = 0, int32_t height = 0 );
    XSize( const XSize& rhs );
    XSize( const xsize& rhs );

    // Assignment operators
    XSize& operator= ( const XSize& rhs );
    XSize& operator= ( const xsize& rhs );

    // Cast back to C point
    operator xsize( ) const { return mSize; }

    // Check if two sizes are equal
    bool operator==( const XSize& rhs ) const;

    // Check if two sizes are NOT equal
    bool operator!=( const XSize& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    int32_t Width( )  const { return mSize.width; }
    int32_t Height( ) const { return mSize.height; }

    void SetWidth( int32_t width )   { mSize.width  = width; }
    void SetHeight( int32_t height ) { mSize.height = height; }
    void Set( int32_t width, int32_t height ) { mSize.width = width; mSize.height = height; }

private:
    xsize mSize;
};

} // namespace CVSandbox

#endif // CVS_XSIZE_HPP
