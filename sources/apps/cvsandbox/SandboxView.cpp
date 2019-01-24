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

#include <set>
#include <algorithm>
#include <QTime>
#include <QMutex>
#include <QMutexLocker>
#include "SandboxView.hpp"
#include "ui_SandboxView.h"
#include "CamerasViewConfiguration.hpp"
#include "MultiCameraView.hpp"
#include "GridButton.hpp"
#include "GridWidget.hpp"
#include "SandboxSettings.hpp"
#include "CameraProjectObject.hpp"
#include "EditVideoSourceRunTimeProperties.hpp"

#include "ServiceManager.hpp"
#include "VideoSourceInAutomationServer.hpp"
#include "CameraInfoDialog.hpp"
#include "VideoProcessingInfoDialog.hpp"

#include <ximaging.h>
#include <XImageInterface.hpp>

#include <QShortcut>
#include <QTimer>
#include <QMenu>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;
using namespace CVSandbox::Video;
using namespace ::Private;

// Specifies update frequency of thumbnail images for cameras
#define THUMBNAIL_IMAGE_UPDATE_FREQUENCY_MS (3000)
// Specifies thumbnail image width (height is calculate to keep aspect ratio)
#define THUMBNAIL_IMAGE_WIDTH (100)

// Register some custom types to use with queued signals (updating image thumbnails)
static struct Init
{
    Init( )
    {
        qRegisterMetaType<CVSandbox::XGuid>( "CVSandbox::XGuid" );
    }
}
init;


namespace Private
{
    class VideoMonitor : public IVideoSourceListener
    {
    public:
        VideoMonitor( SandboxView* parent,
                      const XGuid& cameraObjectId,
                      const shared_ptr<VideoSourceInAutomationServer>& videoSource ) :
            Parent( parent ), CameraObjectId( cameraObjectId ), VideoSource( videoSource ),
            LastUpdated( ), Sync( ), ThumbnailImage( ), ErrorDetected( false )
        {
        }

        // New video frame notification
        virtual void OnNewImage( const shared_ptr<const XImage>& image );

        // Video source error notification
        virtual void OnError( const string& errorMessage );

    public:
        SandboxView*                                Parent;
        XGuid                                       CameraObjectId;
        shared_ptr<VideoSourceInAutomationServer>   VideoSource;

        QTime               LastUpdated;
        QMutex              Sync;
        shared_ptr<XImage>  ThumbnailImage;
        bool                ErrorDetected;
    };

    typedef map<XGuid, shared_ptr<GridButton>>   ViewButtonsMap;
    typedef map<XGuid, shared_ptr<VideoMonitor>> VideoMonitorsMap;

    class SandboxViewData
    {
    public:
        SandboxViewData( Ui::SandboxView* ui, SandboxView* parent ) :
            Ui( ui ), Parent( parent ),
            VideoSourceMap( ), ThreadsMap( ), Views( ), DefaultViewId( ), CurrentView( 0 ),
            ViewButtons( ), VideoMonitors( ),
            AutoSizeView( true ), PreferredContentSize( 0, 0 ),ViewsSettings( ),
            ViewShortcuts( ), RotationTimer( ), LastMousePoint( -1, -1 ), UserInactivityCounter( 0 ), IsRotationPaused( false ),
            CameraContextMenu( nullptr ), CameraInfoAction( nullptr), VideoProcessingInfoAction( nullptr ),
            LastClickedRow( -1 ), LastClickedColumn( -1 )
        {
            RotationTimer.setInterval( 1000 );

            CreateCameraContextMenu( parent );
        }

        ~SandboxViewData( )
        {
            delete CameraInfoAction;
            delete VideoProcessingInfoAction;
            delete CameraContextMenu;
        }

        void SetCurrentView( MultiCameraView* view, QLayout* parentLayout );
        void SetCurrentView( const XGuid& viewId );
        void SetCurrentView( int index );
        void RotateViews( );
        bool CheckIfViewRotationIsRequired( );

        void ShowAvailableViews( QWidget* parent, QLayout* parentLayout );
        void StartVideoMonitoringForViewButtons( );
        const set<XGuid> GetViewableCameras( );
        void NotifyNewThumnailImage( const XGuid& cameraObjectId );

        const QSize GetCamerasViewSize( const QSize& totalAvailableSize );
        const XGuid GetCameraAt( int row, int column );

    private:

        // Create context menu used for video source in the sandbox
        void CreateCameraContextMenu( QWidget* parent )
        {
            // create context menu
            CameraContextMenu = new QMenu( parent );

            // create actions
            CameraInfoAction = new QAction( parent );
            CameraInfoAction->setText( "Camera info" );
            CameraInfoAction->setStatusTip( "Show information for the highlighted camera" );
            CameraInfoAction->setIcon( QIcon( QPixmap( ":/images/icons/info.png" ) ) );

            CameraSettingsAction = new QAction( parent );
            CameraSettingsAction->setText( "Camera settings" );
            CameraSettingsAction->setStatusTip( "Show camera settings to change at run time" );
            CameraSettingsAction->setIcon( QIcon( QPixmap( ":/images/icons/properties.png" ) ) );

            VideoProcessingInfoAction = new QAction( parent );
            VideoProcessingInfoAction->setText( "Video processing info" );
            VideoProcessingInfoAction->setStatusTip( "Show video processing graph info for the camera" );
            VideoProcessingInfoAction->setIcon( QIcon( QPixmap( ":/images/icons/video_processing_info.png" ) ) );

            TakeSnapshotAction = new QAction( parent );
            TakeSnapshotAction->setText( "Take snapshot" );
            TakeSnapshotAction->setStatusTip( "Take snapshot of the video" );
            TakeSnapshotAction->setIcon( QIcon( QPixmap( ":/images/icons/thumbnail.png" ) ) );

            CameraContextMenu->addAction( CameraInfoAction );
            CameraContextMenu->addAction( CameraSettingsAction );
            CameraContextMenu->addAction( VideoProcessingInfoAction );
            CameraContextMenu->addSeparator( );
            CameraContextMenu->addAction( TakeSnapshotAction );
        }

    public:
        Ui::SandboxView*                    Ui;
        SandboxView*                        Parent;
        map<XGuid, uint32_t>                VideoSourceMap;
        map<XGuid, uint32_t>                ThreadsMap;
        vector<CamerasViewConfiguration>    Views;
        XGuid                               DefaultViewId;
        XGuid                               CurrentViewId;

        MultiCameraView*                    CurrentView;
        ViewButtonsMap                      ViewButtons;
        VideoMonitorsMap                    VideoMonitors;

        bool                                AutoSizeView;
        QSize                               PreferredContentSize;
        SandboxSettings                     ViewsSettings;  // SandboxSettings should be split into more and
                                                            // here we should have only views setting

        vector<QShortcut*>                  ViewShortcuts;
        QTimer                              RotationTimer;
        QPoint                              LastMousePoint;
        int                                 UserInactivityCounter;
        bool                                IsRotationPaused;

        QMenu*                              CameraContextMenu;
        QAction*                            CameraInfoAction;
        QAction*                            CameraSettingsAction;
        QAction*                            VideoProcessingInfoAction;
        QAction*                            TakeSnapshotAction;
        int                                 LastClickedRow;
        int                                 LastClickedColumn;
    };
}

// SandboxView class constructor
SandboxView::SandboxView( const XGuid& objectId, QWidget* parent ) :
    ProjectObjectViewFrame( objectId, parent ),
    ui( new Ui::SandboxView ),
    mData( new SandboxViewData( ui, this ) )
{

    ui->setupUi( this );

    ui->horizontalSeparatorLine->setVisible( false );
    ui->horizontalViewsListFrame->setVisible( false );
    ui->verticalSeparatorLine->setVisible( false );
    ui->verticalViewsListFrame->setVisible( false );

    ui->horizontalViewsListFrame->setStyleSheet( "#horizontalViewsListFrame { background-color:#404040; }" );
    ui->verticalViewsListFrame->setStyleSheet( "#verticalViewsListFrame { background-color:#404040; }" );

    connect( this, SIGNAL(OnThumbnailImageUpdateRequest(CVSandbox::XGuid)),
             this, SLOT(me_OnThumbnailImageUpdateRequest(CVSandbox::XGuid)), Qt::QueuedConnection );
    connect( &mData->RotationTimer, SIGNAL(timeout()), this, SLOT(me_RotationTimer_timeout()) );

    for ( int i = 1; i <= 9; i++ )
    {
        mData->ViewShortcuts.push_back( new QShortcut( QKeySequence( QString( "Ctrl+%0" ).arg( i ) ), this ) );
    }

    connect( mData->ViewShortcuts[0], SIGNAL(activated()), this, SLOT(shortcut1_activated()) );
    connect( mData->ViewShortcuts[1], SIGNAL(activated()), this, SLOT(shortcut2_activated()) );
    connect( mData->ViewShortcuts[2], SIGNAL(activated()), this, SLOT(shortcut3_activated()) );
    connect( mData->ViewShortcuts[3], SIGNAL(activated()), this, SLOT(shortcut4_activated()) );
    connect( mData->ViewShortcuts[4], SIGNAL(activated()), this, SLOT(shortcut5_activated()) );
    connect( mData->ViewShortcuts[5], SIGNAL(activated()), this, SLOT(shortcut6_activated()) );
    connect( mData->ViewShortcuts[6], SIGNAL(activated()), this, SLOT(shortcut7_activated()) );
    connect( mData->ViewShortcuts[7], SIGNAL(activated()), this, SLOT(shortcut8_activated()) );
    connect( mData->ViewShortcuts[8], SIGNAL(activated()), this, SLOT(shortcut9_activated()) );

    // context menu
    connect( mData->CameraContextMenu, SIGNAL(aboutToShow()), this, SLOT(menuCamera_aboutToShow()) );
    connect( mData->CameraInfoAction, SIGNAL(triggered()), this, SLOT(actionCameraInfo_triggered()) );
    connect( mData->CameraSettingsAction, SIGNAL(triggered()), this, SLOT(actionCameraSettings_triggered()) );
    connect( mData->VideoProcessingInfoAction, SIGNAL(triggered()), this, SLOT(actionVideoProcessingInfo_triggered()) );
    connect( mData->TakeSnapshotAction, SIGNAL(triggered()), this, SLOT(actionTakeSnapshot_triggered()) );
}

SandboxView::~SandboxView( )
{
    const shared_ptr<XAutomationServer>& server = ServiceManager::Instance( ).GetAutomationServer( );

    // delete shortcuts
    for_each( mData->ViewShortcuts.begin( ), mData->ViewShortcuts.end( ), []( QShortcut* shortcut )
    {
        delete shortcut;
    } );

    // stop all video monitors
    for ( VideoMonitorsMap::iterator it = mData->VideoMonitors.begin( ); it != mData->VideoMonitors.end( ); it++ )
    {
        it->second->VideoSource->SetListener( 0 );
    }
    mData->VideoMonitors.clear( );

    // delete all view buttons
    mData->ViewButtons.clear( );

    // finalize all video sources of the sandbox
    for ( map<XGuid, uint32_t>::const_iterator it = mData->VideoSourceMap.begin( );
          it != mData->VideoSourceMap.end( ); ++it )
    {
        server->FinalizeVideoSource( it->second );
    }
    // finalize all threads of the sandbox
    for ( map<XGuid, uint32_t>::const_iterator it = mData->ThreadsMap.begin( );
          it != mData->ThreadsMap.end( ); ++it )
    {
        server->FinalizeThread( it->second );
    }

    // clear any possible variable stored on the server
    server->ClearAllVariables( );

    mData->SetCurrentView( 0, ui->currentViewFrame->layout( ) );

    delete mData;
    delete ui;
}

// Custom comparer of camera view configurations used for sorting
static bool ViewConfigurationLessByName( const CamerasViewConfiguration& view1, const CamerasViewConfiguration& view2 )
{
    return view1.Name( ) < view2.Name( );
}

// Set configuration of views
void SandboxView::SetViews( const vector<CamerasViewConfiguration>& views,
                            const XGuid& defaultViewId,
                            const map<XGuid, uint32_t>& videoSourceMap,
                            const SandboxSettings& settings )
{
    bool multiView = ( views.size( ) > 1 );

    // the method is really supposed to be called once for now
    if ( mData->VideoSourceMap.empty( ) )
    {
        mData->Views          = views;
        mData->DefaultViewId  = defaultViewId;
        mData->VideoSourceMap = videoSourceMap;
        mData->ViewsSettings  = settings;

        sort( mData->Views.begin( ), mData->Views.end( ), ViewConfigurationLessByName );

        if ( mData->ViewsSettings.GetViewButtonsPosition( ) == SandboxSettings::ViewButtonsPosition::VerticalRight )
        {
            switch ( mData->ViewsSettings.GetViewButtonsAlignment( ) )
            {
            case SandboxSettings::ViewButtonsAlignment::TopLeft:
                ui->verticalTopSpacer->changeSize( 10, 0, QSizePolicy::Minimum, QSizePolicy::Ignored );
                break;
            case SandboxSettings::ViewButtonsAlignment::BottomRight:
                ui->verticalBottomSpacer->changeSize( 10, 0, QSizePolicy::Minimum, QSizePolicy::Ignored );
                break;
            default:
                break;
            }

            ui->verticalSeparatorLine->setVisible( multiView );
            ui->verticalViewsListFrame->setVisible( multiView );

            mData->ShowAvailableViews( this, ui->verticalLayoutsFrame->layout( ) );
        }
        else
        {
            switch ( mData->ViewsSettings.GetViewButtonsAlignment( ) )
            {
            case SandboxSettings::ViewButtonsAlignment::TopLeft:
                ui->horizontalLeftSpacer->changeSize( 0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum );
                break;
            case SandboxSettings::ViewButtonsAlignment::BottomRight:
                ui->horizontalRightSpacer->changeSize( 0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum );
                break;
            default:
                break;
            }

            ui->horizontalSeparatorLine->setVisible( multiView );
            ui->horizontalViewsListFrame->setVisible( multiView );

            mData->ShowAvailableViews( this, ui->horizontalLayoutsFrame->layout( ) );
        }

        mData->SetCurrentView( defaultViewId );
        mData->StartVideoMonitoringForViewButtons( );

        if ( ( settings.IsViewsRotationEnabled( ) ) && ( views.size( ) > 1 ) )
        {
            mData->LastMousePoint        = QPoint( -1, -1 );
            mData->UserInactivityCounter = 0;
            mData->IsRotationPaused      = false;

            mData->RotationTimer.start( );
        }
    }
}

// Set IDs of threads running for the sandbox
void SandboxView::SetThreads( const std::map<XGuid, uint32_t>& threadsMap )
{
    mData->ThreadsMap = threadsMap;
}

// Set preferred size of the project's view content or let it auto size
void SandboxView::SetPreferredContentSize( bool autoSize, const QSize& size )
{
    if ( ( mData->CurrentView != nullptr ) &&
         ( ( !mData->AutoSizeView  ) ||( mData->AutoSizeView != autoSize ) ) )
    {
        mData->CurrentView->SetPreferredContentSize( autoSize, mData->GetCamerasViewSize( size ) );
    }

    mData->AutoSizeView         = autoSize;
    mData->PreferredContentSize = size;
}

// On grid button clicked
void SandboxView::gridButton_clicked( QObject* sender )
{
    mData->SetCurrentView( XGuid( static_cast<GridButton*>( sender )->objectName( ).toStdString( ) ) );
}

// Update thumbnail image for the given camera ID
void SandboxView::me_OnThumbnailImageUpdateRequest( XGuid cameraObjectId )
{
    shared_ptr<VideoMonitor> monitor = mData->VideoMonitors[cameraObjectId];

    if ( monitor )
    {
        shared_ptr<XImage>  thumbnailImage;
        bool                errorDetected;

        {
            QMutexLocker locker( &monitor->Sync );

            errorDetected = monitor->ErrorDetected;

            if ( monitor->ThumbnailImage )
            {
                thumbnailImage = monitor->ThumbnailImage->Clone( );
            }
        }

        for ( vector<CamerasViewConfiguration>::const_iterator it = mData->Views.begin( ); it != mData->Views.end( ); it++ )
        {
            int                      rowsCount    = it->RowsCount( );
            int                      columnsCount = it->ColumnsCount( );
            ViewButtonsMap::iterator itButton     = mData->ViewButtons.find( it->Id( ) );

            if ( itButton != mData->ViewButtons.end( ) )
            {
                GridWidget* grid = itButton->second->GetGridWidget( );

                for ( int row = 0; row < rowsCount; row++ )
                {
                    for ( int column = 0; column < columnsCount; column++ )
                    {
                        if ( it->IsCellVisible( row, column ) )
                        {
                            const CameraCellConfiguration* cellConfig = it->GetCellConfiguration( row, column );

                            if ( cellConfig != 0 )
                            {
                                if ( cellConfig->CameraId( ) == cameraObjectId )
                                {
                                    grid->SetCellBorderColor( row, column, ( errorDetected ) ? Qt::red : Qt::black );

                                    if ( thumbnailImage )
                                    {
                                        grid->SetCellImage( row, column, XImageInterface::XtoQimage( thumbnailImage ) );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// Rotation timer has timed out - lets rotate views
void SandboxView::me_RotationTimer_timeout( )
{
    if ( mData->CheckIfViewRotationIsRequired( ) )
    {
        mData->RotateViews( );
    }
}

// View buttons' shortcuts
void SandboxView::shortcut1_activated( )
{
    mData->SetCurrentView( 0 );
}
void SandboxView::shortcut2_activated( )
{
    mData->SetCurrentView( 1 );
}
void SandboxView::shortcut3_activated( )
{
    mData->SetCurrentView( 2 );
}
void SandboxView::shortcut4_activated( )
{
    mData->SetCurrentView( 3 );
}
void SandboxView::shortcut5_activated( )
{
    mData->SetCurrentView( 4 );
}
void SandboxView::shortcut6_activated( )
{
    mData->SetCurrentView( 5 );
}
void SandboxView::shortcut7_activated( )
{
    mData->SetCurrentView( 6 );
}
void SandboxView::shortcut8_activated( )
{
    mData->SetCurrentView( 7 );
}
void SandboxView::shortcut9_activated( )
{
    mData->SetCurrentView( 8 );
}

// Mouse button pressed in one of the camera views
void SandboxView::view_CameraMousePress( int row, int column, bool isLeftButton, QPoint globalPos )
{
    const XGuid cameraId = mData->GetCameraAt( row, column );

    if ( !cameraId.IsEmpty( ) )
    {
        mData->LastClickedRow    = row;
        mData->LastClickedColumn = column;

        if ( !isLeftButton )
        {
            mData->CameraContextMenu->popup( globalPos );
        }
    }
}

// Camera's context menu is about to show
void SandboxView::menuCamera_aboutToShow( )
{
    const XGuid cameraId = mData->GetCameraAt( mData->LastClickedRow, mData->LastClickedColumn );

    if ( !cameraId.IsEmpty( ) )
    {
        mData->CameraSettingsAction->setVisible( IsRunTimeConfigurationAvailable( cameraId ) );
    }
}

// Show camera information
void SandboxView::actionCameraInfo_triggered( )
{
    const XGuid cameraId = mData->GetCameraAt( mData->LastClickedRow, mData->LastClickedColumn );

    if ( ( !cameraId.IsEmpty( ) ) && ( mData->CurrentView != nullptr ) )
    {
        CameraInfoDialog cameraInfoDialog( this );

        mData->CurrentView->HighlightCell( mData->LastClickedRow, mData->LastClickedColumn, true );
        mData->IsRotationPaused = true;

        cameraInfoDialog.SetCameraId( cameraId, mData->VideoSourceMap[cameraId] );
        cameraInfoDialog.exec( );

        mData->CurrentView->HighlightCell( mData->LastClickedRow, mData->LastClickedColumn, false );
        mData->IsRotationPaused      = false;
        mData->UserInactivityCounter = 0;
    }
}

// Show configuration of camera's run time properties
void SandboxView::actionCameraSettings_triggered( )
{
    const XGuid cameraId = mData->GetCameraAt( mData->LastClickedRow, mData->LastClickedColumn );

    if ( !cameraId.IsEmpty( ) )
    {
        EditRunTimeConfiguration( cameraId, mData->VideoSourceMap[cameraId], this );
    }
}

// Help requested for the plug-in of the clicked video source
void SandboxView::on_HelpRequested( QWidget* sender )
{
    const XGuid cameraId = mData->GetCameraAt( mData->LastClickedRow, mData->LastClickedColumn );

    if ( !cameraId.IsEmpty( ) )
    {
        ServiceManager& serviceManager = ServiceManager::Instance( );

        shared_ptr<CameraProjectObject> poCamera = static_pointer_cast<CameraProjectObject>(
                serviceManager.GetProjectManager( )->GetProjectObject( cameraId ) );

        if ( poCamera )
        {
            serviceManager.GetHelpService( )->ShowPluginDescription( sender, poCamera->PluginId( ) );
        }
    }
}

// Show video processing information
void SandboxView::actionVideoProcessingInfo_triggered( )
{
    const XGuid cameraId = mData->GetCameraAt( mData->LastClickedRow, mData->LastClickedColumn );

    if ( ( !cameraId.IsEmpty( ) ) && ( mData->CurrentView != nullptr ) )
    {
        VideoProcessingInfoDialog videoProcessingInfoDialog( this );

        mData->CurrentView->HighlightCell( mData->LastClickedRow, mData->LastClickedColumn, true );
        mData->IsRotationPaused = true;

        videoProcessingInfoDialog.SetCameraId( ProjectObjectId( ), cameraId, mData->VideoSourceMap[cameraId] );
        videoProcessingInfoDialog.exec( );

        mData->CurrentView->HighlightCell( mData->LastClickedRow, mData->LastClickedColumn, false );
        mData->IsRotationPaused      = false;
        mData->UserInactivityCounter = 0;
    }
}

// Take snapshot of a video source
void SandboxView::actionTakeSnapshot_triggered( )
{
    const XGuid cameraId                    = mData->GetCameraAt( mData->LastClickedRow, mData->LastClickedColumn );
    shared_ptr<ProjectObject> cameraObject  = ServiceManager::Instance( ).GetProjectManager( )->GetProjectObject( cameraId );
    shared_ptr<ProjectObject> sandboxObject = ServiceManager::Instance( ).GetProjectManager( )->GetProjectObject( ProjectObjectId( ) );

    if ( ( !cameraId.IsEmpty( ) ) && ( cameraObject ) && ( sandboxObject ) )
    {
        ServiceManager::Instance( ).GetMainWindowService( )->
                TakeSnapshotForVideoSource( mData->VideoSourceMap[cameraId],
                                            QString( "%0 / %1" ).arg( QString::fromUtf8( sandboxObject->Name( ).c_str( ) ) ).
                                                                 arg( QString::fromUtf8( cameraObject->Name( ).c_str( ) ) ) );
    }
}

// Changes current view of the sandbox
void SandboxViewData::SetCurrentView( MultiCameraView* view, QLayout* parentLayout )
{
    if ( CurrentView != NULL )
    {
        parentLayout->removeWidget( CurrentView );
        delete CurrentView;
        CurrentView = NULL;
    }

    if ( view != NULL )
    {
        CurrentView = view;
        parentLayout->addWidget( CurrentView );

        view->connect( view, SIGNAL(CameraMousePress(int,int,bool,QPoint)),
                       Parent, SLOT(view_CameraMousePress(int,int,bool,QPoint)) );
    }
}

// Set current view of the sandbox
void SandboxViewData::SetCurrentView( const XGuid& viewId )
{
    if ( !Views.empty( ) )
    {
        CamerasViewConfiguration viewConfiguration;
        XGuid                    selectedId;

        for ( vector<CamerasViewConfiguration>::const_iterator it = Views.begin( ); it != Views.end( ); it++ )
        {
            if ( it->Id( ) == viewId )
            {
                viewConfiguration = *it;
                selectedId        = viewId;
                break;
            }
        }

        if ( selectedId.IsEmpty( ) )
        {
            viewConfiguration = Views.front( );
            selectedId        = viewConfiguration.Id( );
        }

        if ( selectedId != CurrentViewId )
        {
            MultiCameraView* cameraView = new MultiCameraView( Parent );

            cameraView->SetView( viewConfiguration, VideoSourceMap,
                                 AutoSizeView, GetCamerasViewSize( PreferredContentSize ) );
            SetCurrentView( cameraView, Ui->currentViewFrame->layout( ) );

            // select corresponding button
            for ( ViewButtonsMap::iterator it = ViewButtons.begin( ); it != ViewButtons.end( ); it++ )
            {
                it->second->SetIsSelected( selectedId == it->first );
            }

            CurrentViewId = selectedId;
        }
    }
}

// Set view with the specified index as the current view
void SandboxViewData::SetCurrentView( int index )
{
    if ( ( index >= 0 ) && ( index < static_cast<int>( Views.size( ) ) ) )
    {
        SetCurrentView( Views[index].Id( ) );
    }
}

// Switch to the next view in the list
void SandboxViewData::RotateViews( )
{
    if ( !Views.empty( ) )
    {
        int viewIndexToSwitchTo = 0;
        int counter             = 1;

        for ( vector<CamerasViewConfiguration>::const_iterator it = Views.begin( ); it != Views.end( ); ++it, ++counter )
        {
            if ( it->Id( ) == CurrentViewId )
            {
                viewIndexToSwitchTo = counter;
            }
        }

        if ( viewIndexToSwitchTo >= static_cast<int>( Views.size( ) ) )
        {
            viewIndexToSwitchTo = 0;
        }

        SetCurrentView( viewIndexToSwitchTo );
    }
}

// Check if it is time to rotate view
bool SandboxViewData::CheckIfViewRotationIsRequired( )
{
    QPoint point = QCursor::pos( );
    bool   ret   = false;

    if ( !IsRotationPaused )
    {
        if ( LastMousePoint == point )
        {
            UserInactivityCounter++;
        }
        else
        {
            UserInactivityCounter = 0;
        }

        LastMousePoint = point;

        if ( UserInactivityCounter >= ViewsSettings.RotationStartTime( ) )
        {
            if ( ( ( UserInactivityCounter - ViewsSettings.RotationStartTime( ) ) % ViewsSettings.ViewsRotationTime( ) ) == 0 )
            {
                ret = true;
            }
        }
    }

    return ret;
}

// Show available views to choose from
void SandboxViewData::ShowAvailableViews( QWidget* parent, QLayout* parentLayout )
{
    int counter = 1;

    for ( vector<CamerasViewConfiguration>::const_iterator it = Views.begin( ); it != Views.end( ); it++ )
    {
        shared_ptr<GridButton>  button( new GridButton( parentLayout->parentWidget( ) ) );
        GridWidget*             grid         = button->GetGridWidget( );
        int                     rowsCount    = it->RowsCount( );
        int                     columnsCount = it->ColumnsCount( );

        grid->SetRowsCount( rowsCount );
        grid->SetColumnsCount( columnsCount);
        grid->SetCellsDefaultBackgroundColor( QColor( 64, 64, 64 ) );

        for ( int row = 0; row < rowsCount; row++ )
        {
            for ( int column = 0; column < columnsCount; column++ )
            {
                const CameraCellConfiguration* cellConfig = it->GetCellConfiguration( row, column );

                if ( cellConfig != 0 )
                {
                    grid->SetCellSpan( row, column, cellConfig->RowSpan( ), cellConfig->ColumnSpan( ) );
                }
            }
        }

        button->setObjectName( QString::fromStdString( it->Id( ).ToString( ) ) );
        button->setMinimumSize( 90, 90 );
        button->setMaximumSize( 90, 90 );

        if ( counter < 10 )
        {
            button->setToolTip( QString( "Ctrl+%0" ).arg( counter++ ) );
        }

        // set bold title for default view
        if ( it->Id( ) == DefaultViewId )
        {
            QFont font = button->TitleFont( );

            font.setBold( true );
            button->SetTitleFont( font );
        }

        parent->connect( button.get( ), SIGNAL( clicked( QObject* ) ), parent, SLOT( gridButton_clicked( QObject* ) ) );

        button->setVisible( true );
        button->SetTitle( QString::fromUtf8( it->Name( ).c_str( ) ) );
        parentLayout->addWidget( button.get( ) );

        ViewButtons.insert( ViewButtonsMap::value_type( it->Id( ), button ) );
    }
}

// Start monitoring cameras of the sandbox, so thumbnails could be genrated
void SandboxViewData::StartVideoMonitoringForViewButtons( )
{
    if ( Views.size( ) > 1 )
    {
        const shared_ptr<XAutomationServer>& server           = ServiceManager::Instance( ).GetAutomationServer( );
        const set<XGuid>&                    camerasToMonitor = GetViewableCameras( );

        for ( set<XGuid>::const_iterator it = camerasToMonitor.begin( ); it != camerasToMonitor.end( ); it++ )
        {
            const XGuid&                         cameraId = *it;
            map<XGuid, uint32_t>::const_iterator vsIt     = VideoSourceMap.find( cameraId );

            if ( vsIt != VideoSourceMap.end( ) )
            {
                shared_ptr<VideoSourceInAutomationServer> videoSource = VideoSourceInAutomationServer::Create( server, vsIt->second );
                shared_ptr<VideoMonitor> monitor( new VideoMonitor( Parent, cameraId, videoSource ) );

                videoSource->SetListener( monitor.get( ) );

                VideoMonitors.insert( VideoMonitorsMap::value_type( cameraId, monitor ) );
            }
        }
    }
}

// Get set of cameras which are added to some view
const set<XGuid> SandboxViewData::GetViewableCameras( )
{
    set<XGuid> viewableCameras;

    for ( vector<CamerasViewConfiguration>::const_iterator it = Views.begin( ); it != Views.end( ); it++ )
    {
        int rowsCount    = it->RowsCount( );
        int columnsCount = it->ColumnsCount( );

        for ( int row = 0; row < rowsCount; row++ )
        {
            for ( int column = 0; column < columnsCount; column++ )
            {
                const CameraCellConfiguration* cellConfig = it->GetCellConfiguration( row, column );

                if ( cellConfig != NULL )
                {
                    viewableCameras.insert( cellConfig->CameraId( ) );
                }
            }
        }
    }

    return viewableCameras;
}

// Get available view size for cameras, which excludes area for view buttons
const QSize SandboxViewData::GetCamerasViewSize( const QSize& totalAvailableSize )
{
    QSize ret = totalAvailableSize - QSize( 30, 30 );

    if ( Views.size( ) > 1 )
    {
        if ( ViewsSettings.GetViewButtonsPosition( ) == SandboxSettings::ViewButtonsPosition::VerticalRight )
        {
            ret -= QSize( Ui->verticalViewsListFrame->width( ), 0 );
        }
        else
        {
            ret -= QSize( 0, Ui->horizontalViewsListFrame->height( ) );
        }
    }

    return ret;
}

// Get ID of the camera displayed currently at the specified row/column
const XGuid SandboxViewData::GetCameraAt( int row, int column )
{
    XGuid cameraId;

    if ( ( row >= 0 ) && ( column >= 0 ) )
    {
        for ( vector<CamerasViewConfiguration>::const_iterator it = Views.begin( ); it != Views.end( ); ++it )
        {
            if ( ( it->Id( ) == CurrentViewId ) && ( row < it->RowsCount( ) ) && ( column < it->ColumnsCount( ) ) )
            {
                const CameraCellConfiguration* cellConfig = it->GetCellConfiguration( row, column );

                if ( cellConfig != NULL )
                {
                    cameraId = cellConfig->CameraId( );
                }
            }
        }
    }

    return cameraId;
}

// New video frame notification - create thumbnail image
void VideoMonitor::OnNewImage( const shared_ptr<const XImage>& image )
{
    if ( ( LastUpdated.isNull( ) ) || ( LastUpdated.elapsed( ) >= THUMBNAIL_IMAGE_UPDATE_FREQUENCY_MS ) )
    {
        uint32_t            resizedWidth  = THUMBNAIL_IMAGE_WIDTH;
        uint32_t            resizedHeight = ( image->Height( ) * resizedWidth ) / image->Width( );
        shared_ptr<XImage>  smallerImage  = XImage::Allocate( resizedWidth, resizedHeight, image->Format( ) );

        ResizeImageBilinear( image->ImageData( ), smallerImage->ImageData( ) );

        {
            QMutexLocker locker( &Sync );
            ThumbnailImage = smallerImage;
            ErrorDetected = false;
        }

        LastUpdated.restart( );

        emit Parent->OnThumbnailImageUpdateRequest( CameraObjectId );
    }
}

// Video source error notification
void VideoMonitor::OnError( const string& errorMessage )
{
    XUNREFERENCED_PARAMETER( errorMessage );

    {
        QMutexLocker locker( &Sync );
        ErrorDetected = true;
    }

    emit Parent->OnThumbnailImageUpdateRequest( CameraObjectId );
}

