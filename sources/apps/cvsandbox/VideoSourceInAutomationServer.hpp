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
#ifndef CVS_VIDEO_SOURCE_IN_AUTOMATION_SERVER_HPP
#define CVS_VIDEO_SOURCE_IN_AUTOMATION_SERVER_HPP

#include <memory>
#include <IVideoSource.hpp>
#include <XInterfaces.hpp>
#include <XAutomationServer.hpp>

class VideoSourceInAutomationServer : public CVSandbox::Video::IVideoSource,
                                      public CVSandbox::Automation::IAutomationVideoSourceListener,
                                      private CVSandbox::Uncopyable
{
private:
    VideoSourceInAutomationServer(
            const std::shared_ptr<CVSandbox::Automation::XAutomationServer>& server,
            uint32_t videoSourceId );

public:
    static std::shared_ptr<VideoSourceInAutomationServer> Create(
            const std::shared_ptr<CVSandbox::Automation::XAutomationServer>& server,
            uint32_t videoSourceId );

    uint32_t VideoSourceId( ) const;

public:     // IVideoSource interface

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

public:     // IAutomationVideoSourceListener interface

    virtual void OnNewVideoFrame( uint32_t videoSourceId, const std::shared_ptr<const CVSandbox::XImage>& image );
    virtual void OnErrorMessage( uint32_t videoSourceId, const std::string& errorMessage );

private:
    const std::shared_ptr<CVSandbox::Automation::XAutomationServer> mServer;
    uint32_t                                                        mVideoSourceId;
    CVSandbox::Video::IVideoSourceListener*                         mListener;
};

#endif // CVS_VIDEO_SOURCE_IN_AUTOMATION_SERVER_HPP
