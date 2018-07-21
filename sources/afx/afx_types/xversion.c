/*
    Core types library of Computer Vision Sandbox

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

#include <stdio.h>
#include <string.h>
#include "xtypes.h"

// Check if two version are equal or not
bool XVersionAreEqual( const xversion* version1, const xversion* version2 )
{
    return ( ( version1 != 0 ) && ( version2 != 0 ) &&
             ( version1->major == version2->major ) &&
             ( version1->minor == version2->minor ) &&
             ( version1->revision == version2->revision ) ) ?
        true : false;
}

// Copy source version into destination
void XVersionCopy( const xversion* src, xversion* dst )
{
    if ( ( src != 0 ) && ( dst != 0 ) )
    {
        dst->major    = src->major;
        dst->minor    = src->minor;
        dst->revision = src->revision;
    }
}

// Get string representation of the specified version. Returns number of characters (including 0 terminator) put into the buffer.
uint32_t XVersionToString( const xversion* version, char* buffer, uint32_t bufferLen )
{
    uint32_t outLen = 0;
    char     tempBuf[16];

    outLen = sprintf( tempBuf, "%u.%u.%u", version->major, version->minor, version->revision );

    if ( bufferLen < outLen )
    {
        outLen = 0;
    }
    else
    {
        strncpy( buffer, tempBuf, bufferLen );
    }

    return outLen;
}

// Convert given string into version
XErrorCode XStringToVersion( const char* buffer, xversion* version )
{
    XErrorCode ret = ErrorInvalidFormat;

    if ( ( buffer != 0 ) && ( version != 0 ) )
    {
        if ( strlen( buffer ) <= 13 )
        {
            uint32_t major, minor, revision;

            if ( sscanf( buffer, "%u.%u.%u", &major, &minor, &revision ) == 3 )
            {
                version->major    = (uint8_t) major;
                version->minor    = (uint8_t) minor;
                version->revision = (uint16_t) revision;
                ret = SuccessCode;
            }
        }
    }
    else
    {
        ret = ErrorNullParameter;
    }

    return ret;
}
