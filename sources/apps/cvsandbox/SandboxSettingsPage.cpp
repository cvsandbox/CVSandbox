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

#include "SandboxSettingsPage.hpp"
#include "ui_SandboxSettingsPage.h"

#include "SandboxSettings.hpp"

namespace Private
{
    class SandboxSettingsPageData
    {
    public:
        SandboxSettingsPageData( Ui::SandboxSettingsPage* parentUi ) :
            ui( parentUi ), settings( )
        {
        }

    private:
        Ui::SandboxSettingsPage* ui;

    public:
        SandboxSettings settings;
    };
}

SandboxSettingsPage::SandboxSettingsPage( QWidget* parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::SandboxSettingsPage ),
    mData( new ::Private::SandboxSettingsPageData( ui ) )
{
    ui->setupUi( this );

    ui->viewButtonsPositionComboBox->addItem( "Vertical (Right)" );
    ui->viewButtonsPositionComboBox->addItem( "Horizontal (Bottom)" );
    ui->viewButtonsPositionComboBox->setCurrentIndex( 0 );

    ui->viewButtonsAlignmentComboBox->addItem( "Top/Left" );
    ui->viewButtonsAlignmentComboBox->addItem( "Center" );
    ui->viewButtonsAlignmentComboBox->addItem( "Bottom/Right" );
    ui->viewButtonsAlignmentComboBox->setCurrentIndex( 1 );
}

SandboxSettingsPage::~SandboxSettingsPage( )
{
    delete mData;
    delete ui;
}

// Always return true, since sittings on this page are not critical
bool SandboxSettingsPage::CanGoNext( ) const
{
    return true;
}

// Get/Set sanbox settings
const SandboxSettings SandboxSettingsPage::Settings( ) const
{
    return mData->settings;
}
void SandboxSettingsPage::SetSettings( const SandboxSettings& settings )
{
    mData->settings = settings;

    ui->viewRotationCheckBox->setChecked( settings.IsViewsRotationEnabled( ) );
    ui->rotationStartSpinBox->setValue( settings.RotationStartTime( ) );
    ui->rotationTimeSpinBox->setValue( settings.ViewsRotationTime( ) );
    ui->viewButtonsPositionComboBox->setCurrentIndex( static_cast<int>( settings.GetViewButtonsPosition( ) ) );
    ui->viewButtonsAlignmentComboBox->setCurrentIndex( static_cast<int>( settings.GetViewButtonsAlignment( ) ) );
    ui->dropFramesCheckBox->setChecked( settings.DropFramesOnSlowProcessing( ) );
}

void SandboxSettingsPage::on_viewRotationCheckBox_clicked( bool checked )
{
    mData->settings.EnableViewsRotation( checked );
}

void SandboxSettingsPage::on_rotationStartSpinBox_valueChanged( int value )
{
    mData->settings.SetRotationStartTime( value );
}

void SandboxSettingsPage::on_rotationTimeSpinBox_valueChanged( int value )
{
    mData->settings.SetViewsRotationTime( value );
}

void SandboxSettingsPage::on_viewButtonsPositionComboBox_currentIndexChanged( int index )
{
    mData->settings.SetViewButtonsPosition( static_cast<SandboxSettings::ViewButtonsPosition>( index ) );
}

void SandboxSettingsPage::on_viewButtonsAlignmentComboBox_currentIndexChanged( int index )
{
    mData->settings.SetViewButtonsAlignment( static_cast<SandboxSettings::ViewButtonsAlignment>( index ) );
}

void SandboxSettingsPage::on_dropFramesCheckBox_clicked( bool checked )
{
    mData->settings.SetDropFramesOnSlowProcessing( checked );
}
