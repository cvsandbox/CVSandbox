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
#include <QMap>
#include "ProjectTreeFrame.hpp"
#include "ui_ProjectTreeFrame.h"
#include "ServiceManager.hpp"
#include "ProjectObjectIcon.hpp"

using namespace std;
using namespace CVSandbox;
using namespace Private;

namespace Private
{
    // Custom derived class from QTreeWidgetItem to enable own sorting logic in project tree widget
    class ProjectTreeItem : public QTreeWidgetItem
    {
    public:
        ProjectTreeItem( QTreeWidget* parent, ProjectObjectType type ) : QTreeWidgetItem( parent ), mType( type ) { }
        ProjectTreeItem( QTreeWidgetItem* parent, ProjectObjectType type ) : QTreeWidgetItem( parent ), mType( type ) { }
        ProjectTreeItem( QTreeWidget* parent, ProjectObjectType type, const QStringList& strings ) : QTreeWidgetItem ( parent, strings ), mType( type ) { }

        bool operator< ( const QTreeWidgetItem &other ) const
        {
            int               sortCol   = treeWidget( )->sortColumn( );
            const QString&    myText    = text( sortCol );
            const QString&    otherText = other.text( sortCol );
            ProjectObjectType otherType = static_cast<const ProjectTreeItem&>( other ).mType;
            bool              ret;

            if ( ( ( mType != ProjectObjectType::Folder ) && ( otherType != ProjectObjectType::Folder ) ) ||
                 ( ( mType == ProjectObjectType::Folder ) && ( otherType == ProjectObjectType::Folder ) ) )
            {
                ret = ( myText < otherText );
            }
            else
            {
                ret = ( mType == ProjectObjectType::Folder );
            }

            return ret;
        }

    private:
        ProjectObjectType mType;
    };

    class ProjectTreeFrameData
    {
    public:
        ProjectTreeFrameData( ) :
            DefaultContextMenu( nullptr ), ContextMenus( ),
            SelectedTreeItem( nullptr ),
            RootItem( ),
            SelectNewItemOnAdding( false ),
            AllowDragAndDrop( true ),
            TriggerDefaultActionOnDoubleClick( false ),
            ProjectItemsMap( )
        {
        }

    public:
        QMenu*                          DefaultContextMenu;
        QMap<ProjectObjectType, QMenu*> ContextMenus;
        const QTreeWidgetItem*          SelectedTreeItem;
        ProjectTreeItem*                RootItem;
        bool                            SelectNewItemOnAdding;
        bool                            AllowDragAndDrop;
        bool                            TriggerDefaultActionOnDoubleClick;

        QMap<XGuid, ProjectTreeItem*>   ProjectItemsMap;
    };
}

ProjectTreeFrame::ProjectTreeFrame( QWidget *parent ) :
    QFrame( parent ),
    ui( new Ui::ProjectTreeFrame ),
    mData( new ProjectTreeFrameData )
{
    ui->setupUi( this );

    CreateBlankProjectTree( );

    // enable dragging in the project's tree
    ui->projectTreeWidget->setAcceptDrops( true );
}

ProjectTreeFrame::~ProjectTreeFrame( )
{
    delete mData;
    delete ui;
}

// Set default context menu for items which don't have their own menu
void ProjectTreeFrame::SetDefaultContextMenu( QMenu* menu )
{
    mData->DefaultContextMenu = menu;
}

// Set context menu to display for project objects of the specified type
void ProjectTreeFrame::SetProjectObjectContextMenu( ProjectObjectType poType, QMenu* menu )
{
    mData->ContextMenus[poType] = menu;
}

// Sets if newly added items should be automatically selected or
void ProjectTreeFrame::SetSelectionOfNewItems( bool set )
{
    mData->SelectNewItemOnAdding = set;
}

// Sets if drag and drop is allowed inside the control
void ProjectTreeFrame::SetAllowDragAndDrop( bool set )
{
    mData->AllowDragAndDrop = set;
}

// Sets if default action should be triggered on double click (taken from item's context menu)
void ProjectTreeFrame::SetTriggerDefaultActionOnDoubleClick( bool set )
{
    mData->TriggerDefaultActionOnDoubleClick = set;
}

// Sets if project objects' description should be visible or not
void ProjectTreeFrame::ShowProjectObjectsDescription( bool show )
{
    ui->descriptionLabel->setVisible( show );

    if ( show )
    {
        this->setFrameShape( QFrame::StyledPanel );
        ui->verticalLayout->setMargin( 3 );
    }
    else
    {
        this->setFrameShape( QFrame::NoFrame );
        ui->verticalLayout->setMargin( 0 );
    }
}

// Expand or collapse root item
void ProjectTreeFrame::ExpandRootItem( bool expand )
{
    if ( mData->RootItem != 0 )
    {
        if ( expand )
        {
            ui->projectTreeWidget->expandItem( mData->RootItem );
        }
        else
        {
            ui->projectTreeWidget->collapseItem( mData->RootItem );
        }
    }
}

// Set selection on the root item
void ProjectTreeFrame::SelectRootItem( )
{
    if ( mData->RootItem != 0 )
    {
        if ( ui->projectTreeWidget->currentItem( ) != mData->RootItem )
        {
            ui->projectTreeWidget->setCurrentItem( mData->RootItem );
        }
        else
        {
            on_projectTreeWidget_itemSelectionChanged( );
        }
    }
}

// Highlight item with the specified ID
void ProjectTreeFrame::SetItemHighlight( const XGuid& objectId, bool highlight )
{
    if ( mData->ProjectItemsMap.contains( objectId ) )
    {
        ProjectTreeItem* treeItem = mData->ProjectItemsMap[objectId];
        QFont            font     = treeItem->font( 0 );

        font.setBold( highlight );
        treeItem->setFont( 0, font );
    }
}

// The frame got focus, so transfer it to the child control we need
void ProjectTreeFrame::focusInEvent( QFocusEvent* event )
{
    ui->projectTreeWidget->setFocus( );
    QFrame::focusInEvent( event );
}

// Show list of available cameras and their groups
void ProjectTreeFrame::CreateBlankProjectTree( )
{
    XGuid rootId;

    ui->projectTreeWidget->clear( );
    mData->ProjectItemsMap.clear( );

    // create tree item
    mData->RootItem = new ProjectTreeItem( ui->projectTreeWidget, ProjectObjectType::Folder );
    mData->RootItem->setText( 0, QString::fromStdString( "Project" ) );
    mData->RootItem->setIcon( 0, QIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Folder  ) ) );
    mData->RootItem->setData( 0, Qt::UserRole, rootId.ToString( ).c_str( ) );

    // add the item into map
    mData->ProjectItemsMap.insert( rootId, mData->RootItem );

    // select the root note
    ui->projectTreeWidget->setCurrentItem( mData->RootItem );
}

// Check if selected item is a tree root item
bool ProjectTreeFrame::IsRootSelected( ) const
{
    bool ret = false;

    if ( ( mData->SelectedTreeItem != nullptr ) && ( mData->SelectedTreeItem->parent( ) == nullptr ) )
    {
        ret = true;
    }

    return ret;
}

// Get ID of the selected item
XGuid ProjectTreeFrame::GetSelectedItem( ) const
{
    XGuid ret;

    if ( mData->SelectedTreeItem != nullptr )
    {
        ret = XGuid( mData->SelectedTreeItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) );
    }

    return ret;
}

// Get ID of the selected folder or the folder containing selected item
XGuid ProjectTreeFrame::GetSelectedOrParentFolder( ) const
{
    XGuid ret = GetSelectedItem( );

    if ( GetProjectObjectType( ret ) != ProjectObjectType::Folder )
    {
        QTreeWidgetItem* parentItem = mData->SelectedTreeItem->parent( );

        if ( parentItem != nullptr )
        {
            ret = XGuid( parentItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) );
        }
    }

    return ret;
}

// Set selected item ID
void ProjectTreeFrame::SetSelectedItem( const XGuid& id )
{
    if ( mData->ProjectItemsMap.contains( id ) )
    {
        ProjectTreeItem* item = mData->ProjectItemsMap[id];

        ui->projectTreeWidget->setCurrentItem( item );
        ui->projectTreeWidget->scrollToItem( item );
    }
}

// Get type of the selected item
ProjectObjectType ProjectTreeFrame::GetSelectedItemType( ) const
{
    ProjectObjectType type = ProjectObjectType::None;

    if ( mData->SelectedTreeItem != nullptr )
    {
        type = GetProjectObjectType( GetSelectedItem( ) );
    }

    return type;
}

// Get type of the item with the given ID
ProjectObjectType ProjectTreeFrame::GetProjectObjectType( const XGuid& id )
{
    ProjectObjectType type = ProjectObjectType::None;

    if ( id.IsEmpty( ) )
    {
        // root is selected, which is folder
        type = ProjectObjectType::Folder;
    }
    else
    {
        type = ServiceManager::Instance( ).GetProjectManager( )->GetProjectObjectType( id );
    }

    return type;
}

// ==========================
// <<<<< Public slots >>>>
// ==========================

// Add project object to the project tree
void ProjectTreeFrame::AddObject(  const shared_ptr<ProjectObject>& po )
{
    if ( mData->ProjectItemsMap.contains( po->ParentId( ) ) )
    {
        ProjectTreeItem* parentItem = mData->ProjectItemsMap[po->ParentId( )];
        ProjectTreeItem* treeItem   = new ProjectTreeItem( parentItem, po->Type( ) );
        QString          iconName   = ProjectObjectIcon::GetResourceName( po->Type( ) );

        treeItem->setText( 0, QString::fromUtf8( po->Name( ).c_str( ) ) );
        treeItem->setData( 0, Qt::UserRole, po->Id( ).ToString( ).c_str( ) );

        if ( !iconName.isEmpty( ) )
        {
            treeItem->setIcon( 0, QIcon( iconName ) );
        }

        // add the item into map
        mData->ProjectItemsMap.insert( po->Id( ), treeItem );

        parentItem->sortChildren( 0, Qt::AscendingOrder );
        if ( mData->SelectNewItemOnAdding )
        {
            ui->projectTreeWidget->setCurrentItem( treeItem );
        }
    }
}

// Update project object in the project tree
void ProjectTreeFrame::UpdateObject( const shared_ptr<ProjectObject>& po )
{
    if ( mData->ProjectItemsMap.contains( po->Id( ) ) )
    {
        ProjectTreeItem* treeItem     = mData->ProjectItemsMap[po->Id( )];
        QTreeWidgetItem* parentItem   = treeItem->parent( );
        XGuid            parentInTree = XGuid::FromString( parentItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) );

        // update items label
        treeItem->setText( 0, QString::fromUtf8( po->Name( ).c_str( ) ) );

        if ( mData->SelectedTreeItem == treeItem )
        {
            ui->descriptionLabel->setPlainText( QString::fromUtf8( po->Description( ).c_str( ) ) );
        }

        if ( parentInTree == po->ParentId( ) )
        {
            // same parent; re-sort items at least in the case name was changed
            parentItem->sortChildren( 0, Qt::AscendingOrder );
        }
        else
        {
            // the object was moved to a new parent
            QTreeWidgetItem* newParentItem = mData->ProjectItemsMap[po->ParentId( )];

            parentItem->removeChild( treeItem );
            newParentItem->addChild( treeItem );
            newParentItem->sortChildren( 0, Qt::AscendingOrder );

            ui->projectTreeWidget->setCurrentItem( treeItem );
        }
    }
}

// Delete project object from the project tree
void ProjectTreeFrame::DeleteObject( const shared_ptr<ProjectObject>& po )
{
    XGuid poId = po->Id( );

    if ( mData->ProjectItemsMap.contains( poId ) )
    {
        ProjectTreeItem* treeItem   = mData->ProjectItemsMap[po->Id( )];
        QTreeWidgetItem* parentItem = treeItem->parent( );

        mData->ProjectItemsMap.remove( poId );
        parentItem->removeChild( treeItem );
    }
}

// ==========================
// <<<<< Action handlers >>>>
// ==========================

// Show context menu for the camera's tree
void ProjectTreeFrame::on_projectTreeWidget_customContextMenuRequested( const QPoint& pos )
{
    ProjectObjectType selectedType = GetSelectedItemType( );
    QMenu*            menuToShow   = nullptr;

    if ( mData->ContextMenus.contains( selectedType ) )
    {
        menuToShow = mData->ContextMenus[selectedType];
    }
    else
    {
        menuToShow = mData->DefaultContextMenu;
    }

    if ( menuToShow != nullptr )
    {
        QTreeWidgetItem* itemForMenu = ui->projectTreeWidget->itemAt( pos );

        if ( ( itemForMenu != nullptr ) && ( itemForMenu == mData->SelectedTreeItem ) )
        {
            menuToShow->popup( ui->projectTreeWidget->mapToGlobal( pos ) );
        }
    }
}

// Selection has changed in camera's tree
void ProjectTreeFrame::on_projectTreeWidget_itemSelectionChanged( )
{
    QList<QTreeWidgetItem*> selectedItems    = ui->projectTreeWidget->selectedItems( );
    QString                 descriptionToSet = QString::null;

    if ( selectedItems.count( ) == 1 )
    {
        QTreeWidgetItem* selectedItem = selectedItems.at( 0 );

        if ( selectedItem != mData->SelectedTreeItem )
        {
            mData->SelectedTreeItem = selectedItem;

            // get ID of the selected item
            XGuid selectedId( selectedItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) );

            if ( !selectedId.IsEmpty( ) )
            {
                // get project object for the selected item
                shared_ptr<ProjectObject> po = ServiceManager::Instance( ).GetProjectManager( )->GetProjectObject( selectedId );

                if ( po )
                {
                    descriptionToSet = QString::fromUtf8( po->Description( ).c_str( ) );
                }
            }

            emit ObjectSelected( selectedId );
        }
        else
        {
            emit ObjectSelected( XGuid( mData->SelectedTreeItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) ) );
        }
    }
    else
    {
        mData->SelectedTreeItem = nullptr;
    }

    ui->descriptionLabel->setPlainText( descriptionToSet );
}

// Dragging is about to start in the project's tree - handle it
void ProjectTreeFrame::on_projectTreeWidget_draggingAboutToStart( TreeDraggingStartEvent* event )
{
    //qDebug( "About to drag: %s", event->TreeItem( )->data( 0, Qt::UserRole ).toString( ).toAscii( ).data( ) );

    event->SetUrl( QUrl( event->TreeItem( )->data( 0, Qt::UserRole ).toString( ) ) );
    event->SetCanStartDragging( mData->AllowDragAndDrop );
}

// Dragging is in progress and now it is time to check if the target item can accept the draggable item
void ProjectTreeFrame::on_projectTreeWidget_dropTargetCheck( TreeDropTargetCheckEvent* event )
{
    XGuid   dragItemId = XGuid::FromString( event->Url( ).toString( ).toStdString( ) );
    XGuid   targetId   = XGuid( event->TargetTreeItem( )->data( 0, Qt::UserRole ).toString( ).toStdString( ) );

    //qDebug( "Check target: %s", event->TargetTreeItem( )->text( 0 ).toAscii( ).data( ) );
    //qDebug( "Target: %s, Source: %s", targetId.ToString().c_str( ), dragItemId.ToString().c_str() );

    event->SetCanTargetAccept( ServiceManager::Instance( ).GetProjectManager( )->
                               CheckIfObectCanMoveToNewParent( dragItemId, targetId ) );
}

// An item was dropped in the project's tree - handle item move
void ProjectTreeFrame::on_projectTreeWidget_itemDropped( TreeItemDroppedEvent* event )
{
    XGuid   dragItemId = XGuid::FromString( event->Url( ).toString( ).toStdString( ) );
    XGuid   targetId   = XGuid( event->TargetTreeItem( )->data( 0, Qt::UserRole ).toString( ).toStdString( ) );

    //qDebug( "Dropping item: %s", event->Url( ).toString( ).toAscii( ).data( ) );
    //qDebug( "Dropping at: %s", event->TargetTreeItem( )->text( 0 ).toAscii( ).data( ) );

    ServiceManager::Instance( ).GetProjectManager( )->MoveObjectToNewParent( dragItemId, targetId );
}

// An item is double clicked in the project tree
void ProjectTreeFrame::on_projectTreeWidget_itemDoubleClicked( QTreeWidgetItem* item, int )
{
    if ( mData->TriggerDefaultActionOnDoubleClick )
    {
        TriggerItemsDefaultAction( item );
    }
    else
    {
        emit ObjectDoubleClicked( XGuid( item->data( 0, Qt::UserRole ).toString( ).toStdString( ) ) );
    }
}

// Trigger default (marked by bold font) action for the item
void ProjectTreeFrame::TriggerItemsDefaultAction( QTreeWidgetItem* item )
{
    XGuid             itemId = XGuid( item->data( 0, Qt::UserRole ).toString( ).toStdString( ) );
    ProjectObjectType type   = GetProjectObjectType( itemId );

    if ( mData->ContextMenus.contains( type ) )
    {
        QAction* defaultAction = mData->ContextMenus[type]->defaultAction( );

        if ( defaultAction != nullptr )
        {
            defaultAction->trigger( );
        }
    }
}
