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

#include "XThreadImpl.hpp"
#include "../XMutex.hpp"
#include "../XManualResetEvent.hpp"
#include <stdio.h>
#include <assert.h>

#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Thread management implementation using POSIX PThreads (thread + mutex + condition variable)

namespace CVSandbox { namespace Threading { namespace Private {

using namespace CVSandbox::Threading;

class XThreadImplData
{
public:
    static void* WorkerThread( void* threadid );

public:
    pthread_t       Thread;
    pthread_mutex_t Mutex;
    pthread_cond_t  Cond;

    XThreadFunction UserCallback;
    void*           UserParam;

    uint32_t        Counter;
    bool            IsRunning;
};

XThreadImpl::XThreadImpl( ) :
    mData( new XThreadImplData( ) )
{
    mData->IsRunning = false;
    mData->Counter   = 0;
    mData->Thread    = 0;

    int e1 = pthread_mutex_init( &mData->Mutex, 0 );
    int e2 = pthread_cond_init( &mData->Cond, 0 );

    assert( ( e1 | e2 ) == 0 );
}

XThreadImpl::~XThreadImpl( )
{
    if ( IsRunning( ) )
    {
        assert( false );
        Terminate( );
    }

    pthread_cond_destroy( &mData->Cond );
    pthread_mutex_destroy( &mData->Mutex );

    delete mData;
}

// Create and start running thread
bool XThreadImpl::Create( XThreadFunction callback, void* param )
{
    pthread_mutex_lock( &mData->Mutex );
    bool ret = false;

    if ( !mData->IsRunning )
    {
        mData->UserCallback = callback;
        mData->UserParam    = param;

        mData->IsRunning = ( pthread_create( &mData->Thread, 0, XThreadImplData::WorkerThread, mData ) == 0 );
        ret = mData->IsRunning;
    }

    pthread_mutex_unlock( &mData->Mutex );
    return ret;
}

// NOTE: pthread condition variable is used for both Join() and Join(int) versions.
// Although pthread_join() could be used for Join() version, there is not way to
// to use time-out for waiting till thread is done. So using condition variable instead
// will allow doing so.

// Join the thread - wait till it is done
void XThreadImpl::Join( )
{
    pthread_mutex_lock( &mData->Mutex );
    uint32_t lastCounterValue = mData->Counter;

    while ( ( mData->IsRunning ) && ( mData->Counter == lastCounterValue ) )
    {
         pthread_cond_wait( &mData->Cond, &mData->Mutex );
    }

    if ( !mData->IsRunning )
    {
        mData->Thread = 0;
    }

    pthread_mutex_unlock( &mData->Mutex );
}

// Wait for the specified amount of time till the thread is done
bool XThreadImpl::Join( uint32_t msec )
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

    pthread_cond_timedwait( &mData->Cond, &mData->Mutex, &waitTill );
    bool ret = ( ( !mData->IsRunning ) || ( mData->Counter != lastCounterValue ) );

    if ( !mData->IsRunning )
    {
        mData->Thread = 0;
    }

    pthread_mutex_unlock( &mData->Mutex );

    return ret;
}

// Check if the thread is still running
bool XThreadImpl::IsRunning( ) const
{
    pthread_mutex_lock( &mData->Mutex );
    bool ret = mData->IsRunning;

    if ( ( !ret ) && ( mData->Thread != 0 ) )
    {
        mData->Thread = 0;
    }

    pthread_mutex_unlock( &mData->Mutex );

    return ret;
}

// Terminate the thread (try to avoid using it ever - too dangerous)
void XThreadImpl::Terminate( )
{
    pthread_mutex_lock( &mData->Mutex );

    if ( mData->IsRunning )
    {
        pthread_cancel( mData->Thread );
        mData->Thread = 0;

        mData->IsRunning = false;
        mData->Counter++;
        pthread_cond_broadcast( &mData->Cond );
    }

    pthread_mutex_unlock( &mData->Mutex );
}

// Platform specific thread's worker function
void* XThreadImplData::WorkerThread( void* param )
{
    XThreadImplData* myData = (XThreadImplData*) param;
    myData->UserCallback( myData->UserParam );

    // update status and notify anyone who may wait
    pthread_mutex_lock( &myData->Mutex );

    myData->IsRunning = false;
    myData->Counter++;
    pthread_cond_broadcast( &myData->Cond );

    pthread_mutex_unlock( &myData->Mutex );
}

// Put current thread into sleep state for the specified amount of time
void XThreadImpl::Sleep( uint32_t msec )
{
    usleep( (useconds_t ) msec * 1000 );
}

// Give CPU to another waiting thread
void XThreadImpl::YieldCpu( )
{
    pthread_yield( );
}

// Get ID of the current thread
uint32_t XThreadImpl::ThreadId( )
{
    return static_cast<uint32_t>( pthread_self( ) );
}

} } } // namespace CVSandbox::Threading::Private
