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
#ifndef CVS_VIDEOSOURCEPLAYER_HPP
#define CVS_VIDEOSOURCEPLAYER_HPP

#include <IVideoSource.hpp>

class QWidget;
class QSize;

namespace Private
{
    class VideoSourcePlayerData;
}

class VideoSourcePlayer : private CVSandbox::Video::IVideoSourceListener
{
public:
    explicit VideoSourcePlayer( );
    virtual ~VideoSourcePlayer( );

    // Get/Set auto size mode
    bool AutoSize( ) const;
    void SetAutoSize( bool autoSize );

    // Get/Set if aspect ratio must be respected or not (if no, then in non-auto size mode the control is resize to whatever size specified)
    bool RespectAspectRatio( ) const;
    void SetRespectAspectRatio( bool respect );

    // Helper method to resize the control (sets its min/max size to the specified)
    void SetContolSize( const QSize& size );

    // Set new video source to play
    void SetVideoSource( const std::shared_ptr<CVSandbox::Video::IVideoSource>& videoSource );
    // Get current video source of the player
    std::shared_ptr<CVSandbox::Video::IVideoSource> VideoSource( );
    // Get UI widget of the the player
    virtual QWidget* PlayerWidget( ) = 0;

    // Set/clear player highlight
    virtual void SetHighlighted( bool highlighted );

protected:
    // On auto size mode changed
    virtual void OnAutoSizeChanged( ) = 0;
    // On new video source is set
    virtual void OnNewVideoSource( ) = 0;
    // On new video frame arrived
    virtual void OnNewImageImpl( const std::shared_ptr<const CVSandbox::XImage>& image ) = 0;
    // On new error message from video source
    virtual void OnErrorImpl( const std::string& errorMessage ) = 0;
    // Unsubscribe from NewFrame events, making sure the event handler is not doing anything now
    void UnsubscribeFromVideoSourceEvents( );

    // Provide the size, which control would take if it was in auto size mode (the side which show video image without scaling)
    virtual QSize NonScaledSize( ) = 0;
    // The method which derived classes call, when control should be resized to the size set by client, but respecting NonScaledSize()
    void ResizeToPreferredKeepingAspectRatio( );

private: // IVideoSourceListener

    // New video frame notification
    virtual void OnNewImage( const std::shared_ptr<const CVSandbox::XImage>& image );
    // Video source error notification
    virtual void OnError( const std::string& errorMessage );

private:
    std::shared_ptr<CVSandbox::Video::IVideoSource> mVideoSource;
    Private::VideoSourcePlayerData*                 mData;
};

#endif // CVS_VIDEOSOURCEPLAYER_HPP
