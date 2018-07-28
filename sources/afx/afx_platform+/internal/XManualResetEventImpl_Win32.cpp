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

#include "XManualResetEventImpl.hpp"

// Manual reset synchronization object implementation using Win32 API
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace CVSandbox { namespace Threading { namespace Private {

class XManualResetEventImplData
{
public:
    HANDLE Event;
};

XManualResetEventImpl::XManualResetEventImpl( ) :
    mData( new XManualResetEventImplData( ) )
{
}

XManualResetEventImpl::~XManualResetEventImpl( )
{
    delete mData;
}

// Create system's synchronization object
bool XManualResetEventImpl::Create( )
{
    mData->Event = CreateEvent( NULL, TRUE, FALSE, NULL );
    return ( mData->Event != 0 );
}

// Destroy system's synchronization object
void XManualResetEventImpl::Destroy( )
{
    if ( mData->Event != 0 )
    {
        CloseHandle( mData->Event );
    }
}

// Set event to not signalled state
void XManualResetEventImpl::Reset( )
{
    if ( mData->Event != 0 )
    {
        ResetEvent( mData->Event );
    }
}

// Set event to signalled state
void XManualResetEventImpl::Signal( )
{
    if ( mData->Event != 0 )
    {
        SetEvent( mData->Event );
    }
}

// Wait till the event gets into signalled state
void XManualResetEventImpl::Wait( )
{
    if ( mData->Event != 0 )
    {
        WaitForSingleObject( mData->Event, INFINITE );
    }
}

// Wait the specified amount of time (milliseconds) till the event gets signalled
bool XManualResetEventImpl::Wait( uint32_t msec )
{
    bool ret = true;

    if ( mData->Event != 0 )
    {
        ret = ( WaitForSingleObject( mData->Event, msec ) == WAIT_OBJECT_0 );
    }

    return ret;
}

} } } // namespace CVSandbox::Threading::Private
