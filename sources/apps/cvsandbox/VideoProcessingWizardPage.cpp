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

#include "VideoProcessingWizardPage.hpp"
#include "ui_VideoProcessingWizardPage.h"

#include "ServiceManager.hpp"
#include "CameraProjectObject.hpp"
#include "ProjectObjectIcon.hpp"
#include "RenameProcessingStepDialog.hpp"

#include <PluginsListFrame.hpp>
#include <XImageInterface.hpp>
#include <ConfigurePluginPropertiesDialog.hpp>
#include <UITools.hpp>

#include <stack>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

typedef map<XGuid, XVideoSourceProcessingGraph>::const_iterator  GraphConstIter;
typedef map<XGuid, XVideoSourceProcessingGraph>::iterator        GraphIter;

namespace Private
{
    class VideoProcessingWizardPageData
    {
    public:
        VideoProcessingWizardPageData( Ui::VideoProcessingWizardPage* parentUi, VideoProcessingWizardPage* parent ) :
            ui( parentUi ), Parent( parent ),
            CanGoNext( false ), AvailablePluginsContextMenu( nullptr ),
            AddPluginAction( nullptr ), GetPluginHelpAction( nullptr ),
            GraphPluginsContextMenu( nullptr ), RemovePluginFromGraphAction( nullptr ),
            ConfigurePluginAction( nullptr ), RenameStepAction( nullptr ),
            MovePluginUpAction( nullptr ), MovePluginDownAction( nullptr ),
            GetAddedPluginHelpAction( nullptr ), ConfiguredPluginId( )
        {
            // create context menu for available plug-ins
            AvailablePluginsContextMenu = new QMenu( );

            // create actions
            AddPluginAction = new QAction( nullptr );
            AddPluginAction->setText( "&Add plug-in" );
            AddPluginAction->setStatusTip( "Add plug-in to the video processing graph" );
            AddPluginAction->setIcon( QIcon( QPixmap( ":/images/icons/add.png" ) ) );

            GetPluginHelpAction = new QAction( nullptr );
            GetPluginHelpAction->setText( "&Description" );
            GetPluginHelpAction->setStatusTip( "Show description/help for the selected plug-in" );
            GetPluginHelpAction->setIcon( QIcon( QPixmap( ":/images/icons/idea.png" ) ) );

            AddPluginToFavouritesAction = new QAction( nullptr );
            AddPluginToFavouritesAction->setText( "Add to &favourites" );
            AddPluginToFavouritesAction->setStatusTip( "Add plug-in to the list of favourite plug-ins" );
            AddPluginToFavouritesAction->setIcon( QIcon( QPixmap( ":/images/icons/favourite_add.png" ) ) );

            RemovePluginFromFavouritesAction = new QAction( nullptr );
            RemovePluginFromFavouritesAction->setText( "&Remove from favourites" );
            RemovePluginFromFavouritesAction->setStatusTip( "Remove plug-in from the list of favourite plug-ins" );
            RemovePluginFromFavouritesAction->setIcon( QIcon( QPixmap( ":/images/icons/favourite_remove.png" ) ) );

            QAction* separator1 = new QAction( nullptr );
            QAction* separator2 = new QAction( nullptr );
            separator1->setSeparator( true );
            separator2->setSeparator( true );

            // add actions to the context menu of available plug-ins list
            AvailablePluginsContextMenu->addAction( AddPluginAction );
            AvailablePluginsContextMenu->addAction( separator1 );
            AvailablePluginsContextMenu->addAction( AddPluginToFavouritesAction );
            AvailablePluginsContextMenu->addAction( RemovePluginFromFavouritesAction );
            AvailablePluginsContextMenu->addAction( separator2 );
            AvailablePluginsContextMenu->addAction( GetPluginHelpAction );
            AvailablePluginsContextMenu->setDefaultAction( AddPluginAction );

            // create context menu for plug-ins added to processing graph
            GraphPluginsContextMenu = new QMenu( );

            ConfigurePluginAction = new QAction( nullptr );
            ConfigurePluginAction->setText( "&Configure" );
            ConfigurePluginAction->setStatusTip( "Configure selected step of the processing graph" );
            ConfigurePluginAction->setIcon( QIcon( QPixmap( ":/images/icons/properties.png" ) ) );

            RenameStepAction = new QAction( nullptr );
            RenameStepAction->setText( "&Rename" );
            RenameStepAction->setStatusTip( "Rename video processing step" );

            RemovePluginFromGraphAction = new QAction( nullptr );
            RemovePluginFromGraphAction->setText( "&Remove" );
            RemovePluginFromGraphAction->setStatusTip( "Remove selected step from processing graph" );
            RemovePluginFromGraphAction->setIcon( QIcon( QPixmap( ":/images/icons/remove.png" ) ) );

            MovePluginUpAction = new QAction( nullptr );
            MovePluginUpAction->setText( "Move &up" );
            MovePluginUpAction->setStatusTip( "Move the step up in the processing graph" );
            MovePluginUpAction->setIcon( QIcon( QPixmap( ":/images/icons/up.png" ) ) );

            MovePluginDownAction = new QAction( nullptr );
            MovePluginDownAction->setText( "Move d&own" );
            MovePluginDownAction->setStatusTip( "Move the step down in the processing graph" );
            MovePluginDownAction->setIcon( QIcon( QPixmap( ":/images/icons/down.png" ) ) );

            GetAddedPluginHelpAction = new QAction( nullptr );
            GetAddedPluginHelpAction->setText( "&Description" );
            GetAddedPluginHelpAction->setStatusTip( "Show description/help for the selected plug-in" );
            GetAddedPluginHelpAction->setIcon( QIcon( QPixmap( ":/images/icons/idea.png" ) ) );

            QAction* separator3 = new QAction( nullptr );
            QAction* separator4 = new QAction( nullptr );
            QAction* separator5 = new QAction( nullptr );
            separator3->setSeparator( true );
            separator4->setSeparator( true );
            separator5->setSeparator( true );

            // add actions to the context menu of video processing graph
            GraphPluginsContextMenu->addAction( ConfigurePluginAction );
            GraphPluginsContextMenu->addAction( RenameStepAction );
            GraphPluginsContextMenu->addAction( separator3 );
            GraphPluginsContextMenu->addAction( RemovePluginFromGraphAction );
            GraphPluginsContextMenu->addAction( separator4 );
            GraphPluginsContextMenu->addAction( MovePluginUpAction );
            GraphPluginsContextMenu->addAction( MovePluginDownAction );
            GraphPluginsContextMenu->addAction( separator5 );
            GraphPluginsContextMenu->addAction( GetAddedPluginHelpAction );
            GraphPluginsContextMenu->setDefaultAction( ConfigurePluginAction );
        }

        ~VideoProcessingWizardPageData( )
        {
            delete AddPluginAction;
            delete GetPluginHelpAction;
            delete AddPluginToFavouritesAction;
            delete RemovePluginFromFavouritesAction;
            delete AvailablePluginsContextMenu;

            delete GetAddedPluginHelpAction;
            delete RemovePluginFromGraphAction;
            delete ConfigurePluginAction;
            delete MovePluginUpAction;
            delete MovePluginDownAction;
            delete GraphPluginsContextMenu;
        }

        int32_t GetSelectedStepIndex( ) const;
        int32_t GetCurrentGraphSize( ) const;
        const XGuid GetSelectedProcessingStepId( ) const;
        const XGuid GetSelectedCameraId( ) const;
        int32_t AddVideoProcessingStep( const string& stepName, const XGuid& pluginId );

        void UpdateAddPluginButtonState( );
        void UpdateProcessingGraphButtonsState( );
        void UpdateProcessingGraphActionsState( );
        void ShowVideoProcessingGraphForSelectedCamera( );
        void ConfigureSelectedProcessingStep( );
        void RenameSelectedProcessingStep( );
        void MoveSelectedStep( bool moveUp );

    private:
        Ui::VideoProcessingWizardPage* ui;
        VideoProcessingWizardPage*     Parent;

    public:
        bool        CanGoNext;
        QMenu*      AvailablePluginsContextMenu;
        QAction*    AddPluginAction;
        QAction*    GetPluginHelpAction;
        QAction*    AddPluginToFavouritesAction;
        QAction*    RemovePluginFromFavouritesAction;


        QMenu*      GraphPluginsContextMenu;
        QAction*    RemovePluginFromGraphAction;
        QAction*    ConfigurePluginAction;
        QAction*    RenameStepAction;
        QAction*    MovePluginUpAction;
        QAction*    MovePluginDownAction;
        QAction*    GetAddedPluginHelpAction;

        vector<XGuid>                           AvailableCameras;
        map<XGuid, XVideoSourceProcessingGraph> ProcessingGraphs;

        XGuid                                   ConfiguredPluginId;
    };
}

VideoProcessingWizardPage::VideoProcessingWizardPage( QWidget* parent ) :
    WizardPageFrame( parent ),
    ui( new Ui::VideoProcessingWizardPage ),
    mData( new ::Private::VideoProcessingWizardPageData( ui, this ) )
{
    ui->setupUi( this );

    connect( ui->availablePluginsListFrame, SIGNAL( SelectedPluginChanged( ) ), this, SLOT( on_SelectedPluginChanged( ) ) );

    auto&   servicrManager = ServiceManager::Instance( );
    auto    pluginsEngine  = servicrManager.GetPluginsEngine( );

    // set modules to display in plug-ins list frame
    ui->availablePluginsListFrame->SetModulesCollection( pluginsEngine->GetFamilies( ), pluginsEngine->GetModules( ),
                                                         servicrManager.GetFavouritePluginsManager( )->PluginsCollection( ),
                                                         PluginType_ImageProcessingFilter |
                                                         PluginType_VideoProcessing |
                                                         PluginType_ScriptingEngine |
                                                         PluginType_Detection );

    // attach context menus to the tree of available plug-in
    ui->availablePluginsListFrame->SetPluginsContextMenu( mData->AvailablePluginsContextMenu,
                                                          mData->AddPluginToFavouritesAction,
                                                          mData->RemovePluginFromFavouritesAction );

    // connect handlers
    connect( mData->AddPluginAction, SIGNAL(triggered()), this, SLOT(on_addPluginButton_clicked()) );
    connect( mData->GetPluginHelpAction, SIGNAL(triggered()), this, SLOT(on_GetPluginHelpAction_triggered()) );

    connect( mData->RemovePluginFromGraphAction, SIGNAL(triggered()), this, SLOT(on_removeButton_clicked()) );
    connect( mData->ConfigurePluginAction, SIGNAL(triggered()), this, SLOT(on_configureButton_clicked()) );
    connect( mData->RenameStepAction, SIGNAL(triggered()), this, SLOT(on_renameStepAction_clicked()) );
    connect( mData->MovePluginUpAction, SIGNAL(triggered()), this, SLOT(on_moveStepUpButton_clicked()) );
    connect( mData->MovePluginDownAction, SIGNAL(triggered()), this, SLOT(on_moveStepDownButton_clicked()) );
    connect( mData->GetAddedPluginHelpAction, SIGNAL(triggered()), this, SLOT(on_GetAddedPluginHelpAction_triggered()) );

    connect( mData->GraphPluginsContextMenu, SIGNAL(aboutToShow()), this, SLOT(on_GraphPluginsContextMenu_aboutToShow()) );
}

VideoProcessingWizardPage::~VideoProcessingWizardPage( )
{
    ServiceManager::Instance( ).GetFavouritePluginsManager( )->Save( );
    delete mData;
    delete ui;
}

// Set list of cameras to configure video processing graph for
void VideoProcessingWizardPage::SetCamerasToConfigure( const vector<XGuid>& camerasList )
{
    shared_ptr<IProjectManager> projectManager = ServiceManager::Instance( ).GetProjectManager( );
    QIcon                       defaultIcon    = QIcon( ProjectObjectIcon::GetResourceName( ProjectObjectType::Camera, false ) );

    mData->AvailableCameras = camerasList;
    ui->camerasComboBox->clear( );

    for_each( camerasList.begin( ), camerasList.end( ), [this, projectManager, &defaultIcon]( const XGuid& id )
    {
        shared_ptr<const ProjectObject> po = projectManager->GetProjectObject( id );

        if ( ( po ) && ( po->Type( ) == ProjectObjectType::Camera ) )
        {
            shared_ptr<const CameraProjectObject> cameraObject = static_pointer_cast<const CameraProjectObject>( po );

            ui->camerasComboBox->addItem( defaultIcon,
                                          QString::fromUtf8( cameraObject->Name( ).c_str( ) ),
                                          QVariant( QString( po->Id( ).ToString( ).c_str( ) ) ) );
        }
    } );

    mData->UpdateAddPluginButtonState( );
    mData->UpdateProcessingGraphButtonsState( );

    mData->CanGoNext = ui->camerasComboBox->count( ) != 0;
    emit CompleteStatusChanged( mData->CanGoNext );
}

// Set/Get video processing graphs for all cameras
const map<XGuid, XVideoSourceProcessingGraph> VideoProcessingWizardPage::GetCamerasProcessingGraphs( ) const
{
    return mData->ProcessingGraphs;
}
void VideoProcessingWizardPage::SetCamerasProcessingGraphs( const map<XGuid, XVideoSourceProcessingGraph>& processingGraphs )
{
    mData->ProcessingGraphs = processingGraphs;
    mData->ShowVideoProcessingGraphForSelectedCamera( );
}

// Check if the page is in a valid state and wizard can go to the next one
bool VideoProcessingWizardPage::CanGoNext( ) const
{
    return mData->CanGoNext;
}

// Selected plug-in changed
void VideoProcessingWizardPage::on_SelectedPluginChanged( )
{
    mData->UpdateAddPluginButtonState( );
}

// Selection changed in the cameras' drop down
void VideoProcessingWizardPage::on_camerasComboBox_currentIndexChanged( int )
{
    mData->ShowVideoProcessingGraphForSelectedCamera( );
}

// Selection changed in the processing graph's tree view
void VideoProcessingWizardPage::on_processinGraphTreeView_itemSelectionChanged( )
{
    mData->UpdateProcessingGraphButtonsState( );
}

// Add video processing step for the current camera
void VideoProcessingWizardPage::on_addPluginButton_clicked( )
{
    const XGuid pluginID = ui->availablePluginsListFrame->GetSelectedPluginID( );

    if ( !pluginID.IsEmpty( ) )
    {
        // get description of the plugin with the specified ID
        shared_ptr<const XPluginDescriptor> pluginDesc =
                ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( pluginID );

        // add the step to the graph
        mData->AddVideoProcessingStep( pluginDesc->Name( ), pluginID );

        QString                  name     = QString::fromUtf8( pluginDesc->Name( ).c_str( ) );
        shared_ptr<const XImage> icon     = pluginDesc->Icon( );
        QTreeWidgetItem*         treeNode = new QTreeWidgetItem( ui->processinGraphTreeView );

        treeNode->setText( 0, name );

        if ( icon )
        {
            shared_ptr<QImage> qimage = XImageInterface::XtoQimage( icon );
            treeNode->setIcon( 0, QIcon( QPixmap::fromImage( *qimage ) ) );
        }

        ui->processinGraphTreeView->setCurrentItem( treeNode, 0 );

        mData->UpdateProcessingGraphButtonsState( );
        mData->ConfigureSelectedProcessingStep( );
    }
}

// Remove selected video processing step
void VideoProcessingWizardPage::on_removeButton_clicked( )
{
    int32_t     selectedStepIndex = mData->GetSelectedStepIndex( );
    const XGuid cameraId          = mData->GetSelectedCameraId( );

    if ( ( !cameraId.IsEmpty( ) ) && ( selectedStepIndex != -1 ) )
    {
        GraphIter graphIt = mData->ProcessingGraphs.find( cameraId );

        if ( graphIt != mData->ProcessingGraphs.end( ) )
        {
            // remove from the graph
            graphIt->second.RemoveStep( selectedStepIndex );
            // and from the UI
            delete ui->processinGraphTreeView->topLevelItem( selectedStepIndex );

            mData->UpdateProcessingGraphButtonsState( );
        }
    }
}

// Configure selected video processing step (plug-in)
void VideoProcessingWizardPage::on_configureButton_clicked( )
{
    mData->ConfigureSelectedProcessingStep( );
}

// Reneme selected video processing step to something which may have more sense
void VideoProcessingWizardPage::on_renameStepAction_clicked( )
{
    mData->RenameSelectedProcessingStep( );
}

// Move selected step up in its processing graph
void VideoProcessingWizardPage::on_moveStepUpButton_clicked( )
{
    mData->MoveSelectedStep( true );
}

// Move selected step down in its processing graph
void VideoProcessingWizardPage::on_moveStepDownButton_clicked( )
{
    mData->MoveSelectedStep( false );
}

// A step item was double clicked in the graph tree
void VideoProcessingWizardPage::on_processinGraphTreeView_itemDoubleClicked( QTreeWidgetItem*, int )
{
    mData->ConfigureSelectedProcessingStep( );
}

// Show description/help for the selected plug-in in the available tree
void VideoProcessingWizardPage::on_GetPluginHelpAction_triggered( )
{
    const XGuid pluginID = ui->availablePluginsListFrame->GetSelectedPluginID( );

    if ( !pluginID.IsEmpty( ) )
    {
        // show description for the plugin with the specified ID
        ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( UITools::GetParentDialog( this ), pluginID );
    }
}

// Help requested for a plug-in configuration dialog
void VideoProcessingWizardPage::on_propertiesDialog_HelpRequested( QWidget* sender )
{
    if ( !mData->ConfiguredPluginId.IsEmpty( ) )
    {
        // show description for the plugin currently being editted
        ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( sender, mData->ConfiguredPluginId );
    }
}

// Show context menu for processing graph item
void VideoProcessingWizardPage::on_processinGraphTreeView_customContextMenuRequested( const QPoint& pos )
{
    QTreeWidgetItem* itemForMenu = ui->processinGraphTreeView->itemAt( pos );

    if ( itemForMenu != 0 )
    {
        mData->GraphPluginsContextMenu->popup( ui->processinGraphTreeView->mapToGlobal( pos ) );
    }
}

// Show description/help for the selected plug-in in the processing graph tree
void VideoProcessingWizardPage::on_GetAddedPluginHelpAction_triggered( )
{
    XGuid pluginID = mData->GetSelectedProcessingStepId( );

    if ( !pluginID.IsEmpty( ) )
    {
        // show description for the plug-in with the specified ID
        ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( UITools::GetParentDialog( this ), pluginID );
    }
}

// The context menu for processing graph is about to show
void VideoProcessingWizardPage::on_GraphPluginsContextMenu_aboutToShow( )
{
    mData->UpdateProcessingGraphActionsState( );
}

namespace Private
{

// Get index of the currently selected processing step in the current graph
int32_t VideoProcessingWizardPageData::GetSelectedStepIndex( ) const
{
    QList<QTreeWidgetItem*> selectedItems = ui->processinGraphTreeView->selectedItems( );
    int32_t                 index         = -1;

    if ( selectedItems.count( ) == 1 )
    {
        index = static_cast<int32_t>( ui->processinGraphTreeView->indexOfTopLevelItem( selectedItems.at( 0 ) ) );
    }

    return index;
}

// Get steps count in the current video processing graph
int32_t VideoProcessingWizardPageData::GetCurrentGraphSize( ) const
{
    const XGuid cameraId  = GetSelectedCameraId( );
    int32_t     graphSize = 0;

    if ( !cameraId.IsEmpty( ) )
    {
        GraphConstIter graphIt = ProcessingGraphs.find( cameraId );

        if ( graphIt != ProcessingGraphs.end( ) )
        {
            graphSize = graphIt->second.StepsCount( );
        }
    }

    return graphSize;
}

// Get ID of the plug-in for the currently selected video processing step
const XGuid VideoProcessingWizardPageData::GetSelectedProcessingStepId( ) const
{
    int32_t     selectedStepIndex = GetSelectedStepIndex( );
    const XGuid cameraId          = GetSelectedCameraId( );
    XGuid       selectedStepId;

    if ( ( !cameraId.IsEmpty( ) ) && ( selectedStepIndex != -1 ) )
    {
        GraphConstIter graphIt = ProcessingGraphs.find( cameraId );

        if ( graphIt != ProcessingGraphs.end( ) )
        {
            selectedStepId = graphIt->second.GetStep( selectedStepIndex ).PluginId( );
        }
    }

    return selectedStepId;
}

// Get ID of the camera currently selected in the cameras' drop down list
const XGuid VideoProcessingWizardPageData::GetSelectedCameraId( ) const
{
    XGuid   selectedCameraId;
    int     selectedIndex    = ui->camerasComboBox->currentIndex( );

    if ( selectedIndex != -1 )
    {
        selectedCameraId = XGuid( ui->camerasComboBox->itemData( selectedIndex ).toString( ).toStdString( ) );
    }

    return selectedCameraId;
}

// Add new video processing step (un-configured for now) to the currently selected camera
int32_t VideoProcessingWizardPageData::AddVideoProcessingStep( const string& stepName, const XGuid& pluginId )
{
    XVideoSourceProcessingStep processingStep( stepName, pluginId );
    const XGuid                cameraId  = GetSelectedCameraId( );
    int32_t                    stepIndex = -1;

    if ( !cameraId.IsEmpty( ) )
    {
        GraphIter graphIt = ProcessingGraphs.find( cameraId );

        if ( graphIt == ProcessingGraphs.end( ) )
        {
            graphIt = ProcessingGraphs.insert( pair<XGuid, XVideoSourceProcessingGraph>( cameraId, XVideoSourceProcessingGraph( ) ) ).first;
        }

        stepIndex = graphIt->second.AddStep( processingStep );
    }

    return stepIndex;
}

// Enable/disable "Add Plug-in" button state
void VideoProcessingWizardPageData::UpdateAddPluginButtonState( )
{
    ui->addPluginButton->setEnabled( ( ui->camerasComboBox->count( ) != 0 ) &&
                                     ( !ui->availablePluginsListFrame->GetSelectedPluginID( ).IsEmpty( ) ) );
}

// Enable/disable buttons related to the items in the processing graph's tree view
void VideoProcessingWizardPageData::UpdateProcessingGraphButtonsState( )
{
    const XGuid pluginID          = GetSelectedProcessingStepId( );
    int32_t     selectedStepIndex = GetSelectedStepIndex( );

    // remove/up/down buttons
    if ( selectedStepIndex == -1 )
    {
        ui->removeButton->setEnabled( false );
        ui->moveStepUpButton->setEnabled( false );
        ui->moveStepDownButton->setEnabled( false );
    }
    else
    {
        int32_t stepsCount = GetCurrentGraphSize( );

        ui->removeButton->setEnabled( true );
        ui->moveStepUpButton->setEnabled( ( selectedStepIndex > 0 ) );
        ui->moveStepDownButton->setEnabled( ( selectedStepIndex < stepsCount - 1 ) );
    }

    // configuration button
    if ( pluginID.IsEmpty( ) )
    {
        ui->configureButton->setEnabled( false );
    }
    else
    {
        // get description of the plug-in with the specified ID
        shared_ptr<const XPluginDescriptor> pluginDesc =
                ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( pluginID );

        ui->configureButton->setEnabled( ( pluginDesc ) && ( pluginDesc->ConfigurablePropertiesCount( ) != 0 ) );
    }
}

// Enable/disable actions related to the items in the processing graph's tree view
void VideoProcessingWizardPageData::UpdateProcessingGraphActionsState( )
{
    int32_t selectedStepIndex = GetSelectedStepIndex( );
    int32_t stepsCount        = GetCurrentGraphSize( );

    MovePluginUpAction->setEnabled( selectedStepIndex > 0 );
    MovePluginDownAction->setEnabled( selectedStepIndex < stepsCount - 1 );

    // get description of the plug-in with the specified ID
    shared_ptr<const XPluginDescriptor> pluginDesc =
        ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( GetSelectedProcessingStepId( ) );

    ConfigurePluginAction->setEnabled( ( pluginDesc ) && ( pluginDesc->ConfigurablePropertiesCount( ) != 0 ) );
    GetAddedPluginHelpAction->setEnabled( static_cast<bool>( pluginDesc ) );
}

// Show video processing steps (graph) for the currently selected video camera
void VideoProcessingWizardPageData::ShowVideoProcessingGraphForSelectedCamera( )
{
    const XGuid selectedCameraId = GetSelectedCameraId( );

    ui->processinGraphTreeView->clear( );

    if ( !selectedCameraId.IsEmpty( ) )
    {
        GraphIter graphIt = ProcessingGraphs.find( selectedCameraId );

        if ( graphIt != ProcessingGraphs.end( ) )
        {
            const shared_ptr<const XPluginsEngine> pluginEngine    = ServiceManager::Instance( ).GetPluginsEngine( );
            XVideoSourceProcessingGraph&           processingGraph = graphIt->second;
            int                                    stepsCounter    = 0;

            for ( auto it = processingGraph.begin( ); it != processingGraph.end( ); ++it, ++stepsCounter )
            {
                shared_ptr<const XPluginDescriptor> pluginDesc = pluginEngine->GetPlugin( it->PluginId( ) );

                QString          name     = QString::fromUtf8( it->Name( ).c_str( ) );
                QTreeWidgetItem* treeNode = new QTreeWidgetItem( ui->processinGraphTreeView );

                treeNode->setText( 0, name );

                if ( pluginDesc )
                {
                    shared_ptr<const XImage> icon = pluginDesc->Icon( );

                    if ( icon )
                    {
                        shared_ptr<QImage> qimage = XImageInterface::XtoQimage( icon );
                        treeNode->setIcon( 0, QIcon( QPixmap::fromImage( *qimage ) ) );
                    }
                }
                else
                {
                    treeNode->setTextColor( 0, QColor::fromRgb( 255, 0, 0 ) );
                    treeNode->setToolTip( 0, "Missing plug-in" );
                }
            }
        }
    }
}

// Show dialog to configure currently selected video processing step
void VideoProcessingWizardPageData::ConfigureSelectedProcessingStep( )
{
    const XGuid cameraId          = GetSelectedCameraId( );
    int32_t     selectedStepIndex = GetSelectedStepIndex( );
    GraphIter   graphIt           = ProcessingGraphs.find( cameraId );

    if ( ( !cameraId.IsEmpty( ) ) && ( selectedStepIndex != -1 ) && ( graphIt != ProcessingGraphs.end( ) ) )
    {
        XVideoSourceProcessingStep processingStep = graphIt->second.GetStep( selectedStepIndex );

        if ( processingStep.IsValid( ) )
        {
            ConfiguredPluginId = processingStep.PluginId( );

            // get description of the plug-in with the specified ID
            const shared_ptr<const XPluginDescriptor> pluginDesc = ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( ConfiguredPluginId );

            if ( ( pluginDesc ) && ( pluginDesc->ConfigurablePropertiesCount( ) != 0 ) )
            {
                shared_ptr<XPlugin> plugin = pluginDesc->CreateInstance( );

                if ( plugin )
                {
                    int dialogResult = QDialog::Rejected;

                    // set plug-in configuration
                    pluginDesc->SetPluginConfiguration( plugin, processingStep.PluginConfiguration( ) );

                    switch ( pluginDesc->Type( ) )
                    {
                    case PluginType_ImageProcessingFilter:
                    case PluginType_VideoProcessing:
                    case PluginType_Detection:
                    case PluginType_ScriptingEngine:

                        ConfigurePluginPropertiesDialog configureForm( pluginDesc, plugin, false, UITools::GetParentDialog( Parent ) );

                        configureForm.EnableHelpButton( true );
                        Parent->connect( &configureForm, SIGNAL(HelpRequested(QWidget*)), Parent, SLOT(on_propertiesDialog_HelpRequested(QWidget*)) );

                        dialogResult = configureForm.exec( );

                        break;
                    }

                    if ( dialogResult == QDialog::Accepted )
                    {
                        // set configuration back
                        processingStep.SetPluginConfiguration( pluginDesc->GetPluginConfiguration( plugin ) );
                        graphIt->second.SetStep( selectedStepIndex, processingStep );
                    }
                }
            }

            ConfiguredPluginId = XGuid( );
        }
    }
}

// Show dialog for renaming selected video processing step
void VideoProcessingWizardPageData::RenameSelectedProcessingStep( )
{
    const XGuid cameraId          = GetSelectedCameraId( );
    int32_t     selectedStepIndex = GetSelectedStepIndex( );
    GraphIter   graphIt           = ProcessingGraphs.find( cameraId );

    if ( ( !cameraId.IsEmpty( ) ) && ( selectedStepIndex != -1 ) && ( graphIt != ProcessingGraphs.end( ) ) )
    {
        XVideoSourceProcessingStep processingStep = graphIt->second.GetStep( selectedStepIndex );
        RenameProcessingStepDialog renameDialog( QString::fromUtf8( processingStep.Name( ).c_str( ) ),
                                                 this->Parent );

        if ( renameDialog.exec( ) == QDialog::Accepted )
        {
            processingStep.SetName( renameDialog.NewName( ).toUtf8( ).data( ) );
            graphIt->second.SetStep( selectedStepIndex, processingStep );

            QList<QTreeWidgetItem*> selectedItems = ui->processinGraphTreeView->selectedItems( );

            if ( selectedItems.count( ) == 1 )
            {
                selectedItems[0]->setText( 0, renameDialog.NewName( ) );
            }
        }
    }
}

// Move selected step up/down in its video processing graph
void VideoProcessingWizardPageData::MoveSelectedStep( bool moveUp )
{
    const XGuid selectedCameraId = GetSelectedCameraId( );

    if ( !selectedCameraId.IsEmpty( ) )
    {
        GraphIter graphIt = ProcessingGraphs.find( selectedCameraId );

        if ( graphIt != ProcessingGraphs.end( ) )
        {
            QList<QTreeWidgetItem*>      selectedItems = ui->processinGraphTreeView->selectedItems( );
            XVideoSourceProcessingGraph& graph         = graphIt->second;

            if ( selectedItems.count( ) == 1 )
            {
                QTreeWidgetItem* selectedItem      = selectedItems.at( 0 );
                int32_t          selectedItemIndex = static_cast<int32_t>( ui->processinGraphTreeView->indexOfTopLevelItem( selectedItem ) );
                int32_t          newIndex          = -1;

                if ( moveUp )
                {
                    // moving up
                    if ( selectedItemIndex > 0 )
                    {
                        newIndex = selectedItemIndex - 1;
                    }
                }
                else
                {
                    // moving down
                    if ( selectedItemIndex < graph.StepsCount( ) - 1 )
                    {
                        newIndex = selectedItemIndex + 1;
                    }
                }

                // perform the actual movement
                if ( newIndex != -1 )
                {
                    // graph part
                    const XVideoSourceProcessingStep step = graph.GetStep( selectedItemIndex );
                    graph.RemoveStep( selectedItemIndex );
                    graph.InsertStep( newIndex, step );

                    // UI part
                    ui->processinGraphTreeView->invisibleRootItem( )->removeChild( selectedItem );
                    ui->processinGraphTreeView->insertTopLevelItem( newIndex, selectedItem );
                    selectedItem->setSelected( true );
                }

                // make sure we have only one item selected - the one we moved
                selectedItems = ui->processinGraphTreeView->selectedItems( );

                for ( int i = 0; i < selectedItems.count( ); i++ )
                {
                    if ( selectedItems.at( i ) != selectedItem )
                    {
                        selectedItems.at( i )->setSelected( false );
                    }
                }
            }
        }
    }
}

} // namespace Private
