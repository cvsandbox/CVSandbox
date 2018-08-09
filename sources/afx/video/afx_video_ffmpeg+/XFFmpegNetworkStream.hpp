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
#ifndef CVS_XFFMPEG_NETWORK_STREAM_HPP
#define CVS_XFFMPEG_NETWORK_STREAM_HPP

#include <XInterfaces.hpp>
#include <IVideoSource.hpp>

namespace CVSandbox { namespace Video { namespace FFmpeg
{

namespace Private
{
    class XFFmpegNetworkStreamData;
};

// Class which provides access to network streams like RTPS using FFmpeg library
class XFFmpegNetworkStream : public IVideoSource, private Uncopyable
{
    friend class Private::XFFmpegNetworkStreamData;

private:
    XFFmpegNetworkStream( const std::string& streamUrl );

public:
    ~XFFmpegNetworkStream( );

    static const std::shared_ptr<XFFmpegNetworkStream> Create( const std::string& streamUrl = std::string( ) );

    // Start video device (start its background thread which manages video acquisition)
    virtual XErrorCode Start( );
    // Signal video source to stop and finalize its background thread
    virtual void SignalToStop( );
    // Wait till video source stops
    virtual void WaitForStop( );
    // Check if video source (its background thread) is running or not
    virtual bool IsRunning( );
    // Terminate video source (call it ONLY as the last action of stopping video source, when nothing else helps)
    virtual void Terminate( );
    // Get number of frames received since the the start of the video source
    virtual uint32_t FramesReceived( );

    // Set video source listener
    virtual void SetListener( IVideoSourceListener* listener );

    // Set new URL of the network video stream (RTSP, etc)
    bool SetUrl( const std::string& streamUrl );
    // Set new authentication credentials (user name and password)
    bool SetAuthenticationCredentials( const std::string& userName, const std::string& password );
    // Set probing size for the FFMPEG library
    bool SetProbSize( uint32_t probSize );

private:
    // Run video acquisition loop
    void RunVideo( );

private:
    Private::XFFmpegNetworkStreamData* mData;
};

} } } // namespace CVSandbox::Video::FFmpeg

#endif // CVS_XFFMPEG_NETWORK_STREAM_HPP
