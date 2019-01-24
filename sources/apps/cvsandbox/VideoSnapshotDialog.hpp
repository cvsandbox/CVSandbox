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
#ifndef CVS_VIDEO_SNAPSHOT_DIALOG_HPP
#define CVS_VIDEO_SNAPSHOT_DIALOG_HPP

#include <QDialog>
#include <stdint.h>

namespace Ui
{
    class VideoSnapshotDialog;
}

namespace Private
{
    class VideoSnapshotDialogData;
}

class VideoSnapshotDialog : public QDialog
{
    Q_OBJECT

friend class Private::VideoSnapshotDialogData;

public:
    explicit VideoSnapshotDialog( QWidget* parent = nullptr, bool allowMinimize = false,
                                  QWidget* widgetToActivateOnClose = nullptr );
    ~VideoSnapshotDialog( );

    void TakeSnapshot( uint32_t videoSourceId, const QString& title );

protected:
    void closeEvent( QCloseEvent* event );
    bool event( QEvent* e );
    void mouseMoveEvent( QMouseEvent* ev );
    void leaveEvent( QEvent* event );

public slots:
    virtual void reject( );

private slots:
    void on_SnapshotTaken( );
    void on_ReloadAction_triggered( );
    void on_SaveAction_triggered( );
    void on_SetToClipboardAction_triggered( );
    void on_ColorSpaceBox_currentIndexChanged( int index );
    void on_SwitchToRgbAction_triggered( );
    void on_SwitchToNrgbAction_triggered( );
    void on_SwitchToHslAction_triggered( );
    void on_SwitchToHsvAction_triggered( );
    void on_SwitchRedHistogramAction_triggered( );
    void on_SwitchGreenHistogramAction_triggered( );
    void on_SwitchBlueHistogramAction_triggered( );

signals:
    void SnapshotTaken( );

private:
    Ui::VideoSnapshotDialog*          ui;
    Private::VideoSnapshotDialogData* mData;
};

#endif // CVS_VIDEO_SNAPSHOT_DIALOG_HPP
