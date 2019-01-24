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
#ifndef CVS_SINGLE_CAMERA_VIEW_HPP
#define CVS_SINGLE_CAMERA_VIEW_HPP

#include "ProjectObjectViewFrame.hpp"
#include "VideoSourceInAutomationServer.hpp"

namespace Ui
{
    class SingleCameraView;
}

namespace Private
{
    class SingleCameraViewData;
}

class SingleCameraView : public ProjectObjectViewFrame
{
    Q_OBJECT
    
public:
    explicit SingleCameraView( const CVSandbox::XGuid& objectId, QWidget* parent = 0 );
    ~SingleCameraView( );

    // Set video source to display
    void SetVideoSource( const std::shared_ptr<VideoSourceInAutomationServer>& videoSource );

    // Set preferred size of the project's view content or let it autosize
    void SetPreferredContentSize( bool autoSize, const QSize& size = QSize( ) );

public:
    virtual bool eventFilter( QObject* target, QEvent* event );

public slots:
    void on_HelpRequested( QWidget* sender );

private slots:
    void menuCamera_aboutToShow( );
    void actionCameraInfo_triggered( );
    void actionCameraSettings_triggered( );
    void actionTakeSnapshot_triggered( );

private:
    Ui::SingleCameraView*           ui;
    Private::SingleCameraViewData*  mData;
};

#endif // CVS_SINGLE_CAMERA_VIEW_HPP
