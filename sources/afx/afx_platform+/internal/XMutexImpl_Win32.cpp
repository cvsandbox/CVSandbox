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

#include "XMutexImpl.hpp"

// Mutual exclusion synchronization object implementation using Win32 API
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace CVSandbox { namespace Threading { namespace Private {

class XMutexImplData
{
public:
    HANDLE Mutex;
};

XMutexImpl::XMutexImpl( ) :
    mData( new XMutexImplData( ) )
{
}

XMutexImpl::~XMutexImpl( )
{
    delete mData;
}

// Create system's synchronization object
bool XMutexImpl::Create( )
{
    mData->Mutex = CreateMutex( NULL, FALSE, NULL );
    return ( mData->Mutex != 0 );
}

// Destroy system's synchronization object
void XMutexImpl::Destroy( )
{
    if ( mData->Mutex != 0 )
    {
        CloseHandle( mData->Mutex );
    }
}

// Lock the mutex
void XMutexImpl::Lock( )
{
    if ( mData->Mutex != 0 )
    {
        WaitForSingleObject( mData->Mutex, INFINITE );
    }
}

// Try locking the mutex
bool XMutexImpl::TryLock( )
{
    bool ret = false;

    if ( mData->Mutex != 0 )
    {
        ret = ( WaitForSingleObject( mData->Mutex, 0 ) == WAIT_OBJECT_0 );
    }

    return ret;
}

// Unlock the mutex
void XMutexImpl::Unlock( )
{
    if ( mData->Mutex != 0 )
    {
        ReleaseMutex( mData->Mutex );
    }
}

} } } // namespace CVSandbox::Threading::Private
