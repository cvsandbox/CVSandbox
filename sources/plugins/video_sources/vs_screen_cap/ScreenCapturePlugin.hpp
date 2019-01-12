/*
    Screen capture plug-ins of Computer Vision Sandbox

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
#ifndef CVS_SCREEN_CAPTURE_PLUGIN_HPP
#define CVS_SCREEN_CAPTURE_PLUGIN_HPP

#include <iplugintypescpp.hpp>
#include <list>

namespace Private
{
    class ScreenCapturePluginData;
}

class ScreenCapturePlugin : public IVideoSourcePlugin
{
public:
    ScreenCapturePlugin( );
    virtual ~ScreenCapturePlugin( );

    // IPluginBase interface
    virtual void Dispose( );

    virtual XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    virtual XErrorCode SetProperty( int32_t id, const xvariant* value );

    // IVideoSource interface

    // Start video source so it initializes and begins providing video frames
    virtual XErrorCode Start( );
    // Signal video to stop, so it could finalize and clean-up
    virtual void SignalToStop( );
    // Wait till video source (its thread) stops
    virtual void WaitForStop( );
    // Check if video source (its thread) is still running
    virtual bool IsRunning( );

    // Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
    // by itself when signalled (ideally this method should not exist and be called at all)
    virtual void Terminate( );

    // Get number of frames received since the the start of the video source
    virtual uint32_t FramesReceived( );

    // Set callbacks for the video source
    virtual void SetCallbacks( const VideoSourcePluginCallbacks* callbacks, void* userParam );

public:
    static std::list<xrect> GetDisplays( );

private:
    ::Private::ScreenCapturePluginData* mData;
    static const PropertyDescriptor**   propertiesDescription;
};

#endif // CVS_SCREEN_CAPTURE_PLUGIN_HPP
