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

#include <QTimer>
#include <QMenu>
#include "VideoProcessingInfoDialog.hpp"
#include "ui_VideoProcessingInfoDialog.h"
#include "ServiceManager.hpp"
#include "SandboxProjectObject.hpp"

#include <XImageInterface.hpp>
#include <UITools.hpp>
#include <XVideoSourceFrameInfo.hpp>
#include <ConfigurePluginPropertiesDialog.hpp>
#include <UITools.hpp>

#include <numeric>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

static const QString STR_COL1_WIDTH = QString( "wcol1" );
static const QString STR_COL2_WIDTH = QString( "wcol2" );
static const QString STR_COL3_WIDTH = QString( "wcol3" );

namespace Private
{
    class VideoProcessingInfoDialogData
    {
    public:
        VideoProcessingInfoDialogData( VideoProcessingInfoDialog* parent, Ui::VideoProcessingInfoDialog* ui ) :
            Parent( parent ), Ui( ui ), SandboxId( ), CameraId( ), VideoSourceId( 0 ),
            InfoUpdateTimer( ), PersistedUiSettings( ),
            LastValueOfFramesBlocked( 0 ), SubCounter( 0 ), EditBoxDefaultColor( ),
            LabelUpdated( false ), VideoProcessingGraph( ),
            ConfiguredStepIndex( -1 ), ConfiguredStepDescriptor( ),
            ConfiguredStepPlugin( ), CurrentStepConfiguration( ),
            ProcessingStepsContextMenu( nullptr ),
            ConfigurePluginAction( nullptr ), GetPluginHelpAction( nullptr )
        {
            InfoUpdateTimer.setInterval( 1000 );

            PersistedUiSettings[STR_COL1_WIDTH] = 200;
            PersistedUiSettings[STR_COL2_WIDTH] = 80;
            PersistedUiSettings[STR_COL3_WIDTH] = 80;

            // create context menu for video processing steps
            ProcessingStepsContextMenu = new QMenu( );

            ConfigurePluginAction = new QAction( nullptr );
            ConfigurePluginAction->setText( "&Configure" );
            ConfigurePluginAction->setStatusTip( "Configure selected step of the processing graph" );
            ConfigurePluginAction->setIcon( QIcon( QPixmap( ":/images/icons/properties.png" ) ) );

            GetPluginHelpAction = new QAction( nullptr );
            GetPluginHelpAction->setText( "&Description" );
            GetPluginHelpAction->setStatusTip( "Show description/help for the selected plug-in" );
            GetPluginHelpAction->setIcon( QIcon( QPixmap( ":/images/icons/idea.png" ) ) );

            QAction* separator = new QAction( nullptr );
            separator->setSeparator( true );

            ProcessingStepsContextMenu->addAction( ConfigurePluginAction );
            ProcessingStepsContextMenu->addAction( separator );
            ProcessingStepsContextMenu->addAction( GetPluginHelpAction );
            ProcessingStepsContextMenu->setDefaultAction( ConfigurePluginAction );
        }

        ~VideoProcessingInfoDialogData( )
        {
            delete ConfigurePluginAction;
            delete GetPluginHelpAction;
            delete ProcessingStepsContextMenu;
        }

        void FindVideoProcessingGraph( const XGuid& sandboxId, const XGuid& cameraId );
        void ShowVideoProcessingGraph( );
        void UpdateVideoProcessingGraph( );
        void CheckForPerformanceIssues( );

        shared_ptr<const XPluginDescriptor> GetSelectedPluginDescriptor( int32_t* stepIndex );
        void EditPluginProperties( );
        void UpdatePluginProperties( );

        void SavePersistedUiValues( );
        void LoadPersistedUiValues( );

    public:
        VideoProcessingInfoDialog*      Parent;
        Ui::VideoProcessingInfoDialog*  Ui;
        XGuid                           SandboxId;
        XGuid                           CameraId;
        uint32_t                        VideoSourceId;
        QTimer                          InfoUpdateTimer;
        QMap<QString, QVariant>         PersistedUiSettings;

        uint32_t                        LastValueOfFramesBlocked;
        uint32_t                        SubCounter;
        QColor                          EditBoxDefaultColor;

        bool                            LabelUpdated;

        XVideoSourceProcessingGraph     VideoProcessingGraph;
        map<int, map<string, XVariant>> OriginalPluginProperties;
        map<int, bool>                  PluginUpdated;

        int32_t                             ConfiguredStepIndex;
        shared_ptr<const XPluginDescriptor> ConfiguredStepDescriptor;
        shared_ptr<XPlugin>                 ConfiguredStepPlugin;
        map<string, XVariant>               CurrentStepConfiguration;

        QMenu*      ProcessingStepsContextMenu;
        QAction*    ConfigurePluginAction;
        QAction*    GetPluginHelpAction;
    };
}

VideoProcessingInfoDialog::VideoProcessingInfoDialog( QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::VideoProcessingInfoDialog ),
    mData( new ::Private::VideoProcessingInfoDialogData( this, ui ) )
{
    ui->setupUi( this );

    ui->processinGraphTreeView->setColumnWidth( 0, 200 );
    ui->processinGraphTreeView->setColumnWidth( 1, 80 );
    ui->processinGraphTreeView->setColumnWidth( 2, 80 );
    ui->processinGraphTreeView->setColumnWidth( 3, 0 );
    ui->processinGraphTreeView->header( )->setMovable( false );

    if ( QTreeWidgetItem* header = ui->processinGraphTreeView->headerItem( ) )
    {
        header->setText( 0, "Step name" );
        header->setText( 1, "Time ms" );
        header->setText( 2, "Time %" );
        header->setText( 3, QString::null );

        header->setTextAlignment( 1, Qt::AlignRight );
        header->setTextAlignment( 2, Qt::AlignRight );
    }

    // disable "?" button on title bar
    setWindowFlags( ( windowFlags( ) & ~Qt::WindowContextHelpButtonHint ) );

    // restore size/position
    ServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this, QString( ), &mData->PersistedUiSettings );
    mData->LoadPersistedUiValues( );

    // connect timer
    connect( &mData->InfoUpdateTimer, SIGNAL(timeout()), this, SLOT(on_InfoUpdateTimer_timeout()) );

    // connect action handlers
    connect( mData->ConfigurePluginAction, SIGNAL(triggered()), this, SLOT(on_pluginPropertiesButton_clicked()) );
    connect( mData->GetPluginHelpAction, SIGNAL(triggered()), this, SLOT(on_GetPluginHelpAction_triggered()) );
    connect( mData->ProcessingStepsContextMenu, SIGNAL(aboutToShow()), this, SLOT(on_ProcessingStepsContextMenu_aboutToShow()) );

    // hide performance issues by default
    ui->performanceGroupBox->setVisible( false );
    mData->EditBoxDefaultColor = UITools::GetWidgetBackgroundColor( ui->delayedFramesEdit );
}

VideoProcessingInfoDialog::~VideoProcessingInfoDialog( )
{
    if ( mData->VideoSourceId != 0 )
    {
        ServiceManager::Instance( ).GetAutomationServer( )->EnableVideoProcessingPerformanceMonitor( mData->VideoSourceId, false );
    }

    delete mData;
    delete ui;
}

void VideoProcessingInfoDialog::on_closeButton_clicked( )
{
    this->accept( );
}

// Widget is closed
void VideoProcessingInfoDialog::closeEvent( QCloseEvent* )
{
    SaveUi( );
}

// Dialog is accepted
void VideoProcessingInfoDialog::accept( )
{
    bool graphUpdated = false;

    for ( auto pluginUpdate : mData->PluginUpdated )
    {
        graphUpdated |= pluginUpdate.second;
    }

    if ( graphUpdated )
    {
        if ( UITools::GetUserConfirmation( "<b>Settings of some plug-ins were changed.</b><br><br>Would you like to update sandbox configuration ?", this ) )
        {
            auto& serviceManager = ServiceManager::Instance( );
            auto  sandboxObject  = static_pointer_cast<SandboxProjectObject>( serviceManager.GetProjectManager( )->GetProjectObject( mData->SandboxId ) );

            if ( sandboxObject )
            {
                auto graphs = sandboxObject->GetCamerasProcessingGraphs( );

                graphs[mData->CameraId] = mData->VideoProcessingGraph;
                sandboxObject->SetCamerasProcessingGraphs( graphs );

                serviceManager.GetProjectManager( )->UpdateProjectObject( sandboxObject );
            }
        }
    }

    SaveUi( );
    QDialog::accept( );
}

// Dialog is rejected
void VideoProcessingInfoDialog::reject( )
{
    SaveUi( );
    QDialog::reject( );
}

void VideoProcessingInfoDialog::SaveUi( )
{
    mData->SavePersistedUiValues( );
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this, QString( ), &mData->PersistedUiSettings );
}

// Set Id of the camera to display information for
void VideoProcessingInfoDialog::SetCameraId( const XGuid& sandboxId, const XGuid& cameraId, uint32_t videoSourceId )
{
    auto& serviceManager  = ServiceManager::Instance( );
    auto automationServer = serviceManager.GetAutomationServer( );
    auto cameraObject     = serviceManager.GetProjectManager( )->GetProjectObject( cameraId );

    if ( mData->VideoSourceId != 0 )
    {
        automationServer->EnableVideoProcessingPerformanceMonitor( mData->VideoSourceId, false );
    }

    if ( cameraObject )
    {
        setWindowTitle( windowTitle( ) + QString( " : %0" ).arg( QString::fromUtf8( cameraObject->Name( ).c_str( ) ) ) );
    }

    mData->SandboxId     = sandboxId;
    mData->CameraId      = cameraId;
    mData->VideoSourceId = videoSourceId;

    if ( mData->LabelUpdated )
    {
        QString strLabel = ui->framesDelayedLabel->text( );

        strLabel.replace( "dropped", "delayed" );
        ui->framesDelayedLabel->setText( strLabel );
        mData->LabelUpdated  = false;
    }

    mData->FindVideoProcessingGraph( sandboxId, cameraId );
    mData->ShowVideoProcessingGraph( );
    automationServer->EnableVideoProcessingPerformanceMonitor( videoSourceId, true );

    mData->InfoUpdateTimer.start( );
}

// Update video processing graph with new performance information
void VideoProcessingInfoDialog::on_InfoUpdateTimer_timeout( )
{
    mData->UpdateVideoProcessingGraph( );
    mData->CheckForPerformanceIssues( );
}

// Selection has changed in the plug-ins list
void VideoProcessingInfoDialog::on_processinGraphTreeView_itemSelectionChanged( )
{
    shared_ptr<const XPluginDescriptor> descriptor = mData->GetSelectedPluginDescriptor( nullptr );

    ui->pluginPropertiesButton->setEnabled( ( descriptor ) &&
                                            ( descriptor->Type( ) == PluginType_ImageProcessingFilter ) &&
                                            ( descriptor->ConfigurablePropertiesCount( ) != 0 ) );
}

// Edit properties of the plug-in (image processing filter only for now)
void VideoProcessingInfoDialog::on_pluginPropertiesButton_clicked( )
{
    mData->EditPluginProperties( );
}

// Double click on video processing step
void VideoProcessingInfoDialog::on_processinGraphTreeView_itemDoubleClicked( QTreeWidgetItem*, int )
{
    mData->EditPluginProperties( );
}

// Show context menu for processing graph item
void VideoProcessingInfoDialog::on_processinGraphTreeView_customContextMenuRequested( const QPoint& pos )
{
    QTreeWidgetItem* itemForMenu = ui->processinGraphTreeView->itemAt( pos );

    if ( itemForMenu != nullptr )
    {
        mData->ProcessingStepsContextMenu->popup( ui->processinGraphTreeView->viewport( )->mapToGlobal( pos ) );
    }
}

// Some property of a plug-in was changed - update automation server
void VideoProcessingInfoDialog::on_configurationEditor_ConfigurationUpdated( )
{    
    mData->UpdatePluginProperties( );
}

// Help requested for a plug-in configuration dialog
void VideoProcessingInfoDialog::on_configurationEditor_HelpRequested( QWidget* sender )
{
    shared_ptr<const XPluginDescriptor> pluginDesc = mData->GetSelectedPluginDescriptor( nullptr );

    if ( pluginDesc )
    {
        ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( sender, pluginDesc->ID( ) );
    }
}

// Help requested for the currently selected video processing step
void VideoProcessingInfoDialog::on_GetPluginHelpAction_triggered( )
{
    shared_ptr<const XPluginDescriptor> pluginDesc = mData->GetSelectedPluginDescriptor( nullptr );

    if ( pluginDesc )
    {
        ServiceManager::Instance( ).GetHelpService( )->ShowPluginDescription( this, pluginDesc->ID( ) );
    }
}

// Enable/disable actions in the plug-ins context menu
void VideoProcessingInfoDialog::on_ProcessingStepsContextMenu_aboutToShow( )
{
    shared_ptr<const XPluginDescriptor> descriptor = mData->GetSelectedPluginDescriptor( nullptr );

    mData->ConfigurePluginAction->setEnabled( ( descriptor ) &&
                                              ( descriptor->Type( ) == PluginType_ImageProcessingFilter ) &&
                                              ( descriptor->ConfigurablePropertiesCount( ) != 0 ) );
}

namespace Private
{

// Find video processing graph for the specified camera/sandbox
void VideoProcessingInfoDialogData::FindVideoProcessingGraph( const XGuid& sandboxId, const XGuid& cameraId )
{
    if ( ( !sandboxId.IsEmpty( ) ) && ( !cameraId.IsEmpty( ) ) )
    {
        auto& serviceManager = ServiceManager::Instance( );
        auto  sandboxObject  = serviceManager.GetProjectManager( )->GetProjectObject( sandboxId );
        auto  pluginEngine   = serviceManager.GetPluginsEngine( );

        if ( sandboxObject )
        {
            auto graphs   = static_pointer_cast<SandboxProjectObject>( sandboxObject )->GetCamerasProcessingGraphs( );
            auto searchIt = graphs.find( cameraId );

            if ( ( graphs.end( ) != searchIt ) && ( searchIt->second.StepsCount( ) != 0 ) )
            {
                VideoProcessingGraph = searchIt->second;
            }
        }
    }
}

// Show steps of a video processing graph
void VideoProcessingInfoDialogData::ShowVideoProcessingGraph( )
{
    auto pluginEngine  = ServiceManager::Instance( ).GetPluginsEngine( );
    bool showGraphTree = true;

    Ui->processinGraphTreeView->clear( );
    Ui->pluginPropertiesButton->setEnabled( false );

    if ( VideoProcessingGraph.StepsCount( ) != 0 )
    {
        for ( XVideoSourceProcessingGraph::ConstIterator it = VideoProcessingGraph.begin( ); it != VideoProcessingGraph.end( ); ++it )
        {
            shared_ptr<const XPluginDescriptor> pluginDesc = pluginEngine->GetPlugin( it->PluginId( ) );

            QString                  name     = QString::fromUtf8( it->Name( ).c_str( ) );
            QTreeWidgetItem*         treeNode = new QTreeWidgetItem( Ui->processinGraphTreeView );

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

            treeNode->setTextAlignment( 1, Qt::AlignRight );
            treeNode->setTextAlignment( 2, Qt::AlignRight );
        }
    }
    else
    {
        showGraphTree = false;
    }

    Ui->messageLabel->setVisible( !showGraphTree );
    Ui->processinGraphTreeView->setVisible( showGraphTree );
    Ui->totalTimeInfoFrame->setVisible( showGraphTree );
    Ui->pluginPropertiesButton->setVisible( showGraphTree );
}

// Show timing for the steps of video processing graph
void VideoProcessingInfoDialogData::UpdateVideoProcessingGraph( )
{
    float         totalGraphTime = 0.0f;

    vector<float> timing = ServiceManager::Instance( ).GetAutomationServer( )->GetVideoProcessingGraphTiming( VideoSourceId, &totalGraphTime );
    float         total  = accumulate( timing.begin( ), timing.end( ), 0.0f );

    if ( total == 0.0f )
    {
        total = 1.0f;
    }

    for ( int i = 0, n = timing.size( ); i < n; i++ )
    {
        QTreeWidgetItem* treeNode = Ui->processinGraphTreeView->topLevelItem( i );

        treeNode->setText( 1, QString::number( timing[i], 'f', 2 ) );
        treeNode->setText( 2, QString::number( timing[i] / total * 100, 'f', 1 ) );
    }

    Ui->totalTimeLabel->setText( QString::number( totalGraphTime, 'f', 2 ) );
}

// Check for performance issues of the graph
void VideoProcessingInfoDialogData::CheckForPerformanceIssues( )
{
    XVideoSourceFrameInfo frameInfo;

    ServiceManager::Instance( ).GetAutomationServer( )->GetVideoSourceFrameInfo( VideoSourceId, &frameInfo );

    if ( ( frameInfo.FramesBlocked != 0 ) || ( frameInfo.FramesDropped != 0 ) )
    {
        if ( !Ui->performanceGroupBox->isVisible( ) )
        {
            Ui->performanceGroupBox->setVisible( true );
        }

        Ui->delayedFramesEdit->setText( QString::number( ( frameInfo.FramesDropped != 0 ) ?
                                                           frameInfo.FramesDropped : frameInfo.FramesBlocked ) );

        if ( ( frameInfo.FramesDropped != 0 ) && ( !LabelUpdated ) )
        {
            QString strLabel = Ui->framesDelayedLabel->text( );

            strLabel.replace( "delayed", "dropped" );
            Ui->framesDelayedLabel->setText( strLabel );

            LabelUpdated = true;
        }

        if ( SubCounter == 0 )
        {
            UITools::SetWidgetBackgroundColor( Ui->delayedFramesEdit,
                ( frameInfo.FramesBlocked == LastValueOfFramesBlocked ) ? EditBoxDefaultColor : QColor( 255, 200, 200 ) );

            LastValueOfFramesBlocked = frameInfo.FramesBlocked;
        }
        SubCounter = ( SubCounter + 1 ) % 5;
    }
}

// Get descriptor of the selected plug-in
shared_ptr<const XPluginDescriptor> VideoProcessingInfoDialogData::GetSelectedPluginDescriptor( int32_t* stepIndex )
{
    QList<QTreeWidgetItem*> selectedItems = Ui->processinGraphTreeView->selectedItems( );
    shared_ptr<const XPluginDescriptor> descriptor;

    if ( selectedItems.count( ) == 1 )
    {
        int32_t index = static_cast<int32_t>( Ui->processinGraphTreeView->indexOfTopLevelItem( selectedItems.at( 0 ) ) );

        if ( ( index >= 0 ) && ( index < VideoProcessingGraph.StepsCount( ) ) )
        {
            const XVideoSourceProcessingStep step = VideoProcessingGraph.GetStep( index );

            descriptor = ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( step.PluginId( ) );

            if ( ( descriptor ) && ( stepIndex != nullptr ) )
            {
                *stepIndex = index;
            }
        }
    }

    return descriptor;
}

// Edit properties of the selected plug-in
void VideoProcessingInfoDialogData::EditPluginProperties( )
{
    ConfiguredStepDescriptor = GetSelectedPluginDescriptor( &ConfiguredStepIndex );

    if ( ( ConfiguredStepDescriptor ) &&
         ( ConfiguredStepDescriptor->Type( ) == PluginType_ImageProcessingFilter ) &&
         ( ConfiguredStepDescriptor->ConfigurablePropertiesCount( ) != 0 ) )
    {
        map<string, XVariant> pluginConfig = ServiceManager::Instance( ).GetAutomationServer( )->
                GetVideoProcessingStepConfiguration( VideoSourceId, ConfiguredStepIndex );

        // save original configuration of the plug-in
        if ( OriginalPluginProperties.find( ConfiguredStepIndex ) == OriginalPluginProperties.end( ) )
        {
            OriginalPluginProperties.insert( pair<int, map<string, XVariant>>( ConfiguredStepIndex, pluginConfig ) );
        }

        ConfiguredStepPlugin = ConfiguredStepDescriptor->CreateInstance( );

        if ( ConfiguredStepPlugin )
        {
            ConfiguredStepDescriptor->SetPluginConfiguration( ConfiguredStepPlugin, pluginConfig );

            ConfigurePluginPropertiesDialog configureForm( ConfiguredStepDescriptor, ConfiguredStepPlugin, false, Parent );

            configureForm.EnableHelpButton( true );

            Parent->connect( &configureForm, SIGNAL( ConfigurationUpdated() ), Parent, SLOT( on_configurationEditor_ConfigurationUpdated() ) );
            Parent->connect( &configureForm, SIGNAL( HelpRequested(QWidget*) ), Parent, SLOT( on_configurationEditor_HelpRequested(QWidget*) ) );

            CurrentStepConfiguration = pluginConfig;

            if ( configureForm.exec( ) == QDialog::Accepted )
            {
                map<string, XVariant> pluginUpdatedConfig = ConfiguredStepDescriptor->GetPluginConfiguration( ConfiguredStepPlugin );

                if ( pluginConfig != pluginUpdatedConfig )
                {
                    ServiceManager::Instance( ).GetAutomationServer( )->
                            SetVideoProcessingStepConfiguration( VideoSourceId, ConfiguredStepIndex, pluginUpdatedConfig );

                    // store new configuration in the graph of the video source
                    XVideoSourceProcessingStep step = VideoProcessingGraph.GetStep( ConfiguredStepIndex );
                    step.SetPluginConfiguration( pluginUpdatedConfig );
                    VideoProcessingGraph.SetStep( ConfiguredStepIndex, step );

                    PluginUpdated[ConfiguredStepIndex] = ( pluginUpdatedConfig != OriginalPluginProperties[ConfiguredStepIndex] );
                }
            }
            else
            {
                // restor configuration
                if ( pluginConfig != CurrentStepConfiguration )
                {
                    ServiceManager::Instance( ).GetAutomationServer( )->
                            SetVideoProcessingStepConfiguration( VideoSourceId, ConfiguredStepIndex, pluginConfig );
                }
            }
        }

        ConfiguredStepDescriptor.reset( );
        ConfiguredStepPlugin.reset( );
        ConfiguredStepIndex = -1;
    }
}

// Update properties of the currently edited plug-in
void VideoProcessingInfoDialogData::UpdatePluginProperties( )
{
    if ( ( ConfiguredStepDescriptor ) && ( ConfiguredStepPlugin ) )
    {
        map<string, XVariant> pluginUpdatedConfig = ConfiguredStepDescriptor->GetPluginConfiguration( ConfiguredStepPlugin );

        if ( pluginUpdatedConfig != CurrentStepConfiguration )
        {
            ServiceManager::Instance( ).GetAutomationServer( )->
                    SetVideoProcessingStepConfiguration( VideoSourceId, ConfiguredStepIndex, pluginUpdatedConfig );

            CurrentStepConfiguration = pluginUpdatedConfig;
        }
    }
}

// Save UI values to be persisten into persistence map
void VideoProcessingInfoDialogData::SavePersistedUiValues( )
{
    PersistedUiSettings[STR_COL1_WIDTH] = Ui->processinGraphTreeView->columnWidth( 0 );
    PersistedUiSettings[STR_COL2_WIDTH] = Ui->processinGraphTreeView->columnWidth( 1 );;
    PersistedUiSettings[STR_COL3_WIDTH] = Ui->processinGraphTreeView->columnWidth( 2 );;
}

// Update UI elements using persisted values from the persistence map
void VideoProcessingInfoDialogData::LoadPersistedUiValues( )
{
    if ( PersistedUiSettings[STR_COL1_WIDTH].isValid( ) )
    {
        Ui->processinGraphTreeView->setColumnWidth( 0, PersistedUiSettings[STR_COL1_WIDTH].toInt( ) );
    }
    if ( PersistedUiSettings[STR_COL2_WIDTH].isValid( ) )
    {
        Ui->processinGraphTreeView->setColumnWidth( 1, PersistedUiSettings[STR_COL2_WIDTH].toInt( ) );
    }
    if ( PersistedUiSettings[STR_COL3_WIDTH].isValid( ) )
    {
        Ui->processinGraphTreeView->setColumnWidth( 2, PersistedUiSettings[STR_COL3_WIDTH].toInt( ) );
    }
}


} // namespace Private
