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
#ifndef CVS_XFFMPEG_VIDEO_FILE_READER_HPP
#define CVS_XFFMPEG_VIDEO_FILE_READER_HPP

#include <string>
#include <memory>
#include <XImage.hpp>
#include <XInterfaces.hpp>
#include <XSize.hpp>

namespace CVSandbox { namespace Video { namespace FFmpeg
{

namespace Private
{
    class XFFmpegVideoFileReaderData;
};

// Class which allows readeing video files using FFmpeg library
class XFFmpegVideoFileReader : private Uncopyable
{
private:
    XFFmpegVideoFileReader( );

public:
    ~XFFmpegVideoFileReader( );

    static const std::shared_ptr<XFFmpegVideoFileReader> Create( );

    // Open video file with the specified name
    XErrorCode Open( std::string fileName  );
    // Close currently opened video file
    void Close( );
    // Check if there is anything opened by the reader
    bool IsOpen( ) const;

    // Get next video frame of the opened file
    XErrorCode GetNextFrame( std::shared_ptr<CVSandbox::XImage>& image );

    // Get codec name
    const std::string CodecName( ) const;
    const std::string CodecLongName( ) const;

    // Frame size, rate and count
    const CVSandbox::XSize FrameSize( ) const;
    float FrameRate( ) const;
    int64_t FramesTotal( ) const;

private:
    Private::XFFmpegVideoFileReaderData* mData;
};

} } } // namespace CVSandbox::Video::FFmpeg

#endif // CVS_XFFMPEG_VIDEO_FILE_READER_HPP
