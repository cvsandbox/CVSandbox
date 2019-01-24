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
#ifndef CVS_CAMERA_INFO_DIALOG_HPP
#define CVS_CAMERA_INFO_DIALOG_HPP

#include <QDialog>
#include <XGuid.hpp>

namespace Ui
{
    class CameraInfoDialog;
}

namespace Private
{
    class CameraInfoDialogData;
}

class CameraInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CameraInfoDialog( QWidget* parent = 0 );
    ~CameraInfoDialog( );

    // Set Id of the camera to display information for
    void SetCameraId( const CVSandbox::XGuid& cameraId, uint32_t videoSourceId );

protected:
    void closeEvent( QCloseEvent* event );

public slots:
    virtual void accept( );
    virtual void reject( );

private slots:
    void on_closeButton_clicked( );
    void on_FrameInfoUpdateTimer_timeout( );

private:
    Ui::CameraInfoDialog*          ui;
    Private::CameraInfoDialogData* mData;
};

#endif // CVS_CAMERA_INFO_DIALOG_HPP
