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

#include "XFFmpegVideoFileReader.hpp"

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
using namespace CVSandbox;

namespace CVSandbox { namespace Video { namespace FFmpeg
{

// Namespace with some private stuff to hide
namespace Private
{
    #ifdef PIXEL_FORMAT_BGRA
        #define dstFormatRgb AV_PIX_FMT_BGR24
    #else
        #define dstFormatRgb AV_PIX_FMT_RGB24
    #endif

    class XFFmpegVideoFileReaderData
    {
    private:
        AVFormatContext* FormatContext;
        AVCodecContext*  CodecContext;
        AVDictionary*    CodecOptions;
        SwsContext*      FrameConversionContext;
        AVPacket         Packet;

        AVFrame*         NativeFrame;
        AVFrame*         RgbFrame;

        int              VideoStreamIndex;
        int              BytesRemaining;

    public:
        string  CodecName;
        string  CodecLongName;
        int32_t FrameWidth;
        int32_t FrameHeight;
        float   FrameRate;
        int64_t FramesTotal;

    public:
        XFFmpegVideoFileReaderData( ) :
            FormatContext( nullptr ), CodecContext( nullptr ), CodecOptions( nullptr ), FrameConversionContext( nullptr ), Packet( ),
            NativeFrame( nullptr ), RgbFrame( nullptr ), VideoStreamIndex( -1 ), BytesRemaining( 0 ),
            CodecName( ), CodecLongName( ), FrameWidth( 0 ), FrameHeight( 0 ), FrameRate( 0 ), FramesTotal( 0 )
        {
            // register all formats known to FFmpeg
            av_register_all( );
            // ignore logging, since there is nothing we can do about it
            av_log_set_callback( nullptr );
        }

        XErrorCode Open( string fileName );
        XErrorCode GetNextFrame( shared_ptr<XImage>& image );
        void Close( );
        bool IsOpen( );

    private:
        XErrorCode GetFrame( shared_ptr<XImage>& image );

    };
}

// Class constructor
XFFmpegVideoFileReader::XFFmpegVideoFileReader( ) :
    mData( new Private::XFFmpegVideoFileReaderData( ) )
{
}

// Class destructor
XFFmpegVideoFileReader::~XFFmpegVideoFileReader( )
{
    Close( );
    delete mData;
}

// Create class instance
const shared_ptr<XFFmpegVideoFileReader> XFFmpegVideoFileReader::Create( )
{
    return shared_ptr<XFFmpegVideoFileReader>( new (nothrow) XFFmpegVideoFileReader( ) );
}

// Open video file with the specified name
XErrorCode XFFmpegVideoFileReader::Open( string fileName )
{
    return mData->Open( fileName );
}

// Close currently opened video file
void XFFmpegVideoFileReader::Close( )
{
    mData->Close( );
}

// Check if there is anything open by the writer
bool XFFmpegVideoFileReader::IsOpen( ) const
{
    return mData->IsOpen( );
}

// Get next video frame of the opened file
XErrorCode XFFmpegVideoFileReader::GetNextFrame( shared_ptr<XImage>& image )
{
    return ( mData->IsOpen( ) ) ? mData->GetNextFrame( image ) : ErrorFailed;
}

// Get codec name
const string XFFmpegVideoFileReader::CodecName( ) const
{
    return mData->CodecName;
}
const string XFFmpegVideoFileReader::CodecLongName( ) const
{
    return mData->CodecLongName;
}

// Frame size, rate and count
const XSize XFFmpegVideoFileReader::FrameSize( ) const
{
    return XSize( mData->FrameWidth, mData->FrameHeight );
}
float XFFmpegVideoFileReader::FrameRate( ) const
{
    return mData->FrameRate;
}
int64_t XFFmpegVideoFileReader::FramesTotal( ) const
{
    return mData->FramesTotal;
}

namespace Private
{

// Open video file
XErrorCode XFFmpegVideoFileReaderData::Open( string fileName )
{
    XErrorCode ret = SuccessCode;

    Close( );

    // allocate context
    FormatContext = avformat_alloc_context( );

    if ( FormatContext == nullptr )
    {
        ret = ErrorOutOfMemory;
    }
    else
    {
        if ( avformat_open_input( &FormatContext, fileName.c_str( ), nullptr, nullptr ) != 0 )
        {
            ret = ErrorIOFailure;
        }
        else
        {
            // find information about available streams
            if ( avformat_find_stream_info( FormatContext, nullptr ) < 0 )
            {
                ret = ErrorUnknownVideoFileFormat;
            }
            else
            {
                AVStream* videoStream = nullptr;

                // find the first video stream
                for ( int i = 0; i < static_cast<int>( FormatContext->nb_streams ); ++i )
                {
                    if ( FormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
                    {
                        videoStream      = FormatContext->streams[i];
                        VideoStreamIndex = i;
                        break;
                    }
                }

                if ( VideoStreamIndex == -1 )
                {
                    ret = ErrorVideoStreamMissing;
                }
                else
                {
                    CodecContext = FormatContext->streams[VideoStreamIndex]->codec;

                    AVCodec* codec = avcodec_find_decoder( CodecContext->codec_id );

                    if ( codec == 0 )
                    {
                        ret = ErrorCodecNotFound;
                    }
                    else
                    {
                        CodecName     = string( codec->name );
                        CodecLongName = string( codec->long_name );

                        FrameWidth  = CodecContext->width;
                        FrameHeight = CodecContext->height;
                        FrameRate   = (float) videoStream->r_frame_rate.num / videoStream->r_frame_rate.den;
                        FramesTotal = videoStream->nb_frames;

                        if ( codec->capabilities & AV_CODEC_CAP_TRUNCATED )
                        {
                            // we do not send complete frames
                            CodecContext->flags |= AV_CODEC_FLAG_TRUNCATED;
                        }

                        if ( avcodec_open2( CodecContext, codec, &CodecOptions ) < 0 )
                        {
                            ret = ErrorCodecInitFailure;
                        }
                        else
                        {
                            // allocate native video frame
                            NativeFrame = av_frame_alloc( );
                            // allocate RGB video frame
                            RgbFrame = av_frame_alloc( );

                            if ( ( NativeFrame == NULL ) || ( RgbFrame == NULL ) )
                            {
                                ret = ErrorOutOfMemory;
                            }
                            else
                            {
                                FrameConversionContext = sws_getContext( FrameWidth, FrameHeight, CodecContext->pix_fmt,
                                                                         FrameWidth, FrameHeight, dstFormatRgb, SWS_BILINEAR,
                                                                         nullptr, nullptr, nullptr );
                            }
                        }
                    }
                }
            }
        }
    }

    if ( ret != SuccessCode )
    {
        // free all resources on failure
        Close( );
    }

    return ret;
}

// Close opened video file if any
void XFFmpegVideoFileReaderData::Close( )
{
    if ( CodecOptions != nullptr )
    {
        av_dict_free( &CodecOptions );
        CodecOptions = nullptr;
    }

    if ( CodecContext != nullptr )
    {
        avcodec_close( CodecContext );
        CodecContext = nullptr;
    }

    if ( FormatContext != nullptr )
    {
        avformat_close_input( &FormatContext );
        FormatContext = nullptr;
    }

    if ( FrameConversionContext != nullptr )
    {
        sws_freeContext( FrameConversionContext );
        FrameConversionContext = nullptr;
    }

    if ( NativeFrame != nullptr )
    {
        av_free( NativeFrame );
        NativeFrame = nullptr;
    }

    if ( RgbFrame != nullptr )
    {
        av_free( RgbFrame );
        RgbFrame = nullptr;
    }

    if ( Packet.data != nullptr )
    {
        av_free_packet( &Packet );
        Packet.data = nullptr;
    }

    VideoStreamIndex = -1;
    BytesRemaining   = 0;

    CodecName.clear( );
    CodecLongName.clear( );

    FrameWidth  = 0;
    FrameHeight = 0;
    FrameRate   = 0;
    FramesTotal = 0;
}

// Check if there is an open video file
bool XFFmpegVideoFileReaderData::IsOpen( )
{
    return ( FormatContext != nullptr );
}

// Get the next frame of the opened video file
XErrorCode XFFmpegVideoFileReaderData::GetNextFrame( shared_ptr<XImage>& image )
{
    XErrorCode ret           = SuccessCode;
    int        bytesDecoded  = 0;
    int        frameFinished = 0;
    int        errorCode     = 0;

    while ( ret == SuccessCode )
    {
        if ( BytesRemaining > 0 )
        {
            do
            {
                // decode the next chunk of data
                bytesDecoded = avcodec_decode_video2( CodecContext, NativeFrame, &frameFinished, &Packet );

                if ( bytesDecoded < 0 )
                {
                    ret = ErrorFailedVideoDecoding;
                    break;
                }

                BytesRemaining -= bytesDecoded;
            }
            while ( ( !frameFinished ) && ( bytesDecoded > 0 ) && ( BytesRemaining > 0 ) );

            if ( ret != SuccessCode )
            {
                break;
            }

            if ( frameFinished )
            {
                ret = GetFrame( image );
                break;
            }
        }

        do
        {
            // free old packet if any
            if ( Packet.data != nullptr )
            {
                av_free_packet( &Packet );
                Packet.data = nullptr;
            }

            errorCode = av_read_frame( FormatContext, &Packet );

            if ( errorCode < 0 )
            {
                ret = ( errorCode == AVERROR_EOF ) ? ErrorEOF : ErrorIOFailure;
                break;
            }
        }
        while ( Packet.stream_index != VideoStreamIndex );

        BytesRemaining += Packet.size;
    }

    return ret;
}

// Get decoded frame out of native FFmpeg frame into XImage
XErrorCode XFFmpegVideoFileReaderData::GetFrame( shared_ptr<XImage>& image )
{
    XErrorCode ret = SuccessCode;

    if ( ( !image ) || ( image->Width( ) != FrameWidth ) || ( image->Height( ) != FrameHeight ) || ( image->Format( ) != XPixelFormatRGB24 ) )
    {
        image = XImage::AllocateRaw( FrameWidth, FrameHeight, XPixelFormatRGB24 );
    }

    if ( !image )
    {
        ret = ErrorOutOfMemory;
    }
    else
    {
        // assign appropriate parts of buffer to image planes in pFrameRGB
        avpicture_fill( (AVPicture*) RgbFrame, image->Data( ), dstFormatRgb, FrameWidth, FrameHeight );
        RgbFrame->linesize[0] = image->Stride( );

        // convert the image from its native format to RGB
        sws_scale( FrameConversionContext, (uint8_t const * const *) NativeFrame->data, NativeFrame->linesize, 0,
                   CodecContext->height, RgbFrame->data, RgbFrame->linesize );
    }

    return ret;
}

} // namespace Private

} } } // namespace CVSandbox::Video::FFmpeg
