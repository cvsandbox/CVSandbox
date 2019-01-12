/*
    FFMPEG video source plug-ins of Computer Vision Sandbox

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

#include "FileVideoSourcePlugin.hpp"
#include <memory.h>
#include <string>
#include <chrono>
#include <memory>
#include <XError.hpp>
#include <XImage.hpp>
#include <XFFmpegVideoFileReader.hpp>
#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>

using namespace std;
using namespace std::chrono;
using namespace CVSandbox;
using namespace CVSandbox::Threading;
using namespace CVSandbox::Video::FFmpeg;

namespace Private
{
    static const char* STR_ERROR_OUT_OF_MEMORY = "Out of memory";

    // Internal class which hides private parts of the FileVideoSourcePlugin class,
    // so those are not exposed in the main class
    class FileVideoSourcePluginData
    {
    public:
        FileVideoSourcePluginData( ) : UserCallbacks( { 0 } ), UserParam( nullptr ),
            VideoFile( ), FrameInterval( 40 ), OverrideFrameInterval( false )
        {
        }

        // Video thread entry point
        static void WorkerThreadHandler( void* param );
        // Notify client about new video frame
        void NewFrameNotify( const ximage* image );
        // Notify client about error in the video source
        void ErrorMessageNotify( const char* errorMessage );
        // Get rectangle of the specified window
        bool GetWindowRectangle( xrect* windowRect );
        // Run video loop in a background worker thread
        void VideoSourceWorker( );

    public:
        VideoSourcePluginCallbacks  UserCallbacks;
        void*                       UserParam;

        string              VideoFile;
        uint16_t            FrameInterval;
        bool                OverrideFrameInterval;

        XMutex              Sync;
        XManualResetEvent   ExitEvent;
        XThread             BackgroundThread;
        uint32_t            FramesCounter;
    };
}

// ==========================================================================

FileVideoSourcePlugin::FileVideoSourcePlugin( ) :
    mData( new ::Private::FileVideoSourcePluginData( ) )
{
}

FileVideoSourcePlugin::~FileVideoSourcePlugin( )
{
    delete mData;
}

void FileVideoSourcePlugin::Dispose( )
{
    delete this;
}

// Start video source so it initializes and begins providing video frames
XErrorCode FileVideoSourcePlugin::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    mData->FramesCounter = 0;
    mData->ExitEvent.Reset( );

    if ( mData->BackgroundThread.Create( ::Private::FileVideoSourcePluginData::WorkerThreadHandler, mData ) )
    {
        ret = SuccessCode;
    }

    return ret;
}

// Signal video to stop, so it could finalize and clean-up
void FileVideoSourcePlugin::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till video source stops
void FileVideoSourcePlugin::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its thread) is still running
bool FileVideoSourcePlugin::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
// by itself when signalled (ideally this method should not exist and be called at all)
void FileVideoSourcePlugin::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t FileVideoSourcePlugin::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set callbacks for the video source
void FileVideoSourcePlugin::SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam )
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
XErrorCode FileVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_String;
        value->value.strVal = XStringAlloc( mData->VideoFile.c_str( ) );
        break;

    case 1:
        value->type = XVT_Bool;
        value->value.boolVal = mData->OverrideFrameInterval;
        break;

    case 2:
        value->type = XVT_U2;
        value->value.usVal = mData->FrameInterval;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode FileVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = SuccessCode;
    XScopedLock lock( &mData->Sync );

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 3, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->VideoFile = convertedValue.value.strVal;
            break;

        case 1:
            mData->OverrideFrameInterval = convertedValue.value.boolVal;
            break;

        case 2:
            mData->FrameInterval = convertedValue.value.usVal;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}

namespace Private
{
    // Video thread entry point
    void FileVideoSourcePluginData::WorkerThreadHandler( void* param )
    {
        static_cast<FileVideoSourcePluginData*>( param )->VideoSourceWorker( );
    }

    // Notify client about new video frame
    void FileVideoSourcePluginData::NewFrameNotify( const ximage* image )
    {
        XScopedLock lock( &Sync );

        FramesCounter++;

        // provide image only if someone needs it
        if ( UserCallbacks.NewImageCallback != nullptr )
        {
            UserCallbacks.NewImageCallback( UserParam, image );
        }
    }

    // Notify client about error in the video source
    void FileVideoSourcePluginData::ErrorMessageNotify( const char* errorMessage )
    {
        XScopedLock lock( &Sync );

        if ( UserCallbacks.ErrorMessageCallback != nullptr )
        {
            UserCallbacks.ErrorMessageCallback( UserParam, errorMessage );
        }
    }

    // Run video loop in a background thread
    void FileVideoSourcePluginData::VideoSourceWorker( )
    {
        string   videoFileName;
        uint16_t frameInterval;
        bool     overrideFrameInterval;

        // get copies of the properties we need
        {
            XScopedLock lock( &Sync );

            videoFileName         = VideoFile;
            frameInterval         = FrameInterval;
            overrideFrameInterval = OverrideFrameInterval;
        }

        shared_ptr<XFFmpegVideoFileReader> videoFile = XFFmpegVideoFileReader::Create( );

        if ( !videoFile )
        {
            ErrorMessageNotify( STR_ERROR_OUT_OF_MEMORY );
        }
        else
        {
            XErrorCode ecode = videoFile->Open( videoFileName );

            if ( ecode != SuccessCode )
            {
                ErrorMessageNotify( XError::Description( ecode ).c_str( ) );
            }
            else
            {
                shared_ptr<XImage> videoFrame;
                float              fps               = videoFile->FrameRate( );
                uint32_t           timeBetweenFrames = static_cast<uint32_t>( 1000.0f / ( ( fps == 0 ) ? 30.f : fps ) );
                uint32_t           timeToSleep       = 0;
                uint32_t           timeTaken;

                if ( overrideFrameInterval )
                {
                    timeBetweenFrames = frameInterval;
                }

                do
                {
                    steady_clock::time_point captureStartTime = steady_clock::now( );

                    ecode = videoFile->GetNextFrame( videoFrame );

                    if ( ecode != SuccessCode )
                    {
                        ErrorMessageNotify( XError::Description( ecode ).c_str( ) );

                        if ( ecode == ErrorEOF )
                        {
                            break;
                        }
                    }
                    else
                    {
                        NewFrameNotify( videoFrame->ImageData( ) );
                    }

                    // decide how much to sleep
                    timeTaken   = static_cast<uint32_t>( duration_cast<std::chrono::milliseconds>( steady_clock::now( ) - captureStartTime ).count( ) );
                    timeToSleep = ( timeTaken > timeBetweenFrames ) ? 0 : timeBetweenFrames - timeTaken;
                }
                while ( !ExitEvent.Wait( timeToSleep ) );
            }
        }
    }
}
