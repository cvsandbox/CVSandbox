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

// Mutual exclusion synchronization object implementation using POSIX PThreads
#include <pthread.h>

namespace CVSandbox { namespace Threading { namespace Private {

class XMutexImplData
{
public:
    pthread_mutex_t Mutex;
};

XMutexImpl::XMutexImpl( ) :
    mData( new XMutexImplData( ) )
{
}

XMutexImpl::~XMutexImpl( )
{
    delete mData;
}

// Create system mutex object
bool XMutexImpl::Create( )
{
    pthread_mutexattr_t mutexAttr;
    bool ret = ( ( pthread_mutexattr_init( &mutexAttr ) == 0 ) &&
                 ( pthread_mutexattr_settype( &mutexAttr, PTHREAD_MUTEX_RECURSIVE ) == 0 ) &&
                 ( pthread_mutex_init( &mData->Mutex, &mutexAttr ) == 0 ) );

    pthread_mutexattr_destroy( &mData->MutexAttr );

    return ret;
}

// Destroy system mutex object
void XMutexImpl::Destroy( )
{
    pthread_mutex_destroy( &mData->Mutex );
}

// Lock the mutex
void XMutexImpl::Lock( )
{
    int code = pthread_mutex_lock( &mData->Mutex );
}

// Try locking the mutex
bool XMutexImpl::TryLock( )
{
    return ( pthread_mutex_trylock( &mData->Mutex ) == 0 );
}

// Unlock the mutex
void XMutexImpl::Unlock( )
{
    pthread_mutex_unlock( &mData->Mutex );
}

} } } // namespace CVSandbox::Threading::Private
