/*
    Common tools for Computer Vision Sandbox

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

#include "ui_PluginsListFrame.h"
#include "PluginsListFrame.hpp"
#include "XImageInterface.hpp"

using namespace std;
using namespace CVSandbox;

PluginsListFrame::PluginsListFrame( QWidget *parent ) :
    QFrame( parent ),
    ui( new Ui::PluginsListFrame ),
    mFamiliesCollection( ),
    mModulesCollection( ),
    mFavouritePlugins( ),
    mPluginTypesMask( PluginType_All ),
    mPluginsContextMenu( nullptr ),
    mAddPluginToFavouritesAction( nullptr ),
    mRemovePluginFromFavouritesAction( nullptr ),
    mSelectedComboIndex( -1 ),
    mSelectedTreeItem( nullptr )
{
    ui->setupUi( this );
}

PluginsListFrame::~PluginsListFrame( )
{
    Cleanup( );
    delete ui;
}

// Perform cleanup
void PluginsListFrame::Cleanup( )
{
    ui->modulesCombo->clear( );
    ui->pluginsTree->clear( );

    mFamiliesCollection.reset( );
    mModulesCollection.reset( );
}

// Set collection of modules modules to gather plug-ins from
void PluginsListFrame::SetModulesCollection(
        const shared_ptr<const XFamiliesCollection>& familiesCollection,
        const shared_ptr<const XModulesCollection>& modulesCollection,
        const shared_ptr<XPluginsCollection>& favouritePlugins,
        PluginType typesMask )
{
    Cleanup( );

    if ( ( familiesCollection ) && ( modulesCollection ) )
    {
        mFamiliesCollection = familiesCollection;
        mModulesCollection  = modulesCollection;
        mFavouritePlugins   = favouritePlugins;
        mPluginTypesMask    = typesMask;

        ShowModulesList( );
    }
}

// Set context menu to display for plug-ins
void PluginsListFrame::SetPluginsContextMenu( QMenu* menu, QAction* addPluginToFavouritesAction,
                                                           QAction* removePluginFromFavouritesAction )
{
    mPluginsContextMenu = menu;
    mAddPluginToFavouritesAction = addPluginToFavouritesAction;
    mRemovePluginFromFavouritesAction = removePluginFromFavouritesAction;

    if ( ( mAddPluginToFavouritesAction != nullptr ) && ( mRemovePluginFromFavouritesAction != nullptr ) )
    {
        connect( mPluginsContextMenu, SIGNAL(aboutToShow()), this, SLOT(on_PluginsContextMenu_aboutToShow()) );
        connect( mAddPluginToFavouritesAction, SIGNAL(triggered()), this, SLOT(on_AddPluginToFavouritesAction_triggered()) );
        connect( mRemovePluginFromFavouritesAction, SIGNAL(triggered()), this, SLOT(on_RemovePluginFromFavouritesAction_triggered()) );
    }
}

// Get ID of the currently selected plug-in
const XGuid PluginsListFrame::GetSelectedPluginID( ) const
{
    XGuid ret;

    if ( ( mSelectedTreeItem != 0 ) && ( mSelectedTreeItem->parent( ) != 0 ) )
    {
        ret = XGuid( mSelectedTreeItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) );
    }

    return ret;
}

// Show available modules
void PluginsListFrame::ShowModulesList( )
{
    ui->modulesCombo->clear( );

    // add all available modules first
    for ( XModulesCollection::ConstIterator it = mModulesCollection->begin( ); it != mModulesCollection->end( ); it++  )
    {
        shared_ptr<const XPluginsModule> module = *it;

        if ( module->CountType( mPluginTypesMask ) == 0 )
            continue;

        shared_ptr<const XImage> icon = module->Icon( );

        QString itemTitle = module->Name( ).c_str( );
        QVariant itemData = QString( module->ID( ).ToString( ).c_str( ) );

        if ( icon )
        {
            shared_ptr<QImage> qimage = XImageInterface::XtoQimage( icon );
            ui->modulesCombo->addItem( QIcon( QPixmap::fromImage( *qimage ) ), itemTitle, itemData );
        }
        else
        {
            ui->modulesCombo->addItem( itemTitle, itemData );
        }
    }

    // sort modules in alphabetic order
    ui->modulesCombo->model( )->sort( 0 );

    // now insert All and Favourites to make those always on top
    ui->modulesCombo->insertItem( 0, QIcon( QString::fromUtf8( ":/images/icons/block.png" ) ),
                                     tr( "All Plug-ins" )  );

    if ( mFavouritePlugins )
    {
        ui->modulesCombo->insertItem( 0, QIcon( QString::fromUtf8( ":/images/icons/favourite.png" ) ),
                                         tr( "Favourite Plug-ins" ) );
    }


    ui->modulesCombo->setCurrentIndex( ( mFavouritePlugins ) ? 1 : 0 );

}

// Get collection of plug-ins to show based on currently selected module
const shared_ptr<const XPluginsCollection> PluginsListFrame::GetPluginsToShow( ) const
{
    shared_ptr<const XPluginsCollection> pluginsCollection;
    int selectedComboIndex = ui->modulesCombo->currentIndex( );

    if ( selectedComboIndex != -1 )
    {
        XGuid moduleGuid( ui->modulesCombo->itemData( selectedComboIndex ).toString( ).toStdString( ) );

        if ( !moduleGuid.IsEmpty( ) )
        {
            shared_ptr<const XPluginsModule> module = mModulesCollection->GetModule( moduleGuid );

            if ( module )
            {
                pluginsCollection = module->GetPluginsOfType( mPluginTypesMask );
            }
        }
        else
        {
            pluginsCollection =
                ( ( mFavouritePlugins ) && ( ui->modulesCombo->currentIndex( ) == 0 ) ) ?
                    mFavouritePlugins->GetPluginsOfType( mPluginTypesMask ) :
                    mModulesCollection->GetPluginsOfType( mPluginTypesMask );
        }
    }

    return pluginsCollection;
}

// Create tree item for the family with specified ID
QTreeWidgetItem* PluginsListFrame::CtreateFamilyItem( const shared_ptr<const XFamily>& family )
{
    QTreeWidgetItem* familyNode = 0;

    if ( family )
    {
        familyNode = new QTreeWidgetItem( ui->pluginsTree );
        familyNode->setText( 0, family->Name( ).c_str( ) );
        familyNode->setData( 0, Qt::UserRole, QVariant( QString( family->ID( ).ToString( ).c_str( ) ) ) );

        shared_ptr<const XImage> icon = family->Icon( );

        if ( icon )
        {
            shared_ptr<QImage> qimage = XImageInterface::XtoQimage( icon );
            familyNode->setIcon(0, QIcon( QPixmap::fromImage( *qimage ) ) );
        }
    }

    return familyNode;
}

// Update plug-ins list after module selection was done
void PluginsListFrame::UpdatePluginsList( )
{
    shared_ptr<const XPluginsCollection> pluginsCollection = GetPluginsToShow( );

    ui->pluginsTree->clear( );

    if ( pluginsCollection )
    {
        QMap<XGuid, QTreeWidgetItem*> familyNodes;

        for ( XPluginsCollection::ConstIterator it = pluginsCollection->begin( ); it != pluginsCollection->end( ); it++ )
        {
            shared_ptr<const XPluginDescriptor> plugin = *it;
            shared_ptr<const XFamily> family = mFamiliesCollection->GetFamily( plugin->FamilyID( ) );

            if ( !family )
            {
                family = XFamily::DefaultFamily( );

                if ( !family )
                    continue;
            }

            XGuid            familyID   = family->ID( );
            QTreeWidgetItem* familyNode = 0;

            // get family node for the family ID
            if ( familyNodes.contains( familyID ) )
            {
                familyNode = familyNodes[familyID];
            }
            else
            {
                familyNode = CtreateFamilyItem( family );
                if ( familyNode != 0 )
                {
                    familyNodes[familyID] = familyNode;
                }
            }

            if ( familyNode != 0 )
            {
                // add plug-in node if family node was located/created
                QTreeWidgetItem* pluginNode = new QTreeWidgetItem( familyNode );
                pluginNode->setText( 0, plugin->Name( ).c_str( ) );
                pluginNode->setData( 0, Qt::UserRole, QVariant( QString( plugin->ID( ).ToString( ).c_str( ) ) ) );

                shared_ptr<const XImage> icon = plugin->Icon( );

                if ( icon )
                {
                    shared_ptr<QImage> qimage = XImageInterface::XtoQimage( icon );
                    pluginNode->setIcon(0, QIcon( QPixmap::fromImage( *qimage ) ) );
                }
            }
        }

        ui->pluginsTree->sortByColumn( 0, Qt::AscendingOrder );

        // select the first top item
        if ( ui->pluginsTree->topLevelItemCount( ) > 0 )
        {
            ui->pluginsTree->setCurrentItem( ui->pluginsTree->topLevelItem( 0 ), 0 );
        }
    }
}

// Selection in modules' combo box was changed, so update plug-in's list
void PluginsListFrame::on_modulesCombo_currentIndexChanged( int index )
{
    if ( mSelectedComboIndex != index)
    {
        mSelectedComboIndex = index;
        UpdatePluginsList( );
    }
}

// Handle selection change in plug-in's tree view
void PluginsListFrame::on_pluginsTree_itemSelectionChanged( )
{
    QList<QTreeWidgetItem*> selectedItems = ui->pluginsTree->selectedItems( );

    if ( selectedItems.count( ) == 1 )
    {
        QTreeWidgetItem* selectedItem = selectedItems.at( 0 );

        if ( selectedItem != mSelectedTreeItem )
        {
            mSelectedTreeItem = selectedItem;

            XGuid guid( selectedItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) );

            if ( !guid.IsEmpty( ) )
            {
                if ( selectedItem->parent( ) == 0 )
                {
                    OnFamilySelected( guid );
                }
                else
                {
                    OnPluginSelected( guid );
                }
            }

            emit SelectedPluginChanged( );
        }
    }
    else
    {
        mSelectedTreeItem = 0;
        ui->descriptionLabel->clear( );
    }
}

// Show context menu for plug-ins
void PluginsListFrame::on_pluginsTree_customContextMenuRequested( const QPoint &pos )
{
    if ( mPluginsContextMenu != 0 )
    {
        QTreeWidgetItem* itemForMenu = ui->pluginsTree->itemAt( pos );

        if ( ( itemForMenu != 0 ) && ( itemForMenu->parent( ) != 0 ) && ( itemForMenu == mSelectedTreeItem ) )
        {
            mPluginsContextMenu->popup( ui->pluginsTree->mapToGlobal( pos ) );
        }
    }
}

// Trigger default context menu's action on double click
void PluginsListFrame::on_pluginsTree_itemDoubleClicked( QTreeWidgetItem *item, int )
{
    if ( ( item != nullptr ) && ( mPluginsContextMenu != nullptr ) )
    {
        QAction* defaultAction = mPluginsContextMenu->defaultAction( );

        if ( defaultAction != nullptr )
        {
            defaultAction->trigger( );
        }
    }
}

// Family was selected - show its description
void PluginsListFrame::OnFamilySelected( const XGuid& id )
{
    shared_ptr<const XFamily> family = mFamiliesCollection->GetFamily( id );

    if ( family )
    {
        ui->descriptionLabel->setText( family->Description( ).c_str( ) );
    }
}

// Plug-in was selected - show its description
void PluginsListFrame::OnPluginSelected( const XGuid& id )
{
    shared_ptr<const XPluginDescriptor> plugin = mModulesCollection->GetPlugin( id );

    if ( plugin )
    {
        ui->descriptionLabel->setText( plugin->Description( ).c_str( ) );
    }
}

// The control got focus - set it to tree
void PluginsListFrame::focusInEvent( QFocusEvent* event )
{
    ui->pluginsTree->setFocus( );
    QFrame::focusInEvent( event );
}


// Plug-ins context menu is about to be shown - show/hide some actions
void PluginsListFrame::on_PluginsContextMenu_aboutToShow( )
{
    bool showAdd    = false;
    bool showRemove = false;

    if ( mFavouritePlugins )
    {
        if ( ui->modulesCombo->currentIndex( ) == 0 )
        {
            showRemove = true;
        }
        else
        {
            showAdd = true;
        }
    }

    mAddPluginToFavouritesAction->setVisible( showAdd );
    mRemovePluginFromFavouritesAction->setVisible( showRemove );
}

// Add currently selected plug-in to the favourites' list
void PluginsListFrame::on_AddPluginToFavouritesAction_triggered( )
{
    XGuid selectedId = GetSelectedPluginID( );

    if ( ( !selectedId.IsEmpty( ) ) && ( !mFavouritePlugins->GetPlugin( selectedId ) ) )
    {
        shared_ptr<const XPluginDescriptor> plugin = mModulesCollection->GetPlugin( selectedId );

        if ( plugin )
        {
            mFavouritePlugins->Add( plugin );
        }
    }
}

// Remove currently selected plug-in from the favourites' list
void PluginsListFrame::on_RemovePluginFromFavouritesAction_triggered( )
{
    XGuid selectedId = GetSelectedPluginID( );

    if ( ( !selectedId.IsEmpty( ) ) && ( mFavouritePlugins->GetPlugin( selectedId ) ) )
    {
        QTreeWidgetItem* selection = ui->pluginsTree->currentItem( );
        QTreeWidgetItem* parent    = selection->parent( );

        mFavouritePlugins->Remove( selectedId );

        delete selection;
        if ( parent->childCount( ) == 0 )
        {
            delete parent;
        }
    }
}
