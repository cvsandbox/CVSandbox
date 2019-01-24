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
#ifndef CVS_ADD_CAMERA_DIALOG_HPP
#define CVS_ADD_CAMERA_DIALOG_HPP

#include <map>
#include <XGuid.hpp>
#include <XVariant.hpp>

#include "WizardDialog.hpp"

namespace Private
{
    class AddCameraDialogData;
}

class AddCameraDialog : public WizardDialog
{
    Q_OBJECT

public:
    explicit AddCameraDialog( const CVSandbox::XGuid& parentObjetId, QWidget* parentWidget = 0 );
    ~AddCameraDialog( );

    // Get name of the camera to add
    const std::string GetCameraName( ) const;
    // Get description of the camera to add
    const std::string GetCameraDescription( ) const;
    // Get video source plug-in ID for the camera
    const CVSandbox::XGuid GetSelectedPluginID( ) const;
    // Get configuration (properties) of the video source plug-in
    const std::map<std::string, CVSandbox::XVariant> GetPluginConfiguration( ) const;

protected:
    // Configuration page change event handler
    virtual void OnChangingPage( int nextPage );

private:
    Private::AddCameraDialogData* mData;
};

#endif // CVS_ADD_CAMERA_DIALOG_HPP
