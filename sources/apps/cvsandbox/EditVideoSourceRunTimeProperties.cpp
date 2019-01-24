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

#include "EditVideoSourceRunTimeProperties.hpp"
#include "ServiceManager.hpp"
#include "CameraProjectObject.hpp"

#include <UITools.hpp>
#include <ConfigurePluginPropertiesDialog.hpp>

using namespace std;
using namespace CVSandbox;

// Check if the camera project object provides run time configuration
bool IsRunTimeConfigurationAvailable( const XGuid& poId )
{
    ServiceManager& serviceManager          = ServiceManager::Instance( );
    bool            cameraSettingsAvailable = false;

    if ( serviceManager.GetProjectManager( )->GetProjectObjectType( poId ) == ProjectObjectType::Camera )
    {
        shared_ptr<CameraProjectObject> poCamera = static_pointer_cast<CameraProjectObject>(
                ServiceManager::Instance( ).GetProjectManager( )->GetProjectObject( poId ) );

        if ( poCamera )
        {
            shared_ptr<const XPluginDescriptor> cameraPluginDescriptor =
                    ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( poCamera->PluginId( ) );

            if ( cameraPluginDescriptor )
            {
                cameraSettingsAvailable = ( cameraPluginDescriptor->DeviceRuntimeConfigurablePropertiesCount( ) != 0 );
            }
        }
    }

    return cameraSettingsAvailable;
}

// Edit run time properties of the specified camera project object (and it running video source ID)
void EditRunTimeConfiguration( const XGuid& poId, uint32_t videoSourceId, QWidget* parent )
{
    ServiceManager& serviceManager = ServiceManager::Instance( );

    if ( serviceManager.GetProjectManager( )->GetProjectObjectType( poId ) == ProjectObjectType::Camera )
    {
        shared_ptr<CameraProjectObject> poCamera = static_pointer_cast<CameraProjectObject>(
                serviceManager.GetProjectManager( )->GetProjectObject( poId ) );
        shared_ptr<XPluginDescriptor> cameraPluginDescriptor =
                serviceManager.GetPluginsEngine( )->GetPlugin( poCamera->PluginId( ) )->Clone( );
        shared_ptr<XVideoSourcePlugin> videoSourcePlugin =
                serviceManager.GetAutomationServer( )->GetRunningVideoSource( videoSourceId );

        if ( videoSourcePlugin )
        {
            videoSourcePlugin->UpdateDescription( cameraPluginDescriptor );

            if ( cameraPluginDescriptor->DeviceRuntimeConfigurablePropertiesCount( ) == 0 )
            {
                UITools::ShowInfoMessage( "The instance of the video source does not provide any run time properties to configure." );
            }
            else
            {
                map<string, XVariant> oldRunTimeConfiguration = cameraPluginDescriptor->GetPluginDeviceRuntimeConfiguration( videoSourcePlugin );

                ConfigurePluginPropertiesDialog configureForm( cameraPluginDescriptor, videoSourcePlugin, true,
                                                               serviceManager.GetMainWindowService( )->GetMainAppWindow( ) );

                configureForm.EnableDefaultsButton( true );
                configureForm.EnableHelpButton( true );
                parent->connect( &configureForm, SIGNAL(HelpRequested(QWidget*)), parent, SLOT(on_HelpRequested(QWidget*)) );

                if ( configureForm.exec( ) == QDialog::Accepted )
                {
                    map<string, XVariant> newRunTimeConfiguration = cameraPluginDescriptor->GetPluginDeviceRuntimeConfiguration( videoSourcePlugin );

                    if ( ( newRunTimeConfiguration != oldRunTimeConfiguration ) &&
                         ( UITools::GetUserConfirmation( QString( "<b>Some properties of the [%0] video source were changed.</b><br><br>Would you like to update its project object to keep the settings?" ).
                                                         arg( QString::fromUtf8( poCamera->Name( ).c_str( ) ) ), parent ) ) )
                    {
                        poCamera->MergePluginProperties( newRunTimeConfiguration );
                        serviceManager.GetProjectManager( )->UpdateProjectObject( poCamera );
                    }
                }
                else
                {
                    cameraPluginDescriptor->SetPluginConfiguration( videoSourcePlugin, oldRunTimeConfiguration );
                }
            }
        }
    }
}
