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
#ifndef CVS_ADD_SANDBOX_DIALOG_HPP
#define CVS_ADD_SANDBOX_DIALOG_HPP

#include <memory>
#include <map>
#include "XGuid.hpp"
#include "WizardDialog.hpp"
#include "ProjectObject.hpp"

class CamerasViewConfiguration;

namespace Private
{
    class AddSandboxDialogData;
}

class AddSandboxDialog : public WizardDialog
{
    Q_OBJECT

public:
    explicit AddSandboxDialog( const CVSandbox::XGuid& parentObjectId, QWidget* parentWidget = 0 );
    ~AddSandboxDialog( );

    // Set list of devices to choose from
    void SetAvailableDevices( const std::vector<std::shared_ptr<ProjectObject>>& camerasList );

    // Get name of the sandbox to add
    const std::string GetSandboxName( ) const;
    // Get description of the sandbox to add
    const std::string GetSandboxDescription( ) const;

    // Get list of sandbox devices
    const std::vector<CVSandbox::XGuid> GetSandboxDevices( ) const;
    // Get FPS report state for the selected devices
    std::map<CVSandbox::XGuid, bool> GetDevicesFpsReportState( ) const;
    // Get default view configuration
    const std::vector<CamerasViewConfiguration> GetViewConfigurations( ) const;
    // Get default view ID
    const CVSandbox::XGuid DefaultViewId( ) const;

protected:
    // Configuration page change event handler
    virtual void OnChangingPage( int nextPage );

private:
    Private::AddSandboxDialogData* mData;
};

#endif // CVS_ADD_SANDBOX_DIALOG_HPP
