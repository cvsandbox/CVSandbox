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
#include <pthread.h>
#include <time.h>

// Manual reset synchronization object implementation using POSIX PThreads (mutex + condition variable)

namespace CVSandbox { namespace Threading { namespace Private {

class XManualResetEventImplData
{
public:
    uint32_t        Counter;
    bool            Triggered;
    pthread_mutex_t Mutex;
    pthread_cond_t  Cond;
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
    mData->Counter   = 0;
    mData->Triggered = false;
    return ( ( pthread_mutex_init( &mData->Mutex, 0 ) == 0 ) &&
             ( pthread_cond_init( &mData->Cond, 0 ) == 0 ) );
}

// Destroy system's synchronization object
void XManualResetEventImpl::Destroy( )
{
    pthread_cond_destroy( &mData->Cond );
    pthread_mutex_destroy( &mData->Mutex );
}

// Set event to not signalled state
void XManualResetEventImpl::Reset( )
{
    pthread_mutex_lock( &mData->Mutex );
    mData->Triggered = false;
    pthread_mutex_unlock( &mData->Mutex );
}

// Set event to signalled state
void XManualResetEventImpl::Signal( )
{
    pthread_mutex_lock( &mData->Mutex );

    mData->Triggered = true;
    mData->Counter++;
    pthread_cond_broadcast( &mData->Cond );

    pthread_mutex_unlock( &mData->Mutex );
}

// Wait till the event gets into signalled state
void XManualResetEventImpl::Wait( )
{
    pthread_mutex_lock( &mData->Mutex );
    uint32_t lastCounterValue = mData->Counter;

    while ( ( !mData->Triggered ) && ( mData->Counter == lastCounterValue ) )
    {
         pthread_cond_wait( &mData->Cond, &mData->Mutex );
    }

    pthread_mutex_unlock( &mData->Mutex );
}

// Wait the specified amount of time (milliseconds) till the event gets signalled
bool XManualResetEventImpl::Wait( uint32_t msec )
{
    // get current time
    struct timespec waitTill;
    clock_gettime( CLOCK_REALTIME , &waitTill );
    // add time to sleep
    long totalNs = waitTill.tv_nsec + msec * 1000 * 1000;
    waitTill.tv_nsec = ( totalNs % 1000000000 );
    waitTill.tv_sec += ( totalNs / 1000000000 );

    // now wait for signal
    pthread_mutex_lock( &mData->Mutex );
    uint32_t lastCounterValue = mData->Counter;

    if ( !mData->Triggered )
    {
        pthread_cond_timedwait( &mData->Cond, &mData->Mutex, &waitTill );
    }

    bool ret = ( ( mData->Triggered ) || ( mData->Counter != lastCounterValue ) );

    pthread_mutex_unlock( &mData->Mutex );

    return ret;
}

} } } // namespace CVSandbox::Threading::Private
