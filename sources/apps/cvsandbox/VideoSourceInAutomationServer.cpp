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

#include <assert.h>
#include <QObject>
#include "VideoSourceInAutomationServer.hpp"

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;
using namespace CVSandbox::Automation;

VideoSourceInAutomationServer::VideoSourceInAutomationServer( const shared_ptr<XAutomationServer>& server, uint32_t videoSourceId ) :
    mServer( server ), mVideoSourceId( videoSourceId ), mListener( 0 )
{
}

shared_ptr<VideoSourceInAutomationServer> VideoSourceInAutomationServer::Create(
        const shared_ptr<XAutomationServer>& server, uint32_t videoSourceId )
{
    return shared_ptr<VideoSourceInAutomationServer>( new (nothrow) VideoSourceInAutomationServer( server, videoSourceId ) );
}

// Get ID of the video source
uint32_t VideoSourceInAutomationServer::VideoSourceId( ) const
{
    return mVideoSourceId;
}

// Do nothing for the below method, since video source's life cycle is controlled by automation server
// ---------------------------------------------------------------------------------------------------
XErrorCode VideoSourceInAutomationServer::Start( )
{
    return SuccessCode;
}
void VideoSourceInAutomationServer::SignalToStop( )
{
}
void VideoSourceInAutomationServer::WaitForStop( )
{
}
void VideoSourceInAutomationServer::Terminate( )
{
}
bool VideoSourceInAutomationServer::IsRunning( )
{
    return true;
}
// ===================================================================================================

// Get number of frames received since the the start of the video source
uint32_t VideoSourceInAutomationServer::FramesReceived( )
{
    return 0;
}

// Set video source listener
void VideoSourceInAutomationServer::SetListener( IVideoSourceListener* listener )
{
    mServer->RemoveVideoSourceListener( mVideoSourceId, static_cast<IAutomationVideoSourceListener*>( this ) ) ;

    mListener = listener;

    if ( mListener != 0 )
    {
        mServer->AddVideoSourceListener( mVideoSourceId, static_cast<IAutomationVideoSourceListener*>( this ) ) ;
    }
}


// New image is received from automation server - pass it to user
void VideoSourceInAutomationServer::OnNewVideoFrame( uint32_t videoSourceId, const shared_ptr<const XImage>& image )
{
    assert( mVideoSourceId == videoSourceId );

    if ( ( mVideoSourceId == videoSourceId ) && ( mListener != 0 ) )
    {
        mListener->OnNewImage( image );
    }
}

// Error message is received from automation server
void VideoSourceInAutomationServer::OnErrorMessage( uint32_t videoSourceId, const string& errorMessage )
{
    assert( mVideoSourceId == videoSourceId );

    if ( ( mVideoSourceId == videoSourceId ) && ( mListener != 0 ) )
    {
        mListener->OnError( errorMessage );
    }
}
