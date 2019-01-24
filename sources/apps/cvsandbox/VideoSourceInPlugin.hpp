/*
    Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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
#ifndef CVS_VIDEO_SOURCE_IN_PLUGIN_HPP
#define CVS_VIDEO_SOURCE_IN_PLUGIN_HPP

#include <memory>
#include <IVideoSource.hpp>
#include <XVideoSourcePlugin.hpp>

class VideoSourceInPlugin : public CVSandbox::Video::IVideoSource, private IVideoSourcePluginListener, private CVSandbox::Uncopyable
{
private:
    VideoSourceInPlugin( const std::shared_ptr<XVideoSourcePlugin>& plugin );

public:
    static std::shared_ptr<VideoSourceInPlugin> Create( const std::shared_ptr<XVideoSourcePlugin>& plugin );

    // Start video source
    virtual XErrorCode Start( );
    // Signal video source to stop
    virtual void SignalToStop( );
    // Wait till video source stops
    virtual void WaitForStop( );
    // Check if video source is running
    virtual bool IsRunning( );
    // Terminate video source (in the case it is not willing to stop)
    virtual void Terminate( );
    // Get number of frames received since the the start of the video source
    virtual uint32_t FramesReceived( );
    // Set video source listener
    virtual void SetListener( CVSandbox::Video::IVideoSourceListener* listener );

private:

    // New video frame notification
    virtual void OnNewImage( const std::shared_ptr<const CVSandbox::XImage>& image );
    // Video source error notification
    virtual void OnError( const std::string& errorMessage );

    static void NewImagePluginCallback( void* userParam, const std::shared_ptr<const CVSandbox::XImage>& image );

private:
    std::shared_ptr<XVideoSourcePlugin>     mPlugin;
    CVSandbox::Video::IVideoSourceListener* mListener;
};

#endif // CVS_VIDEO_SOURCE_IN_PLUGIN_HPP
