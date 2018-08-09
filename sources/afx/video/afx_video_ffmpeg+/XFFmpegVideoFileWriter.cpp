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

#include "XFFmpegVideoFileWriter.hpp"

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

static const AVCodecID FFmpegVideoCodecs[] =
{
    AV_CODEC_ID_MPEG4,
    AV_CODEC_ID_WMV1,
    AV_CODEC_ID_WMV2,
    AV_CODEC_ID_MSMPEG4V2,
    AV_CODEC_ID_MSMPEG4V3,
    AV_CODEC_ID_H263P,
    AV_CODEC_ID_FLV1,
    AV_CODEC_ID_MPEG2VIDEO
};

static const AVPixelFormat FFmpegPixelFormats[] =
{
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV420P,
    AV_PIX_FMT_YUV420P
};

// Namespace with some private stuff to hide
namespace Private
{
    // Allocate FFmpeg picture of the specified format and size
    static AVFrame* AllocatePicture( AVPixelFormat pix_fmt, int width, int height )
    {
        AVFrame* picture = av_frame_alloc( );

        if ( picture != nullptr )
        {
            void* picture_buf = av_malloc( avpicture_get_size( pix_fmt, width, height ) );

            if ( picture_buf == nullptr )
            {
                av_free( picture );
                picture = nullptr;
            }
            else
            {
                avpicture_fill( (AVPicture*) picture, static_cast<uint8_t*>( picture_buf ), pix_fmt, width, height );
            }
        }

        return picture;
    }

    // Internal class which hides private parts of the XFFmpegVideoFileWriter class,
    // so those are not exposed in the main class
    class XFFmpegVideoFileWriterData
    {
    public:
        AVFormatContext*    FormatContext;
        AVStream*           VideoStream;
        AVFrame*            VideoFrame;
        SwsContext*         ConvertContextRgb;
        SwsContext*         ConvertContextRgbA;
        SwsContext*         ConvertContextGrayscale;

        uint8_t*            VideoOutputBuffer;
        int                 VideoOutputBufferSize;

        int                 VideoFrameWidth;
        int                 VideoFrameHeight;

    public:
        XFFmpegVideoFileWriterData( ) :
            FormatContext( nullptr ), VideoStream( nullptr ),
            VideoFrame( nullptr ), ConvertContextRgb( nullptr ), ConvertContextRgbA( nullptr ), ConvertContextGrayscale( nullptr ),
            VideoOutputBuffer( nullptr ), VideoOutputBufferSize( 0 ),
            VideoFrameWidth( 0 ), VideoFrameHeight( 0 )
        {
            // register all formats known to FFmpeg
            av_register_all( );
            // ignore logging, since there is nothing we can do about it here except annoying user
            av_log_set_callback( nullptr );
        }

        // Close currently opened video file
        void Close( )
        {
            if ( FormatContext != nullptr )
            {
                /* get the delayed frames */
                for ( int got_output = 1, error_code = 0; ( got_output != 0 ) && ( error_code == 0 ); )
                {
                    AVPacket packet;
                    av_init_packet( &packet );

                    packet.data = VideoOutputBuffer;
                    packet.size = VideoOutputBufferSize;

                    error_code = avcodec_encode_video2( VideoStream->codec, &packet, NULL, &got_output );

                    if ( ( error_code == 0 ) && ( got_output != 0 ) )
                    {
                        error_code = av_interleaved_write_frame( FormatContext, &packet );
                    }

                    packet.data = nullptr;
                    packet.size = 0;

                    av_free_packet( &packet );
                }

                // write trailer and close the file
                if ( FormatContext->pb != nullptr )
                {
                    av_write_trailer( FormatContext );
                    avio_close( FormatContext->pb );
                }

                // close codec
                if ( VideoStream != nullptr )
                {
                    avcodec_close( VideoStream->codec );
                    VideoStream = nullptr;
                }

                // free video format context
                avformat_free_context( FormatContext );
                FormatContext = nullptr;
            }

            // free video frame and output buffer
            if ( VideoFrame != nullptr )
            {
                av_free( VideoFrame->data[0] );
                av_frame_free( &VideoFrame );
                VideoFrame = nullptr;
            }
            if ( VideoOutputBuffer != nullptr )
            {
                av_free( VideoOutputBuffer );
                VideoOutputBuffer = nullptr;
            }
            VideoOutputBufferSize = 0;

            // free image conversion contexts
            if ( ConvertContextRgb != nullptr )
            {
                sws_freeContext( ConvertContextRgb );
                ConvertContextRgb = nullptr;
            }
            if ( ConvertContextRgbA != nullptr )
            {
                sws_freeContext( ConvertContextRgbA );
                ConvertContextRgbA = nullptr;
            }
            if ( ConvertContextGrayscale != nullptr )
            {
                sws_freeContext( ConvertContextGrayscale );
                ConvertContextGrayscale = nullptr;
            }

            VideoFrameWidth  = 0;
            VideoFrameHeight = 0;
        }

        // Add video stream to the current format context
        XErrorCode AddVideoStream( int width, int height, int frameRate, int bitRate, AVCodecID codecId, AVPixelFormat pixelFormat )
        {
            XErrorCode ret = ErrorFailed;

            if ( FormatContext != nullptr )
            {
                // create new stream
                VideoStream = avformat_new_stream( FormatContext, nullptr );

                if ( VideoStream == nullptr )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    AVCodecContext* codecContex = VideoStream->codec;

                    // set video stream ID
                    VideoStream->id = 0;

                    // set codec parameters
                    codecContex->codec_id   = codecId;
                    codecContex->codec_type = AVMEDIA_TYPE_VIDEO;

                    // set sample parameters
                    codecContex->bit_rate = bitRate;
                    codecContex->width    = width;
                    codecContex->height   = height;

                    // time base: this is the fundamental unit of time (in seconds) in terms
                    // of which frame timestamps are represented. for fixed-fps content,
                    // timebase should be 1/(frame rate) and time stamp increments should be
                    // identically 1.
                    codecContex->time_base.den = frameRate;
                    codecContex->time_base.num = 1;
                    // TOFIX? : The FFmpeg version in use puts a warning saying that time base must
                    // be set on video stream, but not on codec context (which is depricated now).
                    // Doing so leads to an error, saying that frame rate is not set. However
                    // duplicating this makes it all happy.
                    VideoStream->time_base.den = frameRate;
                    VideoStream->time_base.num = 1;

                    codecContex->gop_size = 12; // emit one intra frame every twelve frames at most
                    codecContex->pix_fmt  = pixelFormat;

                    if ( codecContex->codec_id == AV_CODEC_ID_MPEG1VIDEO )
                    {
                        // Needed to avoid using macro blocks in which some coefficient overflow.
                        // This does not happen with normal video, it just happens here as
                        // the motion of the chroma plane does not match the luma plane.
                        codecContex->mb_decision = 2;
                    }

                    // some formats want stream headers to be separate
                    if ( FormatContext->oformat->flags & AVFMT_GLOBALHEADER )
                    {
                        codecContex->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                    }

                    VideoFrameWidth  = width;
                    VideoFrameHeight = height;

                    ret = SuccessCode;
                }
            }

            return ret;
        }

        // Opne the video stream assosiate with the current
        XErrorCode OpenVideoStream( string fileName )
        {
            XErrorCode ret = ErrorFailed;

            if ( FormatContext != nullptr )
            {
                AVCodecContext* codecContext = VideoStream->codec;
                AVCodec*        codec        = avcodec_find_encoder( codecContext->codec_id );

                if ( codec == nullptr )
                {
                    ret = ErrorCodecNotFound;
                }
                else
                {
                    // open the codec
                    if ( avcodec_open2( codecContext, codec, nullptr ) < 0 )
                    {
                        ret = ErrorCodecInitFailure;
                    }
                    else
                    {
                        // allocate output buffer
                        VideoOutputBufferSize = 6 * codecContext->width * codecContext->height; // more than enough even for raw video
                        VideoOutputBuffer     = static_cast<uint8_t*>( av_malloc( VideoOutputBufferSize ) );

                        // allocate the encoded raw picture
                        VideoFrame = AllocatePicture( codecContext->pix_fmt, codecContext->width, codecContext->height );

                        #ifdef PIXEL_FORMAT_BGRA
                            AVPixelFormat srcFormatRgb  = AV_PIX_FMT_BGR24;
                            AVPixelFormat srcFormatRgbA = AV_PIX_FMT_BGRA;
                        #else
                            AVPixelFormat srcFormatRgb  = AV_PIX_FMT_RGB24;
                            AVPixelFormat srcFormatRgbA = AV_PIX_FMT_RGBA;
                        #endif

                        // prepare scaling context to convert RGB, RGBA and Grayscale images to video format
                        ConvertContextRgb = sws_getContext( codecContext->width, codecContext->height, srcFormatRgb,
                            codecContext->width, codecContext->height, codecContext->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr );
                        ConvertContextRgbA = sws_getContext( codecContext->width, codecContext->height, srcFormatRgbA,
                            codecContext->width, codecContext->height, codecContext->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr );
                        // prepare scaling context to convert grayscale image to video format
                        ConvertContextGrayscale = sws_getContext( codecContext->width, codecContext->height, AV_PIX_FMT_GRAY8,
                            codecContext->width, codecContext->height, codecContext->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr );

                        if ( ( VideoOutputBuffer == nullptr ) || ( VideoFrame == nullptr ) ||
                             ( ConvertContextRgb == nullptr ) || ( ConvertContextRgbA == nullptr ) || ( ConvertContextGrayscale == nullptr ) )
                        {
                            ret = ErrorOutOfMemory;
                        }
                        else
                        {
                            // open output file and write header
                            if ( ( avio_open( &FormatContext->pb, fileName.c_str( ), AVIO_FLAG_WRITE ) != 0 ) ||
                                 ( avformat_write_header( FormatContext, nullptr ) != 0 ) )
                            {
                                ret = ErrorIOFailure;
                            }
                            else
                            {
                                ret = SuccessCode;
                            }
                        }
                    }
                }
            }

            return ret;
        }

        // Write video frame to the opened video file
        XErrorCode WriteVideoFrame( )
        {
            XErrorCode ret = ErrorFailed;

            if ( FormatContext != nullptr )
            {
                AVCodecContext* codecContext = VideoStream->codec;
                AVPacket        packet;
                int             got_output;

                av_init_packet( &packet );

                packet.data = VideoOutputBuffer;
                packet.size = VideoOutputBufferSize;

                if ( avcodec_encode_video2( codecContext, &packet, VideoFrame, &got_output ) == 0 )
                {
                    if ( ( got_output == 0 ) ||
                            ( av_interleaved_write_frame( FormatContext, &packet ) == 0 ) )
                    {
                        ret = SuccessCode;
                    }
                }

                packet.data = nullptr;
                packet.size = 0;
                av_free_packet( &packet );
            }

            return ret;
        }
    };
} // namespace Private

// Class constructor
XFFmpegVideoFileWriter::XFFmpegVideoFileWriter( ) :
    mData( new Private::XFFmpegVideoFileWriterData( ) )
{
}

// Class destructor
XFFmpegVideoFileWriter::~XFFmpegVideoFileWriter( )
{
    Close( );
    delete mData;
}

// Create class instance
const shared_ptr<XFFmpegVideoFileWriter> XFFmpegVideoFileWriter::Create( )
{
    return shared_ptr<XFFmpegVideoFileWriter>( new (nothrow) XFFmpegVideoFileWriter( ) );
}

// Open video file with the specified name, frame size/rate, codec and bitrate
XErrorCode XFFmpegVideoFileWriter::Open( string fileName, int width, int height, int framRate, Codec codec, int bitrate )
{
    XErrorCode ret = ErrorFailed;

    // close currently file, if anything is open
    Close( );

    if ( ( width < 16 ) || ( height < 16 ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( ( width > 2048 ) || ( height > 2048 ) )
    {
        ret = ErrorImageIsTooBig;
    }
    else if ( ( width % 4 != 0 ) || ( height % 4 != 0 ) )
    {
        ret = ErrorInvalidImageSize;
    }
    else if ( ( codec < Codec::Default ) || ( codec >= Codec::LastValue ) )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        // guess about destination file format from its file name
        AVOutputFormat* outputFormat = av_guess_format( nullptr, fileName.c_str( ), nullptr );

        if ( !outputFormat )
        {
            // guess about destination file format from its short name
            outputFormat = av_guess_format( "mpeg", nullptr, nullptr );
        }

        if ( !outputFormat )
        {
            ret = ErrorUnknownVideoFileFormat;
        }
        else
        {
            // prepare format context
            mData->FormatContext = avformat_alloc_context( );

            if ( mData->FormatContext == nullptr )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                mData->FormatContext->oformat = outputFormat;

                ret = mData->AddVideoStream( width, height,
                        XINRANGE( framRate, 1, 60 ),
                        XMAX( 10000, bitrate ),
                        ( codec == Codec::Default ) ? outputFormat->video_codec : FFmpegVideoCodecs[static_cast<int>( codec )],
                        ( codec == Codec::Default ) ? AV_PIX_FMT_YUV420P : FFmpegPixelFormats[static_cast<int>( codec )] );

                if ( ret == SuccessCode )
                {
                    ret = mData->OpenVideoStream( fileName );
                }
            }
        }

        if ( ret != SuccessCode )
        {
            Close( );
        }
    }

    return ret;
}

// Close currently opened video file
void XFFmpegVideoFileWriter::Close( )
{
    mData->Close( );
}

// Check if there is anything open by the writer
bool XFFmpegVideoFileWriter::IsOpen( ) const
{
    return ( mData->FormatContext != nullptr );
}

// Write video frame to the currently opened video file
XErrorCode XFFmpegVideoFileWriter::WriteVideFrame( const shared_ptr<const XImage>& image, int64_t presentationTime )
{
    XErrorCode ret = SuccessCode;

    if ( mData->FormatContext == nullptr )
    {
        ret = ErrorInitializationFailed;
    }
    else if ( !image )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->Width( ) != mData->VideoFrameWidth ) || ( image->Height( ) != mData->VideoFrameHeight ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        uint8_t*   srcData[4]     = { image->Data( ), nullptr, nullptr, nullptr };
        int        srcLinesize[4] = { image->Stride( ), 0, 0, 0 };

        // convert source image to the format of the video file
        switch ( image->Format( ) )
        {
        case XPixelFormatGrayscale8:
            sws_scale( mData->ConvertContextGrayscale, srcData, srcLinesize, 0, image->Height( ), mData->VideoFrame->data, mData->VideoFrame->linesize );
            break;

        case XPixelFormatRGB24:
            sws_scale( mData->ConvertContextRgb, srcData, srcLinesize, 0, image->Height( ), mData->VideoFrame->data, mData->VideoFrame->linesize );
            break;

        case XPixelFormatRGBA32:
            sws_scale( mData->ConvertContextRgbA, srcData, srcLinesize, 0, image->Height( ), mData->VideoFrame->data, mData->VideoFrame->linesize );
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }

        if ( ret == SuccessCode )
        {
            if ( presentationTime >= 0 )
            {
                mData->VideoFrame->pts = presentationTime;
            }
            ret = mData->WriteVideoFrame( );
        }
    }

    return ret;
}

} } } // namespace CVSandbox::Video::FFmpeg
