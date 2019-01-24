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

#include <memory>
#include <QIcon>
#include "AddCameraDialog.hpp"
#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"

#include "ProjectObjectNamePage.hpp"
#include "SelectPluginPage.hpp"
#include "ConfigurePluginPage.hpp"

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class AddCameraDialogData
    {
    public:
        AddCameraDialogData( ) :
            objectNamePage( 0 ),
            selectPluginPage( 0 ),
            configurePluginPage( 0 )
        {
        }

    public:
        ProjectObjectNamePage*  objectNamePage;
        SelectPluginPage*       selectPluginPage;
        ConfigurePluginPage*    configurePluginPage;

    public:
        shared_ptr<const XPluginDescriptor> PluginDescription;
        shared_ptr<XPlugin>                 PluginToConfigure;
    };
}

AddCameraDialog::AddCameraDialog( const XGuid& parentObjectId, QWidget* parentWidget ) :
    WizardDialog( "AddCameraWizard", parentWidget ),
    mData( new Private::AddCameraDialogData )
{
    this->setWindowTitle( "Add camera wizard" );
    this->setWindowIcon( QIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Camera, false ) ) );

    // set up pages
    mData->objectNamePage = new ProjectObjectNamePage( this );
    mData->selectPluginPage = new SelectPluginPage( PluginType_VideoSource, this );
    mData->configurePluginPage = new ConfigurePluginPage( false, this );

    mData->objectNamePage->SetObjectProperties( parentObjectId );
    mData->objectNamePage->SetObjectIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Camera, true ) );
    mData->objectNamePage->SetObjectTypeName( QString::fromUtf8( "Camera" ) );
    mData->selectPluginPage->SetTitle( "Select video source plug-in" );
    mData->configurePluginPage->SetTitle( "Configure video source plug-in" );

    // add pages to the wizard
    AddPage( mData->objectNamePage );
    AddPage( mData->selectPluginPage );
    AddPage( mData->configurePluginPage );
}

AddCameraDialog::~AddCameraDialog( )
{
    delete mData;
}

// Configuration page change event handler
void AddCameraDialog::OnChangingPage( int nextPage )
{
    if ( nextPage == 2 )
    {
        XGuid pluginID = static_cast<SelectPluginPage*>( mData->selectPluginPage )->GetSelectedPluginID( );

        if ( !pluginID.IsEmpty( ) )
        {
            shared_ptr<const XPluginsEngine> pluginsEngine =
                    ServiceManager::Instance( ).GetPluginsEngine( );

            // get description of the plug-in with specified ID
            shared_ptr<const XPluginDescriptor> pluginDesc = pluginsEngine->GetPlugin( pluginID );

            if ( ( pluginDesc ) && ( pluginDesc->Type( ) == PluginType_VideoSource ) )
            {
                mData->PluginDescription = pluginDesc;
                mData->PluginToConfigure = pluginDesc->CreateInstance( );

                mData->configurePluginPage->SetPluginToConfigure( pluginDesc, mData->PluginToConfigure );
            }
        }
    }
}

// Get name of the camera to add
const string AddCameraDialog::GetCameraName( ) const
{
    return mData->objectNamePage->GetObjectName( );
}

// Get description of the camera to add
const string AddCameraDialog::GetCameraDescription( ) const
{
    return mData->objectNamePage->GetObjectDescription( );
}

// Get video source plug-in ID for the camera
const XGuid AddCameraDialog::GetSelectedPluginID( ) const
{
    return mData->selectPluginPage->GetSelectedPluginID( );
}

// Get configuration (properties) of the video source plug-in
const std::map<std::string, XVariant> AddCameraDialog::GetPluginConfiguration( ) const
{
    return mData->PluginDescription->GetPluginConfiguration( mData->PluginToConfigure );
}
