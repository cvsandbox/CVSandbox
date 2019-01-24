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
#ifndef CVS_VIDEOSOURCEPLAYERQT_HPP
#define CVS_VIDEOSOURCEPLAYERQT_HPP

#include <QWidget>
#include "VideoSourcePlayer.hpp"

namespace Private
{
    class VideoSourcePlayerQtData;
}

class VideoSourcePlayerQt : public QWidget, public VideoSourcePlayer
{
    Q_OBJECT

public:
    explicit VideoSourcePlayerQt( QWidget* parent = 0 );
    virtual ~VideoSourcePlayerQt( );

    // Get UI widget of the the player
    virtual QWidget* PlayerWidget( );

    // Set/clear player highlight
    virtual void SetHighlighted( bool highlighted );

private:
    // On auto size mode changed
    virtual void OnAutoSizeChanged( );
    // On new video source is set
    virtual void OnNewVideoSource( );
    // On new video frame arrived
    virtual void OnNewImageImpl( const std::shared_ptr<const CVSandbox::XImage>& image );
    // On new error message from video source
    virtual void OnErrorImpl( const std::string& errorMessage );

    // Provide the size, which control would take if it was in auto size mode (the side which show video image without scaling)
    virtual QSize NonScaledSize( );

private:
    virtual void paintEvent( QPaintEvent* event );
    virtual void enterEvent( QEvent* event );
    virtual void leaveEvent( QEvent* event );

private:
    void SetMyContolSize( );

signals:
    void OnUpdateRequired( );

private:
    Private::VideoSourcePlayerQtData* mData;
};

#endif // CVS_VIDEOSOURCEPLAYERQT_HPP
