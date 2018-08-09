/*
    MJPEG streams video source library of Computer Vision Sandbox

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

#include <time.h>
#include "XJpegHttpStream.hpp"

#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <XTimer.hpp>
#include <ximaging_formats.h>

#include <curl/curl.h>
#include "base64.hpp"
#include "tools.hpp"

using namespace std;
using namespace CVSandbox::Threading;
using namespace CVSandbox::Video;

namespace CVSandbox { namespace Video { namespace MJpeg
{

// Namespace with some private stuff to hide
namespace Private
{
    // Internal class which hides private parts of the XJpegHttpStream class,
    // so those are not exposed in the main class
    class XJpegHttpStreamData
    {
    public:
        XJpegHttpStreamData( const string& jpegUrl ) :
            JpegUrl( jpegUrl ), UserName( ), Password( ), UserAgent( ), ForceBasicAuthorization( false ), FrameIntervalMs( 100 ),
            Listener( 0 ),
            Sync( ), ExitEvent( ), BackgroundThread( ), FramesCounter( 0 ),
            TimeToSleepBeforeNextTry( 0 ), FailureDetected( false ),
            CommunicationBuffer( 0 ), ReadSoFar( 0 )
        {
        }

        // Run video loop in a background worker thread
        static void WorkerThreadHandler( void* param );

        // Notify about error in the video source
        void NotifyError( const string& errorMessage );

    public:
        string                JpegUrl;
        string                UserName;
        string                Password;
        string                UserAgent;
        bool                  ForceBasicAuthorization;
        uint16_t              FrameIntervalMs;

        IVideoSourceListener* Listener;

        XMutex                Sync;
        XManualResetEvent     ExitEvent;
        XThread               BackgroundThread;
        uint32_t              FramesCounter;
        uint32_t              TimeToSleepBeforeNextTry;
        bool                  FailureDetected;

        uint8_t*              CommunicationBuffer;
        uint32_t              ReadSoFar;
    };

    // Magic word of JPEG image
    static const uint8_t JpegMagic[3]  = { 0xFF, 0xD8, 0xFF };
    static const int     JpegMagicSize = sizeof( JpegMagic );

    // Maximum size of internal read buffer
    static const int MaxBufferSize = 1024 * 1024;

    // Pause length (in milliseconds) to do when error happened
    static const uint32_t PauseOnErrorMs = 1000;

    // Connection timeout value (in milliseconds)
    static const uint32_t ConnectionTimeoutMs = 10000;

    // Callback function called by libcurl when data arrives
    static size_t CurlWriteMemoryCallback( void* contents, size_t size, size_t nmemb, void* userp );
} // namespace Private

// ==========================================================================

// Class constructor
XJpegHttpStream::XJpegHttpStream( const string& jpegUrl ) :
    mData( new Private::XJpegHttpStreamData( jpegUrl ) )
{
}

// Class destructor
XJpegHttpStream::~XJpegHttpStream( )
{
    // stop it brutally if user did not care
    Terminate( );
    delete mData;
}

// Create class instance
const shared_ptr<XJpegHttpStream> XJpegHttpStream::Create( const string& jpegUrl )
{
    return shared_ptr<XJpegHttpStream>( new XJpegHttpStream( jpegUrl ) );
}

// Start video device (start its background thread which manages video acquisition)
XErrorCode XJpegHttpStream::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    if ( mData->JpegUrl.empty( ) )
    {
        ret = ErrorInvalidConfiguration;
    }
    else
    {
        mData->FramesCounter = 0;
        mData->ExitEvent.Reset( );

        if ( mData->BackgroundThread.Create( Private::XJpegHttpStreamData::WorkerThreadHandler, this ) )
        {
            ret = SuccessCode;
        }
    }

    return ret;
}

// Signal video source to stop and finalize its background thread
void XJpegHttpStream::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till video source stops
void XJpegHttpStream::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its background thread) is running or not
bool XJpegHttpStream::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source (call it ONLY as the last action of stopping video source, when nothing else helps)
void XJpegHttpStream::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t XJpegHttpStream::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set video source listener
void XJpegHttpStream::SetListener( IVideoSourceListener* listener )
{
    XScopedLock lock( &mData->Sync );
    mData->Listener = listener;
}

// Set new JPEG URL to download images from
bool XJpegHttpStream::SetJpegUrl( const string& jpegUrl )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->JpegUrl = jpegUrl;
        ret = true;
    }

    return ret;
}

// Set new authentication credentials (user name and password)
bool XJpegHttpStream::SetAuthenticationCredentials( const string& userName, const string& password )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->UserName = userName;
        mData->Password = password;
        ret = true;
    }

    return ret;
}

// Set user agent
bool XJpegHttpStream::SetUserAgent( const string& userAgent )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->UserAgent = userAgent;
        ret = true;
    }

    return ret;
}

// Set if basic authentcation must be forced
bool XJpegHttpStream::SetForceBasicAuthorization( bool setForceBasic )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->ForceBasicAuthorization = setForceBasic;
        ret = true;
    }

    return ret;
}

// Set interval between frames in milliseconds
bool XJpegHttpStream::SetFrameInterval( uint16_t frameIntervalMs )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->FrameIntervalMs = frameIntervalMs;
        ret = true;
    }

    return ret;
}

// Run video acquisition loop
void XJpegHttpStream::RunVideo( )
{
    ximage*     image          = 0;
    uint8_t*    buffer         = static_cast<uint8_t*>( malloc( Private::MaxBufferSize ) );
    char*       url            = static_cast<char*>( malloc( mData->JpegUrl.size( ) + 32 ) );
    char        paramSeparator = ( mData->JpegUrl.find( '?' ) == string::npos ) ? '?' : '&';

    if ( ( buffer != nullptr ) && ( url != nullptr ) )
    {
        // initialize libcurl session
        CURL*           curl        = curl_easy_init( );
        CURLM*          multiHandle = curl_multi_init( );
        CURLMcode       curlMcode   = CURLM_OK;
        int             stillRunning;
        bool            firstTry    = true;

        mData->CommunicationBuffer      = buffer;
        mData->TimeToSleepBeforeNextTry = 0;

        srand( static_cast<uint32_t>( time( 0 ) ) );

        if ( ( curl != nullptr ) && ( multiHandle != nullptr ) )
        {
            struct curl_slist* extraHeaderList = nullptr;

            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Private::CurlWriteMemoryCallback );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, mData );
            curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

            //curl_easy_setopt( curl, CURLOPT_VERBOSE, 1L );

            if ( !mData->UserAgent.empty( ) )
            {
                string header = "User-Agent: " + mData->UserAgent;
                extraHeaderList = curl_slist_append( extraHeaderList, header.c_str( ) );
            }

            // set user name/password if it was supplied
            if ( !mData->UserName.empty( ) )
            {
                if ( !mData->ForceBasicAuthorization )
                {
                    curl_easy_setopt( curl, CURLOPT_USERNAME, mData->UserName.c_str( ) );
                    curl_easy_setopt( curl, CURLOPT_PASSWORD, mData->Password.c_str( ) );
                    curl_easy_setopt( curl, CURLOPT_HTTPAUTH, (long) CURLAUTH_ANY );
                }
                else
                {
                    // use custom header for basic authorization instead of libcurl's API,
                    // because found a camera, which keeps rejecting libcurl's authorization

                    string  loginWithPassword = mData->UserName + ":" + mData->Password;
                    int     authLen           = Private::Base64EncodeLength( loginWithPassword.size( ) + 21 );
                    char*   authorization     = new char[authLen];

                    strcpy( authorization, "Authorization: Basic " );

                    Private::Base64Encode( &(authorization[21]), loginWithPassword.c_str( ), loginWithPassword.size( ) );

                    extraHeaderList = curl_slist_append( extraHeaderList, authorization );
                    delete [] authorization;
                }
            }

            if ( extraHeaderList != nullptr )
            {
                curl_easy_setopt( curl, CURLOPT_HTTPHEADER, extraHeaderList );
            }

            while ( !mData->ExitEvent.Wait( mData->TimeToSleepBeforeNextTry ) )
            {
                uint64_t startTickCount = XTimer::GetTickCount( );
                uint64_t lastTickCount  = XTimer::GetTickCount( );

                // set URL
                sprintf( url, "%s%c%d", mData->JpegUrl.c_str( ), paramSeparator, rand( ) );
                curl_easy_setopt( curl, CURLOPT_URL, url );

                // reset read buffer and (re)add handle
                mData->ReadSoFar       = 0;
                mData->FailureDetected = false;
                curl_multi_add_handle( multiHandle, curl );

                // perform the initial request
                curlMcode = curl_multi_perform( multiHandle, &stillRunning );
                if ( curlMcode != CURLM_OK )
                {
                    mData->NotifyError( string( "Request error: " ) + curl_multi_strerror( curlMcode ) );
                }
                else
                {
                    while ( ( !mData->ExitEvent.IsSignaled( ) ) && ( !mData->FailureDetected ) )
                    {
                        if ( stillRunning != 0 )
                        {
                            int numfds = 0;

                            curlMcode = curl_multi_wait( multiHandle, nullptr, 0, 2000, &numfds );

                            if ( curlMcode != CURLM_OK )
                            {
                                mData->NotifyError( string( "Request error: " ) + curl_multi_strerror( curlMcode ) );
                            }
                            {
                                if ( numfds != 0 )
                                {
                                    lastTickCount = XTimer::GetTickCount( );
                                }
                                else
                                {
                                    // check for timeout
                                    uint64_t nowTickCount = XTimer::GetTickCount( );
                                    uint32_t ticksDone    = static_cast<uint32_t>( nowTickCount - lastTickCount );

                                    if ( ticksDone > Private::ConnectionTimeoutMs )
                                    {
                                        mData->NotifyError( "Connection time out" );
                                    }
                                    // WORKAROUND start ----------------------------------
                                    // When starting several cameras simultaneously, sometimes some get
                                    // stuck on the very first try and then work fine after time-out happens.
                                    // Not sure what happens exactly, but as a workaround we use small time-out
                                    // for the very first try.
                                    else if ( ( ticksDone > 1000 ) && ( firstTry ) )
                                    {
                                        firstTry = false;
                                        // break to try the easy handle again
                                        break;
                                    }
                                    // WORKAROUND end ------------------------------------
                                }
                            }

                            curlMcode = curl_multi_perform( multiHandle, &stillRunning );
                            if ( curlMcode != CURLM_OK )
                            {
                                mData->NotifyError( string( "Request error: " ) + curl_multi_strerror( curlMcode ) );
                            }
                        }

                        if ( stillRunning == 0 )
                        {
                            // request has finished - check response code
                            long responseCode = 0;
                            curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &responseCode );

                            if ( responseCode != 200 )
                            {
                                string errorMessage;

                                switch ( responseCode )
                                {
                                case 0:
                                    errorMessage = "Connection failed";
                                    break;

                                case 401:
                                    errorMessage = "HTTP 401: Not authorized";
                                    break;

                                case 404:
                                    errorMessage = "HTTP 404: File not found";
                                    break;

                                default:
                                    {
                                        char msgBuffer[64];
                                        sprintf( msgBuffer, "HTTP response code: %d", static_cast<int32_t>( responseCode ) );
                                        errorMessage = msgBuffer;

                                        if ( mData->ReadSoFar != 0 )
                                        {
                                            buffer[mData->ReadSoFar] = 0;

                                            bool   isHtml    = false;
                                            string htmlTitle = Private::ExtractTitle( (char*) buffer, &isHtml );

                                            if ( !htmlTitle.empty( ) )
                                            {
                                                errorMessage += ", ";
                                                errorMessage += htmlTitle;
                                            }
                                            else if ( !isHtml )
                                            {
                                                char* newLinePos = strchr( (char*) buffer, '\n' );

                                                errorMessage += ", ";
                                                errorMessage += string( (char*) buffer, ( newLinePos == nullptr ) ? mData->ReadSoFar : ( (uint8_t*) newLinePos - buffer ) );
                                            }
                                        }
                                    }
                                    break;
                                }

                                mData->NotifyError( errorMessage );
                            }
                            else
                            {
                                if ( !mData->FailureDetected )
                                {
                                    int jpegStartIndex = Private::MemFind( buffer, mData->ReadSoFar, Private::JpegMagic, Private::JpegMagicSize );

                                    if ( jpegStartIndex >= 0 )
                                    {
                                        XScopedLock lock( &mData->Sync );

                                        mData->FramesCounter++;

                                        if ( mData->Listener != 0 )
                                        {
                                            // decode image only if someone needs it
                                            XErrorCode ret = XDecodeJpegFromMemory( &buffer[jpegStartIndex], mData->ReadSoFar - jpegStartIndex, &image );

                                            if ( ( ret == SuccessCode ) && ( !mData->ExitEvent.IsSignaled( ) ) )
                                            {
                                                shared_ptr<const XImage> guardedImage = XImage::Create( image );

                                                if ( guardedImage )
                                                {
                                                    mData->Listener->OnNewImage( guardedImage );
                                                }
                                            }
                                            else
                                            {
                                                mData->NotifyError( "Failed decoding JPEG image" );
                                            }
                                        }
                                    }
                                    else
                                    {
                                        mData->NotifyError( "The response does not contain JPEG image" );
                                    }
                                }
                            }
                            break;
                        }
                    }
                }

                curl_multi_remove_handle( multiHandle, curl );

                if ( !mData->FailureDetected )
                {
                    uint64_t endTickCount = XTimer::GetTickCount( );
                    uint32_t timeTakenMs  = static_cast<uint32_t>( endTickCount - startTickCount );

                    // on success get new frame according to the configured frame interval
                    mData->TimeToSleepBeforeNextTry = ( timeTakenMs > mData->FrameIntervalMs ) ? 0 : ( mData->FrameIntervalMs - timeTakenMs );
                }
            }

            if ( extraHeaderList != nullptr )
            {
                curl_slist_free_all( extraHeaderList );
            }
        }
        else
        {
            mData->NotifyError( "Fatal: Failed initializing libcurl session" );
        }

        // clean up
        if ( multiHandle != nullptr )
        {
            curl_multi_cleanup( multiHandle );
        }
        if ( curl != nullptr )
        {
            curl_easy_cleanup( curl );
        }
    }
    else
    {
        mData->NotifyError( "Fatal: Failed allocating communication buffer" );
    }

    if ( buffer != nullptr )
    {
        free( buffer );
    }
    if ( url != nullptr )
    {
        free( url );
    }

    XImageFree( &image );
}

namespace Private
{
    // Run video loop in a background thread
    void XJpegHttpStreamData::WorkerThreadHandler( void* param )
    {
        static_cast<XJpegHttpStream*>( param )->RunVideo( );
    }

    // Notify about error in the video source
    void XJpegHttpStreamData::NotifyError( const string& errorMessage )
    {
        XScopedLock lock( &Sync );

        if ( !FailureDetected )
        {
            FailureDetected = true;

            // make a pause before next try if error happened
            TimeToSleepBeforeNextTry = PauseOnErrorMs;

            if ( Listener != 0 )
            {
                Listener->OnError( errorMessage );
            }
        }
    }

    // Callback function called by libcurl when data arrives
    size_t CurlWriteMemoryCallback( void* contents, size_t size, size_t nmemb, void* userp )
    {
        size_t                  realSize = size * nmemb;
        XJpegHttpStreamData*    data     = static_cast<XJpegHttpStreamData*>( userp );

        if ( realSize <= MaxBufferSize - data->ReadSoFar )
        {
            memcpy( &(data->CommunicationBuffer[data->ReadSoFar]), contents, realSize );
            data->ReadSoFar += realSize;
        }
        else
        {
            data->NotifyError( "Too small communication buffer" );
        }

        return realSize;
    }

} // namespace Private

} } } // namespace CVSandbox::Video::MJpeg
