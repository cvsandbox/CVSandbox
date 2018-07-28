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
#include <stdio.h>
#include <assert.h>

// Thread management implementation using Win32 API
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace CVSandbox { namespace Threading { namespace Private {

using namespace CVSandbox::Threading;

class XThreadImplData
{
public:
    static DWORD WINAPI WorkerThread( LPVOID lpParam );

public:
    HANDLE          Thread;
    XThreadFunction UserCallback;
    void*           UserParam;

    bool            IsRunning;
    XMutex          Sync;
};

XThreadImpl::XThreadImpl( ) :
    mData( new XThreadImplData( ) )
{
    mData->IsRunning = false;
    mData->Thread    = 0;
}

XThreadImpl::~XThreadImpl( )
{
    if ( IsRunning( ) )
    {
        // ideally user should take care of thread before object's destructor
        assert( false );
        Terminate( );
    }

    delete mData;
}

// Create and start running thread
bool XThreadImpl::Create( XThreadFunction callback, void* param )
{
    XScopedLock lock( &mData->Sync );
    bool ret = false;

    if ( !mData->IsRunning )
    {
        mData->UserCallback = callback;
        mData->UserParam    = param;
        mData->Thread       = CreateThread( NULL, 0, XThreadImplData::WorkerThread, mData, 0, NULL );
        mData->IsRunning    = ( mData->Thread != 0 );

        ret = mData->IsRunning;
    }

    return ret;
}

// Join the thread - wait till it is done
void XThreadImpl::Join( )
{
    HANDLE threadToWait = 0;

    mData->Sync.Lock( );
    if ( IsRunning( ) )
    {
        threadToWait = mData->Thread;
    }
    mData->Sync.Unlock( );

    if ( threadToWait != 0 )
    {
        WaitForSingleObject( threadToWait, INFINITE );
        IsRunning( );
    }
}

// Wait for the specified amount of time till the thread is done
bool XThreadImpl::Join( uint32_t msec )
{
    HANDLE threadToWait = 0;
    bool ret = true;

    mData->Sync.Lock( );
    if ( IsRunning( ) )
    {
        threadToWait = mData->Thread;
    }
    mData->Sync.Unlock( );

    if ( threadToWait != 0 )
    {
        ret = ( WaitForSingleObject( threadToWait, msec ) != WAIT_TIMEOUT );
    }

    return ret;
}

// Check if the thread is still running
bool XThreadImpl::IsRunning( ) const
{
    XScopedLock lock( &mData->Sync );

    if ( ( !mData->IsRunning ) && ( mData->Thread != 0 ) )
    {
        CloseHandle( mData->Thread );
        mData->Thread = 0;
    }

    return mData->IsRunning;
}

// Terminate the thread (try to avoid using it ever - too dangerous)
void XThreadImpl::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        TerminateThread( mData->Thread, 0xFFFFFFFF );

        mData->Thread    = 0;
        mData->IsRunning = false;
    }
}

// Platform specific thread's worker function
DWORD WINAPI XThreadImplData::WorkerThread( LPVOID lpParam )
{
    XThreadImplData* myData = (XThreadImplData*) lpParam;
    myData->UserCallback( myData->UserParam );

    {
        XScopedLock lock( &myData->Sync );
        myData->IsRunning = false;
    }

    return 0;
}

// Put current thread into sleep state for the specified amount of time
void XThreadImpl::Sleep( uint32_t msec )
{
    ::Sleep( (DWORD) msec );
}

// Give CPU to another waiting thread
void XThreadImpl::YieldCpu( )
{
    SwitchToThread( );
}

// Get ID of the current thread
uint32_t XThreadImpl::ThreadId( )
{
    return static_cast<uint32_t>( GetCurrentThreadId( ) );
}

} } } // namespace CVSandbox::Threading::Private
