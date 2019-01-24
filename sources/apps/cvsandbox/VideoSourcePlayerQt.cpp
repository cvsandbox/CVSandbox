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

#include "VideoSourcePlayerQt.hpp"

#include <QPixmap>
#include <QPainter>
#include <XImageInterface.hpp>
#include <QMutex>
#include <QMutexLocker>
#include <QMouseEvent>

using namespace std;
using namespace CVSandbox;
using namespace CVSandbox::Video;

// Default image (control) size
#define DEFAULT_IMAGE_WIDTH  (480)
#define DEFAULT_IMAGE_HEIGHT (360)

// Border width around image
#define BORDER_WIDTH (4)

// Message to display when waiting for camera
static const QString STR_WAITING_VIDEO_SOURCE  = QString::fromUtf8( "Waiting for video source ..." );

namespace Private
{
    // Class for private data
    class VideoSourcePlayerQtData
    {
    public:
        VideoSourcePlayerQtData( ) :
            syncMutex( ),
            imageWidth( DEFAULT_IMAGE_WIDTH ), imageHeight( DEFAULT_IMAGE_HEIGHT ),
            isWaitingForFirstFrame( false ), isFirstFrame( false ), isMouseHighlighted( false ), isHighlighted( false ),
            imageToDisplay( 0 ), errorMessage( )
        {
        }

        ~VideoSourcePlayerQtData( )
        {
        }

    public:
        QMutex  syncMutex;
        int     imageWidth;
        int     imageHeight;

        bool    isWaitingForFirstFrame;
        bool    isFirstFrame;
        bool    isMouseHighlighted;
        bool    isHighlighted;

        shared_ptr<QImage> imageToDisplay;
        string             errorMessage;
    };
}

VideoSourcePlayerQt::VideoSourcePlayerQt( QWidget* parent ) :
    QWidget( parent ),
    mData( new Private::VideoSourcePlayerQtData( ) )
{
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setAttribute( Qt::WA_NoSystemBackground, true );

    SetMyContolSize( );

    connect( this, SIGNAL( OnUpdateRequired( ) ), this, SLOT( update( ) ), Qt::QueuedConnection );
}

VideoSourcePlayerQt::~VideoSourcePlayerQt( )
{
    UnsubscribeFromVideoSourceEvents( );
    delete mData;
}

// Helper method to resize the control according to auto size policy
void VideoSourcePlayerQt::SetMyContolSize( )
{
    if ( AutoSize( ) )
    {
        QSize size = NonScaledSize( );

        PlayerWidget( )->setMinimumSize( size );
        PlayerWidget( )->setMaximumSize( size );
    }
    else
    {
        if ( RespectAspectRatio( ) )
        {
            ResizeToPreferredKeepingAspectRatio( );
        }
    }
}

// Provide the size, which control would take if it was in auto size mode (the side which show video image without scaling)
QSize VideoSourcePlayerQt::NonScaledSize( )
{
    return QSize( mData->imageWidth  + BORDER_WIDTH * 2,  mData->imageHeight + BORDER_WIDTH * 2 );
}

// Get UI widget of the the player
QWidget* VideoSourcePlayerQt::PlayerWidget( )
{
    return static_cast<QWidget*>( this );
}

// Set/clear player highlight
void VideoSourcePlayerQt::SetHighlighted( bool highlighted )
{
    mData->isHighlighted = highlighted;
    update( );
}

// On auto size mode changed
void VideoSourcePlayerQt::OnAutoSizeChanged( )
{
    SetMyContolSize( );
}

// On new video source is set
void VideoSourcePlayerQt::OnNewVideoSource( )
{
    // clear the last image if any
    {
        QMutexLocker locker( &mData->syncMutex );
        mData->imageToDisplay.reset( );
        mData->errorMessage.clear( );
        mData->imageWidth  = DEFAULT_IMAGE_WIDTH;
        mData->imageHeight = DEFAULT_IMAGE_HEIGHT;
    }

    SetMyContolSize( );

    mData->isFirstFrame           = false;
    mData->isWaitingForFirstFrame = true;
}

// On new video frame arrived
void VideoSourcePlayerQt::OnNewImageImpl( const shared_ptr<const XImage>& image )
{
    QMutexLocker locker( &mData->syncMutex );

    // copy video frame into Qt image
    XImageInterface::XtoQimage( image, mData->imageToDisplay );
    // reset error message
    mData->errorMessage.clear( );

    if ( mData->isWaitingForFirstFrame )
    {
        mData->imageWidth             = image->Width( );
        mData->imageHeight            = image->Height( );
        mData->isFirstFrame           = true;
        mData->isWaitingForFirstFrame = false;
    }
    else if ( ( mData->imageWidth  != image->Width( ) ) ||
              ( mData->imageHeight != image->Height( ) ) )
    {
        // a hack to resize control if video size has changed
        mData->imageWidth   = image->Width( );
        mData->imageHeight  = image->Height( );
        mData->isFirstFrame = true;
    }

    emit OnUpdateRequired( );
}

// On new error message from video source
void VideoSourcePlayerQt::OnErrorImpl( const std::string& errorMessage )
{
    QMutexLocker locker( &mData->syncMutex );

    mData->errorMessage = errorMessage;

    emit OnUpdateRequired( );
}

// Paint the control
void VideoSourcePlayerQt::paintEvent( QPaintEvent* )
{
    // resize control if required
    if ( mData->isFirstFrame )
    {
        SetMyContolSize( );
        mData->isFirstFrame = false;
    }

    QPainter painter;
    QRect    myRect = rect( );

    painter.begin( this );
    painter.setRenderHints( QPainter::Antialiasing            |
                            QPainter::TextAntialiasing        |
                            QPainter::SmoothPixmapTransform   |
                            QPainter::HighQualityAntialiasing |
                            QPainter::NonCosmeticDefaultPen, false );

    // draw image or fill with solid color if image is not yet available
    {
        QMutexLocker locker( &mData->syncMutex );

        if ( mData->imageToDisplay )
        {
            if ( ( mData->imageWidth  != myRect.width( ) - BORDER_WIDTH * 2  ) ||
                 ( mData->imageHeight != myRect.height( ) - BORDER_WIDTH * 2 ) )
            {
                painter.setRenderHints( QPainter::SmoothPixmapTransform, true );
            }

            painter.setCompositionMode( QPainter::CompositionMode_Source );
            painter.drawImage( QRect( BORDER_WIDTH, BORDER_WIDTH,
                                      myRect.width( ) - BORDER_WIDTH * 2,
                                      myRect.height( ) - BORDER_WIDTH * 2 ),
                                      *mData->imageToDisplay );
        }
        else
        {
            painter.fillRect( myRect, QColor( 64, 64, 64 ) );
        }

        // prepare for drawing text
        QFont currentFont = font( );

        painter.setPen( Qt::white );
        currentFont.setBold( true );
        setFont( currentFont );

        if ( !mData->errorMessage.empty( ) )
        {
            // show error message
            painter.fillRect( QRect( BORDER_WIDTH, BORDER_WIDTH, myRect.width( ) - BORDER_WIDTH * 2, 20 ), Qt::red );
            painter.drawText( QRect( BORDER_WIDTH * 2, BORDER_WIDTH * 2, myRect.width( ) - BORDER_WIDTH * 4, 18 ),
                              Qt::AlignLeft | Qt::AlignTop, QString::fromUtf8( mData->errorMessage.c_str( ) ) );
        }
        else
        {
            if ( !mData->imageToDisplay )
            {
                // still waiting for something: image or error
                painter.drawText( QRect( BORDER_WIDTH * 2, BORDER_WIDTH * 2, myRect.width( ) - BORDER_WIDTH * 4, 18 ),
                                  Qt::AlignLeft | Qt::AlignTop, STR_WAITING_VIDEO_SOURCE );
            }
        }
    }

    // draw a border around the image
    painter.setPen( Qt::black );
    for ( int i = 0; i < BORDER_WIDTH; i++ )
    {
        painter.drawRect( i, i, myRect.width( ) - 1 - i * 2, myRect.height( ) - 1 - i * 2 );
    }
    if ( ( mData->isHighlighted ) || ( mData->isMouseHighlighted ) )
    {
        painter.setPen( ( mData->isMouseHighlighted ) ? QColor::fromRgb( 0xA6, 0xE2, 0xFF ) :
                                                        QColor::fromRgb( 0x84, 0xFF, 0xA3 ) );
        for ( int i = 0; i < BORDER_WIDTH / 2; i++ )
        {
            painter.drawRect( i, i, myRect.width( ) - 1 - i * 2, myRect.height( ) - 1 - i * 2 );
        }
    }
    painter.end( );
}

// Mouse entered the control - highlight it
void VideoSourcePlayerQt::enterEvent( QEvent* event )
{
    mData->isMouseHighlighted = true;
    update( );
    QWidget::enterEvent( event );
}

// Mouse left the control - remove any highlighting it
void VideoSourcePlayerQt::leaveEvent( QEvent* event )
{
    mData->isMouseHighlighted = false;
    update( );
    QWidget::leaveEvent( event );

}
