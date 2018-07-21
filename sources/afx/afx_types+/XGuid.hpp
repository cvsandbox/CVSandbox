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
#ifndef CVS_XGUID_HPP
#define CVS_XGUID_HPP

#include <xtypes.h>
#include <string>

namespace CVSandbox
{

class XGuid
{
public:
    // Constructors
    XGuid( );
    XGuid( uint32_t part1, uint32_t part2, uint32_t part3, uint32_t part4 );
    XGuid( const XGuid& rhs );
    XGuid( const xguid& rhs );
    explicit XGuid( const std::string& str );

    // Assignment operators
    XGuid& operator= ( const XGuid& rhs );
    XGuid& operator= ( const xguid& rhs );

    // Cast back to C xguid
    operator xguid() const { return mGuid; }

    // Check if two guids are equal
    bool operator==( const XGuid& rhs ) const;

    // Check if two guids are NOT equal
    bool operator!=( const XGuid& rhs  ) const
    {
        return ( !( (*this) == rhs ) ) ;
    }

    // Check if one guid is less than another (required for STL maps)
    bool operator<( const XGuid& rhs ) const;

    // Check if the guid is empty (all parts are zeros)
    bool IsEmpty( ) const;

    // Convert guid to string
    const std::string ToString( ) const;

    // Convert string to guid
    static const XGuid FromString( std::string str ) { return XGuid( str ); }

    // Set values of the guid
    void Set( uint32_t part1, uint32_t part2, uint32_t part3, uint32_t part4 );

    // Access parts of the guid
    uint32_t Part1( ) const { return mGuid.part1; }
    uint32_t Part2( ) const { return mGuid.part2; }
    uint32_t Part3( ) const { return mGuid.part3; }
    uint32_t Part4( ) const { return mGuid.part4; }

private:
    xguid mGuid;
};

} // namespace CVSandbox

#endif // CVS_XGUID_HPP
