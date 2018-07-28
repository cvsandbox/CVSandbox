/*
    Library to wrap some platform specific code of Computer Vision Sandbox

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

#include "XTimerImpl.hpp"
#include <time.h>

namespace CVSandbox { namespace Private
{

// Get tick count in milliseconds
uint64_t XTimerImpl::GetTickCount( )
{
    uint64_t        ticks = 0;
    struct timespec now;

    if ( clock_gettime( CLOCK_MONOTONIC, &now ) == 0 )
    {
        ticks  = static_cast<uint64_t>( now.tv_sec  ) * 1000;
        ticks += static_cast<uint64_t>( now.tv_nsec ) / 1000000;
    }

    return ticks;
}

} } // namespace CVSandbox
