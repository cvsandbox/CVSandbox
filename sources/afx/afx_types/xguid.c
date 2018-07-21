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

// Check if two guids are equal or not
bool XGuidAreEqual( const xguid* guid1, const xguid* guid2 )
{
    return ( ( guid1 != 0 ) && ( guid2 != 0 ) &&
             ( guid1->part1 == guid2->part1 ) &&
             ( guid1->part2 == guid2->part2 ) &&
             ( guid1->part3 == guid2->part3 ) &&
             ( guid1->part4 == guid2->part4 ) ) ? true : false;
}

// Copy source GUID into destination
void XGuidCopy( const xguid* src, xguid* dst )
{
    if ( ( src != 0 ) && ( dst != 0 ) )
    {
        dst->part1 = src->part1;
        dst->part2 = src->part2;
        dst->part3 = src->part3;
        dst->part4 = src->part4;
    }
}

// Get string representation of the specified GUID. Returns number of characters (including 0 terminator) put into the buffer.
uint32_t XGuidToString( const xguid* guid, char* buffer, uint32_t strLen )
{
    uint32_t outLen = 0;

    if ( ( buffer != 0 ) && ( strLen >= 36 ) )
    {
        if ( guid == 0 )
        {
            strcpy( buffer, "<null>" );
            outLen = 7;
        }
        else
        {
            outLen = sprintf( buffer, "%08X-%08X-%08X-%08X", guid->part1, guid->part2, guid->part3, guid->part4 ) + 1;
        }
    }

    return outLen;
}

// Convert given string into GUID
XErrorCode XStringToGuid( const char* buffer, xguid* guid )
{
    XErrorCode ret = ErrorInvalidFormat;

    if ( ( buffer != 0 ) && ( guid != 0 ) )
    {
        if ( strlen( buffer ) == 35 )
        {
            // TODO: improve hex integer parsing
            if ( sscanf( buffer, "%08X-%08X-%08X-%08X", &guid->part1, &guid->part2, &guid->part3, &guid->part4 ) == 4 )
            {
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
