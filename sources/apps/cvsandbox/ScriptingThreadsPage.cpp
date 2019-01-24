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

#include "ScriptingThreadsPage.hpp"
#include "ui_ScriptingThreadsPage.h"

#include "EditScriptingThreadDialog.hpp"
#include "ServiceManager.hpp"

#include <XImageInterface.hpp>
#include <UITools.hpp>
#include <ConfigurePluginPropertiesDialog.hpp>

#include <set>

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class ScriptingThreadsPageData
    {
    private:
        Ui::ScriptingThreadsPage* ui;
        ScriptingThreadsPage*     Parent;

    public:
        map<XGuid, ScriptingThreadDesc> Threads;
        XGuid                           ConfiguredPluginId;

    public:
        ScriptingThreadsPageData( Ui::ScriptingThreadsPage* parentUi, ScriptingThreadsPage* parent ) :
            ui( parentUi ), Parent( parent ), Threads( ), ConfiguredPluginId( )
        {
        }

        XGuid GetSelectedThreadId( ) const;
        void AddThreadTreeItem( const ScriptingThreadDesc& threadDesc, bool selectItem = false );
        void UpdateThreadTreeItem( const ScriptingThreadDesc& threadDesc );

        set<string> GetThreadNames( ) const;

        void EditSelectedThread( );
    };
}

ScriptingThreadsPage::ScriptingThreadsPage( QWidget* parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::ScriptingThreadsPage ),
    mData( new ::Private::ScriptingThreadsPageData( ui, this ) )
{
    ui->setupUi( this );

    ui->threadsTree->setColumnWidth( 0, 150 );
    ui->threadsTree->setColumnWidth( 1, 100 );
    ui->threadsTree->setColumnWidth( 2, 300 );

    on_threadsTree_itemSelectionChanged( );
}

ScriptingThreadsPage::~ScriptingThreadsPage( )
{
    delete ui;
}

// Get configured scripting threads
map<XGuid, ScriptingThreadDesc> ScriptingThreadsPage::Threads( ) const
{
    return mData->Threads;
}

// Set threads' description to edit
void ScriptingThreadsPage::SetThreads( const map<XGuid, ScriptingThreadDesc>& threads )
{
    mData->Threads = threads;

    for ( auto pair : threads )
    {
        mData->AddThreadTreeItem( pair.second );
    }

    //
}

// Selected item has changed in the threads' tree view
void ScriptingThreadsPage::on_threadsTree_itemSelectionChanged( )
{
    bool itemSelected = ( ui->threadsTree->currentItem( ) != nullptr );

    ui->editThreadButton->setEnabled( itemSelected );
    ui->deleteThreadButton->setEnabled( itemSelected );

    if ( itemSelected )
    {
        XGuid threadId = mData->GetSelectedThreadId( );
        XGuid pluginId = mData->Threads[threadId].PluginId( );

        shared_ptr<const XPluginDescriptor> pluginDesc = ServiceManager::Instance( ).GetPluginsEngine( )->
                                                         GetPlugin( pluginId );

        ui->editThreadPropertiesButton->setEnabled( static_cast<bool>( pluginDesc ) );
    }
    else
    {
        ui->editThreadPropertiesButton->setEnabled( false );
    }
}

// "Add New" button was clicked - add new scripting thread
void ScriptingThreadsPage::on_addThreadButton_clicked( )
{
    EditScriptingThreadDialog dialog( ScriptingThreadDesc( ), this );

    dialog.SetExistingNames( mData->GetThreadNames( ) );

    if ( dialog.exec( ) == QDialog::Accepted )
    {
        ScriptingThreadDesc threadDesc = dialog.ThreadDescription( );

        mData->Threads.insert( pair<XGuid, ScriptingThreadDesc>( threadDesc.Id( ), threadDesc ) );
        mData->AddThreadTreeItem( threadDesc, true );
    }

    ui->threadsTree->setFocus( );
}

// "Edit" button clicked - edit description of the selected thread
void ScriptingThreadsPage::on_editThreadButton_clicked( )
{
    if ( ui->threadsTree->currentItem( ) != nullptr )
    {
        mData->EditSelectedThread( );
    }
}

// "Delete" button clicked - delete selected thread
void ScriptingThreadsPage::on_deleteThreadButton_clicked( )
{
    if ( ui->threadsTree->currentItem( ) != nullptr )
    {
        if ( UITools::GetUserConfirmation( "Are you sure you would like to delete the selected scripting thread?", this ) )
        {
            mData->Threads.erase( mData->GetSelectedThreadId( ) );
            delete ui->threadsTree->currentItem( );
        }
    }

    ui->threadsTree->setFocus( );
}

// Edit properties of the scripting plug-in for the selected thread
void ScriptingThreadsPage::on_editThreadPropertiesButton_clicked( )
{
    if ( ui->threadsTree->currentItem( ) != nullptr )
    {
        XGuid threadId = mData->GetSelectedThreadId( );
        XGuid pluginId = mData->Threads[threadId].PluginId( );

        shared_ptr<const XPluginDescriptor> pluginDesc = ServiceManager::Instance( ).GetPluginsEngine( )->
                                                         GetPlugin( pluginId );

        if ( ( pluginDesc ) && ( pluginDesc->ConfigurablePropertiesCount( ) != 0 ) )
        {
            shared_ptr<XPlugin> plugin = pluginDesc->CreateInstance( );

            if ( plugin )
            {
                // set plug-in configuration
                pluginDesc->SetPluginConfiguration( plugin, mData->Threads[threadId].PluginConfiguration( ) );

                ConfigurePluginPropertiesDialog configureForm( pluginDesc, plugin, false, UITools::GetParentDialog( this ) );

                configureForm.EnableHelpButton( true );
                connect( &configureForm, SIGNAL(HelpRequested(QWidget*)), this, SLOT(on_propertiesDialog_HelpRequested(QWidget*)) );

                mData->ConfiguredPluginId = pluginId;

                if ( configureForm.exec( ) == QDialog::Accepted )
                {
                    mData->Threads[threadId].SetPluginConfiguration( pluginDesc->GetPluginConfiguration( plugin ) );
                }
            }
        }
    }

    ui->threadsTree->setFocus( );
}

// Item double clicked in threads tree view - edit it
void ScriptingThreadsPage::on_threadsTree_itemDoubleClicked( QTreeWidgetItem* /* item */, int /* column */ )
{
    if ( ui->threadsTree->currentItem( ) != nullptr )
    {
        mData->EditSelectedThread( );
    }
}

// Requested help for the scripting engine of the currently edited thread
void ScriptingThreadsPage::on_propertiesDialog_HelpRequested( QWidget* sender )
{
    // show description for the plug-in currently being edited
    ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( sender, mData->ConfiguredPluginId );
}

namespace Private
{

// Get name of the selected thread
XGuid ScriptingThreadsPageData::GetSelectedThreadId( ) const
{
    QTreeWidgetItem* selectedItem = ui->threadsTree->currentItem( );
    XGuid            threadId;

    if ( selectedItem != nullptr )
    {
        threadId = XGuid::FromString( selectedItem->data( 0, Qt::UserRole ).toString( ).toStdString( ) );
    }

    return threadId;
}

// Add an item into the tree view of threads
void ScriptingThreadsPageData::AddThreadTreeItem( const ScriptingThreadDesc& threadDesc, bool selectItem )
{
    QTreeWidgetItem* treeNode = new QTreeWidgetItem( ui->threadsTree );
    QString          name     = QString::fromUtf8( threadDesc.Name( ).c_str( ) );
    QString          desc     = QString::fromUtf8( threadDesc.Description( ).c_str( ) );

    shared_ptr<const XPluginDescriptor> pluginDesc = ServiceManager::Instance( ).GetPluginsEngine( )->
                                                     GetPlugin( threadDesc.PluginId( ) );

    treeNode->setText( 0, name );
    treeNode->setText( 1, QString( "%0" ).arg( threadDesc.Interval( ) ) );
    treeNode->setText( 2, desc );
    treeNode->setData( 0, Qt::UserRole, QString::fromStdString( threadDesc.Id( ).ToString( ) ) );

    if ( pluginDesc )
    {
        shared_ptr<const XImage> icon = pluginDesc->Icon( );

        if ( icon )
        {
            shared_ptr<QImage> qimage = XImageInterface::XtoQimage( icon );
            treeNode->setIcon( 0, QIcon( QPixmap::fromImage( *qimage ) ) );
        }
    }

    if ( selectItem )
    {
        ui->threadsTree->setCurrentItem( treeNode );
    }
}

// Update selected item's description
void ScriptingThreadsPageData::UpdateThreadTreeItem( const ScriptingThreadDesc& threadDesc )
{
    QTreeWidgetItem* treeNode = ui->threadsTree->currentItem( );
    QString          name     = QString::fromUtf8( threadDesc.Name( ).c_str( ) );
    QString          desc     = QString::fromUtf8( threadDesc.Description( ).c_str( ) );

    treeNode->setText( 0, name );
    treeNode->setText( 1, QString( "%0" ).arg( threadDesc.Interval( ) ) );
    treeNode->setText( 2, desc );
}

// Get names of all configured threads
set<string> ScriptingThreadsPageData::GetThreadNames( ) const
{
    set<string> names;

    for ( auto pair : Threads )
    {
        names.insert( pair.second.Name( ) );
    }

    return names;
}

// Edit currently selected thread
void ScriptingThreadsPageData::EditSelectedThread( )
{
    XGuid                     threadId      = GetSelectedThreadId( );
    ScriptingThreadDesc       threadDesc    = Threads[threadId];
    set<string>               existingNames = GetThreadNames( );
    EditScriptingThreadDialog dialog( threadDesc, Parent );

    existingNames.erase( threadDesc.Name( ) );
    dialog.SetExistingNames( existingNames );

    if ( dialog.exec( ) == QDialog::Accepted )
    {
        threadDesc = dialog.ThreadDescription( );

        Threads[threadId] = threadDesc;
        UpdateThreadTreeItem( threadDesc );
    }

    ui->threadsTree->setFocus( );
}

} // namespace Private
