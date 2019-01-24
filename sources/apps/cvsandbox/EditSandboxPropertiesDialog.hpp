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
#ifndef CVS_EDIT_SANDBOX_PROPERTIES_DIALOG_HPP
#define CVS_EDIT_SANDBOX_PROPERTIES_DIALOG_HPP

#include "PropertyPagesDialog.hpp"
#include "SandboxProjectObject.hpp"

namespace Private
{
    class EditSandboxPropertiesDialogData;
}

class EditSandboxPropertiesDialog : public PropertyPagesDialog
{
    Q_OBJECT

public:
    explicit EditSandboxPropertiesDialog( const std::shared_ptr<SandboxProjectObject>& po,
                                          QWidget* parent = 0 );
    ~EditSandboxPropertiesDialog( );

    // Set list of devices to choose from
    void SetAvailableDevices( const std::vector<std::shared_ptr<ProjectObject>>& camerasList );

protected:
    void ApplyCurrentPage( int index );
    void ApplyAllPages( );

private:
    bool ApplyDescriptionPage( );
    bool ApplySelectedDevicesPage( );
    bool ApplyViewConfigurationPage( );
    bool ApplySettingsPage( );

private:
    Private::EditSandboxPropertiesDialogData* mData;
};

#endif // CVS_EDIT_SANDBOX_PROPERTIES_DIALOG_HPP
