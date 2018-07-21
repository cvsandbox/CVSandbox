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

#include <stdlib.h>
#include <string.h>
#include "xtypes.h"

// Allocate/copy x-string
xstring XStringAlloc( const char* src )
{
    xstring str = 0;
    int     len;

    if ( src != 0 )
    {
        len = (int) strlen( src ) + 1;
        str = (xstring) XMAlloc( len );

        if ( str != 0 )
        {
            memcpy( (void*) str, src, len );
        }
    }

    return str;
}

// Free memory taken by x-string
void XStringFree( xstring* str )
{
    XFree( (void**) str );
}
