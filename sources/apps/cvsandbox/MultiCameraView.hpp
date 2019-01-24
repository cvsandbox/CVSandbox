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
#ifndef CVS_MULTI_CAMERA_VIEW_HPP
#define CVS_MULTI_CAMERA_VIEW_HPP

#include <memory>
#include <map>
#include <QFrame>
#include <QMouseEvent>
#include <XGuid.hpp>

class CamerasViewConfiguration;

namespace Ui
{
    class MultiCameraView;
}

namespace Private
{
    class MultiCameraViewData;
}

class MultiCameraView : public QFrame
{
    Q_OBJECT

public:
    explicit MultiCameraView( QWidget* parent = 0 );
    ~MultiCameraView( );

    // Set view configuration
    void SetView( const CamerasViewConfiguration& view,
                  const std::map<CVSandbox::XGuid, uint32_t>& videoSourceMap,
                  bool autoSizeView, const QSize& preferredSize );
    // Set/clear specified cell highlighting
    void HighlightCell( int row, int column, bool highlight );

    // Set preferred size of the total area available for cameras' view
    void SetPreferredContentSize( bool autoSize, const QSize& size );

public:
    virtual bool eventFilter( QObject* target, QEvent* event );

signals:
    void CameraMousePress( int row, int column, bool isLeftButton, QPoint globalPos );

private slots:
    void me_NotifyMouseClick( );

private:
    Ui::MultiCameraView*            ui;
    Private::MultiCameraViewData*   mData;
};

#endif // CVS_MULTI_CAMERA_VIEW_HPP
