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
#ifndef CVS_MAIN_VIEW_FRAME_HPP
#define CVS_MAIN_VIEW_FRAME_HPP

#include <QFrame>
#include <XGuid.hpp>

namespace Ui
{
    class MainViewFrame;
}

namespace Private
{
    class MainViewFrameData;
}

class MainViewFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MainViewFrame( QWidget* parent = 0 );
    ~MainViewFrame( );

    // Get/Set content widget of the view
    QWidget* ContentWidget( ) const;
    void SetContentWidget( QWidget* widget );

    // Get ID of the project object shown in the view (if any)
    const CVSandbox::XGuid DisplayedProjectObjectId( ) const;

    // Enable content fitting to the widget
    void EnableContentFitting( bool enable );

protected:
    virtual void resizeEvent( QResizeEvent* event );

private:
    void UpdateContentSize( );

private:
    Ui::MainViewFrame*          ui;
    Private::MainViewFrameData* mData;
};

#endif // CVS_MAIN_VIEW_FRAME_HPP
