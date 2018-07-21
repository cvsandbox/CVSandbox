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
#include "XVersion.hpp"

using namespace std;

namespace CVSandbox
{

XVersion::XVersion( ) :
    mVersion( )
{
    mVersion.major    = 0;
    mVersion.minor    = 0;
    mVersion.revision = 0;
}

XVersion::XVersion( uint8_t major, uint8_t minor, uint16_t revision ) :
    mVersion( )
{
    mVersion.major    = major;
    mVersion.minor    = minor;
    mVersion.revision = revision;
}

XVersion::XVersion( const XVersion& rhs ) :
    mVersion( rhs.mVersion )
{
}

XVersion::XVersion( const xversion& rhs ) :
    mVersion( rhs )
{
}

XVersion& XVersion::operator=( const XVersion& rhs )
{
    mVersion = rhs.mVersion;
    return *this;
}

XVersion& XVersion::operator=( const xversion& rhs )
{
    mVersion = rhs;
    return *this;
}

// Check if two versions are equal
bool XVersion::operator==( const XVersion& rhs ) const
{
    return ( ( mVersion.major    == rhs.mVersion.major ) &&
             ( mVersion.minor    == rhs.mVersion.minor ) &&
             ( mVersion.revision == rhs.mVersion.revision ) );
}

// Convert version to string
const string XVersion::ToString( ) const
{
    string ret;
    char buffer[16] = "\0";

    if ( XVersionToString( &mVersion, buffer, 16 ) != 0 )
    {
        ret.assign( buffer );
    }
    else
    {
        assert( 0 );
    }

    return ret;
}

} // namespace CVSandbox
