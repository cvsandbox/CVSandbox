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
#ifndef CVS_XVERSION_HPP
#define CVS_XVERSION_HPP

#include <xtypes.h>
#include <string>

namespace CVSandbox
{

// Class encapsulating version values, like major/minor/revision
class XVersion
{
public:
    // Constructors
    XVersion( );
    XVersion( uint8_t major, uint8_t minor, uint16_t revision = 0 );
    XVersion( const XVersion& rhs );
    XVersion( const xversion& rhs );

    // Assignment operators
    XVersion& operator= ( const XVersion& rhs );
    XVersion& operator= ( const xversion& rhs );

    // Cast back to C version
    operator xversion( ) const { return mVersion; }

    // Check if two versions are equal
    bool operator==( const XVersion& rhs ) const;

    // Check if two versions are NOT equal
    bool operator!=( const XVersion& rhs ) const
    {
        return ( !( ( *this ) == rhs ) );
    }

    // Convert version to string
    const std::string ToString( ) const;

    // Getters
    uint8_t  Major( ) const { return mVersion.major; }
    uint8_t  Minor( ) const { return mVersion.minor; }
    uint16_t Revision( ) const { return mVersion.revision; }

    // Setters
    void SetMajor( uint8_t major ) { mVersion.major = major; }
    void SetMinor( uint8_t minor ) { mVersion.minor = minor; }
    void SetRevision( uint16_t revision ) { mVersion.revision = revision; }
    void Set( uint8_t major, uint8_t minor, uint16_t revision = 0 )
        { mVersion.major = major; mVersion.minor = minor; mVersion.revision = revision; }

private:
    xversion mVersion;
};

} // namespace CVSandbox

#endif // CVS_XVERSION_HPP
