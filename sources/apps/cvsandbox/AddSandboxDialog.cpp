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

#include <QIcon>
#include "AddSandboxDialog.hpp"
#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"
#include "XGuidGenerator.hpp"

#include "ProjectObjectNamePage.hpp"
#include "SelectDevicesPage.hpp"
#include "ConfigureCamerasViewPage.hpp"

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class AddSandboxDialogData
    {
    public:
        AddSandboxDialogData( ) :
            objectNamePage( NULL ), selectDevicesPage( NULL ), configureCamerasViewPage( NULL ),
            viewConfigurationIsCreated( false )
        {
        }

    public:
        ProjectObjectNamePage*    objectNamePage;
        SelectDevicesPage*        selectDevicesPage;
        ConfigureCamerasViewPage* configureCamerasViewPage;

        bool                      viewConfigurationIsCreated;
    };
}

AddSandboxDialog::AddSandboxDialog( const XGuid& parentObjectId, QWidget* parentWidget ) :
    WizardDialog( "AddSandboxWizard", parentWidget ),
    mData( new Private::AddSandboxDialogData )
{
    this->setWindowTitle( "Add sandbox wizard" );
    this->setWindowIcon( QIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Sandbox, false ) ) );

    connect( this, SIGNAL( ChangingPage( int ) ), this, SLOT( on_ChangingPage( int ) ) );

    // set up pages
    mData->objectNamePage = new ProjectObjectNamePage( this );
    mData->selectDevicesPage = new SelectDevicesPage( this );
    mData->configureCamerasViewPage = new ConfigureCamerasViewPage( this );

    mData->objectNamePage->SetObjectProperties( parentObjectId );
    mData->objectNamePage->SetObjectIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Sandbox, true ) );
    mData->objectNamePage->SetObjectTypeName( QString::fromUtf8( "Sandbox" ) );

    // add pages to the wizard
    AddPage( mData->objectNamePage );
    AddPage( mData->selectDevicesPage );
    AddPage( mData->configureCamerasViewPage );
}

AddSandboxDialog::~AddSandboxDialog( )
{
    delete mData;
}

// Set list of devices to choose from
void AddSandboxDialog::SetAvailableDevices( const vector<shared_ptr<ProjectObject>>& camerasList )
{
    mData->selectDevicesPage->SetAvailableDevices( camerasList );
}

// Get name of the sandbox to add
const string AddSandboxDialog::GetSandboxName( ) const
{
    return mData->objectNamePage->GetObjectName( );
}

// Get description of the sandbox to add
const string AddSandboxDialog::GetSandboxDescription( ) const
{
    return mData->objectNamePage->GetObjectDescription( );
}

// Get list of sandbox devices
const vector<XGuid> AddSandboxDialog::GetSandboxDevices( ) const
{
    return mData->selectDevicesPage->GetSelectedDevices( );
}

// Get FPS report state for the selected devices
map<XGuid, bool> AddSandboxDialog::GetDevicesFpsReportState( ) const
{
    return mData->selectDevicesPage->GetDevicesFpsReportState( );
}

// Get default view configuration
const vector<CamerasViewConfiguration> AddSandboxDialog::GetViewConfigurations( ) const
{
    return mData->configureCamerasViewPage->GetViewConfigurations( );
}

// Get/Set default view ID
const XGuid AddSandboxDialog::DefaultViewId( ) const
{
    return mData->configureCamerasViewPage->DefaultViewId( );
}

// Configuration page change event handler
void AddSandboxDialog::OnChangingPage( int nextPage )
{
    if ( nextPage == 2 )
    {
        const vector<XGuid>& selectedObject = mData->selectDevicesPage->GetSelectedDevices( );

        mData->configureCamerasViewPage->SetAvalableCameras( selectedObject );

        if ( !mData->viewConfigurationIsCreated )
        {
            int camerasCount = selectedObject.size( );
            int gridSize     = ( camerasCount > 9 ) ? 4 :
                               ( camerasCount > 4 ) ? 3 :
                               ( camerasCount > 1 ) ? 2 : 1;

            vector<CamerasViewConfiguration> viewConfigurations;
            XGuid                            defaultViewId      = XGuidGenerator::Generate( );
            CamerasViewConfiguration         defaultView( defaultViewId, gridSize, gridSize );

            // if we have only one camera, there is little we can miss about it - put into the view
            if ( camerasCount == 1 )
            {
                defaultView.GetCellConfiguration( 0, 0 )->SetCameraId( selectedObject[0] );
            }

            viewConfigurations.push_back( defaultView );

            mData->configureCamerasViewPage->SetViewConfigurations( viewConfigurations );
            mData->configureCamerasViewPage->SetDefaultViewId( defaultViewId );

            mData->viewConfigurationIsCreated = true;
        }
    }
}
