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

#include "HuePickerWidget.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <math.h>

#include <xtypes.h>
#include <ximaging.h>

using namespace CVSandbox;
using namespace ::Private;

#define GAP (5)

namespace Private
{
    class HuePickerWidgetData
    {
    public:
        bool       RangeMode;
        uint16_t   Hue;
        XRange     HueRange;
        HuePointer MouseTracking;

    public:
        HuePickerWidgetData( ) :
            RangeMode( false ),
            Hue( 0 ), HueRange( 0, 359 ),
            MouseTracking( HuePointer::None )
        {

        }
    };
}

HuePickerWidget::HuePickerWidget( QWidget* parent) :
    QWidget( parent ),
    mData( new Private::HuePickerWidgetData( ) )
{
    this->setMouseTracking( true );

}

HuePickerWidget::~HuePickerWidget( )
{
    delete mData;
}

void HuePickerWidget::setHueValue( uint16_t hue )
{
    if ( ( mData->RangeMode ) || ( mData->Hue != hue ) )
    {
        mData->Hue = hue;
        mData->RangeMode = false;
        update( );
    }
}

uint16_t HuePickerWidget::hueValue( ) const
{
    return mData->Hue;
}

void HuePickerWidget::setHueRange( const XRange& hueRange )
{
    if ( ( !mData->RangeMode ) || ( mData->HueRange != hueRange ) )
    {
        mData->HueRange = hueRange;
        mData->RangeMode = true;
        update( );
    }
}

const XRange HuePickerWidget::hueRange( ) const
{
    return mData->HueRange;
}

// Paint the hue picker control
void HuePickerWidget::paintEvent( QPaintEvent* )
{
    QPainter  painter;
    QRect     ctrlRect = rect( );
    int       radius   = XMIN( ctrlRect.width( ), ctrlRect.height( ) ) / 2 - ( GAP + 1 );
    xhsl      hsl      = { 0, 1.0f, 0.5f };
    xargb     rgb      = { 0 };

    QRect     myRect( GAP, GAP, radius * 2, radius * 2 );
    QPen      whiteThickPen( Qt::white, 3 );
    QPen      blackThickPen( Qt::black, 3 );

    painter.begin( this );
    painter.setRenderHint( QPainter::Antialiasing, false );
    painter.setPen( Qt::NoPen );

    for ( int i = 0; i < 360; i++ )
    {
        hsl.Hue = static_cast<uint16_t>( i );
        hsl.Saturation = 1.0f;

        if ( mData->RangeMode )
        {
            int min = mData->HueRange.Min( );
            int max = mData->HueRange.Max( );

            if ( ( ( min < max ) && ( ( i < min ) || ( i > max ) ) ) ||
                 ( ( min > max ) &&   ( i < min ) && ( i > max )   ) )
            {
                hsl.Saturation = 0.0f;
            }
        }

        Hsl2Rgb( &hsl, &rgb );

        painter.setBrush( QBrush( QColor::fromRgb( rgb.components.r, rgb.components.g, rgb.components.b ) ) );
        painter.drawPie( myRect, i * 16,  16 );
    }

    QPen blackPen( Qt::black );
    blackPen.setWidth( 2 );

    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setPen( blackPen );
    painter.setBrush( Qt::NoBrush );
    painter.drawEllipse( myRect );
    painter.setRenderHint( QPainter::Antialiasing, false );

    // center point
    int cx = radius + GAP;
    int cy = radius + GAP;

    // draw 1st pointer - hue value or min value in range mode
    double angleRad = -( ( mData->RangeMode ) ? mData->HueRange.Min( ) : mData->Hue ) * XPI / 180;
    int    dx       = static_cast<int>( cos( angleRad ) * radius );
    int    dy       = static_cast<int>( sin( angleRad ) * radius );

    painter.setPen( whiteThickPen );
    painter.setBrush( Qt::white );
    painter.drawLine( cx, cy, cx + dx, cy + dy );
    painter.setPen( Qt::black );
    painter.drawEllipse( cx + dx - 5, cy + dy - 5, 10, 10 );

    // draw 2nd pointer - max value in range mode
    if ( mData->RangeMode )
    {
        angleRad = -mData->HueRange.Max( ) * XPI / 180;
        dx       = static_cast<int>( cos( angleRad ) * radius );
        dy       = static_cast<int>( sin( angleRad ) * radius );

        painter.setPen( blackThickPen );
        painter.setBrush( Qt::black );
        painter.drawLine( cx, cy, cx + dx, cy + dy );
        painter.setPen( Qt::black );
        painter.drawEllipse( cx + dx - 5, cy + dy - 5, 10, 10 );
    }

    painter.end( );
}

// Check if the specified XY coordinates belong to the hue circle
bool HuePickerWidget::IsInHueCircle( int x, int y, float* pAngle )
{
    QRect   ctrlRect = rect( );
    int     radius   = XMIN( ctrlRect.width( ), ctrlRect.height( ) ) / 2 - ( GAP + 1 );
    int     cx       = radius + GAP;
    int     cy       = radius + GAP;
    int     dx       = x - cx;
    int     dy       = y - cy;
    bool    ret      = ( dx * dx + dy * dy < radius * radius );

    if ( ret )
    {
        float angle = (float) ( atan2( -dy, dx ) * 180 / XPI );

        if ( angle < 0 )
        {
            angle += 360;
        }

        if ( pAngle != nullptr )
        {
            *pAngle = angle;
        }

        qDebug( "inside, x = %d, y = %d, a = %f", dx, -dy, angle );
    }

    return ret;
}

// Check if the specified XY coordinates belong to the specified hue pointer
bool HuePickerWidget::IsInHuePointer( int x, int y, HuePointer pointer )
{
    bool ret = false;

    if ( pointer != HuePointer::None )
    {
        QRect   ctrlRect = rect( );
        int     radius   = XMIN( ctrlRect.width( ), ctrlRect.height( ) ) / 2 - ( GAP + 1 );
        double  angleRad = - ( ( pointer == HuePointer::MinPointer ) ?
                               ( ( mData->RangeMode ) ? mData->HueRange.Min( ) : mData->Hue ) : mData->HueRange.Max( ) )
                                * XPI / 180;
        int     cx       = radius + GAP;
        int     cy       = radius + GAP;
        int     dx       = static_cast<int>( cos( angleRad ) * radius );
        int     dy       = static_cast<int>( sin( angleRad ) * radius );
        int     xdiff    = x - ( cx + dx );
        int     ydiff    = y - ( cy + dy );

        ret = ( xdiff * xdiff + ydiff * ydiff <= 25 );
    }

    return ret;
}

// Check if the XY coordinates belong to any ponter
bool HuePickerWidget::IsInHuePointer( int x, int y )
{
    return ( IsInHuePointer( x, y, HuePointer::MinPointer ) || IsInHuePointer( x, y, HuePointer::MaxPointer ) );
}

// Mouse movement - update hue value if tracking is On
void HuePickerWidget::mouseMoveEvent( QMouseEvent* event )
{
    int mx = event->x( );
    int my = event->y( );

    if ( mData->MouseTracking == HuePointer::None )
    {
        this->setCursor( ( IsInHuePointer( mx, my ) ) ? Qt::OpenHandCursor : Qt::ArrowCursor );
    }
    else
    {
        QRect   ctrlRect = rect( );
        int     radius   = XMIN( ctrlRect.width( ), ctrlRect.height( ) ) / 2 - ( GAP + 1 );
        int     cx       = radius + GAP;
        int     cy       = radius + GAP;
        int     dx       = mx - cx;
        int     dy       = cy - my;
        double  angle    = atan2( dy, dx ) * 180 / XPI;

        if ( angle < 0 )
        {
            angle = 360 + angle;
        }

        if ( !mData->RangeMode )
        {
            mData->Hue = static_cast<uint16_t>( angle );
            emit valueChanged( mData->Hue );
        }
        else
        {
            if ( mData->MouseTracking == HuePointer::MinPointer )
            {
                mData->HueRange.SetMin( angle );
            }
            else
            {
                mData->HueRange.SetMax( angle );
            }
            emit rangeChanged( mData->HueRange );
        }

        update( );

        this->setCursor( Qt::ClosedHandCursor );
    }
}

// Mouse pressed - start tracking if pressed above pointer
void HuePickerWidget::mousePressEvent( QMouseEvent* event )
{
    int   x = event->x( );
    int   y = event->y( );
    float a;

    if ( IsInHuePointer( x, y, HuePointer::MinPointer ) )
    {
        mData->MouseTracking = HuePointer::MinPointer;
    }
    else if ( IsInHuePointer( x, y, HuePointer::MaxPointer ) )
    {
        mData->MouseTracking = HuePointer::MaxPointer;
    }
    else if ( ( !mData->RangeMode ) && ( IsInHueCircle( x, y, &a ) ) )
    {
        mData->Hue = static_cast<uint16_t>( a );
        emit valueChanged( mData->Hue );
        update( );
    }

    if ( mData->MouseTracking != HuePointer::None )
    {
        this->setCursor( Qt::ClosedHandCursor );
    }
}

// Mouse released - stop tracking
void HuePickerWidget::mouseReleaseEvent( QMouseEvent* event )
{
    if ( mData->MouseTracking != HuePointer::None )
    {
        mData->MouseTracking = HuePointer::None;
        this->setCursor( ( IsInHuePointer( event->x( ), event->y( ) ) ) ? Qt::OpenHandCursor : Qt::ArrowCursor );
    }
}
