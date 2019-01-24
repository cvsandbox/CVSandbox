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
#include "EditCameraPropertiesDialog.hpp"
#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"

#include "ProjectObjectNamePage.hpp"
#include "ConfigurePluginPage.hpp"

#include <UITools.hpp>

using namespace CVSandbox;
using namespace std;

namespace Private
{
    class EditCameraPropertiesDialogData
    {
    public:
        EditCameraPropertiesDialogData( const shared_ptr<CameraProjectObject>& po ) :
            CameraToEdit( po ),PluginDescription( ), PluginToConfigure( ),
            cameraNamePage( 0 ), configurePluginPage( 0 )
        {
        }

    public:
        shared_ptr<CameraProjectObject>     CameraToEdit;
        shared_ptr<const XPluginDescriptor> PluginDescription;
        shared_ptr<XPlugin>                 PluginToConfigure;

    public:
        ProjectObjectNamePage* cameraNamePage;
        ConfigurePluginPage*   configurePluginPage;
    };
}

EditCameraPropertiesDialog::EditCameraPropertiesDialog( const shared_ptr<CameraProjectObject>& po,
                                                        QWidget* parent ):
    PropertyPagesDialog( "EditCameraPropertie", parent ),
    mData( new Private::EditCameraPropertiesDialogData( po ) )
{
    assert( po );

    this->setWindowTitle( "Edit camera properties" );
    this->setWindowIcon( QIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Camera ) ) );

    // set up pages
    mData->cameraNamePage      = new ProjectObjectNamePage( this );
    mData->configurePluginPage = new ConfigurePluginPage( false, this );
    mData->configurePluginPage->SetTitle( "Configure video source plug-in" );

    mData->cameraNamePage->SetObjectProperties( po->ParentId( ), po->Name( ), po->Description( ) );
    mData->cameraNamePage->SetObjectIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Camera, true ) );
    mData->cameraNamePage->SetObjectTypeName( QString::fromUtf8( "Camera" ) );

    // fill pages with data
    shared_ptr<const XPluginDescriptor> pluginDesc = ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( po->PluginId( ) );

    if ( pluginDesc )
    {
        if ( pluginDesc->Type( ) == PluginType_VideoSource )
        {
            mData->PluginDescription = pluginDesc;
            mData->PluginToConfigure = pluginDesc->CreateInstance( );

            // set confguration of the plugin
            pluginDesc->SetPluginConfiguration( mData->PluginToConfigure,  mData->CameraToEdit->PluginProperties( ) );

            mData->configurePluginPage->SetPluginToConfigure( pluginDesc, mData->PluginToConfigure );
        }
        else
        {
            UITools::ShowErrorMessage( QString( "<b>The plug-in for this camera object is not of video source type.</b><br>"
                                                "Project file or the plug-in might be broken.<br><br>"
                                                "The plug-in name: %0" ).arg( QString::fromUtf8( pluginDesc->Name( ).c_str( ) ) ), this );
        }
    }

    // add pages to the property dialog
    AddPage( mData->cameraNamePage );
    AddPage( mData->configurePluginPage );
}

EditCameraPropertiesDialog::~EditCameraPropertiesDialog( )
{
    delete mData;
}

// Apply changes made at specified page
void EditCameraPropertiesDialog::ApplyCurrentPage( int index )
{
    bool changeDetected = false;

    switch ( index )
    {
    case 0:
        changeDetected = ApplyDescriptionPage( );
        break;

    case 1:
        changeDetected = ApplyVideoSourceConfigurationPage( );
        break;

    default:
        Q_ASSERT( false );
        break;
    }

    if ( changeDetected )
    {
        ServiceManager::Instance( ).GetProjectManager( )->UpdateProjectObject( mData->CameraToEdit );
    }
}

// Apply changes made on all pages
void EditCameraPropertiesDialog::ApplyAllPages( )
{
    bool changeDetected = false;

    changeDetected |= ApplyDescriptionPage( );
    changeDetected |= ApplyVideoSourceConfigurationPage( );

    if ( changeDetected )
    {
        ServiceManager::Instance( ).GetProjectManager( )->UpdateProjectObject( mData->CameraToEdit );
    }
}

// Apply changes done on project object's description page
bool EditCameraPropertiesDialog::ApplyDescriptionPage( )
{
    string newName        = mData->cameraNamePage->GetObjectName( );
    string newDescription = mData->cameraNamePage->GetObjectDescription( );
    bool   changeDetected = false;

    if ( mData->CameraToEdit->Name( ) != newName )
    {
        mData->CameraToEdit->SetName( newName );
        changeDetected |= true;
    }

    if ( mData->CameraToEdit->Description( ) != newDescription )
    {
        mData->CameraToEdit->SetDescription( mData->cameraNamePage->GetObjectDescription( ) );
        changeDetected |= true;
    }

    return changeDetected;
}

// Apply changes done on plug-in properties configuration page
bool EditCameraPropertiesDialog::ApplyVideoSourceConfigurationPage( )
{
    map<string, XVariant> newPluginConfiguration = mData->PluginDescription->GetPluginConfiguration( mData->PluginToConfigure );
    map<string, XVariant> oldPluginConfiguration = mData->CameraToEdit->PluginProperties( );
    bool                  changeDetected         = false;

    for ( map<string, XVariant>::const_iterator i = newPluginConfiguration.begin( );
                                                i != newPluginConfiguration.end( ); i++ )
    {
        map<string, XVariant>::const_iterator found = oldPluginConfiguration.find( i->first );

        if ( ( found == oldPluginConfiguration.end( ) ) ||
             ( found->second != i->second ) )
        {
            changeDetected = true;
        }
    }

    if ( changeDetected )
    {
        // set information about plug-in and its configuration
        mData->CameraToEdit->SetPluginProperties( newPluginConfiguration );
    }

    return changeDetected;
}
