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

#include <assert.h>
#include <QIcon>
#include "EditSandboxPropertiesDialog.hpp"
#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"

#include "ProjectObjectNamePage.hpp"
#include "SelectDevicesPage.hpp"
#include "ConfigureCamerasViewPage.hpp"
#include "SandboxSettingsPage.hpp"

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class EditSandboxPropertiesDialogData
    {
    public:
        EditSandboxPropertiesDialogData( const shared_ptr<SandboxProjectObject>& po ) :
            SandboxToEdit( po ),
            sandboxNamePage( 0 )
        {
        }

    public:
        shared_ptr<SandboxProjectObject> SandboxToEdit;

    public:
        ProjectObjectNamePage*    sandboxNamePage;
        SelectDevicesPage*        selectDevicesPage;
        ConfigureCamerasViewPage* configureCamerasViewPage;
        SandboxSettingsPage*      sandboxSettingsPage;
    };
}

EditSandboxPropertiesDialog::EditSandboxPropertiesDialog( const shared_ptr<SandboxProjectObject>& po,
                                                          QWidget* parent ) :
    PropertyPagesDialog( "EditSandboxPropertie", parent ),
    mData( new Private::EditSandboxPropertiesDialogData( po ) )
{
    assert( po );

    this->setWindowTitle( "Edit sandbox properties" );
    this->setWindowIcon( QIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Sandbox ) ) );

    // set up pages
    mData->sandboxNamePage          = new ProjectObjectNamePage( this );
    mData->selectDevicesPage        = new SelectDevicesPage( this );
    mData->configureCamerasViewPage = new ConfigureCamerasViewPage( this );
    mData->sandboxSettingsPage      = new SandboxSettingsPage( this );

    mData->sandboxNamePage->SetObjectProperties( po->ParentId( ), po->Name( ), po->Description( ) );
    mData->sandboxNamePage->SetObjectIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Sandbox, true ) );
    mData->sandboxNamePage->SetObjectTypeName( QString::fromUtf8( "Sandbox" ) );

    mData->sandboxSettingsPage->SetSettings( po->Settings( ) );

    // add pages to the property dialog
    AddPage( mData->sandboxNamePage );
    AddPage( mData->selectDevicesPage );
    AddPage( mData->configureCamerasViewPage );
    AddPage( mData->sandboxSettingsPage );
}

EditSandboxPropertiesDialog::~EditSandboxPropertiesDialog( )
{
    delete mData;
}

// Set list of devices to choose from
void EditSandboxPropertiesDialog::SetAvailableDevices( const vector<shared_ptr<ProjectObject>>& camerasList )
{
    const vector<XGuid>    selectedDevices       = mData->SandboxToEdit->GetSandboxDevices( );
    const map<XGuid, bool> devicesFpsReportState = mData->SandboxToEdit->GetDeviceFpsReportState( );

    mData->selectDevicesPage->SetAvailableDevices( camerasList );
    mData->selectDevicesPage->SetSelectedDevices( selectedDevices, devicesFpsReportState );

    mData->configureCamerasViewPage->SetAvalableCameras( selectedDevices );
    mData->configureCamerasViewPage->SetViewConfigurations( mData->SandboxToEdit->GetViews( ) );
    mData->configureCamerasViewPage->SetDefaultViewId( mData->SandboxToEdit->DefaultViewId( ) );
}

// Apply currently active page
void EditSandboxPropertiesDialog::ApplyCurrentPage( int index )
{
    bool changeDetected = false;

    switch ( index )
    {
    case 0:
        changeDetected = ApplyDescriptionPage( );
        break;

    case 1:
        changeDetected = ApplySelectedDevicesPage( );
        break;

    case 2:
        changeDetected = ApplyViewConfigurationPage( );
        break;

    case 3:
        changeDetected = ApplySettingsPage( );
        break;

    default:
        Q_ASSERT( false );
        break;
    }

    if ( changeDetected )
    {
        qDebug( "something changed" );
        ServiceManager::Instance( ).GetProjectManager( )->UpdateProjectObject( mData->SandboxToEdit );
    }
    else
    {
        qDebug( "Nothing change" );
    }
}

// Apply all pages - OK button
void EditSandboxPropertiesDialog::ApplyAllPages( )
{
    bool changeDetected = false;

    changeDetected |= ApplyDescriptionPage( );
    changeDetected |= ApplySelectedDevicesPage( );
    changeDetected |= ApplyViewConfigurationPage( );
    changeDetected |= ApplySettingsPage( );

    if ( changeDetected )
    {
        qDebug( "something changed" );
        ServiceManager::Instance( ).GetProjectManager( )->UpdateProjectObject( mData->SandboxToEdit );
    }
    else
    {
        qDebug( "Nothing change" );
    }
}

// Apply changes done on the project object description page
bool EditSandboxPropertiesDialog::ApplyDescriptionPage( )
{
    string newName        = mData->sandboxNamePage->GetObjectName( );
    string newDescription = mData->sandboxNamePage->GetObjectDescription( );
    bool   changeDetected = false;

    if ( mData->SandboxToEdit->Name( ) != newName )
    {
        mData->SandboxToEdit->SetName( newName );
        changeDetected |= true;
    }

    if ( mData->SandboxToEdit->Description( ) != newDescription )
    {
        mData->SandboxToEdit->SetDescription( mData->sandboxNamePage->GetObjectDescription( ) );
        changeDetected |= true;
    }

    return changeDetected;
}

// Apply changes done on the sandbox devices selection page
bool EditSandboxPropertiesDialog::ApplySelectedDevicesPage( )
{
    vector<XGuid>    newSelectedDevices = mData->selectDevicesPage->GetSelectedDevices( );
    map<XGuid, bool> newFpsReportStates = mData->selectDevicesPage->GetDevicesFpsReportState( );
    bool             changeDetected     = false;

    if ( ( newSelectedDevices != mData->SandboxToEdit->GetSandboxDevices( ) ) ||
         ( newFpsReportStates != mData->SandboxToEdit->GetDeviceFpsReportState( ) ) )
    {
        mData->SandboxToEdit->SetSandboxDevices( newSelectedDevices );
        mData->SandboxToEdit->SetDeviceFpsReportState( newFpsReportStates );
        mData->configureCamerasViewPage->SetAvalableCameras( newSelectedDevices );
        changeDetected = true;
    }

    return changeDetected;
}

// Apply changes done on the view configuration page
bool EditSandboxPropertiesDialog::ApplyViewConfigurationPage( )
{
    vector<CamerasViewConfiguration> views          = mData->SandboxToEdit->GetViews( );
    vector<CamerasViewConfiguration> newViews       = mData->configureCamerasViewPage->GetViewConfigurations( );
    XGuid                            defaultView    = mData->configureCamerasViewPage->DefaultViewId( );
    bool                             changeDetected = false;

    if ( newViews != views )
    {
        mData->SandboxToEdit->SetViews( newViews );
        changeDetected = true;
    }

    if ( defaultView != mData->SandboxToEdit->DefaultViewId( ) )
    {
        mData->SandboxToEdit->SetDefaultViewId( defaultView );
        changeDetected = true;
    }

    return changeDetected;
}

// Apply changes done on the sandbox settings page
bool EditSandboxPropertiesDialog::ApplySettingsPage( )
{
    SandboxSettings newSettings     = mData->sandboxSettingsPage->Settings( );
    bool            changeDetected  = false;

    if ( mData->SandboxToEdit->Settings( ) != newSettings )
    {
        mData->SandboxToEdit->SetSettings( newSettings );
        changeDetected = true;
    }

    return changeDetected;
}
