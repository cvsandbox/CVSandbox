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

#include "XHttpCommandsThread.hpp"

#include <list>

#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <XTimer.hpp>

#include <curl/curl.h>
#include "base64.hpp"

using namespace std;
using namespace CVSandbox::Threading;
using namespace CVSandbox::Video;

namespace CVSandbox { namespace Video { namespace MJpeg
{

// Namespace with some private stuff to hide
namespace Private
{
    enum class HttpRequestType
    {
        Unknown,
        Get,
        Post
    };

    class HttpCommand
    {
    public:
        uint32_t        Id;
        HttpRequestType Type;
        string          Url;
        string          Body;

    public:
        HttpCommand( ) :
            Id( 0 ), Type( HttpRequestType::Unknown ), Url( ), Body( )
        {
        }

        HttpCommand( uint32_t id, HttpRequestType type, const string& url, const string& body = "" ) :
            Id( id ), Type( type ), Url( url ), Body( body )
        {
        }
    };

    class XHttpCommandsThreadData
    {
    public:
        string                      BaseAddress;
        string                      UserName;
        string                      Password;
        string                      UserAgent;
        bool                        ForceBasicAuthorization;

        IHttpCommandResultListener* Listener;

        XMutex                      Sync;
        XMutex                      QueueSync;
        XManualResetEvent           ExitEvent;
        XManualResetEvent           CommandEvent;
        XThread                     BackgroundThread;

        list<HttpCommand>           CommandQueue;
        uint32_t                    CommandCounter;

    private:
        HttpCommand                 CurrentCommand;
        uint8_t*                    CommunicationBuffer;
        uint32_t                    ReadSoFar;
        bool                        FailureDetected;

    public:
        XHttpCommandsThreadData( ) :
            BaseAddress( ), UserName( ), Password( ), UserAgent( ), ForceBasicAuthorization( false ),
            Listener( nullptr ),
            Sync( ), QueueSync( ), ExitEvent( ), CommandEvent( ), BackgroundThread( ),
            CommandQueue( ), CommandCounter( 0 ),
            CurrentCommand( ), CommunicationBuffer( nullptr ), ReadSoFar( 0 ), FailureDetected( false )
        {
        }

        static size_t CurlWriteMemoryCallback( void* contents, size_t size, size_t nmemb, void* userp );
        static void WorkerThreadHandler( void* param );

        void ControlLoop( );
        void NotifyError( uint32_t commandId, const string& errorMessage );
        void NotifyCompletion( );
    };

    // Maximum size of internal read buffer
    static const int MaxBufferSize = 10 * 1024;

    // Connection timeout value (in milliseconds)
    static const uint32_t ConnectionTimeoutMs = 10000;
}

// Class constructor
XHttpCommandsThread::XHttpCommandsThread( ) :
    mData( new Private::XHttpCommandsThreadData( ) )
{
}

// Class destructor
XHttpCommandsThread::~XHttpCommandsThread( )
{
    delete mData;
}

// Create class instance
shared_ptr<XHttpCommandsThread> XHttpCommandsThread::Create( )
{
    return shared_ptr<XHttpCommandsThread>( new XHttpCommandsThread( ) );
}

// Start the thread waiting for commands to run
XErrorCode XHttpCommandsThread::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    if ( mData->BaseAddress.empty( ) )
    {
        ret = ErrorInvalidConfiguration;
    }
    else
    {
        mData->ExitEvent.Reset( );

        if ( mData->BackgroundThread.Create( Private::XHttpCommandsThreadData::WorkerThreadHandler, mData ) )
        {
            ret = SuccessCode;
        }
    }

    return ret;
}

// Signal thread to stop
void XHttpCommandsThread::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
        mData->CommandEvent.Signal( );
    }
}

// Wait till thread stops
void XHttpCommandsThread::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
        mData->CommandEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if control thread is still running
bool XHttpCommandsThread::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Set commands' result listener
void XHttpCommandsThread::SetListener( IHttpCommandResultListener* listener )
{
    XScopedLock lock( &mData->Sync );
    mData->Listener = listener;
}

// Set base address (starts with "http://" and followed by ip:port or DNS name; no final '/')
bool XHttpCommandsThread::SetBaseAddress( const string& baseAddress )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->BaseAddress = baseAddress;
        ret = true;
    }

    return ret;
}

// Set new authentication credentials (user name and password)
bool XHttpCommandsThread::SetAuthenticationCredentials( const string& userName, const string& password )
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
bool XHttpCommandsThread::SetUserAgent( const string& userAgent )
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
bool XHttpCommandsThread::SetForceBasicAuthorization( bool setForceBasic )
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

// Add GET request to the queue
uint32_t XHttpCommandsThread::EnqueueGetRequest( const string& url, bool clearPreviousRequests )
{
    XScopedLock lock( &mData->QueueSync );
    uint32_t    id = ++mData->CommandCounter;

    if ( clearPreviousRequests )
    {
        ClearRequests( url );
    }

    mData->CommandQueue.push_back( Private::HttpCommand( id, Private::HttpRequestType::Get, url ) );
    mData->CommandEvent.Signal( );

    return id;
}

// Add POST request to the queue
uint32_t XHttpCommandsThread::EnqueuePostRequest( const string& url, const string& body, bool clearPreviousRequests )
{
    XScopedLock lock( &mData->QueueSync );
    uint32_t    id = ++mData->CommandCounter;

    if ( clearPreviousRequests )
    {
        ClearRequests( url );
    }

    mData->CommandQueue.push_back( Private::HttpCommand( id, Private::HttpRequestType::Post, url, body ) );
    mData->CommandEvent.Signal( );

    return id;
}

// Clear all queued requests
void XHttpCommandsThread::ClearRequestsQueue( )
{
    XScopedLock lock( &mData->QueueSync );

    mData->CommandEvent.Reset( );
    mData->CommandQueue.clear( );
}

// Clear queued requests for the specified URL
void XHttpCommandsThread::ClearRequests( const string& url )
{
    XScopedLock lock( &mData->QueueSync );

    mData->CommandEvent.Reset( );

    for ( list<Private::HttpCommand>::iterator it = mData->CommandQueue.begin( ); it != mData->CommandQueue.end( ); )
    {
        if ( it->Url == url )
        {
            mData->CommandQueue.erase( it++ );
        }
        else
        {
            ++it;
        }
    }

    if ( !mData->CommandQueue.empty( ) )
    {
        mData->CommandEvent.Signal( );
    }
}

namespace Private
{

// Callback function called by libcurl when data arrives
size_t XHttpCommandsThreadData::CurlWriteMemoryCallback( void* contents, size_t size, size_t nmemb, void* userp )
{
    size_t                   realSize = size * nmemb;
    XHttpCommandsThreadData* data     = static_cast<XHttpCommandsThreadData*>( userp );

    if ( realSize <= MaxBufferSize - data->ReadSoFar )
    {
        memcpy( &( data->CommunicationBuffer[data->ReadSoFar] ), contents, realSize );
        data->ReadSoFar += static_cast<uint32_t>( realSize );
    }
    else
    {
        data->NotifyError( data->CurrentCommand.Id, "Too small communication buffer" );
    }

    return realSize;
}

// Control thread's handler
void XHttpCommandsThreadData::WorkerThreadHandler( void* param )
{
    static_cast<XHttpCommandsThreadData*>( param )->ControlLoop( );
}

// The actuall command loop
void XHttpCommandsThreadData::ControlLoop( )
{
    CommunicationBuffer = static_cast<uint8_t*>( malloc( Private::MaxBufferSize + 1 ) );

    if ( CommunicationBuffer == nullptr )
    {
        NotifyError( 0, "Fatal: Failed allocating communication buffer" );
    }
    else
    {
        // initialize libcurl session
        CURL*       curl        = curl_easy_init( );
        CURLM*      multiHandle = curl_multi_init( );
        CURLMcode   curlMcode   = CURLM_OK;
        int         stillRunning;

        if ( ( curl == nullptr ) || ( multiHandle == nullptr ) )
        {
            NotifyError( 0, "Fatal: Failed initializing libcurl session" );
        }
        else
        {
            struct curl_slist* extraHeaderList = nullptr;

            curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, XHttpCommandsThreadData::CurlWriteMemoryCallback );
            curl_easy_setopt( curl, CURLOPT_WRITEDATA, this );
            curl_easy_setopt( curl, CURLOPT_NOSIGNAL, 1 );

            if ( !UserAgent.empty( ) )
            {
                string header = "User-Agent: " + UserAgent;
                extraHeaderList = curl_slist_append( extraHeaderList, header.c_str( ) );
            }

            // set user name/password if it was supplied
            if ( !UserName.empty( ) )
            {
                if ( !ForceBasicAuthorization )
                {
                    curl_easy_setopt( curl, CURLOPT_USERNAME, UserName.c_str( ) );
                    curl_easy_setopt( curl, CURLOPT_PASSWORD, Password.c_str( ) );
                    curl_easy_setopt( curl, CURLOPT_HTTPAUTH, (long) CURLAUTH_ANY );
                }
                else
                {
                    // use custom header for basic authorization instead of libcurl's API,
                    // because found a camera, which keeps rejecting libcurl's authorization

                    string  loginWithPassword = UserName + ":" + Password;
                    size_t  authLen           = Base64EncodeLength( loginWithPassword.size( ) + 21 );
                    char*   authorization     = new char[authLen];

                    strcpy( authorization, "Authorization: Basic " );

                    Base64Encode( &( authorization[21] ), loginWithPassword.c_str( ), loginWithPassword.size( ) );

                    extraHeaderList = curl_slist_append( extraHeaderList, authorization );
                    delete [] authorization;
                }
            }

            if ( extraHeaderList != nullptr )
            {
                curl_easy_setopt( curl, CURLOPT_HTTPHEADER, extraHeaderList );
            }

            while ( !ExitEvent.Wait( 0 ) )
            {
                CurrentCommand = HttpCommand( );

                // wait for a new command to process
                CommandEvent.Wait( );

                {
                    XScopedLock lock( &QueueSync );

                    if ( !CommandQueue.empty( ) )
                    {
                        CurrentCommand = CommandQueue.front( );
                        CommandQueue.pop_front( );
                    }

                    if ( CommandQueue.empty( ) )
                    {
                        CommandEvent.Reset( );
                    }
                }

                if ( CurrentCommand.Type != HttpRequestType::Unknown )
                {
                    uint64_t lastTickCount = XTimer::GetTickCount( );
                    string   url           = BaseAddress + CurrentCommand.Url;

                    // set URL
                    curl_easy_setopt( curl, CURLOPT_URL, url.c_str( ) );

                    if ( CurrentCommand.Type == HttpRequestType::Post )
                    {
                        curl_easy_setopt( curl, CURLOPT_POSTFIELDS, CurrentCommand.Body.c_str( ) );
                    }

                    // reset read buffer and (re)add handle
                    ReadSoFar       = 0;
                    FailureDetected = false;

                    curl_multi_add_handle( multiHandle, curl );

                    // perform the initial request
                    curlMcode = curl_multi_perform( multiHandle, &stillRunning );
                    if ( curlMcode != CURLM_OK )
                    {
                        NotifyError( CurrentCommand.Id, string( "Request error: " ) + curl_multi_strerror( curlMcode ) );
                    }
                    else
                    {
                        while ( ( !ExitEvent.IsSignaled( ) ) && ( !FailureDetected ) )
                        {
                            if ( stillRunning != 0 )
                            {
                                int numfds = 0;

                                curlMcode = curl_multi_wait( multiHandle, nullptr, 0, 2000, &numfds );

                                if ( curlMcode != CURLM_OK )
                                {
                                    NotifyError( CurrentCommand.Id, string( "Request error: " ) + curl_multi_strerror( curlMcode ) );
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
                                            NotifyError( CurrentCommand.Id, "Connection time out" );
                                        }
                                    }
                                }

                                curlMcode = curl_multi_perform( multiHandle, &stillRunning );
                                if ( curlMcode != CURLM_OK )
                                {
                                    NotifyError( CurrentCommand.Id, string( "Request error: " ) + curl_multi_strerror( curlMcode ) );
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

                                        CommunicationBuffer[ReadSoFar] = 0;

                                        if ( ReadSoFar != 0 )
                                        {
                                            char* newLinePos = strchr( (char*) CommunicationBuffer, '\n' );

                                            errorMessage += ", ";
                                            errorMessage += string( (char*) CommunicationBuffer, ( newLinePos == nullptr ) ? ReadSoFar : ( (uint8_t*) newLinePos - CommunicationBuffer ) );
                                        }
                                    }
                                    break;
                                    }

                                    NotifyError( CurrentCommand.Id, errorMessage );
                                }
                                else
                                {
                                    CommunicationBuffer[ReadSoFar] = 0;

                                    NotifyCompletion( );
                                }

                                // exit the loop
                                break;
                            }
                        }
                    }

                    curl_multi_remove_handle( multiHandle, curl );
                }
            }

            if ( extraHeaderList != nullptr )
            {
                curl_slist_free_all( extraHeaderList );
            }
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

        free( CommunicationBuffer );
    }
}

// Notify about an error happened during processing of a HTTP command
void XHttpCommandsThreadData::NotifyError( uint32_t commandId, const string& errorMessage )
{
    XScopedLock lock( &Sync );

    if ( !FailureDetected )
    {
        FailureDetected = true;

        if ( Listener != 0 )
        {
            Listener->OnError( commandId, errorMessage );
        }
    }
}

// Notify about successful completion of a HTTP command
void XHttpCommandsThreadData::NotifyCompletion( )
{
    XScopedLock lock( &Sync );

    if ( Listener != 0 )
    {
        Listener->OnCompletion( CurrentCommand.Id, CommunicationBuffer, ReadSoFar );
    }
}

} // namespace Private

} } } // namespace CVSandbox::Video::MJpeg
