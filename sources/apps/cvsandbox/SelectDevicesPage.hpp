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
#ifndef CVS_SELECT_DEVICES_PAGE_HPP
#define CVS_SELECT_DEVICES_PAGE_HPP

#include <memory>
#include <map>
#include "WizardPageFrame.hpp"
#include "ProjectObject.hpp"

class QListWidgetItem;

namespace Ui
{
    class SelectDevicesPage;
}

namespace Private
{
    class SelectDevicesPageData;
}

class SelectDevicesPage : public WizardPageFrame
{
    Q_OBJECT

public:
    explicit SelectDevicesPage( QWidget *parent = 0 );
    ~SelectDevicesPage( );

    // Set list of available devices to choose from
    void SetAvailableDevices( const std::vector<std::shared_ptr<ProjectObject>>& devicesList );
    // Get list of the selected devices
    const std::vector<CVSandbox::XGuid> GetSelectedDevices( ) const;
    // Get FPS report state for the selected devices
    std::map<CVSandbox::XGuid, bool> GetDevicesFpsReportState( ) const;
    // Set list of the selected devices
    void SetSelectedDevices( const std::vector<CVSandbox::XGuid>& selectedDevices,
                             const std::map<CVSandbox::XGuid, bool>& devicesFpsReportState );

public:
    virtual bool CanGoNext( ) const;

private slots:
    void on_availableDevicesTree_ObjectSelected( const CVSandbox::XGuid& objectId );
    void on_availableDevicesTree_ObjectDoubleClicked( const CVSandbox::XGuid& objectId );
    void on_selectedDevicesList_itemDoubleClicked( QListWidgetItem* item );
    void on_addObjectButton_clicked( );
    void on_removeObjectButton_clicked( );
    void on_removeAllObjectsButton_clicked( );
    void on_enableFpsReportAction_triggered( );
    void on_selectedDevicesContextMenu_aboutToShow( );
    void on_selectedDevicesList_itemSelectionChanged( );
    void on_selectedDevicesList_customContextMenuRequested( const QPoint &pos );

protected:
    // Handle focus event
    virtual void focusInEvent( QFocusEvent* event );

private:
    void UpdateAddButtonStatus( );
    void UpdateAddButtonStatus( const CVSandbox::XGuid& selectedObjectId );
    bool IsObjectInTheSandbox( const CVSandbox::XGuid& objectId ) const;
    bool AddObjectToTheSandbox( const CVSandbox::XGuid& objectId, bool fpsReportState = true );
    void RemoveObjectFromTheSandbox( const CVSandbox::XGuid& objectId );
    void RemoveAllObjectsFromTheSandbox( );

private:
    Ui::SelectDevicesPage*          ui;
    Private::SelectDevicesPageData* mData;
};

#endif // CVS_SELECT_DEVICES_PAGE_HPP
