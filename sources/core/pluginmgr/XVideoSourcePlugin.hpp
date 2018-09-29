/*
    Plug-ins' management library of Computer Vision Sandbox

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
#ifndef CVS_XVIDEO_SOURCE_PLUGIN_HPP
#define CVS_XVIDEO_SOURCE_PLUGIN_HPP

#include <XImage.hpp>
#include "XPlugin.hpp"

class IVideoSourcePluginListener
{
public:
    virtual ~IVideoSourcePluginListener( ) { }

    // New video frame notification
    virtual void OnNewImage( const std::shared_ptr<const CVSandbox::XImage>& image ) = 0;

    // Video source error notification
    virtual void OnError( const std::string& errorMessage ) = 0;
};

class XVideoSourcePlugin : public XPlugin
{
private:
    XVideoSourcePlugin( void* plugin, bool ownIt );

public:
    virtual ~XVideoSourcePlugin( );

    // Create plug-in wrapper
    static const std::shared_ptr<XVideoSourcePlugin> Create( void* plugin, bool ownIt = true );

    // Start video source so it initializes and begins providing video frames
    XErrorCode Start( );
    // Signal video to stop, so it could finalize and clean-up
    void SignalToStop( );
    // Wait till video source (its thread) stops
    void WaitForStop( );
    // Check if video source (its thread) is still running
    bool IsRunning( );

    // Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
    // by itself when signalled (ideally this method should not exist and be called at all)
    void Terminate( );

    // Get number of frames received since the the start of the video source
    uint32_t FramesReceived( );

    // Set video source plug-in listener
    void SetListener( IVideoSourcePluginListener* listener );

private:
    static void NewImageHandler( void* userParam, const ximage* image );
    static void ErrorMessageHandler( void* userParam, const char* errorMessage );

private:
    IVideoSourcePluginListener*  mListener;
};

#endif // CVS_XVIDEO_SOURCE_PLUGIN_HPP
