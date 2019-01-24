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
#include "SelectPluginPage.hpp"
#include "ui_SelectPluginPage.h"
#include "ServiceManager.hpp"

#include <PluginsListFrame.hpp>
#include <UITools.hpp>
#include <QDialog>

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class SelectPluginPageData
    {
    public:
        SelectPluginPageData( ) :
            AvailablePluginsListFrame( new PluginsListFrame( ) ),
            CanGoNext( false ),
            PluginsContextMenu( nullptr ), GetPluginHelpAction( nullptr )
        {
            // create context menu for plug-ins
            PluginsContextMenu = new QMenu( );

            GetPluginHelpAction = new QAction( nullptr );
            GetPluginHelpAction->setText( "Description" );
            GetPluginHelpAction->setStatusTip( "Show description/help for the selected plug-in" );
            GetPluginHelpAction->setIcon( QIcon( QPixmap( ":/images/icons/idea.png" ) ) );

            AddPluginToFavouritesAction = new QAction( nullptr );
            AddPluginToFavouritesAction->setText( "Add to favourites" );
            AddPluginToFavouritesAction->setStatusTip( "Add plug-in to the list of favourite plug-ins" );
            AddPluginToFavouritesAction->setIcon( QIcon( QPixmap( ":/images/icons/favourite_add.png" ) ) );

            RemovePluginFromFavouritesAction = new QAction( nullptr );
            RemovePluginFromFavouritesAction->setText( "Remove from favourites" );
            RemovePluginFromFavouritesAction->setStatusTip( "Remove plug-in from the list of favourite plug-ins" );
            RemovePluginFromFavouritesAction->setIcon( QIcon( QPixmap( ":/images/icons/favourite_remove.png" ) ) );

            QAction* separator = new QAction( nullptr );
            separator->setSeparator( true );

            PluginsContextMenu->addAction( GetPluginHelpAction );
            PluginsContextMenu->addAction( separator );
            PluginsContextMenu->addAction( AddPluginToFavouritesAction );
            PluginsContextMenu->addAction( RemovePluginFromFavouritesAction );
        }

        ~SelectPluginPageData( )
        {
            delete AvailablePluginsListFrame;
            delete AddPluginToFavouritesAction;
            delete RemovePluginFromFavouritesAction;
        }

    public:
        PluginsListFrame*   AvailablePluginsListFrame;
        bool                CanGoNext;

        QMenu*      PluginsContextMenu;
        QAction*    GetPluginHelpAction;
        QAction*    AddPluginToFavouritesAction;
        QAction*    RemovePluginFromFavouritesAction;

    };
}

SelectPluginPage::SelectPluginPage( PluginType typesMask, QWidget *parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::SelectPluginPage ),
    mData( new Private::SelectPluginPageData )
{
    ui->setupUi( this );
    ui->groupBox->layout( )->addWidget( mData->AvailablePluginsListFrame );

    connect( mData->AvailablePluginsListFrame, SIGNAL( SelectedPluginChanged( ) ), this, SLOT( on_SelectedPluginChanged( ) ) );

    auto&   servicrManager = ServiceManager::Instance( );
    auto    pluginsEngine  = servicrManager.GetPluginsEngine( );

    // set modules to display in plug-ins list frame
    mData->AvailablePluginsListFrame->SetModulesCollection( pluginsEngine->GetFamilies( ), pluginsEngine->GetModules( ),
                                                            servicrManager.GetFavouritePluginsManager( )->PluginsCollection( ),
                                                            typesMask );

    // attach context menus to the tree of available plug-ins
    mData->AvailablePluginsListFrame->SetPluginsContextMenu( mData->PluginsContextMenu,
                                                             mData->AddPluginToFavouritesAction,
                                                             mData->RemovePluginFromFavouritesAction );
    connect( mData->GetPluginHelpAction, SIGNAL(triggered()), this, SLOT(on_GetPluginHelpAction_triggered()) );
}

SelectPluginPage::~SelectPluginPage( )
{
    ServiceManager::Instance( ).GetFavouritePluginsManager( )->Save( );
    delete mData;
    delete ui;
}

// Set title of the group box
void SelectPluginPage::SetTitle(  const QString& title )
{
    ui->groupBox->setTitle( title );
}

// The page got focus, so transfer to the child control we need
void SelectPluginPage::focusInEvent( QFocusEvent* event )
{
    mData->AvailablePluginsListFrame->setFocus( );
    WizardPageFrame::focusInEvent( event );
}

// Check if the page is in a valid state and wizard can go to the next one
bool SelectPluginPage::CanGoNext( ) const
{
    return mData->CanGoNext;
}

// Plug-in selection changed
void SelectPluginPage::on_SelectedPluginChanged( )
{
    bool isPluginSelected = !mData->AvailablePluginsListFrame->GetSelectedPluginID( ).IsEmpty( );

    if ( isPluginSelected != mData->CanGoNext )
    {
        mData->CanGoNext = isPluginSelected;
        emit CompleteStatusChanged( mData->CanGoNext );
    }
}

// Show description for the currently selected plug-in
void SelectPluginPage::on_GetPluginHelpAction_triggered( )
{
    const XGuid pluginID = mData->AvailablePluginsListFrame->GetSelectedPluginID( );

    if ( !pluginID.IsEmpty( ) )
    {
        // show description for the plugin with the specified ID
        ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( UITools::GetParentDialog( this ), pluginID );
    }
}

// Get ID of the currently selected plug-in
const XGuid SelectPluginPage::GetSelectedPluginID( ) const
{
    return ( mData->AvailablePluginsListFrame != 0 ) ?
                mData->AvailablePluginsListFrame->GetSelectedPluginID( ) : XGuid( );
}
