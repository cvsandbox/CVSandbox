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

#pragma once
#ifndef CVS_XFFMPEG_VIDEO_FILE_WRITER_HPP
#define CVS_XFFMPEG_VIDEO_FILE_WRITER_HPP

#include <string>
#include <memory>
#include <XImage.hpp>
#include <XInterfaces.hpp>

namespace CVSandbox { namespace Video { namespace FFmpeg
{

namespace Private
{
    class XFFmpegVideoFileWriterData;
};

// Class which allows writing video files using FFmpeg library
class XFFmpegVideoFileWriter : private Uncopyable
{
public:
    enum class Codec
    {
        Default     = -1,   // Default video codec, which FFmpeg library selects for the specified file format
        MPEG4       = 0,    // MPEG-4 Video (FMP4)
        WMV1,               // Windows Media Video 7 (WMV1)
        WMV2,               // Windows Media Video 8 (WMV2)
        MSMPEG4v2,          // MS MPEG-4 Video v2 (MP42)
        MSMPEG4v3,          // MS MPEG-4 Video v3 (MP43)

        H263P,              // H263 (H263)
        FLV1,               // Flash Video (FLV1)
        MPEG2,              // MPEG-1/2 Video (mpg2)

        LastValue           // Last value of the enum, which is an invalid value for the codec parameter
    };

private:
    XFFmpegVideoFileWriter( );

public:
    ~XFFmpegVideoFileWriter( );

    static const std::shared_ptr<XFFmpegVideoFileWriter> Create( );

    // Open video file with the specified name, frame size/rate, codec and bitrate
    XErrorCode Open( std::string fileName, int width, int height, int framRate = 30, Codec codec = Codec::Default, int bitrate = 500000 );
    // Close currently opened video file
    void Close( );
    // Check if there is anything opened by the writer
    bool IsOpen( ) const;

    // Write video frame to the currently opened video file
    XErrorCode WriteVideFrame( const std::shared_ptr<const CVSandbox::XImage>& image, int64_t presentationTime = -1 );

private:
    Private::XFFmpegVideoFileWriterData* mData;
};

} } } // namespace CVSandbox::Video::FFmpeg

#endif // CVS_XFFMPEG_VIDEO_FILE_WRITER_HPP
