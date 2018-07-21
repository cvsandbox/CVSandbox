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
#include "xtypes.h"

#define SAFE_MEMORY_ACROSS_MODULES

typedef void (*freeHandler)( void* );

// Memory de-allocation implementation
static void freeImpl( void* memblock )
{
    free( memblock );
}

// Allocate memory block of required size - malloc() replacement
void* XMAlloc( size_t size )
{
    #ifdef SAFE_MEMORY_ACROSS_MODULES
        // allocate some extra memory and store pointer to the memory de-allocation function
        uint8_t* memblock = (uint8_t*) malloc( size + sizeof( freeHandler ) );
        if ( memblock != 0 )
        {
            *( (freeHandler*) memblock ) = freeImpl;
            memblock += sizeof( freeHandler );
        }
        return (void*) memblock;
    #else
        return malloc( size );
    #endif
}

// Allocate memory block of required size and zero initialize it - calloc() replacement
void* XCAlloc( size_t count, size_t size )
{
    #ifdef SAFE_MEMORY_ACROSS_MODULES
        void* memblock = XMAlloc( count * size );

        if ( memblock != 0 )
        {
            memset( memblock, 0, count * size );
        }

        return memblock;
    #else
        return calloc( count, size );
    #endif
}

// Free specified block - free() replacement
void XFree( void** memblock )
{
    if ( ( memblock != 0 ) && ( *memblock != 0 ) )
    {
        #ifdef SAFE_MEMORY_ACROSS_MODULES
            uint8_t* nativeMemblock = (uint8_t*) *memblock - sizeof( freeHandler );
            freeHandler freeImplAddress = *( (freeHandler*) nativeMemblock );
            freeImplAddress( nativeMemblock );
        #else
            free( *memblock );
        #endif

        *memblock = 0;
    }
}
