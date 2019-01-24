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
#ifndef CVS_IMAINWINDOWSERVICE_HPP
#define CVS_IMAINWINDOWSERVICE_HPP

#include <XGuid.hpp>

class QWidget;
class QMenu;
class QString;

class IMainWindowService
{
public:
    virtual ~IMainWindowService( ) { }

    // Get main window of the application
    virtual QWidget* GetMainAppWindow( ) const = 0;

    // Set status message displayed in the status bar of the main window
    virtual void SetStatusMessage( const QString& statusMessage ) const = 0;

    // Set sub-title - name of open file/object/whatever
    virtual void SetSubTitle( const QString& subTitle ) = 0;

    // Set widget to be display in the main area
    virtual void SetCentralWidget( QWidget* widget ) = 0;

    // Get project object's ID for the central widget
    virtual const CVSandbox::XGuid CentralWidgetsObjectId( ) const = 0;

    // Take snapshot of the specified video source
    virtual void TakeSnapshotForVideoSource( uint32_t videoSourceId, const QString& title ) = 0;
};

#endif // CVS_IMAINWINDOWSERVICE_HPP
