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

#include "xcpuid.h"

#ifdef _MSC_VER
    #include <intrin.h>
#elif __GNUC__
    #include <cpuid.h>
#endif

static bool cpuChecked = 0;

static bool isSSE      = 0;
static bool isSSE2     = 0;
static bool isSSE3     = 0;
static bool isSSSE3    = 0;
static bool isSSE4_1   = 0;
static bool isSSE4_2   = 0;
static bool isAVX      = 0;

// Check CPU features
static void CheckCPU( )
{
#ifdef _MSC_VER
    int CPUInfo[4];

    __cpuid( CPUInfo, 1 );

    isSSE    = ( ( CPUInfo[3] & ( 1 << 25 ) ) != 0 ) ? true : false;
    isSSE2   = ( ( CPUInfo[3] & ( 1 << 26 ) ) != 0 ) ? true : false;
    isSSE3   = ( ( CPUInfo[2] & ( 1 << 0  ) ) != 0 ) ? true : false;
    isSSSE3  = ( ( CPUInfo[2] & ( 1 << 9  ) ) != 0 ) ? true : false;
    isSSE4_1 = ( ( CPUInfo[2] & ( 1 << 19 ) ) != 0 ) ? true : false;
    isSSE4_2 = ( ( CPUInfo[2] & ( 1 << 20 ) ) != 0 ) ? true : false;
    isAVX    = ( ( CPUInfo[2] & ( 1 << 28 ) ) != 0 ) ? true : false;
#elif __GNUC__
    uint32_t ax, bx, cx, dx;

    __cpuid( 1, ax, bx, cx, dx );

    isSSE    = ( ( dx & bit_SSE    ) != 0 ) ? true : false;
    isSSE2   = ( ( dx & bit_SSE2   ) != 0 ) ? true : false;
    isSSE3   = ( ( cx & bit_SSE3   ) != 0 ) ? true : false;
    isSSSE3  = ( ( cx & bit_SSSE3  ) != 0 ) ? true : false;
    isSSE4_1 = ( ( cx & bit_SSE4_1 ) != 0 ) ? true : false;
    isSSE4_2 = ( ( cx & bit_SSE4_2 ) != 0 ) ? true : false;
    isAVX    = ( ( cx & bit_AVX    ) != 0 ) ? true : false;
#endif

    cpuChecked = true;
}

// Set of functions to check if CPU supports certain instruction sets
bool IsSSE( )
{
    if ( !cpuChecked )
    {
        CheckCPU( );
    }
    return isSSE;
}
bool IsSSE2( )
{
    if ( !cpuChecked )
    {
        CheckCPU( );
    }
    return isSSE2;
}
bool IsSSE3( )
{
    if ( !cpuChecked )
    {
        CheckCPU( );
    }
    return isSSE3;
}
bool IsSSSE3( )
{
    if ( !cpuChecked )
    {
        CheckCPU( );
    }
    return isSSSE3;
}
bool IsSSE4_1( )
{
    if ( !cpuChecked )
    {
        CheckCPU( );
    }
    return isSSE4_1;
}
bool IsSSE4_2( )
{
    if ( !cpuChecked )
    {
        CheckCPU( );
    }
    return isSSE4_2;
}
bool IsAVX( )
{
    if ( !cpuChecked )
    {
        CheckCPU( );
    }
    return isAVX;
}
