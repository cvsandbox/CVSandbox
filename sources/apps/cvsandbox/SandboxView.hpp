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
#ifndef CVS_SANDBOX_VIEW_HPP
#define CVS_SANDBOX_VIEW_HPP

#include <memory>
#include <vector>
#include <map>
#include <XGuid.hpp>
#include "ProjectObjectViewFrame.hpp"

class CamerasViewConfiguration;
class SandboxSettings;

namespace Ui
{
    class SandboxView;
}

namespace Private
{
    class VideoMonitor;
    class SandboxViewData;
}

class SandboxView : public ProjectObjectViewFrame
{
    Q_OBJECT

    friend class Private::VideoMonitor;

public:
    explicit SandboxView( const CVSandbox::XGuid& objectId, QWidget *parent = 0 );
    ~SandboxView( );

    // Set configuration of views
    void SetViews( const std::vector<CamerasViewConfiguration>& views,
                   const CVSandbox::XGuid& defaultViewId,
                   const std::map<CVSandbox::XGuid, uint32_t>& videoSourceMap,
                   const SandboxSettings& settings );
    // Set IDs of threads running for the sandbox
    void SetThreads( const std::map<CVSandbox::XGuid, uint32_t>& threadsMap );

    // Set preferred size of the project's view content or let it auto size
    void SetPreferredContentSize( bool autoSize, const QSize& size );

signals:
    // Dirty hack, so we could queue some internal events on the UI thread
    void OnThumbnailImageUpdateRequest( CVSandbox::XGuid cameraObjectId );

public slots:
    void on_HelpRequested( QWidget* sender );

private slots:
    void gridButton_clicked( QObject* sender );
    void me_OnThumbnailImageUpdateRequest( CVSandbox::XGuid cameraObjectId );
    void me_RotationTimer_timeout( );
    void view_CameraMousePress( int row, int column, bool isLeftButton, QPoint globalPos );
    void menuCamera_aboutToShow( );
    void actionCameraInfo_triggered( );
    void actionCameraSettings_triggered( );
    void actionVideoProcessingInfo_triggered( );
    void actionTakeSnapshot_triggered( );

    void shortcut1_activated( );
    void shortcut2_activated( );
    void shortcut3_activated( );
    void shortcut4_activated( );
    void shortcut5_activated( );
    void shortcut6_activated( );
    void shortcut7_activated( );
    void shortcut8_activated( );
    void shortcut9_activated( );

private:
    Ui::SandboxView*            ui;
    Private::SandboxViewData*   mData;
};

#endif // CVS_SANDBOX_VIEW_HPP
