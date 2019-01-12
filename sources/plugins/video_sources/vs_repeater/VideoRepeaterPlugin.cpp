/*
    Video repeater plug-ins of Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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

#include "VideoRepeaterPlugin.hpp"
#include "VideoRepeaterRegistry.hpp"
#include <memory.h>
#include <string>
#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <XVariant.hpp>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Threading;

static const char* ERROR_MESSAGE = "Failed re-translating image - out of memory";

namespace Private
{
    // Internal class which hides private parts of the VideoRepeaterPlugin class,
    // so those are not exposed in the main class
    class VideoRepeaterPluginData
    {
    public:
        VideoRepeaterPluginData( ) : RepeaterId( ), UserCallbacks( { 0 } ), UserParam( nullptr ),
            PushedImage( nullptr ), NotifiedImage( nullptr )
        {
        }

        ~VideoRepeaterPluginData( )
        {
            XImageFree( &PushedImage );
            XImageFree( &NotifiedImage );
        }

        // Video thread entry point
        static void WorkerThreadHandler( void* param );
        // Notify client about new video frame
        void NewFrameNotify( const ximage* image );
        // Notify client about error in the video source
        void ErrorMessageNotify( const char* errorMessage );
        // Run video loop in a background worker thread
        void VideoSourceWorker( );

    public:
        string                      RepeaterId;
        VideoSourcePluginCallbacks  UserCallbacks;
        void*                       UserParam;

        ximage*             PushedImage;
        ximage*             NotifiedImage;

        XMutex              Sync;
        XMutex              ImageSync;
        XManualResetEvent   ExitEvent;
        XManualResetEvent   NewImageEvent;
        XThread             BackgroundThread;
        uint32_t            FramesCounter;
    };
}

// ==========================================================================

VideoRepeaterPlugin::VideoRepeaterPlugin( ) :
    mData( new ::Private::VideoRepeaterPluginData( ) )
{
}

VideoRepeaterPlugin::~VideoRepeaterPlugin( )
{
    delete mData;
}

void VideoRepeaterPlugin::Dispose( )
{
    delete this;
}

// Start video source so it initializes and begins providing video frames
XErrorCode VideoRepeaterPlugin::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    mData->FramesCounter = 0;
    mData->ExitEvent.Reset( );

    if ( mData->BackgroundThread.Create( ::Private::VideoRepeaterPluginData::WorkerThreadHandler, mData ) )
    {
        VideoRepeaterRegistry::Instance( )->AddRepeater( mData->RepeaterId, this );
        ret = SuccessCode;
    }

    return ret;
}

// Signal video to stop, so it could finalize and clean-up
void VideoRepeaterPlugin::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
        mData->NewImageEvent.Signal( );
    }
}

// Wait till video source stops
void VideoRepeaterPlugin::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
        mData->NewImageEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its thread) is still running
bool VideoRepeaterPlugin::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void VideoRepeaterPlugin::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
        VideoRepeaterRegistry::Instance( )->RemoveRepeater( mData->RepeaterId );
    }
}

// Get number of frames received since the the start of the video source
uint32_t VideoRepeaterPlugin::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set callbacks for the video source
void VideoRepeaterPlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
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

// Get specified property value of the plug-in
XErrorCode VideoRepeaterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->RepeaterId.c_str( ) );
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode VideoRepeaterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;
    XVariant   xvar( *value );

    switch ( id )
    {
    case 0:
        if ( xvar.Type( ) == XVT_String )
        {
            mData->RepeaterId = xvar.ToString( &ret );
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Push image into the repeater causing it to re-translate as a video source
void VideoRepeaterPlugin::PushImage( const ximage* image )
{
    XScopedLock lock( &mData->ImageSync );

    if ( XImageClone( image, &mData->PushedImage ) != SuccessCode )
    {
        mData->ErrorMessageNotify( ERROR_MESSAGE );
    }
    else
    {
        mData->NewImageEvent.Signal( );
    }
}

namespace Private
{
    // Video thread entry point
    void VideoRepeaterPluginData::WorkerThreadHandler( void* param )
    {
        static_cast<VideoRepeaterPluginData*>( param )->VideoSourceWorker( );
    }

    // Notify client about new video frame
    void VideoRepeaterPluginData::NewFrameNotify( const ximage* image )
    {
        XScopedLock lock( &Sync );

        FramesCounter++;

        // provide image only if someone needs it
        if ( UserCallbacks.NewImageCallback != NULL )
        {
            UserCallbacks.NewImageCallback( UserParam, image );
        }
    }

    // Notify client about error in the video source
    void VideoRepeaterPluginData::ErrorMessageNotify( const char* errorMessage )
    {
        XScopedLock lock( &Sync );

        if ( UserCallbacks.ErrorMessageCallback != NULL )
        {
            UserCallbacks.ErrorMessageCallback( UserParam, errorMessage );
        }
    }

    // Run video loop in a background thread
    void VideoRepeaterPluginData::VideoSourceWorker( )
    {
        for ( ; ; )
        {
            NewImageEvent.Wait( );

            if ( ExitEvent.Wait( 0 ) )
            {
                break;
            }

            {
                XScopedLock lock( &ImageSync );

                if ( XImageClone( PushedImage, &NotifiedImage ) != SuccessCode )
                {
                    ErrorMessageNotify( ERROR_MESSAGE );
                }
            }

            NewImageEvent.Reset( );

            if ( NotifiedImage != nullptr )
            {
                NewFrameNotify( NotifiedImage );
            }
        }

        VideoRepeaterRegistry::Instance( )->RemoveRepeater( RepeaterId );
    }
}
