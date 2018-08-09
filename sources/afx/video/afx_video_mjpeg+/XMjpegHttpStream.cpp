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

#include "XMjpegHttpStream.hpp"

#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <XTimer.hpp>
#include <ximaging_formats.h>

#include <curl/curl.h>
#include "base64.hpp"
#include "tools.hpp"

//#define DEBUG_MJPEG_STREAM

using namespace std;
using namespace CVSandbox::Threading;
using namespace CVSandbox::Video;

namespace CVSandbox { namespace Video { namespace MJpeg
{

// Namespace with some private stuff to hide
namespace Private
{
    // Internal class which hides private parts of the XMjpegHttpStream class,
    // so those are not exposed in the main class
    class XMjpegHttpStreamData
    {
    public:
        XMjpegHttpStreamData( const string& mjpegUrl ) :
            MjpegUrl( mjpegUrl ), UserName( ), Password( ), UserAgent( ), ForceBasicAuthorization( false ),
            Listener( 0 ),
            Sync( ), ExitEvent( ), BackgroundThread( ), FramesCounter( 0 ),
            TimeToSleepBeforeNextTry( 0 ), CommunicationBuffer( 0 ), DecodedImage( 0 ),
            ReadSoFar( 0 ), FailureDetected( false ), IsContentTypeChecked( false ), IsBoundaryChecked( false ),
            JpegBoundaryLength( 0 ), JpegBoundary( 0 ), SearchStartIndex( 0 ), JpegImageStart( -1 )
        {
        }

        ~XMjpegHttpStreamData( )
        {
            ResetConnectionState( );
        }

        // Reset connection releated variable
        void ResetConnectionState( )
        {
            ReadSoFar            = 0;
            FailureDetected      = false;
            IsContentTypeChecked = false;
            IsBoundaryChecked    = false;
            JpegBoundaryLength   = 0;
            SearchStartIndex     = 0;
            JpegImageStart       = -1;

            delete [] JpegBoundary;
            JpegBoundary         = nullptr;
        }

        // Run video loop in a background worker thread
        static void WorkerThreadHandler( void* param );

        // Notify about error in the video source
        void NotifyError( const string& errorMessage );

    public:
        string                MjpegUrl;
        string                UserName;
        string                Password;
        string                UserAgent;
        bool                  ForceBasicAuthorization;

        IVideoSourceListener* Listener;

        XMutex                Sync;
        XManualResetEvent     ExitEvent;
        XThread               BackgroundThread;
        uint32_t              FramesCounter;
        uint32_t              TimeToSleepBeforeNextTry;

        uint8_t*              CommunicationBuffer;
        ximage*               DecodedImage;

        uint32_t              ReadSoFar;
        bool                  FailureDetected;
        bool                  IsContentTypeChecked;
        bool                  IsBoundaryChecked;
        uint32_t              JpegBoundaryLength;
        uint8_t*              JpegBoundary;
        int                   SearchStartIndex;
        int                   JpegImageStart;

#ifdef DEBUG_MJPEG_STREAM
        FILE*                 DebugFile;
#endif
    };

    static const char* STR_MEMORY_ALLOCATION_ERROR = "Failed allocating memory";

    // Magic word of JPEG image
    static const uint8_t JpegMagic[3]  = { 0xFF, 0xD8, 0xFF };
    static const int     JpegMagicSize = sizeof( JpegMagic );

    // Maximum size of internal read buffer
    static const int MaxBufferSize = 1024 * 1024;

    // Pause length (in milliseconds) to do when error happened
    static const uint32_t PauseOnErrorMs = 1000;

    // Connection timeout value (in milliseconds)
    static const uint32_t ConnectionTimeoutMs = 10000;

    static size_t CurlReceiveHeadersCallback( void* buffer, size_t size, size_t nmemb, void* userp );
    // Callback function called by libcurl when data arrives
    static size_t CurlWriteMemoryCallback( void* contents, size_t size, size_t nmemb, void* userp );
} // namespace Private

// ==========================================================================

// Class constructor
XMjpegHttpStream::XMjpegHttpStream( const string& mjpegUrl ) :
    mData( new Private::XMjpegHttpStreamData( mjpegUrl ) )
{
}

// Class destructor
XMjpegHttpStream::~XMjpegHttpStream( )
{
    // stop it brutally if user did not care
    Terminate( );
    delete mData;
}

// Create class instance
const shared_ptr<XMjpegHttpStream> XMjpegHttpStream::Create( const string& mjpegUrl )
{
    return shared_ptr<XMjpegHttpStream>( new XMjpegHttpStream( mjpegUrl ) );
}

// Start video device (start its background thread which manages video acquisition)
XErrorCode XMjpegHttpStream::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    if ( mData->MjpegUrl.empty( ) )
    {
        ret = ErrorInvalidConfiguration;
    }
    else
    {
        mData->FramesCounter = 0;
        mData->ExitEvent.Reset( );

        if ( mData->BackgroundThread.Create( Private::XMjpegHttpStreamData::WorkerThreadHandler, this ) )
        {
            ret = SuccessCode;
        }
    }

    return ret;
}

// Signal video source to stop and finalize its background thread
void XMjpegHttpStream::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till video source stops
void XMjpegHttpStream::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its background thread) is running or not
bool XMjpegHttpStream::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source (call it ONLY as the last action of stopping video source, when nothing else helps)
void XMjpegHttpStream::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t XMjpegHttpStream::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set video source listener
void XMjpegHttpStream::SetListener( IVideoSourceListener* listener )
{
    XScopedLock lock( &mData->Sync );
    mData->Listener = listener;
}

// Set new MJPEG URL to download images from
bool XMjpegHttpStream::SetMjpegUrl( const string& mjpegUrl )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->MjpegUrl = mjpegUrl;
        ret = true;
    }

    return ret;
}

// Set new authentication credentials (user name and password)
bool XMjpegHttpStream::SetAuthenticationCredentials( const string& userName, const string& password )
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
bool XMjpegHttpStream::SetUserAgent( const string& userAgent )
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
bool XMjpegHttpStream::SetForceBasicAuthorization( bool setForceBasic )
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

// Run video acquisition loop
void XMjpegHttpStream::RunVideo( )
{
    uint8_t*    buffer = (uint8_t*) malloc( Private::MaxBufferSize );

    if ( buffer != 0 )
    {
        // initialize libcurl session
        CURL*           curl        = curl_easy_init( );
        CURLM*          multiHandle = curl_multi_init( );
        CURLMcode       curlMcode   = CURLM_OK;
        int             stillRunning;
        bool            firstTry    = true;

        mData->CommunicationBuffer      = buffer;
        mData->TimeToSleepBeforeNextTry = 0;

#ifdef DEBUG_MJPEG_STREAM
        mData->DebugFile = fopen( "mjpeg.bin", "wb" );
#endif

        if ( ( curl != nullptr ) && ( multiHandle != nullptr ) )
        {
            struct curl_slist* extraHeaderList = nullptr;

            curl_easy_setopt( curl, CURLOPT_URL, mData->MjpegUrl.c_str( ) );
            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, Private::CurlWriteMemoryCallback );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, mData );

            curl_easy_setopt( curl, CURLOPT_HEADERFUNCTION, Private::CurlReceiveHeadersCallback );
            curl_easy_setopt( curl, CURLOPT_WRITEHEADER, mData );

            curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

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

            if ( extraHeaderList != 0 )
            {
                curl_easy_setopt( curl, CURLOPT_HTTPHEADER, extraHeaderList );
            }

            while ( !mData->ExitEvent.Wait( mData->TimeToSleepBeforeNextTry ) )
            {
                uint64_t lastTickCount  = XTimer::GetTickCount( );

                // reset connection state and (re)add handle
                mData->ResetConnectionState( );
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
                            else
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

                            if ( responseCode == 0 )
                            {
                                // do nothing, just repeat
                            }
                            else if ( responseCode != 200 )
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
                                    // we don't expect MJPEG stream to stop, so report an error
                                    mData->NotifyError( "Connection lost" );
                                }
                            }
                            break;
                        }
                    }
                }

                curl_multi_remove_handle( multiHandle, curl );

                if ( !mData->FailureDetected )
                {
                    mData->TimeToSleepBeforeNextTry = 0;
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

#ifdef DEBUG_MJPEG_STREAM
        if ( mData->DebugFile != nullptr )
        {
            fclose( mData->DebugFile );
        }
#endif
        // clean up
        if ( multiHandle != nullptr )
        {
            curl_multi_cleanup( multiHandle );
        }
        if ( curl != nullptr )
        {
            curl_easy_cleanup( curl );
        }

        free( buffer );
    }
    else
    {
        mData->NotifyError( "Fatal: Failed allocating communication buffer" );
    }

    XImageFree( &mData->DecodedImage );
}

namespace Private
{
    // Run video loop in a background thread
    void XMjpegHttpStreamData::WorkerThreadHandler( void* param )
    {
        static_cast<XMjpegHttpStream*>( param )->RunVideo( );
    }

    // Notify about error in the video source
    void XMjpegHttpStreamData::NotifyError( const string& errorMessage )
    {
        XScopedLock lock( &Sync );

        if ( !FailureDetected )
        {
            FailureDetected = true;

            // make a pause before next try if error happened
            TimeToSleepBeforeNextTry = PauseOnErrorMs;

            if ( Listener != nullptr )
            {
                Listener->OnError( errorMessage );
            }
        }
    }

    size_t CurlReceiveHeadersCallback( void* buffer, size_t size, size_t nmemb, void* userp )
    {
        size_t                  realSize = size * nmemb;
        XMjpegHttpStreamData*   data     = static_cast<XMjpegHttpStreamData*>( userp );
        char*                   header   = static_cast<char*>( buffer );

#ifdef DEBUG_MJPEG_STREAM
        if ( data->DebugFile != nullptr )
        {
            fwrite( buffer, size, nmemb, data->DebugFile );
        }
#endif

        if ( strstr( header, "Content-Type:" ) == header )
        {
            // delete old boundary (should not exist, but to be safe)
            delete [] data->JpegBoundary;
            data->JpegBoundaryLength = 0;
            data->JpegBoundary       = nullptr;

            data->IsContentTypeChecked = true;

            if ( ( strstr( header, "multipart" ) != 0 ) && (  strstr( header, "mixed" ) != 0 ) )
            {
                char* boundaryPtr = strstr( header, "boundary=" );

                if ( boundaryPtr != 0 )
                {
                    boundaryPtr += 9;

                    data->JpegBoundary = new (nothrow) uint8_t[strlen( boundaryPtr ) + 1];

                    if ( data->JpegBoundary == 0 )
                    {
                        data->NotifyError( STR_MEMORY_ALLOCATION_ERROR );
                    }
                    else
                    {
                        while ( ( *boundaryPtr != '\0' ) && ( isspace( *boundaryPtr ) == 0 ) )
                        {
                            data->JpegBoundary[data->JpegBoundaryLength] = static_cast<uint8_t>( *boundaryPtr );
                            data->JpegBoundaryLength++;
                            boundaryPtr++;
                        }
                        data->JpegBoundary[data->JpegBoundaryLength] = 0;
                    }
                }
            }
        }

        return realSize;
    }

    // Callback function called by libcurl when data arrives
    size_t CurlWriteMemoryCallback( void* contents, size_t size, size_t nmemb, void* userp )
    {
        size_t                  realSize = size * nmemb;
        XMjpegHttpStreamData*   data     = static_cast<XMjpegHttpStreamData*>( userp );

#ifdef DEBUG_MJPEG_STREAM
        if ( data->DebugFile != nullptr )
        {
            fwrite( contents, size, nmemb, data->DebugFile );
        }
#endif

        if ( !data->IsContentTypeChecked )
        {
            data->NotifyError( "Unknown content type" );
        }
        else
        {
            if ( realSize > MaxBufferSize - data->ReadSoFar )
            {
                data->NotifyError( "Too small communication buffer" );
            }
            else
            {
                memcpy( &( data->CommunicationBuffer[data->ReadSoFar] ), contents, realSize );
                data->ReadSoFar += realSize;

                // some IP cameras claim that boundary is "myboundary",
                // when it is really "--myboundary". this needs to be corrected.
                if ( ( data->JpegBoundaryLength != 0 ) && ( !data->IsBoundaryChecked ) )
                {
                    int boundaryStart = MemFind( data->CommunicationBuffer, data->ReadSoFar, data->JpegBoundary, data->JpegBoundaryLength );

                    if ( boundaryStart >= 0 )
                    {
                        int checkedBoundaryStart = boundaryStart;

                        while ( ( checkedBoundaryStart > 0 ) && ( isspace( data->CommunicationBuffer[checkedBoundaryStart - 1] ) == 0 ) )
                        {
                            checkedBoundaryStart--;
                        }

                        if ( checkedBoundaryStart != boundaryStart )
                        {
                            data->JpegBoundaryLength += ( boundaryStart - checkedBoundaryStart);

                            delete [] data->JpegBoundary;
                            data->JpegBoundary = new (nothrow) uint8_t[data->JpegBoundaryLength + 1];

                            if ( data->JpegBoundary == 0 )
                            {
                                data->NotifyError( STR_MEMORY_ALLOCATION_ERROR );
                            }
                            else
                            {
                                memcpy( data->JpegBoundary, &( data->CommunicationBuffer[checkedBoundaryStart] ), data->JpegBoundaryLength );
                                data->JpegBoundary[data->JpegBoundaryLength] = 0;
                            }
                        }

                        data->IsBoundaryChecked = true;
                    }
                }

                // search for image start
                if ( ( data->JpegImageStart == -1 ) && ( data->ReadSoFar - data->SearchStartIndex >= Private::JpegMagicSize ) )
                {
                    int index = MemFind( &( data->CommunicationBuffer[data->SearchStartIndex] ),
                                         data->ReadSoFar - data->SearchStartIndex, Private::JpegMagic, Private::JpegMagicSize );

                    if ( index != -1 )
                    {
                        data->JpegImageStart   = data->SearchStartIndex + index;
                        data->SearchStartIndex = data->JpegImageStart + Private::JpegMagicSize;
                    }
                    else
                    {
                        data->SearchStartIndex = data->ReadSoFar - Private::JpegMagicSize + 1;
                    }
                }

                // search for image end (boundary start)
                if ( ( data->JpegImageStart >= 0 ) && ( data->ReadSoFar - data->SearchStartIndex >= data->JpegBoundaryLength ) )
                {
                    int index = -1;

                    if ( data->JpegBoundaryLength )
                    {
                        // search for the next boundary
                        index = MemFind( &( data->CommunicationBuffer[data->SearchStartIndex] ),
                                         data->ReadSoFar - data->SearchStartIndex, data->JpegBoundary, data->JpegBoundaryLength );
                    }
                    else
                    {
                        // if boundary between JPEG images is not known, then search for MAGIC of another JPEG
                        index = MemFind( &( data->CommunicationBuffer[data->SearchStartIndex] ),
                                         data->ReadSoFar - data->SearchStartIndex, Private::JpegMagic, Private::JpegMagicSize );
                    }

                    if ( index != -1 )
                    {
                        int jpegEnd = data->SearchStartIndex + index;

                        {
                            XScopedLock lock( &data->Sync );

                            data->FramesCounter++;

                            if ( data->Listener != 0 )
                            {
                                // decode image only if someone needs it
                                XErrorCode ret = XDecodeJpegFromMemory( &( data->CommunicationBuffer[data->JpegImageStart] ),
                                                                        jpegEnd - data->JpegImageStart, &data->DecodedImage );

                                if ( ( ret == SuccessCode ) && ( !data->ExitEvent.IsSignaled( ) ) )
                                {
                                    shared_ptr<const XImage> guardedImage = XImage::Create( data->DecodedImage );

                                    if ( guardedImage )
                                    {
                                        data->Listener->OnNewImage( guardedImage );
                                    }
                                }
                                else
                                {
                                    data->NotifyError( "Failed decoding JPEG image" );
                                }
                            }
                        }

                        memcpy( data->CommunicationBuffer, &( data->CommunicationBuffer[jpegEnd] ), data->ReadSoFar - jpegEnd );

                        data->JpegImageStart   = -1;
                        data->SearchStartIndex = 0;
                        data->ReadSoFar -= jpegEnd;
                    }
                    else
                    {
                        // end of JPEG image was not found, so shift seach index
                        data->SearchStartIndex = data->ReadSoFar;

                        if ( data->JpegBoundaryLength != 0 )
                        {
                            data->SearchStartIndex -= ( data->JpegBoundaryLength - 1 );
                        }
                    }
                }
            }
        }

        return realSize;
    }

} // namespace Private

} } } // namespace CVSandbox::Video::MJpeg
