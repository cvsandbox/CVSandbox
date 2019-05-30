/*
    MJPEG streams video source library of Computer Vision Sandbox

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

#include "base64.hpp"

namespace CVSandbox { namespace Video { namespace MJpeg { namespace Private
{

static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Get length of encoded source for the given length of a plain source
size_t Base64EncodeLength( size_t  sourceLength )
{
    return ( ( sourceLength + 2 ) / 3 * 4 ) + 1;
}

// Encodes the specifed source using Base64 encoding
size_t  Base64Encode( char* encoded, const char* source, size_t  sourceLength )
{
    char*   ptr = encoded;
    size_t  i;

    for ( i = 0; i < sourceLength - 2; i += 3 )
    {
        *ptr++ = basis_64[( source[i] >> 2 ) & 0x3F];
        *ptr++ = basis_64[( ( source[i] & 0x3 ) << 4 )     | ( (int) ( source[i + 1] & 0xF0 ) >> 4 )];
        *ptr++ = basis_64[( ( source[i + 1] & 0xF ) << 2 ) | ( (int) ( source[i + 2] & 0xC0 ) >> 6 )];
        *ptr++ = basis_64[source[i + 2] & 0x3F];
    }

    if ( i < sourceLength )
    {
        *ptr++ = basis_64[( source[i] >> 2 ) & 0x3F];
        if ( i == ( sourceLength - 1 ) )
        {
            *ptr++ = basis_64[( ( source[i] & 0x3 ) << 4 )];
            *ptr++ = '=';
        }
        else
        {
            *ptr++ = basis_64[( ( source[i] & 0x3 ) << 4 ) | ( (int) ( source[i + 1] & 0xF0 ) >> 4 )];
            *ptr++ = basis_64[( ( source[i + 1] & 0xF ) << 2 )];
        }
        *ptr++ = '=';
    }

    *ptr++ = '\0';
    return ptr - encoded;
}

} } } } // namespace CVSandbox::Video::MJpeg::Private
