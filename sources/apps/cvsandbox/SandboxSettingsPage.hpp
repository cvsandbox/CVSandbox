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
#ifndef CVS_SANDBOX_SETTINGS_PAGE_HPP
#define CVS_SANDBOX_SETTINGS_PAGE_HPP

#include "WizardPageFrame.hpp"

class SandboxSettings;

namespace Ui
{
    class SandboxSettingsPage;
}

namespace Private
{
    class SandboxSettingsPageData;
}

class SandboxSettingsPage : public WizardPageFrame
{
    Q_OBJECT

public:
    explicit SandboxSettingsPage( QWidget* parent = 0 );
    ~SandboxSettingsPage( );

    // Get/Set sanbox settings
    const SandboxSettings Settings( ) const;
    void SetSettings( const SandboxSettings& settings );

public:
    virtual bool CanGoNext( ) const;

private slots:
    void on_viewRotationCheckBox_clicked( bool checked );
    void on_rotationStartSpinBox_valueChanged( int value );
    void on_rotationTimeSpinBox_valueChanged( int value );
    void on_viewButtonsPositionComboBox_currentIndexChanged( int index );
    void on_viewButtonsAlignmentComboBox_currentIndexChanged( int index );
    void on_dropFramesCheckBox_clicked( bool checked );

private:
    Ui::SandboxSettingsPage*            ui;
    Private::SandboxSettingsPageData*   mData;
};

#endif // CVS_SANDBOX_SETTINGS_PAGE_HPP
