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

#include <memory.h>
#include "xpalette.h"

// Allocates palette of the specified size
XErrorCode XPalleteAllocate( int32_t colorsCount, xpalette** pPalette )
{
    XErrorCode ret = SuccessCode;

    if ( pPalette == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( colorsCount <= 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        // check if a palette was allocated before and could be reused
        if ( ( *pPalette == 0 ) || ( (*pPalette)->colorsCount != colorsCount ) )
        {
            // free previously allocated palette if its size does not match
            XFree( (void**) pPalette );

            *pPalette = (xpalette*) XMAlloc( sizeof( xpalette ) );
            if ( *pPalette == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                ( *pPalette )->values = (xargb*) XCAlloc( colorsCount, sizeof( xargb ) );

                if ( ( *pPalette )->values == 0 )
                {
                    XFree( (void**) pPalette );
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    ( *pPalette )->colorsCount = colorsCount;
                }
            }
        }
    }

    return ret;
}

// Frees the specified palette
void XPaletteFree( xpalette** pPalette )
{
    if ( ( pPalette != 0 ) && ( *pPalette != 0 ) )
    {
        if ( (*pPalette)->values != 0 )
        {
            XFree( (void**) &(*pPalette)->values );
        }
        XFree( (void**) pPalette );
    }
}

// Copy palette's colors
XErrorCode XPalleteCopy( const xpalette* src, xpalette* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->colorsCount != dst->colorsCount ) || ( src->values == 0 ) || ( dst->values == 0 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        memcpy( dst->values, src->values, src->colorsCount * sizeof( xargb ) );
    }

    return ret;
}

// Copy palette's colors. If destination palette is not allocated or does not match, then it will be (re)allocated.
XErrorCode XPalleteClone( const xpalette* src, xpalette** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XPalleteAllocate( src->colorsCount, dst );

        if ( ret == SuccessCode )
        {
            ret = XPalleteCopy( src, *dst );

            if ( ret != SuccessCode )
            {
                XPaletteFree( dst );
            }
        }
    }

    return ret;
}
