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

#include "VideoSnapshotDialog.hpp"
#include "ui_VideoSnapshotDialog.h"

#include "ServiceManager.hpp"
#include <XImageInterface.hpp>
#include <UITools.hpp>
#include <XError.hpp>

#include <ximaging.h>

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QStatusTipEvent>
#include <QLabel>
#include <QComboBox>
#include <QClipboard>
#include <QFileInfo>
#include <QMutex>
#include <QThread>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Automation;

namespace Private
{
    class VideoSnapshotDialogData : public IAutomationVideoSourceListener
    {
    public:
        VideoSnapshotDialog*     Parent;
        Ui::VideoSnapshotDialog* Ui;
        QWidget*                 WidgetToActivateOnClose;

        uint32_t            VideoSourceId;
        bool                WaitingAnImage;
        shared_ptr<XImage>  SnapshotImage;

        QPoint          LastMousePoint;
        QMutex          SyncMutex;

        QStatusBar*     StatusBar;
        QAction*        ReloadAction;
        QAction*        SaveAction;
        QAction*        SetToClipboardAction;
        QAction*        CloseAction;

        QAction*        SwitchToRgbAction;
        QAction*        SwitchToNrgbAction;
        QAction*        SwitchToHslAction;
        QAction*        SwitchToHsvAction;

        QAction*        SwitchRedHistogramAction;
        QAction*        SwitchGreenHistogramAction;
        QAction*        SwitchBlueHistogramAction;

        QLabel*         ImageSizeLabel;
        QLabel*         CoordinatesLabel;
        QLabel*         CoordinatesIconLabel;
        QLabel*         ColorLabel;
        QLabel*         ColorIconLabel;
        QComboBox*      ColorSpaceBox;

    public:
        VideoSnapshotDialogData( VideoSnapshotDialog* parent, Ui::VideoSnapshotDialog* ui,
                                 QWidget* widgetToActivateOnClose ) :
            Parent( parent ), Ui( ui ),
            WidgetToActivateOnClose( widgetToActivateOnClose ),
            VideoSourceId( 0 ), WaitingAnImage( false ), SnapshotImage( ),
            LastMousePoint( -1, -1 ), SyncMutex( ),
            StatusBar( nullptr ), ReloadAction( nullptr ), SaveAction( nullptr ),
            SetToClipboardAction( nullptr ), CloseAction( nullptr ),
            SwitchToRgbAction( nullptr ), SwitchToNrgbAction( nullptr ),
            SwitchToHslAction( nullptr ), SwitchToHsvAction( nullptr ),
            SwitchRedHistogramAction( nullptr ), SwitchGreenHistogramAction( nullptr ),
            SwitchBlueHistogramAction( nullptr ),
            ImageSizeLabel( nullptr ), CoordinatesLabel( nullptr ), CoordinatesIconLabel( nullptr ),
            ColorLabel( nullptr ), ColorIconLabel( nullptr ), ColorSpaceBox( nullptr )
        {

        }

        void InitUi( )
        {
            // add menu and status bars
            Parent->layout( )->setMenuBar( CreateMenuBar( ) );
            Parent->layout( )->addWidget( CreateStatusBar( ) );
        }

        void OnNewVideoFrame( uint32_t videoSourceId, const shared_ptr<const XImage>& image );
        void OnErrorMessage( uint32_t videoSourceId, const string& errorMessage );

        void ShowPixelColor( int x, int y );
        void ProcessSnapshot( );

    private:
        QMenuBar* CreateMenuBar( );
        QStatusBar* CreateStatusBar( );
    };
}

VideoSnapshotDialog::VideoSnapshotDialog( QWidget* parent, bool allowMinimize,
                                          QWidget* widgetToActivateOnClose ) :
    QDialog( parent ), ui( new Ui::VideoSnapshotDialog ),
    mData( new ::Private::VideoSnapshotDialogData( this, ui, widgetToActivateOnClose ) )
{
    ui->setupUi( this );
    mData->InitUi( );

    ui->scrollAreaWidgetContents->move( 0, 0 );
    ui->imageLabel->move( 2, 2 );

    // disable "?" button on title bar
    setWindowFlags( windowFlags( ) & ~Qt::WindowContextHelpButtonHint );

    // restore size/position
    ServiceManager::Instance( ).GetUiPersistenceService( )->RestoreWidget( this );

    if ( allowMinimize )
    {
        setWindowFlags( windowFlags( ) | Qt::WindowMinimizeButtonHint );
    }

    // connect signals to slots
    connect( mData->ReloadAction, SIGNAL( triggered() ), this, SLOT( on_ReloadAction_triggered() ) );
    connect( mData->SaveAction, SIGNAL( triggered() ), this, SLOT( on_SaveAction_triggered() ) );
    connect( mData->SetToClipboardAction, SIGNAL( triggered() ), this, SLOT( on_SetToClipboardAction_triggered() ) );
    connect( mData->CloseAction, SIGNAL( triggered() ), this, SLOT( close() ) );

    connect( mData->SwitchToRgbAction, SIGNAL( triggered() ), this, SLOT( on_SwitchToRgbAction_triggered() ) );
    connect( mData->SwitchToNrgbAction, SIGNAL( triggered() ), this, SLOT( on_SwitchToNrgbAction_triggered() ) );
    connect( mData->SwitchToHslAction, SIGNAL( triggered() ), this, SLOT( on_SwitchToHslAction_triggered() ) );
    connect( mData->SwitchToHsvAction, SIGNAL( triggered() ), this, SLOT( on_SwitchToHsvAction_triggered() ) );

    connect( mData->SwitchRedHistogramAction, SIGNAL( triggered() ), this, SLOT( on_SwitchRedHistogramAction_triggered() ) );
    connect( mData->SwitchGreenHistogramAction, SIGNAL( triggered() ), this, SLOT( on_SwitchGreenHistogramAction_triggered() ) );
    connect( mData->SwitchBlueHistogramAction, SIGNAL( triggered() ), this, SLOT( on_SwitchBlueHistogramAction_triggered() ) );

    connect( mData->ColorSpaceBox, SIGNAL( currentIndexChanged(int) ), this, SLOT( on_ColorSpaceBox_currentIndexChanged(int) ) );

    connect( this, SIGNAL( SnapshotTaken() ), this, SLOT( on_SnapshotTaken() ), Qt::QueuedConnection );

    // enable mouse tracking on the label up to the view
    UITools::SetWidgetMouseTracking( ui->imageLabel, this, true );
}

VideoSnapshotDialog::~VideoSnapshotDialog( )
{
    delete mData;
    delete ui;
}

// Widget is about to be closed
void VideoSnapshotDialog::closeEvent( QCloseEvent* )
{
    if ( mData->WaitingAnImage )
    {
        ServiceManager::Instance( ).GetAutomationServer( )->RemoveVideoSourceListener( mData->VideoSourceId, mData );
        mData->WaitingAnImage = false;
    }

    ServiceManager::Instance( ).GetUiPersistenceService( )->SaveWidget( this );

    if ( mData->WidgetToActivateOnClose != nullptr )
    {
        if ( mData->WidgetToActivateOnClose->isMinimized( ) )
        {
            mData->WidgetToActivateOnClose->showNormal( );
        }

        mData->WidgetToActivateOnClose->show( );
        mData->WidgetToActivateOnClose->activateWindow( );
    }

    mData->SnapshotImage.reset( );
    ui->imageLabel->clear( );
}

// Dialog is rejected
void VideoSnapshotDialog::reject( )
{
    // do nothing - don't call QDialog::reject() to ignore Esc
}

// Check if there is anything to show in status bar
bool VideoSnapshotDialog::event( QEvent* e )
{
    bool ret = true;

    if ( e->type( ) == QEvent::StatusTip )
    {
        QStatusTipEvent* ev = static_cast<QStatusTipEvent*>( e );
        mData->StatusBar->showMessage( ev->tip( ) );
    }
    else
    {
        ret = QDialog::event( e );
    }

    return ret;
}

// Take snapshot of the specified video source
void VideoSnapshotDialog::TakeSnapshot( uint32_t videoSourceId, const QString& title )
{
    this->setWindowTitle( title );

    ui->imageLabel->clear( );
    ui->imageLabel->resize( 1, 1 );

    mData->SaveAction->setEnabled( false );
    mData->SetToClipboardAction->setEnabled( false );

    mData->VideoSourceId  = videoSourceId;
    mData->WaitingAnImage = true;

    ServiceManager::Instance( ).GetAutomationServer( )->AddVideoSourceListener( videoSourceId, mData );
}

// Mouse pointer was moved
void VideoSnapshotDialog::mouseMoveEvent( QMouseEvent* ev )
{
    if ( mData->SnapshotImage )
    {
        QPoint imageViewPoint = ui->imageLabel->mapFromGlobal ( ev->globalPos( ) ) - QPoint( 1, 1 );
        QSize  viewSize = ui->imageLabel->size( ) - QSize( 2, 2 );
        int    x = imageViewPoint.x( );
        int    y = imageViewPoint.y( );

        if ( ( x >= 0 ) && ( y >= 0 ) && ( x < viewSize.width( ) ) && ( y < viewSize.height( ) ) )
        {
            mData->CoordinatesLabel->setText( QString( "%0, %1" ).arg( x ).arg( y ) );
            mData->ShowPixelColor( x, y );

            mData->LastMousePoint = imageViewPoint;
        }
        else
        {
            mData->CoordinatesLabel->setText( QString::null );
            mData->ColorLabel->setText( QString::null );
            mData->LastMousePoint = QPoint( -1, -1 );
        }
    }
}

// Mouse pointer has left
void VideoSnapshotDialog::leaveEvent( QEvent* )
{
    mData->CoordinatesLabel->setText( QString::null );
    mData->ColorLabel->setText( QString::null );
    mData->LastMousePoint = QPoint( -1, -1 );
}

// Snapshot was taken - process it
void VideoSnapshotDialog::on_SnapshotTaken( )
{
    mData->ProcessSnapshot( );
}

// Index of color spaces' combo box has changed - update pixel color information
void VideoSnapshotDialog::on_ColorSpaceBox_currentIndexChanged( int index )
{
    XUNREFERENCED_PARAMETER( index )

    if ( mData->LastMousePoint.x( ) != -1 )
    {
        mData->ShowPixelColor( mData->LastMousePoint.x( ), mData->LastMousePoint.y( ) );
    }
}

// Reload/retake snapshot from the same video source
void VideoSnapshotDialog::on_ReloadAction_triggered( )
{
    QMutexLocker locker( &mData->SyncMutex );

    if ( !mData->WaitingAnImage )
    {
        mData->WaitingAnImage = true;
        ServiceManager::Instance( ).GetAutomationServer( )->AddVideoSourceListener( mData->VideoSourceId, mData, false );
    }
}

// Save image into the specified file
void VideoSnapshotDialog::on_SaveAction_triggered( )
{
    static const char* STR_SNAPSHOT    = "Snapshot";
    static const char* STR_LAST_FOLDER = "LastFolder";

    if ( mData->SnapshotImage )
    {
        shared_ptr<IApplicationSettingsService> settingsService = ServiceManager::Instance( ).GetSettingsService( );
        shared_ptr<IImageFileService> fileService = ServiceManager::Instance( ).GetImageFileService( );

        QString lastFolder = settingsService->GetValue( STR_LAST_FOLDER, STR_SNAPSHOT ).toString( );
        QString fileName   = fileService->PromptFileSave( "Save image to file", lastFolder, this );

        if ( !fileName.isEmpty( ) )
        {
            XErrorCode errorCode = fileService->Save( fileName, mData->SnapshotImage );

            if ( errorCode != SuccessCode )
            {
                UITools::ShowErrorMessage( QString( "Failed saving the file:<br><br><b>%0</b><br><br>Error: <b>%1</b>" ).
                                           arg( fileName ).
                                           arg( XError::Description( errorCode ).c_str( ) ), this );
            }

            settingsService->SetValue( STR_LAST_FOLDER, STR_SNAPSHOT, QFileInfo( fileName ).absolutePath( ) );
        }
    }
}

// Set image into system's clipboard
void VideoSnapshotDialog::on_SetToClipboardAction_triggered( )
{
    if ( mData->SnapshotImage )
    {
        shared_ptr<QImage> qimage;

        XImageInterface::XtoQimage( mData->SnapshotImage, qimage );

        if ( qimage )
        {
            QClipboard* clipboard = QApplication::clipboard( );

            if ( clipboard != nullptr )
            {
                clipboard->setImage( *qimage );
            }
        }
    }
}

// Switch color spaces
void VideoSnapshotDialog::on_SwitchToRgbAction_triggered( )
{
    mData->ColorSpaceBox->setCurrentIndex( 0 );
}
void VideoSnapshotDialog::on_SwitchToNrgbAction_triggered( )
{
    mData->ColorSpaceBox->setCurrentIndex( 1 );
}
void VideoSnapshotDialog::on_SwitchToHslAction_triggered( )
{
    mData->ColorSpaceBox->setCurrentIndex( 2 );
}
void VideoSnapshotDialog::on_SwitchToHsvAction_triggered( )
{
    mData->ColorSpaceBox->setCurrentIndex( 3 );
}

// Switch histogram channels
void VideoSnapshotDialog::on_SwitchRedHistogramAction_triggered( )
{
    ui->histogramFrame->SetHistogramChannel( RedIndex );
}
void VideoSnapshotDialog::on_SwitchGreenHistogramAction_triggered( )
{
    ui->histogramFrame->SetHistogramChannel( GreenIndex );
}
void VideoSnapshotDialog::on_SwitchBlueHistogramAction_triggered( )
{
    ui->histogramFrame->SetHistogramChannel( BlueIndex );
}

namespace Private
{

// Create menu bar for the snapshot window
QMenuBar* VideoSnapshotDialogData::CreateMenuBar( )
{
    QMenuBar* menuBar    = new QMenuBar( Parent );
    QMenu*    menuFile   = new QMenu( "&File", menuBar );
    QMenu*    menuTools  = new QMenu( "&Tools", menuBar );

    // File menu
    menuBar->addAction( menuFile->menuAction( ) );

    ReloadAction = new QAction( "&Reload", menuFile );
    ReloadAction->setStatusTip( "Reload snapshot from the same video source" );
    ReloadAction->setIcon( QIcon( QPixmap( ":/images/icons/reload.png" ) ) );
    ReloadAction->setShortcut( QKeySequence( "F5" ) );

    SaveAction = new QAction( "&Save", menuFile );
    SaveAction->setStatusTip( "Save image to the specified file" );
    SaveAction->setIcon( QIcon( QPixmap( ":/images/icons/file_save.png" ) ) );
    SaveAction->setShortcut( QKeySequence( "Ctrl+S" ) );

    SetToClipboardAction = new QAction( "Copy to clipboard", menuFile );
    SetToClipboardAction->setStatusTip( "Set image into the system clipboard" );
    SetToClipboardAction->setIcon( QIcon( QPixmap( ":/images/icons/copy.png" ) ) );
    SetToClipboardAction->setShortcut( QKeySequence( "Ctrl+C" ) );

    CloseAction = new QAction( "&Close", menuFile );
    CloseAction->setStatusTip( "Close the video snapshot window" );
    CloseAction->setIcon( QIcon( QPixmap( ":/images/icons/file_close.png" ) ) );

    menuFile->addAction( ReloadAction );
    menuFile->addSeparator( );
    menuFile->addAction( SaveAction );
    menuFile->addAction( SetToClipboardAction );
    menuFile->addSeparator( );
    menuFile->addAction( CloseAction );

    SaveAction->setEnabled( false );
    SetToClipboardAction->setEnabled( false );

    // Tools menu
    menuBar->addAction( menuTools->menuAction( ) );

    SwitchToRgbAction = new QAction( "RGB", menuTools );
    SwitchToRgbAction->setShortcut( QKeySequence( "Ctrl+1" ) );

    SwitchToNrgbAction = new QAction( "nRGB", menuTools );
    SwitchToNrgbAction->setShortcut( QKeySequence( "Ctrl+2" ) );

    SwitchToHslAction = new QAction( "HSL", menuTools );
    SwitchToHslAction->setShortcut( QKeySequence( "Ctrl+3" ) );

    SwitchToHsvAction = new QAction( "HSV", menuTools );
    SwitchToHsvAction->setShortcut( QKeySequence( "Ctrl+4" ) );

    SwitchRedHistogramAction = new QAction( "Red", menuTools );
    SwitchRedHistogramAction->setShortcut( QKeySequence( "Ctrl+R" ) );

    SwitchGreenHistogramAction = new QAction( "Green", menuTools );
    SwitchGreenHistogramAction->setShortcut( QKeySequence( "Ctrl+G" ) );

    SwitchBlueHistogramAction = new QAction( "Blue", menuTools );
    SwitchBlueHistogramAction->setShortcut( QKeySequence( "Ctrl+B" ) );

    menuTools->addAction( SwitchToRgbAction );
    menuTools->addAction( SwitchToNrgbAction );
    menuTools->addAction( SwitchToHslAction );
    menuTools->addAction( SwitchToHsvAction );

    menuTools->addAction( SwitchRedHistogramAction );
    menuTools->addAction( SwitchGreenHistogramAction );
    menuTools->addAction( SwitchBlueHistogramAction );

    // we need only hot keys, so hide tools menu
    menuTools->menuAction( )->setVisible( false );

    return menuBar;

}

// Create status bar for the snapshot window
QStatusBar* VideoSnapshotDialogData::CreateStatusBar( )
{
    StatusBar = new QStatusBar( Parent );

    // image size/format widget
    ImageSizeLabel = new QLabel( StatusBar );
    ImageSizeLabel->setMinimumWidth( 170 );
    ImageSizeLabel->setFrameShape( QFrame::Panel );
    ImageSizeLabel->setFrameShadow( QFrame::Sunken );

    // image coordinates widget
    CoordinatesIconLabel = new QLabel( StatusBar );
    CoordinatesIconLabel->setPixmap( QPixmap::fromImage( QImage( ":/images/icons/coordinates.png" ) ) );
    CoordinatesIconLabel->setMinimumWidth( 20 );
    CoordinatesIconLabel->setMaximumWidth( 20 );
    CoordinatesIconLabel->setFrameShape( QFrame::NoFrame );

    CoordinatesLabel = new QLabel( StatusBar );
    CoordinatesLabel->setText( QString::null );
    CoordinatesLabel->setMinimumWidth( 70 );
    CoordinatesLabel->setFrameShape( QFrame::NoFrame );

    QFrame* coordinatesFrame = new QFrame( StatusBar );
    coordinatesFrame->setFrameShape( QFrame::Panel );
    coordinatesFrame->setFrameShadow( QFrame::Sunken );

    QHBoxLayout* hlayout = new QHBoxLayout( coordinatesFrame );
    hlayout->setMargin( 0 );
    hlayout->setSpacing( 0 );
    hlayout->addWidget( CoordinatesIconLabel );
    hlayout->addWidget( CoordinatesLabel );

    // pixel color widget
    ColorIconLabel = new QLabel( StatusBar );
    ColorIconLabel->setPixmap( QPixmap::fromImage( QImage( ":/images/icons/colors_gray.png" ) ) );
    ColorIconLabel->setMinimumWidth( 20 );
    ColorIconLabel->setMaximumWidth( 20 );
    ColorIconLabel->setFrameShape( QFrame::NoFrame );

    ColorLabel = new QLabel( StatusBar );
    ColorLabel->setText( QString::null );
    ColorLabel->setMinimumWidth( 120 );
    ColorLabel->setFrameShape( QFrame::NoFrame );

    ColorSpaceBox = new QComboBox( StatusBar );
    ColorSpaceBox->setFrame( QFrame::NoFrame );
    ColorSpaceBox->setMinimumWidth( 60 );
    ColorSpaceBox->setMaximumWidth( 60 );
    ColorSpaceBox->setToolTip( "Use Ctrl+<1 to 4> to switch" );
    ColorSpaceBox->addItem( "RGB" );
    ColorSpaceBox->addItem( "nRGB" );
    ColorSpaceBox->addItem( "HSL" );
    ColorSpaceBox->addItem( "HSV" );

    QFrame* pixelColorFrame = new QFrame( StatusBar );
    pixelColorFrame->setFrameShape( QFrame::Panel );
    pixelColorFrame->setFrameShadow( QFrame::Sunken );

    hlayout = new QHBoxLayout( pixelColorFrame );
    hlayout->setMargin( 0 );
    hlayout->setSpacing( 0 );
    hlayout->addWidget( ColorIconLabel );
    hlayout->addWidget( ColorLabel );
    hlayout->addWidget( ColorSpaceBox );

    // add all widgets to status bar
    StatusBar->addPermanentWidget( ImageSizeLabel );
    StatusBar->addPermanentWidget( coordinatesFrame );
    StatusBar->addPermanentWidget( pixelColorFrame );

    return StatusBar;
}

void VideoSnapshotDialogData::OnNewVideoFrame( uint32_t videoSourceId, const shared_ptr<const XImage>& image )
{
    XUNREFERENCED_PARAMETER( videoSourceId )

    QMutexLocker locker( &SyncMutex );

    if ( WaitingAnImage )
    {
        ServiceManager::Instance( ).GetAutomationServer( )->RemoveVideoSourceListener( VideoSourceId, this );
        WaitingAnImage = false;

        SnapshotImage = image->Clone( );

        emit Parent->SnapshotTaken( );
    }
}

void VideoSnapshotDialogData::OnErrorMessage( uint32_t videoSourceId, const string& errorMessage )
{
    XUNREFERENCED_PARAMETER( videoSourceId )
    XUNREFERENCED_PARAMETER( errorMessage )
}

void VideoSnapshotDialogData::ProcessSnapshot( )
{
    if ( SnapshotImage )
    {
        int32_t             imageWidth  = SnapshotImage->Width( );
        int32_t             imageHeight = SnapshotImage->Height( );
        XPixelFormat        imageFormat = SnapshotImage->Format( );
        shared_ptr<QImage>  displayImage;
        bool                isGray = ( ( imageFormat == XPixelFormatGrayscale8 ) ||
                                       ( imageFormat == XPixelFormatGrayscale16 ) );

        XImageInterface::XtoQimage( SnapshotImage, displayImage );

        Ui->imageLabel->setPixmap( QPixmap::fromImage( *displayImage ) );

        Ui->imageLabel->resize( imageWidth + 2, imageHeight + 2 );
        Ui->scrollAreaWidgetContents->resize( imageWidth + 6, imageHeight + 6  );

        // size & format
        ImageSizeLabel->setText( QString( "%0 x %1, %2" ).arg( imageWidth ).arg( imageHeight ).
                                 arg( XImage::PixelFormatName( imageFormat ).c_str( ) ) );

        // color icon
        ColorIconLabel->setPixmap( QPixmap::fromImage( QImage(
                                   ( isGray ) ? ":/images/icons/colors_gray.png" :
                                                ":/images/icons/colors.png" ) ) );
        ColorSpaceBox->setVisible( !isGray );

        // histogram
        if ( ( imageFormat == XPixelFormatGrayscale8 ) ||
             ( imageFormat == XPixelFormatRGB24 ) ||
             ( imageFormat == XPixelFormatRGBA32 ) )
        {
            Ui->histogramFrame->ShowHistogramForImage( SnapshotImage );
            Ui->histogramContainer->setVisible( true );
        }
        else
        {
            Ui->histogramContainer->setVisible( false );
        }

        SaveAction->setEnabled( true );
        SetToClipboardAction->setEnabled( true );

        // update pixel color label if mouse pointer is above image
        if ( LastMousePoint.x( ) != -1 )
        {
            ShowPixelColor( LastMousePoint.x( ), LastMousePoint.y( ) );
        }
    }
}

void VideoSnapshotDialogData::ShowPixelColor( int x, int y )
{
    QString formattedColor;

    if ( ( SnapshotImage ) && ( x >= 0 ) && ( y >= 0 ) && ( x < SnapshotImage->Width( ) ) && ( SnapshotImage->Height( ) ) )
    {
        XPixelFormat imageFormat = SnapshotImage->Format( );
        const XColor color       = SnapshotImage->GetPixelColor( x, y );


        if ( imageFormat == XPixelFormatGrayscale8 )
        {
            formattedColor = QString( "%0" ).arg( color.Red( ) );
        }
        else if ( ( imageFormat == XPixelFormatRGB24 ) || ( imageFormat == XPixelFormatRGBA32 ) )
        {
            switch ( ColorSpaceBox->currentIndex( ) )
            {
            case 0:
                formattedColor = QString( "%0 %1 %2" ).arg( color.Red( ) ).arg( color.Green( ) ).arg( color.Blue( ) );
                break;

            case 1:
                {
                    int rgbSum = color.Red( ) + color.Green( ) + color.Blue( );
                    formattedColor = QString( "%0 %1 %2" ).
                            arg( QString::number( (float) color.Red( )   / rgbSum, 'f', 2 ) ).
                            arg( QString::number( (float) color.Green( ) / rgbSum, 'f', 2 ) ).
                            arg( QString::number( (float) color.Blue( )  / rgbSum, 'f', 2 ) );
                }
                break;

            case 2:
                {
                    xargb rgb = color;
                    xhsl  hsl;

                    Rgb2Hsl( &rgb, &hsl );

                    formattedColor = QString( "%0 %1 %2" ).
                            arg( hsl.Hue ).
                            arg( QString::number( hsl.Saturation, 'f', 2 ) ).
                            arg( QString::number( hsl.Luminance, 'f', 2 ) );
                }
                break;

            case 3:
                {
                    xargb rgb = color;
                    xhsv  hsv;

                    Rgb2Hsv( &rgb, &hsv );

                    formattedColor = QString( "%0 %1 %2" ).
                            arg( hsv.Hue ).
                            arg( QString::number( hsv.Saturation, 'f', 2 ) ).
                            arg( QString::number( hsv.Value, 'f', 2 ) );
                }
                break;
            }
        }
    }

    ColorLabel->setText( formattedColor );
}

}
