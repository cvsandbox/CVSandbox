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

#include <assert.h>
#include "XGuid.hpp"

using namespace std;

namespace CVSandbox
{

// Default constructor
XGuid::XGuid( ) :
    mGuid( )
{
}

// Initialize object from specified 4 32-bit values
XGuid::XGuid( uint32_t part1, uint32_t part2, uint32_t part3, uint32_t part4 ) :
    mGuid( )
{
    mGuid.part1 = part1;
    mGuid.part2 = part2;
    mGuid.part3 = part3;
    mGuid.part4 = part4;
}

// Copy constructor
XGuid::XGuid( const XGuid& rhs ) :
    mGuid( rhs.mGuid )
{

}

// Copy constructor initializing from C xguid
XGuid::XGuid( const xguid& rhs ) :
    mGuid( rhs )
{
}

// Construct guid from string
XGuid::XGuid( const std::string& str )
{
    if ( XStringToGuid( str.c_str( ), &mGuid ) != SuccessCode )
    {
        mGuid.part1 = mGuid.part2 = mGuid.part3 = mGuid.part4 = 0;
    }
}

// Assignment operator taking C++ XGuid
XGuid& XGuid::operator= ( const XGuid& rhs )
{
    XGuidCopy( &rhs.mGuid, &mGuid );
    return *this;
}

// Assignment operator taking C xguid
XGuid& XGuid::operator= ( const xguid& rhs )
{
    XGuidCopy( &rhs, &mGuid );
    return *this;
}

// Check if two guids are equal
bool XGuid::operator==( const XGuid& rhs ) const
{
    return XGuidAreEqual( &mGuid, &rhs.mGuid );
}

// Check if one guid is less than another (required for STL maps)
bool XGuid::operator<( const XGuid& rhs ) const
{
    return ( ( mGuid.part1  < rhs.mGuid.part1 ) ||
           ( ( mGuid.part1 == rhs.mGuid.part1 ) && ( mGuid.part2  < rhs.mGuid.part2 ) ) ||
           ( ( mGuid.part1 == rhs.mGuid.part1 ) && ( mGuid.part2 == rhs.mGuid.part2 ) && ( mGuid.part3  < rhs.mGuid.part3 ) ) ||
           ( ( mGuid.part1 == rhs.mGuid.part1 ) && ( mGuid.part2 == rhs.mGuid.part2 ) && ( mGuid.part3 == rhs.mGuid.part3 ) && ( mGuid.part4 < rhs.mGuid.part4 ) ) );
}

// Convert guid to string
const string XGuid::ToString( ) const
{
    string ret;
    char buffer[40] = "\0";

    if ( XGuidToString( &mGuid, buffer, 40 ) != 0 )
    {
        ret.assign( buffer );
    }
    else
    {
        assert( 0 );
    }

    return ret;
}

// Set values of the guid
void XGuid::Set( uint32_t part1, uint32_t part2, uint32_t part3, uint32_t part4 )
{
    mGuid.part1 = part1;
    mGuid.part2 = part2;
    mGuid.part3 = part3;
    mGuid.part4 = part4;
}

// Check if the guid is empty (all parts are zeros)
bool XGuid::IsEmpty( ) const
{
    return  ( ( mGuid.part1 == 0 ) && ( mGuid.part2 == 0 ) &&
              ( mGuid.part3 == 0 ) && ( mGuid.part4 == 0 ) );
}

} // namespace CVSandbox
