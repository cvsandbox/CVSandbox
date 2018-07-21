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

#include "xtypes.h"

// Get number of os bits set in the given integer
uint32_t XBitsCount( uint32_t bits )
{
    uint32_t ret = 0;

    while ( bits != 0 )
    {
        bits &= ( bits - 1 );
        ret++;
    }

    return ret;
}

// Get the lowest set bit in the given integer
uint32_t XBitsGetLowestSet( uint32_t bits )
{
    uint32_t ret = 0;
    int      i;

    for ( i = 0; i < 32; i++ )
    {
        if ( ( bits & ( (uint32_t) 1 << i ) ) != 0 )
        {
            ret = i + 1;
            break;
        }
    }

    return ret;
}

// Get the highest set bit in the given integer
uint32_t XBitsGetHighestSet( uint32_t bits )
{
    uint32_t ret = 0;
    int      i;

    for ( i = 31; i >= 0; i-- )
    {
        if ( ( bits & ( (uint32_t) 1 << i ) ) != 0 )
        {
            ret = i + 1;
            break;
        }
    }

    return ret;
}
