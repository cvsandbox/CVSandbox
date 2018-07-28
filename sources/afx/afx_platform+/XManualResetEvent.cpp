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

#include "XManualResetEvent.hpp"
#include "internal/XManualResetEventImpl.hpp"
#include <assert.h>

namespace CVSandbox { namespace Threading {

XManualResetEvent::XManualResetEvent( ) : pimpl( new Private::XManualResetEventImpl( ) ), isValid( false )
{
    isValid = pimpl->Create( );
    assert( isValid );
}

XManualResetEvent::~XManualResetEvent( )
{
    pimpl->Destroy( );
    delete pimpl;
}

// Set event to not signalled state
void XManualResetEvent::Reset( )
{
    pimpl->Reset( );
}

// Set event to signalled state
void XManualResetEvent::Signal( )
{
    pimpl->Signal( );
}

// Wait till the event gets into signalled state
void XManualResetEvent::Wait( )
{
    pimpl->Wait( );
}

// Wait the specified amount of time (milliseconds) till the event gets signalled
bool XManualResetEvent::Wait( uint32_t msec )
{
    return pimpl->Wait( msec );
}

// Check current state of the event
bool XManualResetEvent::IsSignaled( )
{
    return pimpl->Wait( 0 );
}

} } // namespace CVSandbox::Threading
