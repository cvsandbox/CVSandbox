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

#include "VideoSourcePlayer.hpp"

#include <QMutex>
#include <QMutexLocker>
#include <QWidget>
#include <QSize>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;

namespace Private
{
    // Class for private data
    class VideoSourcePlayerData
    {
    public:
        VideoSourcePlayerData( ) :
            SubscribedForEvents( false ), SyncMutex( ), AutoSize( false ), RespectAspectRatio( false ), PreferredSize( 320, 240 )
        {
        }

    public:
        bool    SubscribedForEvents;
        QMutex  SyncMutex;
        bool    AutoSize;
        bool    RespectAspectRatio;
        QSize   PreferredSize;
    };
}

VideoSourcePlayer::VideoSourcePlayer( ) :
    mVideoSource( ),
    mData( new Private::VideoSourcePlayerData( ) )
{
}

VideoSourcePlayer::~VideoSourcePlayer( )
{
    if ( mVideoSource )
    {
        mVideoSource->SetListener( 0 );
        mVideoSource->SignalToStop( );
        mVideoSource->WaitForStop( );
    }

    delete mData;
}

// Get/Set auto size mode
bool VideoSourcePlayer::AutoSize( ) const
{
    return mData->AutoSize;
}
void VideoSourcePlayer::SetAutoSize( bool autoSize )
{
    mData->AutoSize = autoSize;
    OnAutoSizeChanged( );
}

// Get/Set if aspect ratio must be respected or not (if no, then in non-auto size mode the control is resize to watever size specified)
bool VideoSourcePlayer::RespectAspectRatio( ) const
{
    return mData->RespectAspectRatio;
}
void VideoSourcePlayer::SetRespectAspectRatio( bool respect )
{
    mData->RespectAspectRatio = respect;
    if ( !mData->AutoSize )
    {
        ResizeToPreferredKeepingAspectRatio( );
    }
}

// Set new video source to play
void VideoSourcePlayer::SetVideoSource( const shared_ptr<IVideoSource>& videoSource )
{
    // we no longer want to receive any notifications from the previous video source if any
    UnsubscribeFromVideoSourceEvents( );

    // let derived class to do anything it may want to do
    OnNewVideoSource( );

    mVideoSource = videoSource;

    if ( mVideoSource )
    {
        mData->SubscribedForEvents = true;
        mVideoSource->SetListener( static_cast<IVideoSourceListener*>( this ) );
    }
}

// Get current video source of the player
shared_ptr<IVideoSource> VideoSourcePlayer::VideoSource( )
{
    return mVideoSource;
}

// Set/clear player highlight
void VideoSourcePlayer::SetHighlighted( bool )
{

}

// New video frame notification
void VideoSourcePlayer::OnNewImage( const shared_ptr<const XImage>& image )
{
    if ( mData->SubscribedForEvents )
    {
        OnNewImageImpl( image );
    }
}

// Video source error notification
void VideoSourcePlayer::OnError( const string& errorMessage )
{
    if ( mData->SubscribedForEvents )
    {
        OnErrorImpl( errorMessage );
    }
}

// Unsubsribe from NewFrame events, making sure the event handler is not doing anything now
void VideoSourcePlayer::UnsubscribeFromVideoSourceEvents( )
{
    if ( mVideoSource )
    {
        mData->SubscribedForEvents = false;
        // don't need any own lock for removing listener, since video source will not return if
        // calling a listener
        mVideoSource->SetListener( 0 );
    }
}

// Resize the control (set its min/max size to the specified)
void VideoSourcePlayer::SetContolSize( const QSize& size )
{
    if ( !mData->AutoSize )
    {
        mData->PreferredSize = size;

        ResizeToPreferredKeepingAspectRatio( );
    }
}

// The method which derived classes call, when control should be resized to the size set by client, but respecting NonScaledSize()
void VideoSourcePlayer::ResizeToPreferredKeepingAspectRatio( )
{
    QSize size = mData->PreferredSize;

    if ( mData->RespectAspectRatio )
    {
        const QSize& nonScaledSize = NonScaledSize( );
        double       xRatio        = static_cast<double>( size.width( ) )  / nonScaledSize.width( );
        double       yRatio        = static_cast<double>( size.height( ) ) / nonScaledSize.height( );
        double       ratio         = XMIN( xRatio, yRatio );

        size = nonScaledSize * ratio;
    }

    // does not look ideal - still some flickering is there (will leave it for now)

    PlayerWidget( )->setVisible( false );
    PlayerWidget( )->setUpdatesEnabled( false );

    PlayerWidget( )->setMaximumSize( size );
    PlayerWidget( )->setMinimumSize( size );

    PlayerWidget( )->setUpdatesEnabled( true );
    PlayerWidget( )->setVisible( true );
}
