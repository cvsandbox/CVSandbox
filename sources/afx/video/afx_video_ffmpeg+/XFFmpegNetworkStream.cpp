/*
    FFmpeg based video sources library of Computer Vision Sandbox

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

#include "XFFmpegNetworkStream.hpp"

#include <XMutex.hpp>
#include <XManualResetEvent.hpp>
#include <XThread.hpp>
#include <XTimer.hpp>

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

extern "C"
{
    #ifdef _MSC_VER
        #pragma warning(push, 3)
    #endif
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #ifdef _MSC_VER
        #pragma warning(pop)
    #endif
}

using namespace std;
using namespace CVSandbox::Threading;
using namespace CVSandbox::Video;

namespace CVSandbox { namespace Video { namespace FFmpeg
{

// Namespace with some private stuff to hide
namespace Private
{
    // Internal class which hides private parts of the XMjpegHttpStream class,
    // so those are not exposed in the main class
    class XFFmpegNetworkStreamData
    {
    public:
        XFFmpegNetworkStreamData( const string& streamUrl ) :
            StreamUrl( streamUrl ), UserName( ), Password( ), ProbSize( 5000000 ),
            Listener( 0 ),
            Sync( ), ExitEvent( ), BackgroundThread( ), FramesCounter( 0 ),
            TimeToSleepBeforeNextTry( 0 ),
            FailureDetected( false ), TimoutMonitorIsActive( false ), TimedOut( false ), ActionStartTick( 0 ),
            LastLogError( ), ErrorSync( )
        { }

    // Run video loop in a background worker thread
    static void WorkerThreadHandler( void* param );
    // FFmpeg logging callback
    static void LoggingCallback( void* avcl, int level, const char* fmt, va_list vl );
    // Check if FFmpeg must be interrupted or not
    static int CheckIfInterruptIsRequired( void* param );
    // Notify about error in the video source
    void NotifyError( const string& errorMessage );

    // Reset connection releated variable
    void ResetConnectionState( )
    {
        FailureDetected          = false;
        TimeToSleepBeforeNextTry = 0;
        LastLogError.clear( );
    }

    public:
        string                StreamUrl;
        string                UserName;
        string                Password;
        uint32_t              ProbSize;

        IVideoSourceListener* Listener;

        XMutex                Sync;
        XManualResetEvent     ExitEvent;
        XThread               BackgroundThread;
        uint32_t              FramesCounter;
        uint32_t              TimeToSleepBeforeNextTry;

        bool                  FailureDetected;
        bool                  TimoutMonitorIsActive;
        bool                  TimedOut;
        uint64_t              ActionStartTick;

        string                LastLogError;
        XMutex                ErrorSync;
    };

    // Pause length (in milliseconds) to do when error happened
    static const uint32_t PauseOnErrorMs = 1000;

    // Connection timeout value (in milliseconds)
    static const uint32_t ConnectionTimeoutMs = 10000;
}

// Class constructor
XFFmpegNetworkStream::XFFmpegNetworkStream( const string& streamUrl ) :
    mData( new Private::XFFmpegNetworkStreamData( streamUrl ) )
{
}

// Class destructor
XFFmpegNetworkStream::~XFFmpegNetworkStream( )
{
    // stop it brutally if user did not care
    //Terminate( );
    delete mData;
}

// Create class instance
const shared_ptr<XFFmpegNetworkStream> XFFmpegNetworkStream::Create( const string& streamUrl )
{
    return shared_ptr<XFFmpegNetworkStream>( new XFFmpegNetworkStream( streamUrl ) );
}

// Start video device (start its background thread which manages video acquisition)
XErrorCode XFFmpegNetworkStream::Start( )
{
    XScopedLock lock( &mData->Sync );
    XErrorCode  ret = ErrorFailed;

    if ( mData->StreamUrl.empty( ) )
    {
        ret = ErrorInvalidConfiguration;
    }
    else
    {
        mData->FramesCounter = 0;
        mData->ExitEvent.Reset( );

        if ( mData->BackgroundThread.Create( Private::XFFmpegNetworkStreamData::WorkerThreadHandler, this ) )
        {
            ret = SuccessCode;
        }
    }

    return ret;
}

// Signal video source to stop and finalize its background thread
void XFFmpegNetworkStream::SignalToStop( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->ExitEvent.Signal( );
    }
}

// Wait till video source stops
void XFFmpegNetworkStream::WaitForStop( )
{
    if ( IsRunning( ) )
    {
        XScopedLock lock( &mData->Sync );
        mData->ExitEvent.Signal( );
    }

    mData->BackgroundThread.Join( );
}

// Check if video source (its background thread) is running or not
bool XFFmpegNetworkStream::IsRunning( )
{
    XScopedLock lock( &mData->Sync );
    return mData->BackgroundThread.IsRunning( );
}

// Terminate video source (call it ONLY as the last action of stopping video source, when nothing else helps)
void XFFmpegNetworkStream::Terminate( )
{
    XScopedLock lock( &mData->Sync );

    if ( IsRunning( ) )
    {
        mData->BackgroundThread.Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t XFFmpegNetworkStream::FramesReceived( )
{
    XScopedLock lock( &mData->Sync );
    return mData->FramesCounter;
}

// Set video source listener
void XFFmpegNetworkStream::SetListener( IVideoSourceListener* listener )
{
    XScopedLock lock( &mData->Sync );
    mData->Listener = listener;
}

// Set new MJPEG URL to download images from
bool XFFmpegNetworkStream::SetUrl( const string& streamUrl )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->StreamUrl = streamUrl;
        ret = true;
    }

    return ret;
}

// Set new authentication credentials (user name and password)
bool XFFmpegNetworkStream::SetAuthenticationCredentials( const string& userName, const string& password )
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

// Set probing size for the FFmpeg library
bool XFFmpegNetworkStream::SetProbSize( uint32_t probSize )
{
    XScopedLock lock( &mData->Sync );
    bool        ret = false;

    if ( !IsRunning( ) )
    {
        mData->ProbSize = XMAX( 32, probSize );
        ret = true;
    }

    return ret;
}

// Run video acquisition loop
void XFFmpegNetworkStream::RunVideo( )
{
    char* streamUrl = new (nothrow) char[mData->StreamUrl.size( ) + mData->UserName.size( ) + mData->Password.size( ) + 16];

    if ( streamUrl == nullptr )
    {
        mData->NotifyError( "Fatal: Not enough memory" );
    }
    else
    {
        // if user name was not set or is already part of the URL, then just use the URL given by user
        if ( ( mData->UserName.empty( ) ) ||
             ( mData->StreamUrl.find( '@' ) != string::npos ) )
        {
            strcpy( streamUrl, mData->StreamUrl.c_str( ) );
        }
        else
        {
            size_t protocolSeparatorPost = mData->StreamUrl.find( "://" );

            if ( protocolSeparatorPost == string::npos )
            {
                mData->NotifyError( "Fatal: Cannot resolve protocol from the given URL" );
                delete[] streamUrl;
                streamUrl = nullptr;
            }
            else
            {
                sprintf( streamUrl, "%s://%s:%s@%s", mData->StreamUrl.substr( 0, protocolSeparatorPost ).c_str( ),
                                                     mData->UserName.c_str( ),
                                                     mData->Password.c_str( ),
                                                     mData->StreamUrl.substr( protocolSeparatorPost + 3 ).c_str( ) );
            }
        }
    }

    if ( streamUrl != nullptr )
    {
        char    ffmpegErrorString[AV_ERROR_MAX_STRING_SIZE] = { 0 };
        int     ffmpegError;

        // init FFmpeg
        av_register_all( );

        // set custom logging callback
        av_log_set_callback( Private::XFFmpegNetworkStreamData::LoggingCallback );

        while ( !mData->ExitEvent.Wait( mData->TimeToSleepBeforeNextTry ) )
        {
            AVFormatContext* formatContext = nullptr;

            mData->ResetConnectionState( );

            // init network
            avformat_network_init( );

            // allocate context
            formatContext = avformat_alloc_context( );

            if ( formatContext == nullptr )
            {
                mData->NotifyError( "Not enough memory to allocate context" );
            }
            else
            {
                AVDictionary*   connectionOptions = nullptr;
                char            probSizeOption[32];

                sprintf( probSizeOption, "%u", mData->ProbSize );

                // prefer using TCP for network streams
                av_dict_set( &connectionOptions, "rtsp_transport", "tcp", 0 );
                av_dict_set( &connectionOptions, "fflags", "nobuffer", 0 );
                av_dict_set( &connectionOptions, "probesize", probSizeOption, 0 );

                // set interruption callback
                formatContext->interrupt_callback.opaque   = static_cast<void*>( mData );
                formatContext->interrupt_callback.callback = Private::XFFmpegNetworkStreamData::CheckIfInterruptIsRequired;
                // set mData point into format for error callback
                formatContext->opaque                      = static_cast<void*>( mData );

                mData->TimoutMonitorIsActive = true;
                mData->TimedOut              = false;
                mData->ActionStartTick       = XTimer::GetTickCount( );

                ffmpegError = avformat_open_input( &formatContext, streamUrl, nullptr, &connectionOptions );

                mData->TimoutMonitorIsActive = false;

                if ( ffmpegError != 0 )
                {
                    if ( !mData->TimedOut )
                    {
                        if ( !mData->LastLogError.empty( ) )
                        {
                            mData->NotifyError( mData->LastLogError );
                        }
                        else
                        {
                            mData->NotifyError( string( "Connection failed: " ) + av_make_error_string( ffmpegErrorString, AV_ERROR_MAX_STRING_SIZE, ffmpegError ) );
                        }
                    }
                    else
                    {
                        mData->NotifyError( "Connection time out" );
                    }
                }
                else
                {
                    // find information about available streams
                    if ( avformat_find_stream_info( formatContext, nullptr ) < 0 )
                    {
                        mData->NotifyError( "Cannot find stream information" );
                    }
                    else
                    {
                        int videoStreamIndex = -1;

                        // find the first video stream
                        for ( int i = 0; i < static_cast<int>( formatContext->nb_streams ); i++ )
                        {
                            if ( formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
                            {
                                videoStreamIndex = i;
                                break;
                            }
                        }

                        if ( videoStreamIndex == -1 )
                        {
                            mData->NotifyError( "Did not find video stream" );
                        }
                        else
                        {
                            AVCodecContext* codecContext = formatContext->streams[videoStreamIndex]->codec;;
                            AVCodec*        codec        = avcodec_find_decoder( codecContext->codec_id );

                            if ( codec == 0 )
                            {
                                mData->NotifyError( "Cannot find codec" );
                            }
                            else
                            {
                                AVDictionary*   codecOptions = nullptr;

                                if ( avcodec_open2( codecContext, codec, &codecOptions ) < 0 )
                                {
                                    mData->NotifyError( "Cannot not open codec" );
                                }
                                else
                                {
                                    SwsContext* swsContext     = nullptr;
                                    AVFrame*    frame          = nullptr;
                                    AVFrame*    frameRGB       = nullptr;
                                    uint8_t*    buffer         = nullptr;
                                    ximage*     imageToProvide = nullptr;
                                    AVPacket    packet;
                                    int         frameFinished;
                                    int         numBytes       = avpicture_get_size( AV_PIX_FMT_RGB24, codecContext->width, codecContext->height );

                                    // allocate native video frame
                                    frame    = av_frame_alloc( );
                                    // allocate RGB video frame
                                    frameRGB = av_frame_alloc( );
                                    buffer   = static_cast<uint8_t*>( av_malloc( numBytes ) );

                                    if ( ( frame == nullptr ) || ( frameRGB == nullptr ) || ( buffer == nullptr ) )
                                    {
                                        mData->NotifyError( "Failed allocating memory for video frame" );
                                    }
                                    else
                                    {
                                        swsContext = sws_getContext( codecContext->width, codecContext->height, codecContext->pix_fmt, codecContext->width, codecContext->height,
                                                                     AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr );

                                        // assign appropriate parts of buffer to image planes in pFrameRGB
                                        avpicture_fill( (AVPicture*) frameRGB, buffer, AV_PIX_FMT_RGB24,
                                                        codecContext->width, codecContext->height );

                                        // wrap buffer into ximage
                                        XImageCreate( buffer, codecContext->width, codecContext->height, frameRGB->linesize[0], XPixelFormatRGB24, &imageToProvide );

                                        while ( !mData->ExitEvent.IsSignaled( ) )
                                        {
                                            mData->TimoutMonitorIsActive = true;
                                            mData->TimedOut = false;
                                            mData->ActionStartTick = XTimer::GetTickCount( );

                                            ffmpegError = av_read_frame( formatContext, &packet );

                                            mData->TimoutMonitorIsActive = false;

                                            if ( ffmpegError < 0 )
                                            {
                                                if ( !mData->TimedOut )
                                                {
                                                    mData->NotifyError( "Failed reading packet" );
                                                }
                                                else
                                                {
                                                    mData->NotifyError( "Connection time out" );
                                                }

                                                // reset on error
                                                break;
                                            }
                                            else
                                            {
                                                if ( packet.stream_index == videoStreamIndex )
                                                {
                                                    // decode video frame
                                                    avcodec_decode_video2( codecContext, frame, &frameFinished, &packet );

                                                    // did we get a video frame?
                                                    if ( frameFinished )
                                                    {
                                                        XScopedLock lock( &mData->Sync );

                                                        mData->FramesCounter++;

                                                        if ( mData->Listener != 0 )
                                                        {
                                                            // convert the image from its native format to RGB
                                                            sws_scale( swsContext, (uint8_t const * const *) frame->data, frame->linesize, 0,
                                                                       codecContext->height, frameRGB->data, frameRGB->linesize );

                                                            if ( !mData->ExitEvent.IsSignaled( ) )
                                                            {
                                                                shared_ptr<const XImage> guardedImage = XImage::Create( imageToProvide );

                                                                if ( guardedImage )
                                                                {
                                                                    mData->Listener->OnNewImage( guardedImage );
                                                                }
                                                            }
                                                        }
                                                    }
                                                }

                                                // Free the packet that was allocated by av_read_frame
                                                av_free_packet( &packet );
                                            }
                                        }

                                        sws_freeContext( swsContext );
                                    }

                                    // free resources
                                    XImageFree( &imageToProvide );
                                    av_free( frame );
                                    av_free( frameRGB );
                                    av_free( buffer );
                                    av_dict_free( &codecOptions );
                                    avcodec_close( codecContext );
                                }
                            }
                        }
                    }
                }

                avformat_close_input( &formatContext );
                av_dict_free( &connectionOptions );
            }

            avformat_network_deinit( );
        }

        delete [] streamUrl;
    }
}

namespace Private
{
    // Run video loop in a background thread
    void XFFmpegNetworkStreamData::WorkerThreadHandler( void* param )
    {
        static_cast<XFFmpegNetworkStream*>( param )->RunVideo( );
    }

    // Notify about error in the video source
    void XFFmpegNetworkStreamData::NotifyError( const string& errorMessage )
    {
        XScopedLock lock( &Sync );

        if ( !ExitEvent.IsSignaled( ) )
        {
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
    }

    // FFmpeg logging callback
    void XFFmpegNetworkStreamData::LoggingCallback( void* avcl, int level, const char* fmt, va_list vl )
    {
        if ( ( level <= AV_LOG_ERROR ) && ( avcl != nullptr ) )
        {
            AVClass* avc  = static_cast<AVClass**>( avcl )[0];

            if ( ( avc != nullptr ) && ( strncmp( "AVFormatContext", avc->class_name, 15 ) == 0 ) )
            {
                AVFormatContext*          avFormat = static_cast<AVFormatContext*>( avcl );
                XFFmpegNetworkStreamData* data     = static_cast<XFFmpegNetworkStreamData*>( avFormat->opaque );

                if ( data != nullptr )
                {
                    char buffer[1024] = { 0 };
                    int  print_prefix = 1;

                    // format error message
                    av_log_format_line( avcl, level, fmt, vl, buffer, sizeof( buffer ) - 1, &print_prefix );

                    // a bit of hack below assuming internal FFmpeg format of error message; but this is the only public
                    // API for formatting error message, which put too much details
                    string strError       = string( buffer );
                    size_t separatorIndex = strError.find_last_of( ':' );

                    if ( separatorIndex != string::npos )
                    {
                        strError = strError.substr( separatorIndex + 1 );
                        strError.erase( strError.begin( ), std::find_if( strError.begin( ), strError.end( ), std::not1( std::ptr_fun<int, int>( std::isspace ) ) ) );
                    }

                    XScopedLock lock( &data->ErrorSync );
                    data->LastLogError = strError;
                }
            }
        }
    }

    // Check if FFmpeg must be interrupted or not
    int XFFmpegNetworkStreamData::CheckIfInterruptIsRequired( void* param )
    {
        XFFmpegNetworkStreamData* data = static_cast<XFFmpegNetworkStreamData*>( param );
        int                       ret  = 0;

        if ( data->ExitEvent.IsSignaled( ) )
        {
            ret = 1;
        }
        else
        {
            if ( data->TimoutMonitorIsActive )
            {
                uint32_t diff = static_cast<uint32_t>( XTimer::GetTickCount( ) - data->ActionStartTick );

                if ( diff >= ConnectionTimeoutMs )
                {
                    data->TimedOut = true;
                    ret = 1;
                }
            }
        }

        return ret;
    }
}

} } } // namespace CVSandbox::Video::FFmpeg
