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

#include <UITools.hpp>

#include "UserCommands.hpp"
#include "ServiceManager.hpp"
#include "EditFolderDialog.hpp"
#include "AddCameraDialog.hpp"
#include "AddSandboxDialog.hpp"
#include "EditCameraPropertiesDialog.hpp"
#include "EditSandboxPropertiesDialog.hpp"

#include "FolderProjectObject.hpp"
#include "SandboxProjectObject.hpp"
#include "CameraProjectObject.hpp"

#include "CamerasViewConfiguration.hpp"
#include "ProjectObjectOpener.hpp"
#include "SandboxWizardDialog.hpp"

using namespace std;
using namespace CVSandbox;

// Handler of add camera's group command
void AddFolderCommand::Exec( )
{
    const IProjectTreeUI* projectTree = ServiceManager::Instance( ).GetProjectTreeUI( );

    if ( projectTree->GetSelectedItemType( ) != ProjectObjectType::None )
    {
        IMainWindowService* mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
        QWidget*            mainWindow        = mainWindowService->GetMainAppWindow( );
        const XGuid         parentId          = projectTree->GetSelectedOrParentFolder( );
        EditFolderDialog    dialog( parentId, string( ), string( ), mainWindow );

        if ( dialog.exec( ) == QDialog::Accepted )
        {
            const string name = dialog.GetFolderName( );

            // create new folder project object
            shared_ptr<FolderProjectObject> po = FolderProjectObject::CreateNew( name, dialog.GetFolderDescription( ), parentId );
            // and it to project maneger
            if ( !ServiceManager::Instance( ).GetProjectManager( )->AddProjectObject( po ) )
            {
                UITools::ShowErrorMessage( QString( "Failed adding folder: %0" ).
                                           arg( QString::fromUtf8( name.c_str( ) ) ),
                                           mainWindow );
            }
        }
    }
}

// Handler of add camera command
void AddCameraCommand::Exec( )
{
    const IProjectTreeUI* projectTree = ServiceManager::Instance( ).GetProjectTreeUI( );

    if ( projectTree->GetSelectedItemType( ) != ProjectObjectType::None )
    {
        IMainWindowService* mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
        QWidget*            mainWindow        = mainWindowService->GetMainAppWindow( );
        const XGuid         parentId          = projectTree->GetSelectedOrParentFolder( );
        AddCameraDialog     dialog( parentId, mainWindow );

        if ( dialog.exec( ) == QDialog::Accepted )
        {
            const string name = dialog.GetCameraName( );

            // create new camera project object
            shared_ptr<CameraProjectObject> po = CameraProjectObject::CreateNew( name, dialog.GetCameraDescription( ), parentId );

            // set information about plug-in and its configuration
            po->SetPluginConfiguration( dialog.GetSelectedPluginID( ), dialog.GetPluginConfiguration( ) );

            // add it to project manager
            if ( !ServiceManager::Instance( ).GetProjectManager( )->AddProjectObject( po ) )
            {
                UITools::ShowErrorMessage( QString( "Failed adding camera: %0" ).
                                           arg( QString::fromUtf8( name.c_str( ) ) ),
                                           mainWindow );
            }
        }
    }
}

// Handler of add sandbox command
void AddSandboxCommand::Exec( )
{
    const IProjectTreeUI* projectTree = ServiceManager::Instance( ).GetProjectTreeUI( );

    if ( projectTree->GetSelectedItemType( ) != ProjectObjectType::None )
    {
        IMainWindowService*         mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
        shared_ptr<IProjectManager> projectManager    = ServiceManager::Instance( ).GetProjectManager( );
        QWidget*                    mainWindow        = mainWindowService->GetMainAppWindow( );
        const XGuid                 parentId          = projectTree->GetSelectedOrParentFolder( );
        AddSandboxDialog            dialog( parentId, mainWindow );

        vector<shared_ptr<ProjectObject>> camerasList;

        projectManager->GetChildren( XGuid( ), static_cast<ProjectObjectType::Enum>( ProjectObjectType::Camera | ProjectObjectType::Folder ),
                                     true, camerasList );
        dialog.SetAvailableDevices( camerasList );

        if ( dialog.exec( ) == QDialog::Accepted )
        {
            const string name = dialog.GetSandboxName( );

            // create new sandbox project object
            shared_ptr<SandboxProjectObject> po = SandboxProjectObject::CreateNew( name, dialog.GetSandboxDescription( ), parentId );

            // set sandbox properties
            po->SetSandboxDevices( dialog.GetSandboxDevices( ) );
            po->SetDeviceFpsReportState( dialog.GetDevicesFpsReportState( ) );
            po->SetViews( dialog.GetViewConfigurations( ) );
            po->SetDefaultViewId( dialog.DefaultViewId( ) );

            // add it to project manager
            if ( !ServiceManager::Instance( ).GetProjectManager( )->AddProjectObject( po ) )
            {
                UITools::ShowErrorMessage( QString( "Failed adding sandbox: %0" ).
                                           arg( QString::fromUtf8( name.c_str( ) ) ),
                                           mainWindow );
            }
        }
    }
}

// Handler of edit item's properties command
void EditPropertiesCommand::Exec( )
{
    const IProjectTreeUI*       projectTree       = ServiceManager::Instance( ).GetProjectTreeUI( );
    shared_ptr<IProjectManager> projectManager    = ServiceManager::Instance( ).GetProjectManager( );
    IMainWindowService*         mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
    ProjectObjectType           selecteObjectType = projectTree->GetSelectedItemType( );
    QWidget*                    mainWindow        = mainWindowService->GetMainAppWindow( );

    // get project object for the selected item
    const XGuid selectedId = projectTree->GetSelectedItem( );
    shared_ptr<ProjectObject> po = projectManager->GetProjectObject( selectedId );

    if ( selecteObjectType == ProjectObjectType::Folder )
    {
        EditFolderDialog dialog( po->ParentId( ), po->Name( ), po->Description( ), mainWindow );

        if ( dialog.exec( ) == QDialog::Accepted )
        {
            po->SetName( dialog.GetFolderName( ) );
            po->SetDescription( dialog.GetFolderDescription( ) );

            if ( !projectManager->UpdateProjectObject( po ) )
            {
                UITools::ShowErrorMessage( QString( "Failed updating properties of the folder: %0" ).
                                           arg( QString::fromUtf8( po->Name( ).c_str( ) ) ),
                                           mainWindow );
            }
        }
    }
    else if ( selecteObjectType == ProjectObjectType::Camera )
    {
        EditCameraPropertiesDialog dialog( static_pointer_cast<CameraProjectObject>( po ), mainWindow );
        // just call the dialog - it will update camera object if needed
        dialog.exec( );
    }
    else if ( selecteObjectType == ProjectObjectType::Sandbox )
    {
        EditSandboxPropertiesDialog       dialog( static_pointer_cast<SandboxProjectObject>( po ), mainWindow );
        vector<shared_ptr<ProjectObject>> camerasList;

        projectManager->GetChildren( XGuid( ), static_cast<ProjectObjectType::Enum>( ProjectObjectType::Camera | ProjectObjectType::Folder ),
                                     true, camerasList );
        dialog.SetAvailableDevices( camerasList );

        dialog.exec( );
    }
    else
    {
        Q_ASSERT( false );
    }
}

// Handler of delete PO object command
void DeleteProjectObjectCommand::Exec( )
{
    const IProjectTreeUI*       projectTree       = ServiceManager::Instance( ).GetProjectTreeUI( );
    shared_ptr<IProjectManager> projectManager    = ServiceManager::Instance( ).GetProjectManager( );
    IMainWindowService*         mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
    QWidget*                    mainWindow        = mainWindowService->GetMainAppWindow( );
    const XGuid                 selectedId        = projectTree->GetSelectedItem( );
    shared_ptr<ProjectObject>   po                = projectManager->GetProjectObject( selectedId );

    if ( po )
    {
        ProjectObjectType  selecteObjectType = po->Type( );
        QString            message;

        if ( selecteObjectType == ProjectObjectType::Folder )
        {
            if ( projectManager->GetChildrenCount( selectedId ) == 0 )
            {
                message = QString( "Are you sure you would like to delete the selected folder?<br /><br /><b>%0</b>" ).
                                   arg( po->Name( ).c_str( ) );
            }
            else
            {
                message = QString( "Are you sure you would like to delete the selected non-empty folder?<br /><br />All its content will be deleted!<br /><br /><b>%0</b>" ).
                                   arg( po->Name( ).c_str( ) );
            }
        }
        else if ( selecteObjectType != ProjectObjectType::None )
        {
            message = QString( "Are you sure you would like to delete the selected %0 object?<br /><br /><b>%1</b>" ).
                               arg( selecteObjectType.ToString( ).c_str( ) ).
                               arg( po->Name( ).c_str( ) );
        }
        else
        {
            Q_ASSERT( false );
        }

        if ( UITools::GetUserConfirmation( message, mainWindow ) )
        {
            projectManager->DeleteProjectObject( po );
        }
    }
    else
    {
        Q_ASSERT( false );
    }
}

// Handler of open project object command
void OpenProjectObjectCommand::Exec( )
{
    const IProjectTreeUI*             projectTree    = ServiceManager::Instance( ).GetProjectTreeUI( );
    shared_ptr<const IProjectManager> projectManager = ServiceManager::Instance( ).GetProjectManager( );
    shared_ptr<ProjectObject>         po = projectManager->GetProjectObject( projectTree->GetSelectedItem( ) );

    ProjectObjectOpener::Open( po );
}

// Handler of open sandbox wizard command
void OpenSandboxWizardCommand::Exec( )
{
    const IProjectTreeUI*       projectTree       = ServiceManager::Instance( ).GetProjectTreeUI( );
    shared_ptr<IProjectManager> projectManager    = ServiceManager::Instance( ).GetProjectManager( );
    IMainWindowService*         mainWindowService = ServiceManager::Instance( ).GetMainWindowService( );
    QWidget*                    mainWindow        = mainWindowService->GetMainAppWindow( );
    shared_ptr<ProjectObject>   po                = projectManager->GetProjectObject( projectTree->GetSelectedItem( ) );

    SandboxWizardDialog         wizardDialog( static_pointer_cast<SandboxProjectObject>( po ) , mainWindow );

    wizardDialog.exec( );
}
