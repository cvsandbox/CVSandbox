/*
    Video source interface library of Computer Vision Sandbox

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
#ifndef CVS_IVIDEO_SOURCE_HPP
#define CVS_IVIDEO_SOURCE_HPP

#include <xtypes.h>
#include <XImage.hpp>
#include "IVideoSourceListener.hpp"

namespace CVSandbox { namespace Video
{

// Interface of video source continuously providing frames to display/process
class IVideoSource
{
public:
    virtual ~IVideoSource( ) { }

    // Start video source so it initializes and begins providing video frames
    virtual XErrorCode Start( ) = 0;
    // Signal video to stop, so it could finalize and cleanup
    virtual void SignalToStop( ) = 0;
    // Wait till video source (its thread) stops
    virtual void WaitForStop( ) = 0;
    // Check if video source (its thread) is still running
    virtual bool IsRunning( ) = 0;

    // Terminate video source - call *ONLY* if video source looks to be frozen and does not stop
    // by itself when signaled (ideally this method should not exist and be called at all)
    virtual void Terminate( ) = 0;

    // Get number of frames received since the the start of the video source
    virtual uint32_t FramesReceived( ) = 0;

    // Set video source listener
    virtual void SetListener( IVideoSourceListener* listener ) = 0;
};

} } // namespace CVSandbox::Video

#endif // CVS_IVIDEO_SOURCE_HPP
