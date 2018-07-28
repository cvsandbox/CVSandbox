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

#include "XMutex.hpp"
#include "internal/XMutexImpl.hpp"
#include <assert.h>

namespace CVSandbox { namespace Threading {

XMutex::XMutex( ) : pimpl( new Private::XMutexImpl( ) ), isValid( false )
{
    isValid = pimpl->Create( );
    assert( isValid );
}

XMutex::~XMutex( )
{
    pimpl->Destroy( );
    delete pimpl;
}

// Lock the mutex
void XMutex::Lock( )
{
    pimpl->Lock( );
}

// Try locking the mutex
bool XMutex::TryLock( )
{
    return pimpl->TryLock( );
}

// Unlock the mutex
void XMutex::Unlock( )
{
    pimpl->Unlock( );
}


// Lock the specified mutex
XScopedLock::XScopedLock( XMutex* mutex ) : mMutex( mutex )
{
    assert( mMutex != 0 );

    if ( mMutex != 0 )
    {
        mMutex->Lock( );
    }
}

// Unlock the owned mutex
XScopedLock::~XScopedLock( )
{
    if ( mMutex != 0 )
    {
        mMutex->Unlock( );
    }
}

} } // namespace CVSandbox::Threading
