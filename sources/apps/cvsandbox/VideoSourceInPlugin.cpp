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

#include "VideoSourceInPlugin.hpp"
#include <assert.h>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;

VideoSourceInPlugin::VideoSourceInPlugin( const shared_ptr<XVideoSourcePlugin>& plugin ) :
    mPlugin( plugin ), mListener( 0 )
{
}

shared_ptr<VideoSourceInPlugin> VideoSourceInPlugin::Create( const shared_ptr<XVideoSourcePlugin>& plugin )
{
    return shared_ptr<VideoSourceInPlugin>( new VideoSourceInPlugin( plugin ) );
}

// Start video source
XErrorCode VideoSourceInPlugin::Start( )
{
    XErrorCode ret = ErrorInvalidConfiguration;
    assert( mPlugin );

    if ( mPlugin )
    {
        ret = mPlugin->Start( );
    }

    return ret;
}

// Signal video source to stop
void VideoSourceInPlugin::SignalToStop( )
{
    assert( mPlugin );

    if ( mPlugin )
    {
        mPlugin->SignalToStop( );
    }
}

// Wait till video source stops
void VideoSourceInPlugin::WaitForStop( )
{
    assert( mPlugin );

    if ( mPlugin )
    {
        mPlugin->WaitForStop( );
    }
}

// Check if video source is running
bool VideoSourceInPlugin::IsRunning( )
{
    bool isRunning = false;
    assert( mPlugin );

    if ( mPlugin )
    {
        isRunning = mPlugin->IsRunning( );
    }

    return isRunning;
}

// Terminate video source (in the case it is not willing to stop)
void VideoSourceInPlugin::Terminate( )
{
    assert( mPlugin );

    if ( mPlugin )
    {
        mPlugin->Terminate( );
    }
}

// Get number of frames received since the the start of the video source
uint32_t VideoSourceInPlugin::FramesReceived( )
{
    int frames = 0;

    assert( mPlugin );

    if ( mPlugin )
    {
        frames = mPlugin->FramesReceived( );
    }

    return frames;
}

// Set video source listener
void VideoSourceInPlugin::SetListener( IVideoSourceListener* listener )
{
    if ( mPlugin )
    {
        // first make sure plug-in stops calling us
        mPlugin->SetListener( 0 );

        // update listener
        mListener = listener;

        if ( listener != 0 )
        {
            // subscribe to plug-in again
            mPlugin->SetListener( static_cast<IVideoSourcePluginListener*>( this ) );
        }
    }
}

// New video frame notification
void VideoSourceInPlugin::OnNewImage( const shared_ptr<const XImage>& image )
{
    if ( mListener != 0 )
    {
        mListener->OnNewImage( image );
    }
}

// Video source error notification
void VideoSourceInPlugin::OnError( const string& errorMessage )
{
    if ( mListener != 0 )
    {
        mListener->OnError( errorMessage );
    }
}
