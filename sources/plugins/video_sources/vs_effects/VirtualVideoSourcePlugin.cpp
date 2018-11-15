/*
    Virtual video source plug-ins of Computer Vision Sandbox

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

#include "VirtualVideoSourcePlugin.hpp"
#include <string.h>
#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Threading;

namespace Private
{
    // Internal class which hides private parts of the VirtualVideoSourcePlugin class,
    // so those are not exposed in the main class
    class VirtualVideoSourcePluginData
    {
    public:
        VirtualVideoSourcePluginData( ) : UserCallbacks( { 0 } ), UserParam( NULL )
        {
        }

        // Run video loop in a background worker thread
        static void WorkerThreadHandler( void* param );

    public:
        VideoSourcePluginCallbacks  UserCallbacks;
        void*                       UserParam;

        XMutex              Sync;
        XManualResetEvent   ExitEvent;
        XThread             BackgroundThread;
        uint32_t            FramesCounter;
    };
}

// ==========================================================================

VirtualVideoSourcePlugin::VirtualVideoSourcePlugin( ) :
    mData( new ::Private::VirtualVideoSourcePluginData( ) )
{
}

VirtualVideoSourcePlugin::~VirtualVideoSourcePlugin( )
{
    delete mData;
}

void VirtualVideoSourcePlugin::Dispose( )
{
    delete this;
}

// Start video source so it initializes and begins providing video frames
XErrorCode VirtualVideoSourcePlugin::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    mData->FramesCounter = 0;
    mData->ExitEvent.Reset( );

    if ( mData->BackgroundThread.Create( ::Private::VirtualVideoSourcePluginData::WorkerThreadHandler, this ) )
    {
        ret = SuccessCode;
    }

    return ret;
}

// Signal video to stop, so it could finalize and clean-up
void VirtualVideoSourcePlugin::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till video source (its thread) stops
void VirtualVideoSourcePlugin::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its thread) is still running
bool VirtualVideoSourcePlugin::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void VirtualVideoSourcePlugin::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t VirtualVideoSourcePlugin::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set callbacks for the video source
void VirtualVideoSourcePlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
{
    XScopedLock lock( &mData->Sync );

    if ( callbacks != 0 )
    {
        memcpy( &mData->UserCallbacks, callbacks, sizeof( mData->UserCallbacks ) );
        mData->UserParam = userParam;
    }
    else
    {
        memset( &mData->UserCallbacks, 0, sizeof( mData->UserCallbacks ) );
        mData->UserParam = 0;
    }
}

// Wait till (or check if) was signalled to stop
bool VirtualVideoSourcePlugin::WaitTillNeedToStop( uint32_t msec )
{
    return mData->ExitEvent.Wait( msec );
}

// Notify client about new video frame
void VirtualVideoSourcePlugin::NewFrameNotify( const ximage* image )
{
    XScopedLock lock( &mData->Sync );

    mData->FramesCounter++;

    // provide image only if someone needs it
    if ( mData->UserCallbacks.NewImageCallback != NULL )
    {
        mData->UserCallbacks.NewImageCallback( mData->UserParam, image );
    }
}

// Notify client about error in the video source
void VirtualVideoSourcePlugin::ErrorMessageNotify( const char* errorMessage )
{
    XScopedLock lock( &mData->Sync );

    if ( mData->UserCallbacks.ErrorMessageCallback != NULL )
    {
        mData->UserCallbacks.ErrorMessageCallback( mData->UserParam, errorMessage );
    }
}

namespace Private
{
    // Run video loop in a background thread
    void VirtualVideoSourcePluginData::WorkerThreadHandler( void* param )
    {
        static_cast<VirtualVideoSourcePlugin*>( param )->VideoSourceWorker( );
    }
}
