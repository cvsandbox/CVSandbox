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

#include "SingleCameraView.hpp"
#include "ui_SingleCameraView.h"

#include "VideoSourcePlayerFactory.hpp"
#include "ServiceManager.hpp"
#include "CameraInfoDialog.hpp"
#include "EditVideoSourceRunTimeProperties.hpp"
#include "CameraProjectObject.hpp"

#include <QMouseEvent>
#include <QMenu>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;

namespace Private
{
    class SingleCameraViewData
    {
    public:
        SingleCameraViewData( QWidget* parent ) :
            Player( VideoSourcePlayerFactory::CreatePlayer( parent ) ),
            VideoSourceId( 0 ),
            FitContentToWindow( false ),
            CameraContextMenu( nullptr ), CameraInfoAction( nullptr ),
            CameraSettingsAction( nullptr ), TakeSnapshotAction( nullptr )
        {
            Player->SetAutoSize( true );
            Player->SetRespectAspectRatio( true );

            CreateCameraContextMenu( parent );
        }

        ~SingleCameraViewData( )
        {
            delete TakeSnapshotAction;
            delete CameraSettingsAction;
            delete CameraInfoAction;
            delete CameraContextMenu;
        }

    private:

        // Create context menu used for the played video source
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

            TakeSnapshotAction = new QAction( parent );
            TakeSnapshotAction->setText( "Take snapshot" );
            TakeSnapshotAction->setStatusTip( "Take snapshot of the video" );
            TakeSnapshotAction->setIcon( QIcon( QPixmap( ":/images/icons/thumbnail.png" ) ) );

            CameraContextMenu->addAction( CameraInfoAction );
            CameraContextMenu->addAction( CameraSettingsAction );
            CameraContextMenu->addSeparator( );
            CameraContextMenu->addAction( TakeSnapshotAction );
        }

    public:
        shared_ptr<VideoSourcePlayer>   Player;
        uint32_t                        VideoSourceId;
        bool                            FitContentToWindow;

        QMenu*                          CameraContextMenu;
        QAction*                        CameraInfoAction;
        QAction*                        CameraSettingsAction;
        QAction*                        TakeSnapshotAction;

    };
}

SingleCameraView::SingleCameraView( const XGuid& objectId, QWidget* parent ) :
    ProjectObjectViewFrame( objectId, parent ),
    ui( new Ui::SingleCameraView ),
    mData( new Private::SingleCameraViewData( parent ) )
{
    ui->setupUi( this );

    ui->mainFrame->layout( )->removeWidget( ui->defaultLabel );
    ui->defaultLabel->setVisible( false );
    ui->mainFrame->layout( )->addWidget( mData->Player->PlayerWidget( ) );
    mData->Player->PlayerWidget( )->setVisible( true );
    mData->Player->PlayerWidget( )->installEventFilter( this );

    // context menu
    connect( mData->CameraContextMenu, SIGNAL(aboutToShow()),
             this, SLOT(menuCamera_aboutToShow()) );
    connect( mData->CameraInfoAction, SIGNAL(triggered()),
             this, SLOT(actionCameraInfo_triggered()), Qt::QueuedConnection );
    connect( mData->CameraSettingsAction, SIGNAL(triggered()),
             this, SLOT(actionCameraSettings_triggered()), Qt::QueuedConnection );
    connect( mData->TakeSnapshotAction, SIGNAL(triggered()),
             this, SLOT(actionTakeSnapshot_triggered()) );
}

SingleCameraView::~SingleCameraView( )
{
    ServiceManager::Instance( ).GetAutomationServer( )->FinalizeVideoSource( mData->VideoSourceId );
    SetVideoSource( 0 );

    delete mData;
    delete ui;
}

// Set video source to display
void SingleCameraView::SetVideoSource( const shared_ptr<VideoSourceInAutomationServer>& videoSource )
{
    if ( videoSource )
    {
        mData->VideoSourceId = videoSource->VideoSourceId( );
        mData->Player->SetVideoSource( static_pointer_cast<IVideoSource>( videoSource ) );
    }
    else
    {
        mData->VideoSourceId = 0;
        mData->Player->SetVideoSource( shared_ptr<IVideoSource>( ) );
    }
}

// Set preferred size of the project's view content or let it autosize
void SingleCameraView::SetPreferredContentSize( bool autoSize, const QSize& size )
{
    mData->Player->SetAutoSize( autoSize );

    if ( !autoSize )
    {
        mData->Player->SetContolSize( size - QSize( 60, 60 ) );
    }
}

// Handle mouse press in video player widgets
bool SingleCameraView::eventFilter( QObject* target, QEvent* event )
{
    bool ret = false;

    if ( ( event->type( ) == QEvent::MouseButtonPress ) &&
         ( static_cast<QObject*>( mData->Player->PlayerWidget( ) ) == target ) )
    {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>( event );

        if ( mouseEvent->button( ) == Qt::RightButton )
        {
            ret = true;

            mData->CameraContextMenu->popup( mouseEvent->globalPos( ) );
        }
    }

    return ret;
}

// Camera's context menu is about to show
void SingleCameraView::menuCamera_aboutToShow( )
{
    mData->CameraSettingsAction->setVisible( IsRunTimeConfigurationAvailable( ProjectObjectId( ) ) );
}

// Show camera information
void SingleCameraView::actionCameraInfo_triggered( )
{
    CameraInfoDialog cameraInfoDialog( this );

    mData->Player->SetHighlighted( true );
    cameraInfoDialog.SetCameraId( ProjectObjectId( ), mData->VideoSourceId );
    cameraInfoDialog.exec( );
    mData->Player->SetHighlighted( false );
}

// Show configuration of camera's run time properties
void SingleCameraView::actionCameraSettings_triggered( )
{
    EditRunTimeConfiguration( ProjectObjectId( ), mData->VideoSourceId, this );
}

// Take snapshot of the video source
void SingleCameraView::actionTakeSnapshot_triggered( )
{
    shared_ptr<ProjectObject> po = ServiceManager::Instance( ).GetProjectManager( )->GetProjectObject( ProjectObjectId( ) );

    if ( po )
    {
        ServiceManager::Instance( ).GetMainWindowService( )->
                TakeSnapshotForVideoSource( mData->VideoSourceId, QString::fromUtf8( po->Name( ).c_str( ) ) );
    }
}

// Help requested for the plug-in of the played video source
void SingleCameraView::on_HelpRequested( QWidget* sender )
{
    ServiceManager& serviceManager = ServiceManager::Instance( );

    shared_ptr<CameraProjectObject> poCamera = static_pointer_cast<CameraProjectObject>(
            serviceManager.GetProjectManager( )->GetProjectObject( ProjectObjectId( ) ) );

    if ( poCamera )
    {
        serviceManager.GetHelpService( )->ShowPluginDescription( sender, poCamera->PluginId( ) );
    }
}
