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

#include "HistogramWidget.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <math.h>

namespace Private
{
    class HistogramWidgetData
    {
    public:
        HistogramWidgetData( ) :
            Histogram( 0 ),
            Color( 96, 96, 96 ),
            LogView( false ),
            HistogramIndexAtClick( -1 ),
            HistogramIndexAtMouse( -1 ),
            MouseTracking( false )
        {
        }

    public:
        xhistogram* Histogram;
        QColor      Color;
        bool        LogView;
        int         HistogramIndexAtClick;
        int         HistogramIndexAtMouse;
        bool        MouseTracking;
    };
}

HistogramWidget::HistogramWidget( QWidget* parent ) :
    QWidget( parent ),
    mData( new Private::HistogramWidgetData( ) )
{
    setAttribute( Qt::WA_OpaquePaintEvent, true );
    setAttribute( Qt::WA_NoSystemBackground, true );

    XHistogramCreate( 256, &mData->Histogram );

    this->setMouseTracking( true );
    this->setCursor( Qt::CrossCursor );
}

HistogramWidget::~HistogramWidget( )
{
    XHistogramFree( &mData->Histogram );
    delete mData;
}

// Set new histogram to display
void HistogramWidget::SetHistogram( const xhistogram* histogram )
{
    if ( histogram != 0 )
    {
        XHistogramCopy( histogram, mData->Histogram );
    }
    else
    {
        XHistogramClear( mData->Histogram );
    }

    update( );
}

// Set color to use to paint histogram
void HistogramWidget::SetHistogramColor( const QColor& color )
{
    mData->Color = color;
    update( );
}

// Set/Reset logarithmic view of the histogram
void HistogramWidget::SetLogView( bool logView )
{
    mData->LogView = logView;
    update( );
}

// Paint the control - the histogram
void HistogramWidget::paintEvent( QPaintEvent* )
{
    QPainter painter;
    QRect    myRect         = rect( );
    int      selectionStart = qMin( mData->HistogramIndexAtClick, mData->HistogramIndexAtMouse );
    int      selectionEnd   = qMax( mData->HistogramIndexAtClick, mData->HistogramIndexAtMouse );

    painter.begin( this );
    painter.setRenderHint( QPainter::Antialiasing, false );

    // initial empty rectanle
    painter.fillRect( 0, 0, myRect.width( ), myRect.height( ), QBrush( QColor( 248, 248, 248 ), Qt::SolidPattern ) );
    painter.setPen( Qt::black );
    painter.drawRect( 0, 0, myRect.width( ) - 1, myRect.height( ) - 1 );

    // draw the actual histogram
    if ( mData->Histogram->maxHits != 0 )
    {
        uint32_t* values    = mData->Histogram->values;
        double    maxHits   = mData->Histogram->maxHits;
        int       maxHeight = myRect.height( ) - 3;

        if ( mData->LogView )
        {
            maxHits = log10( maxHits );
        }

        if ( mData->MouseTracking )
        {
            painter.fillRect( selectionStart + 1, 1, selectionEnd - selectionStart + 1, myRect.height( ) - 2, QBrush( QColor( 80, 80, 80 ), Qt::SolidPattern ) );
        }

        for ( int i = 0; i < 256; i++ )
        {
            if ( values[i] != 0 )
            {
                double h = values[i];

                if ( mData->LogView )
                {
                    h = log10( h );
                }

                h *= maxHeight / maxHits;

                painter.setPen( mData->Color );

                if ( ( mData->MouseTracking ) && ( i >= selectionStart ) && ( i <= selectionEnd ) )
                {
                    painter.setPen( Qt::white );
                }

                painter.drawLine( i + 1, myRect.height( ) - 2, i + 1, myRect.height( ) - 2 - (int) h );
            }
        }
    }

    painter.end( );
}

// Mouse moved within the widget
void HistogramWidget::mouseMoveEvent( QMouseEvent* event )
{
    QRect myRect = rect( );
    int   pos = event->x( );

    if ( !mData->MouseTracking )
    {
        if ( ( pos >= 1 ) && ( pos < myRect.width( ) - 1 ) )
        {
            pos--;
        }
        else
        {
            pos = -1;
        }

        emit PositionChanged( pos );
    }
    else if ( pos != -1 )
    {
        pos--;

        if ( pos < 0 )
        {
            pos = 0;
        }
        else if ( pos > myRect.width( ) - 3 )
        {
            pos = myRect.width( ) - 3;
        }

        mData->HistogramIndexAtMouse = pos;
        update( );
        emit SelectionChanged( qMin( mData->HistogramIndexAtClick, mData->HistogramIndexAtMouse ),
                               qMax( mData->HistogramIndexAtClick, mData->HistogramIndexAtMouse ) );
    }
}

// Mouse button was pressed within the widget
void HistogramWidget::mousePressEvent( QMouseEvent* event )
{
    QRect myRect = rect( );
    int   pos = event->x( );

    if ( ( pos >= 1 ) && ( pos < myRect.width( ) - 1 ) )
    {
        pos--;
        mData->HistogramIndexAtClick = pos;
        mData->HistogramIndexAtMouse = pos;
        mData->MouseTracking = true;
        update( );
    }
}

// Mouse button was released
void HistogramWidget::mouseReleaseEvent( QMouseEvent* )
{
    if ( mData->MouseTracking )
    {
        mData->HistogramIndexAtClick = -1;
        mData->HistogramIndexAtMouse = -1;
        mData->MouseTracking = false;
        update( );
        emit PositionChanged( -1 );
    }
}

// Mouse left the widget
void HistogramWidget::leaveEvent( QEvent* )
{
    emit PositionChanged( -1 );
}
