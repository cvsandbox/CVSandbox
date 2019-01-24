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

#include "SelectDevicesPage.hpp"
#include "ui_SelectDevicesPage.h"
#include "ProjectTreeFrame.hpp"
#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"
#include "DeviceListWidgetItem.hpp"

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class SelectDevicesPageData
    {
    public:
        SelectDevicesPageData( ) :
            SandboxDevices( ), SandboxItemsMap( ),
            AvailableDevicesContextMenu( nullptr ), AddDeviceAction( nullptr ),
            SelectedDevicesContextMenu( nullptr ), RemoveDeviceAction( nullptr )
        {
            // create context menu for the tree of available devices
            AddDeviceAction = new QAction( nullptr );
            AddDeviceAction->setText( "&Add device" );
            AddDeviceAction->setStatusTip( "Add device to the sandbox" );
            AddDeviceAction->setIcon( QIcon( QPixmap( ":/images/icons/add.png" ) ) );

            AvailableDevicesContextMenu = new QMenu( );
            AvailableDevicesContextMenu->addAction( AddDeviceAction );
            AvailableDevicesContextMenu->setDefaultAction( AddDeviceAction );

            // create context menu for the tree of available devices
            RemoveDeviceAction = new QAction( nullptr );
            RemoveDeviceAction->setText( "&Remove device" );
            RemoveDeviceAction->setStatusTip( "Remove device from the sandbox" );
            RemoveDeviceAction->setIcon( QIcon( QPixmap( ":/images/icons/remove.png" ) ) );

            EnableFpsReportAction = new QAction( nullptr );
            EnableFpsReportAction->setText( "&Enable FPS report" );
            EnableFpsReportAction->setStatusTip( "Enable/disable FPS report in sandbox" );
            EnableFpsReportAction->setIcon( QIcon( QPixmap( ":/images/icons/frame_rate_enabled.png" ) ) );

            SelectedDevicesContextMenu = new QMenu( );
            SelectedDevicesContextMenu->addAction( RemoveDeviceAction );
            SelectedDevicesContextMenu->addSeparator( );
            SelectedDevicesContextMenu->addAction( EnableFpsReportAction );
            SelectedDevicesContextMenu->setDefaultAction( RemoveDeviceAction );
        }

        ~SelectDevicesPageData( )
        {
            delete AddDeviceAction;
            delete AvailableDevicesContextMenu;

            delete RemoveDeviceAction;
            delete SelectedDevicesContextMenu;
        }

    public:
        vector<XGuid>                SandboxDevices;
        map<XGuid, bool>             DevicesFpsReportState;
        map<XGuid, QListWidgetItem*> SandboxItemsMap;

        QMenu*      AvailableDevicesContextMenu;
        QAction*    AddDeviceAction;

        QMenu*      SelectedDevicesContextMenu;
        QAction*    RemoveDeviceAction;
        QAction*    EnableFpsReportAction;
    };
}

SelectDevicesPage::SelectDevicesPage( QWidget *parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::SelectDevicesPage ),
    mData( new Private::SelectDevicesPageData( ) )
{
    ui->setupUi( this );

    ui->fakeButton->setVisible( false );
    ui->availableDevicesTree->SetAllowDragAndDrop( false );
    ui->availableDevicesTree->ShowProjectObjectsDescription( false );
    ui->availableDevicesTree->SetProjectObjectContextMenu( ProjectObjectType::Camera, mData->AvailableDevicesContextMenu );

    // set custom background colors for the items of the selected devices list
    ui->selectedDevicesList->setStyleSheet(
        "QListWidget::item {"
           "background-color: #F6F6F6;"
        "}"
        "QListWidget::item:selected {"
           "background-color: #80D8E9;"
        "}"
        "QListWidget::item:hover {"
           "background-color: #E8F7Fb;"
        "}"
        "QListWidget::item:selected:hover {"
           "background-color: #B9DEEF;"
        "}"
    );

    // connect context menu's actions
    connect( mData->AddDeviceAction, SIGNAL(triggered()), this, SLOT(on_addObjectButton_clicked()) );
    connect( mData->RemoveDeviceAction, SIGNAL(triggered()), this, SLOT(on_removeObjectButton_clicked()) );
    connect( mData->EnableFpsReportAction, SIGNAL(triggered()), this, SLOT(on_enableFpsReportAction_triggered()) );
    connect( mData->SelectedDevicesContextMenu, SIGNAL(aboutToShow()), this, SLOT(on_selectedDevicesContextMenu_aboutToShow()) );
}

SelectDevicesPage::~SelectDevicesPage( )
{
    delete mData;
    delete ui;
}

// Set list of available devices to choose from
void SelectDevicesPage::SetAvailableDevices( const vector<shared_ptr<ProjectObject>>& devicesList )
{
    for ( vector<shared_ptr<ProjectObject>>::const_iterator it = devicesList.begin( ); it != devicesList.end( ); it++ )
    {
        ui->availableDevicesTree->AddObject( *it );
    }
    ui->availableDevicesTree->ExpandRootItem( true );
    ui->availableDevicesTree->SelectRootItem( );
}

// Get list of the selected devices
const vector<XGuid> SelectDevicesPage::GetSelectedDevices( ) const
{
    return mData->SandboxDevices;
}

// Get FPS report state for the selected devices
map<XGuid, bool> SelectDevicesPage::GetDevicesFpsReportState( ) const
{
    return mData->DevicesFpsReportState;
}

// Set list of the selected devices
void SelectDevicesPage::SetSelectedDevices( const vector<XGuid>& selectedDevices,
                                            const map<XGuid, bool>& devicesFpsReportState )
{
    RemoveAllObjectsFromTheSandbox( );

    for ( vector<XGuid>::const_iterator it = selectedDevices.begin( ); it != selectedDevices.end( ); it++ )
    {
        map<XGuid, bool>::const_iterator itFpsState = devicesFpsReportState.find( *it );
        bool fpsReportState = true;

        if ( itFpsState != devicesFpsReportState.end( ) )
        {
            fpsReportState = itFpsState->second;
        }

        AddObjectToTheSandbox( *it, fpsReportState );
    }

    UpdateAddButtonStatus( );
}

// Check if a wizard control can go the next page from this one
bool SelectDevicesPage::CanGoNext( ) const
{
    return ( mData->SandboxDevices.size( ) != 0 );
}

// The page got focus, so transfer it to the child control we need
void SelectDevicesPage::focusInEvent( QFocusEvent* event )
{
    ui->availableDevicesTree->setFocus( );
    WizardPageFrame::focusInEvent( event );
}

// An object was selected in the tree of available devices
void SelectDevicesPage::on_availableDevicesTree_ObjectSelected( const XGuid& objectId )
{
    UpdateAddButtonStatus( objectId );
}

// An object was double clicked in the tree of available devices
void SelectDevicesPage::on_availableDevicesTree_ObjectDoubleClicked( const XGuid& objectId )
{
    if ( AddObjectToTheSandbox( objectId ) )
    {
         ui->addObjectButton->setEnabled( false );
    }
}

// An item is double clicked in the list of selected devices
void SelectDevicesPage::on_selectedDevicesList_itemDoubleClicked( QListWidgetItem* item )
{
    if ( item != 0 )
    {
        RemoveObjectFromTheSandbox( XGuid( item->data( Qt::UserRole ).toString( ).toStdString( ) ) );
        UpdateAddButtonStatus( );
    }
}

// On "Add" button click
void SelectDevicesPage::on_addObjectButton_clicked( )
{
    if ( AddObjectToTheSandbox( ui->availableDevicesTree->GetSelectedItem( ) ) )
    {
         ui->addObjectButton->setEnabled( false );
    }
}

// On "Remove" button click
void SelectDevicesPage::on_removeObjectButton_clicked( )
{
    QList<QListWidgetItem*> selectedItems = ui->selectedDevicesList->selectedItems( );

    while ( selectedItems.count( ) != 0 )
    {
        QListWidgetItem* listItem = selectedItems.takeFirst( );

        RemoveObjectFromTheSandbox( XGuid( listItem->data( Qt::UserRole ).toString( ).toStdString( ) ) );
    }

    UpdateAddButtonStatus( );
}

// On "Remove All" button click
void SelectDevicesPage::on_removeAllObjectsButton_clicked( )
{
    RemoveAllObjectsFromTheSandbox( );
    UpdateAddButtonStatus( );
}

// Enable/disable FPS report for a video source
void SelectDevicesPage::on_enableFpsReportAction_triggered( )
{
    QList<QListWidgetItem*> selectedItems = ui->selectedDevicesList->selectedItems( );

    for ( int i = 0; i < selectedItems.count( ); i++ )
    {
        QListWidgetItem*      listItem   = selectedItems[i];
        DeviceListWidgetItem* itemWidget = static_cast<DeviceListWidgetItem*>( ui->selectedDevicesList->itemWidget( listItem ) );
        XGuid                 deviceId   = XGuid( listItem->data( Qt::UserRole ).toString( ).toStdString( ) );

        mData->DevicesFpsReportState[deviceId] = !mData->DevicesFpsReportState[deviceId];

        itemWidget->enableFpsReport( mData->DevicesFpsReportState[deviceId] );
    }
}

// Context menu for selected devices is about to show
void SelectDevicesPage::on_selectedDevicesContextMenu_aboutToShow( )
{
    QList<QListWidgetItem*> selectedItems    = ui->selectedDevicesList->selectedItems( );
    bool                    firstItem        = true;
    bool                    enableFpsSwitch  = false;
    bool                    enabledFpsReport = false;

    for ( int i = 0; i < selectedItems.count( ); i++ )
    {
        QListWidgetItem* listItem = selectedItems[i];
        XGuid            deviceId = XGuid( listItem->data( Qt::UserRole ).toString( ).toStdString( ) );

        if ( firstItem )
        {
            enabledFpsReport = mData->DevicesFpsReportState[deviceId];
            enableFpsSwitch  = true;
            firstItem        = false;
        }
        else
        {
            enableFpsSwitch &= ( enabledFpsReport == mData->DevicesFpsReportState[deviceId] );
        }
    }

    mData->EnableFpsReportAction->setEnabled( enableFpsSwitch );

    if ( !enableFpsSwitch )
    {
        mData->EnableFpsReportAction->setText( "Enable/Disable FPS report" );
    }
    else
    {
        if ( enabledFpsReport )
        {
            mData->EnableFpsReportAction->setText( "&Disable FPS report" );
        }
        else
        {
            mData->EnableFpsReportAction->setText( "&Enable FPS report" );
        }
    }
}

// Selection was changed in the list of selected devices
void SelectDevicesPage::on_selectedDevicesList_itemSelectionChanged( )
{
    ui->removeObjectButton->setEnabled( ( ui->selectedDevicesList->selectedItems( ).count( ) != 0 ) ? true : false );
}

// Update status of the "Add" button depending on current selection in the tree of available devices
void SelectDevicesPage::UpdateAddButtonStatus( )
{
    UpdateAddButtonStatus( ui->availableDevicesTree->GetSelectedItem( ) );
}
void SelectDevicesPage::UpdateAddButtonStatus( const XGuid& selectedObjectId )
{
    shared_ptr<IProjectManager>  projectManager = ServiceManager::Instance( ).GetProjectManager( );
    ProjectObjectType            selectionType  = projectManager->GetProjectObjectType( selectedObjectId );

    if ( ( selectionType == ProjectObjectType::Camera ) &&
         ( !IsObjectInTheSandbox( selectedObjectId ) ) )
    {
        ui->addObjectButton->setEnabled( true );
    }
    else
    {
        ui->addObjectButton->setEnabled( false );
    }
}

// Check if device with the specified ID is already in the sandbox
bool SelectDevicesPage::IsObjectInTheSandbox( const XGuid& objectId ) const
{
    bool ret = false;

    for ( vector<XGuid>::const_iterator it = mData->SandboxDevices.begin( ); it != mData->SandboxDevices.end( ); it++ )
    {
       if ( objectId == *it )
       {
           ret = true;
           break;
       }
    }

    return ret;
}

// Add object to the list of selected devices
bool SelectDevicesPage::AddObjectToTheSandbox( const XGuid& objectId, bool fpsReportState )
{
    shared_ptr<IProjectManager> projectManager = ServiceManager::Instance( ).GetProjectManager( );
    shared_ptr<ProjectObject>   projectObject  = projectManager->GetProjectObject( objectId );
    bool                        ret            = false;

    if ( ( projectObject ) &&
         ( projectObject->Type( ) != ProjectObjectType::Folder ) &&
         ( !IsObjectInTheSandbox( objectId ) ) )
    {
        string                fullName   = projectManager->GetProjectObjectFullName( projectObject );
        string                objectPath = projectManager->GetProjectObjectPath( projectObject );
        DeviceListWidgetItem* deviceItem = new DeviceListWidgetItem(
                                                QString::fromUtf8( objectPath.c_str( ) ),
                                                QString::fromUtf8( projectObject->Name( ).c_str( ) ),
                                                QString::fromUtf8( fullName.c_str( ) ),
                                                true
                                            );
        QListWidgetItem*      listItem   = new QListWidgetItem( );
        QString               iconName   = ProjectObjectIcon::GetResourceName( projectObject->Type( ) );

        listItem->setData( Qt::UserRole, projectObject->Id( ).ToString( ).c_str( ) );
        listItem->setSizeHint( QSize( 0, 40 ) );

        if ( !iconName.isEmpty( ) )
        {
            deviceItem->setIcon( QIcon( iconName ) );
        }

        deviceItem->enableFpsReport( fpsReportState );

        // add to UI list
        ui->selectedDevicesList->addItem( listItem );
        ui->selectedDevicesList->setItemWidget( listItem, deviceItem );
        // add device ID to the list of selected devices
        mData->SandboxDevices.push_back( objectId );
        mData->DevicesFpsReportState.insert( pair<XGuid, bool>( objectId, fpsReportState ) );
        mData->SandboxItemsMap.insert( pair<XGuid, QListWidgetItem*>( objectId, listItem ) );

        ui->availableDevicesTree->SetItemHighlight( objectId, true );
        emit CompleteStatusChanged( true );

        ret = true;
    }

    return ret;
}

// Remove object from the list of available devices
void SelectDevicesPage::RemoveObjectFromTheSandbox( const XGuid& objectId )
{
    map<XGuid, QListWidgetItem*>::iterator it = mData->SandboxItemsMap.find( objectId );

    if ( it != mData->SandboxItemsMap.end( ) )
    {
        QListWidgetItem* listItem = it->second;

        mData->SandboxItemsMap.erase( it );
        delete listItem;
    }

    mData->SandboxDevices.erase( std::remove(
                                    mData->SandboxDevices.begin( ),
                                    mData->SandboxDevices.end( ),
                                    objectId ),
                                 mData->SandboxDevices.end( ) );
    ui->availableDevicesTree->SetItemHighlight( objectId, false );

    emit CompleteStatusChanged( mData->SandboxItemsMap.size( ) != 0 );
}

// Remove all objects from the selected list
void SelectDevicesPage::RemoveAllObjectsFromTheSandbox( )
{
    while ( ui->selectedDevicesList->count( ) )
    {
        QListWidgetItem* listItem = ui->selectedDevicesList->item( 0 );

        RemoveObjectFromTheSandbox( XGuid( listItem->data( Qt::UserRole ).toString( ).toStdString( ) ) );
    }
}

// Custom menu requested for the list of selected devices
void SelectDevicesPage::on_selectedDevicesList_customContextMenuRequested( const QPoint &pos )
{
    if ( ui->selectedDevicesList->itemAt( pos ) != nullptr )
    {
        mData->SelectedDevicesContextMenu->popup( ui->selectedDevicesList->mapToGlobal( pos ) );
    }
}
