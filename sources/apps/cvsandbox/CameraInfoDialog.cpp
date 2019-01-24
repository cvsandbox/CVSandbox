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

#include <QTime>
#include <QTimer>
#include <vector>
#include <numeric>

#include "CameraInfoDialog.hpp"
#include "ui_CameraInfoDialog.h"

#include <QMutex>

#include "ServiceManager.hpp"
#include "CameraProjectObject.hpp"

#include <UITools.hpp>

#include <XVideoSourceFrameInfo.hpp>

// Number of the last FPS values to average
#define FPS_HISTORY_LENGTH (10)

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

namespace Private
{
    class CameraInfoDialogData : public IAutomationVideoSourceListener
    {
    public:
        CameraInfoDialogData( Ui::CameraInfoDialog* ui ) :
            Ui( ui ), VideoSourceId( 0 ), FrameInfoUpdateTimer( ),
            LastUpdated( ), LastFramesCount( 0 ),
            LastFpsValues( ), NextFpsIndex( 0 ),
            SyncMutex( ), LastVideoSourceError( )
        {
            FrameInfoUpdateTimer.setInterval( 1000 );
        }

        void UpdateFrameInfo( );

        void OnNewVideoFrame( uint32_t videoSourceId, const std::shared_ptr<const XImage>& image );
        void OnErrorMessage( uint32_t videoSourceId, const std::string& errorMessage );

    public:
        Ui::CameraInfoDialog*   Ui;
        uint32_t                VideoSourceId;
        QTimer                  FrameInfoUpdateTimer;

        QTime                   LastUpdated;
        uint32_t                LastFramesCount;

        vector<float>           LastFpsValues;
        int                     NextFpsIndex;

        QMutex                  SyncMutex;
        QString                 LastVideoSourceError;
    };
}

CameraInfoDialog::CameraInfoDialog( QWidget* parent ) :
    QDialog( parent ),
    ui( new Ui::CameraInfoDialog ),
    mData( new ::Private::CameraInfoDialogData( ui ) )
{
    ui->setupUi( this );
    ui->errorGroupBox->setVisible( false );

    UITools::SetWidgetBackgroundColor( ui->errorLineEdit, QColor( 255, 200, 200 ) );

    // disable "?" button on title bar
    setWindowFlags( ( windowFlags( ) & ~Qt::WindowContextHelpButtonHint ) );

    // restore size/position
    ServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );

    // connect timer
    connect( &mData->FrameInfoUpdateTimer, SIGNAL(timeout()), this, SLOT(on_FrameInfoUpdateTimer_timeout()) );
}

CameraInfoDialog::~CameraInfoDialog( )
{
    ServiceManager::Instance( ).GetAutomationServer( )->RemoveVideoSourceListener( mData->VideoSourceId, mData );
    mData->FrameInfoUpdateTimer.stop( );

    delete mData;
    delete ui;
}

void CameraInfoDialog::on_closeButton_clicked( )
{
    this->accept( );
}

// Widget is closed
void CameraInfoDialog::closeEvent( QCloseEvent* )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
}

// Dialog is accepted
void CameraInfoDialog::accept( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::accept( );
}

// Dialog is rejected
void CameraInfoDialog::reject( )
{
    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );
    QDialog::reject( );
}

// Set Id of the camera to display information for
void CameraInfoDialog::SetCameraId( const XGuid& cameraId, uint32_t videoSourceId )
{
    // remove listener for previous video source
    if ( mData->VideoSourceId != 0 )
    {
        ServiceManager::Instance( ).GetAutomationServer( )->RemoveVideoSourceListener( mData->VideoSourceId, mData );
        mData->LastVideoSourceError.clear( );
        ui->errorGroupBox->setVisible( false );
    }

    ui->cameraNameEdit->clear( );
    ui->cameraDescriptionEdit->clear( );
    ui->pluginNameEdit->clear(  );

    if ( !cameraId.IsEmpty( ) )
    {
        shared_ptr<CameraProjectObject> po = static_pointer_cast<CameraProjectObject>(
                ServiceManager::Instance( ).GetProjectManager( )->GetProjectObject( cameraId ) );

        mData->VideoSourceId   = videoSourceId;
        mData->LastFramesCount = 0;
        mData->NextFpsIndex    = 0;
        mData->LastFpsValues.clear( );

        if ( po )
        {
            ui->cameraNameEdit->setText( QString::fromUtf8( po->Name( ).c_str( ) ) );
            ui->cameraDescriptionEdit->setPlainText( QString::fromUtf8( po->Description( ).c_str( ) ) );

            // get description of the plug-in
            shared_ptr<const XPluginDescriptor> pluginDesc =
                ServiceManager::Instance( ).GetPluginsEngine( )->GetPlugin( po->PluginId( ) );

            if ( pluginDesc )
            {
                ui->pluginNameEdit->setText( QString::fromUtf8( pluginDesc->Name( ).c_str( ) ) );
            }
        }

        // get initial info
        mData->UpdateFrameInfo( );

        // start update timer
        mData->FrameInfoUpdateTimer.start( );

        // register listener to get error notification
        ServiceManager::Instance( ).GetAutomationServer( )->AddVideoSourceListener( videoSourceId, mData );
    }
}

// Video frame rate/size information update timer
void CameraInfoDialog::on_FrameInfoUpdateTimer_timeout( )
{
    mData->UpdateFrameInfo( );
}

namespace Private
{

void CameraInfoDialogData::UpdateFrameInfo( )
{
    struct XVideoSourceFrameInfo frameInfo;

    if ( ServiceManager::Instance( ).GetAutomationServer( )->GetVideoSourceFrameInfo( VideoSourceId, &frameInfo ) )
    {
        // frame size
        if ( frameInfo.OriginalFrameWidth != 0 )
        {
            if ( ( frameInfo.OriginalFrameWidth  == frameInfo.ProcessedFrameWidth ) &&
                 ( frameInfo.OriginalFrameHeight == frameInfo.ProcessedFrameHeight ) &&
                 ( frameInfo.OriginalPixelFormat == frameInfo.ProcessedPixelFormat ) )
            {
                Ui->frameSizeEdit->setText( QString( "%0 x %1, %2" )
                                            .arg( frameInfo.OriginalFrameWidth )
                                            .arg( frameInfo.OriginalFrameHeight )
                                            .arg( XImage::PixelFormatName( frameInfo.OriginalPixelFormat ).c_str( ) ) );
            }
            else
            {
                Ui->frameSizeEdit->setText( QString( "%0 x %1, %2 -> %3 x %4, %5" )
                                            .arg( frameInfo.OriginalFrameWidth )
                                            .arg( frameInfo.OriginalFrameHeight )
                                            .arg( XImage::PixelFormatName( frameInfo.OriginalPixelFormat ).c_str( ) )
                                            .arg( frameInfo.ProcessedFrameWidth )
                                            .arg( frameInfo.ProcessedFrameHeight )
                                            .arg( XImage::PixelFormatName( frameInfo.ProcessedPixelFormat ).c_str( ) ) );
            }
        }
        else
        {
            Ui->frameSizeEdit->setText( "Unknown" );
        }

        // frame rate
        if ( ( LastFramesCount != 0 ) && ( LastFramesCount <= frameInfo.FramesReceived ) )
        {
            float fps = static_cast<float>( LastUpdated.elapsed( ) ) * ( frameInfo.FramesReceived - LastFramesCount ) / 1000.0f;

            if ( LastFpsValues.size( ) < FPS_HISTORY_LENGTH )
            {
                LastFpsValues.push_back( fps );
            }
            else
            {
                LastFpsValues[NextFpsIndex++] = fps;
                NextFpsIndex %= FPS_HISTORY_LENGTH;
            }

            // get average value
            double fpsSum = std::accumulate( LastFpsValues.begin( ), LastFpsValues.end( ), 0.0f );

            Ui->frameRateEdit->setText( QString( "%0 fps" ).arg( fpsSum / LastFpsValues.size( ), 0, 'f', 2 ) );
        }

        LastFramesCount = frameInfo.FramesReceived;
        LastUpdated.restart( );
    }

    // check for video source error
    {
        QMutexLocker locker( &SyncMutex );

        if ( !LastVideoSourceError.isEmpty( ) )
        {
            if ( Ui->errorLineEdit->text( ) != LastVideoSourceError )
            {
                Ui->errorLineEdit->setText( LastVideoSourceError );
            }

            Ui->errorGroupBox->setVisible( true );
        }
    }
}

void CameraInfoDialogData::OnNewVideoFrame( uint32_t, const std::shared_ptr<const XImage>& )
{
    // do nothing with image
}

void CameraInfoDialogData::OnErrorMessage( uint32_t videoSourceId, const std::string& errorMessage )
{
    QMutexLocker locker( &SyncMutex );
    LastVideoSourceError = QString::fromStdString( errorMessage );


    XUNREFERENCED_PARAMETER( videoSourceId )
}

} // namespace Private
