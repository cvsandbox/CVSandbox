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

#include "HistogramInfoFrame.hpp"
#include "ui_HistogramInfoFrame.h"

#include "HistogramWidget.hpp"
#include <ximaging.h>

using namespace std;
using namespace CVSandbox;

namespace Private
{
    class HistogramInfoFrameData
    {
    public:
        HistogramInfoFrameData( ) :
            Histogram( 0 ),
            RedHistogram( 0 ), GreenHistogram( 0 ), BlueHistogram( 0 ),
            PixelFormat( XPixelFormatUnknown )
        {

        }

        xhistogram* GetHistogram( int index )
        {
            xhistogram* histogram = 0;

            if ( ( PixelFormat == XPixelFormatRGB24 ) || ( PixelFormat == XPixelFormatRGBA32 ) )
            {
                switch ( index )
                {
                case 0:
                    histogram = RedHistogram;
                    break;

                case 1:
                    histogram = GreenHistogram;
                    break;

                case 2:
                    histogram = BlueHistogram;
                    break;

                default:
                    break;
                }
            }
            else if ( PixelFormat == XPixelFormatGrayscale8 )
            {
                histogram = GreenHistogram;
            }

            return histogram;
        }

    public:
        HistogramWidget* Histogram;
        xhistogram*      RedHistogram;
        xhistogram*      GreenHistogram;
        xhistogram*      BlueHistogram;
        XPixelFormat     PixelFormat;
    };
}

HistogramInfoFrame::HistogramInfoFrame( QWidget *parent ) :
    QFrame( parent ),
    ui( new Ui::HistogramInfoFrame ),
    mData( new Private::HistogramInfoFrameData )
{
    ui->setupUi( this );

    XHistogramCreate( 256, &mData->RedHistogram );
    XHistogramCreate( 256, &mData->GreenHistogram );
    XHistogramCreate( 256, &mData->BlueHistogram );

    mData->Histogram = new HistogramWidget( this );
    mData->Histogram->setMinimumSize( 258, 120 );
    mData->Histogram->setMaximumSize( 258, 120 );

    // add property editor to the form
    QVBoxLayout* vlayout = new QVBoxLayout( ui->histogramPlaceholderFrame );
    vlayout->setMargin( 0 );
    vlayout->addWidget( mData->Histogram );

    connect( mData->Histogram, SIGNAL( PositionChanged(int) ),
             this, SLOT( on_histogram_PositionChanged(int) ) );
    connect( mData->Histogram, SIGNAL( SelectionChanged(int,int) ),
             this, SLOT( on_histogram_SelectionChanged(int,int) ) );

    ClearHistogram( );
}

HistogramInfoFrame::~HistogramInfoFrame( )
{
    XHistogramFree( &mData->RedHistogram );
    XHistogramFree( &mData->GreenHistogram );
    XHistogramFree( &mData->BlueHistogram );

    delete ui;
    delete mData;
}

// Show histogram for the given image
void HistogramInfoFrame::ShowHistogramForImage( const shared_ptr<const XImage>& image )
{
    if ( ( mData->RedHistogram != 0 ) && ( mData->GreenHistogram != 0 )  && ( mData->BlueHistogram != 0 ) )
    {
        int currentIndex = qMax( 0, ui->channelCombo->currentIndex( ) );

        ui->channelCombo->clear( );
        ui->channelCombo->setEnabled( true );

        if ( image )
        {
            mData->PixelFormat = image->Format( );

            switch( mData->PixelFormat )
            {
            case XPixelFormatGrayscale8:

                // collect and display it histogram
                GetGrayscaleImageHistogram( image->ImageData( ), mData->GreenHistogram );

                ui->channelCombo->addItem( "Intensity" );

                ui->channelCombo->setCurrentIndex( 0 );
                break;

            case XPixelFormatRGB24:
            case XPixelFormatRGBA32:

                // collect and display it histogram
                GetColorImageHistograms( image->ImageData( ), mData->RedHistogram,
                                         mData->GreenHistogram, mData->BlueHistogram );

                ui->channelCombo->addItem( "Red" );
                ui->channelCombo->addItem( "Green" );
                ui->channelCombo->addItem( "Blue" );

                ui->channelCombo->setCurrentIndex( currentIndex );
                break;

            default:
                ClearHistogram( );
                ui->channelCombo->addItem( "Not available for the selected image" );
                break;
            }
        }
        else
        {
            ClearHistogram( );
        }
    }
}

// Clear histogram view
void HistogramInfoFrame::ClearHistogram( )
{
    mData->PixelFormat = XPixelFormatUnknown;
    mData->Histogram->SetHistogram( 0 );
    ui->channelCombo->clear( );
    ui->channelCombo->setEnabled( false );
    ClearHistogramInfoLabels( );
    ClearSelectionInfoLabels( );
}

// Set histogram channel to dispay
void HistogramInfoFrame::SetHistogramChannel( XRGBComponent channel )
{
    ui->channelCombo->setCurrentIndex( ( channel == RedIndex ) ? 0 : ( channel == GreenIndex ) ? 1 : 2 );
}

// Clear all histogram info labels
void HistogramInfoFrame::ClearHistogramInfoLabels( )
{
    ui->minLabel->setText( QString::null );
    ui->maxLabel->setText( QString::null );
    ui->meanLabel->setText( QString::null );
    ui->stdDevLabel->setText( QString::null );
}

// Clear all selection info labels
void HistogramInfoFrame::ClearSelectionInfoLabels( )
{
    ui->levelLabel->setText( QString::null );
    ui->countLabel->setText( QString::null );
    ui->percentileLabel->setText( QString::null );
}

// Color channel selection changed in the combo
void HistogramInfoFrame::on_channelCombo_currentIndexChanged( int index )
{
    xhistogram* histogram = 0;
    QColor      histogramColor;

    if ( ( mData->PixelFormat == XPixelFormatRGB24 ) || ( mData->PixelFormat == XPixelFormatRGBA32 ) )
    {
        switch ( index )
        {
        case 0:
            histogram = mData->RedHistogram;
            histogramColor = Qt::red;
            break;

        case 1:
            histogram = mData->GreenHistogram;
            histogramColor = Qt::green;
            break;

        case 2:
            histogram = mData->BlueHistogram;
            histogramColor = Qt::blue;
            break;

        default:
            break;
        }
    }
    else if ( mData->PixelFormat == XPixelFormatGrayscale8 )
    {
        histogram = mData->GreenHistogram;
        histogramColor = QColor( 96, 96, 96 );
    }

    if ( histogram != 0 )
    {
        mData->Histogram->SetHistogram( histogram );
        mData->Histogram->SetHistogramColor( histogramColor );

        ui->minLabel->setText( QString( "%0" ).arg( histogram->min ) );
        ui->maxLabel->setText( QString( "%0" ).arg( histogram->max ) );
        ui->meanLabel->setText( QString( "%0" ).arg( histogram->mean, 0, 'f', 2 ) );
        ui->stdDevLabel->setText( QString( "%0" ).arg( histogram->stddev, 0, 'f', 2 ) );
    }
    else
    {
        ClearHistogramInfoLabels( );
    }

    ClearSelectionInfoLabels( );
}

// Changing view type - logarithmic or not
void HistogramInfoFrame::on_logCheckBox_clicked( bool checked )
{
    mData->Histogram->SetLogView( checked );
}

// Cursor position has changed within the histogram control
void HistogramInfoFrame::on_histogram_PositionChanged( int pos )
{
    xhistogram* histogram = 0;

    if ( pos != -1 )
    {
        histogram =  mData->GetHistogram( ui->channelCombo->currentIndex( ) );
    }

    if ( histogram != 0 )
    {
        ui->levelLabel->setText( QString( "%0" ).arg( pos ) );
        ui->countLabel->setText( QString( "%0" ).arg( histogram->values[pos] ) );
        ui->percentileLabel->setText( QString( "%0" ).arg( (double) histogram->values[pos] * 100 / histogram->total, 0, 'f', 2 ) );
    }
    else
    {
        ClearSelectionInfoLabels( );
    }
}

// Selection change in the histogram widget
void HistogramInfoFrame::on_histogram_SelectionChanged( int start, int end )
{
    xhistogram* histogram = mData->GetHistogram( ui->channelCombo->currentIndex( ) );

    if ( histogram != 0 )
    {
        uint32_t totalSelected = 0;

        for ( int i = start; i <= end; i++ )
        {
            totalSelected += histogram->values[i];
        }

        ui->levelLabel->setText( QString( "%0 .. %1" ).arg( start ).arg( end ) );
        ui->countLabel->setText( QString( "%0" ).arg( totalSelected ) );
        ui->percentileLabel->setText( QString( "%0" ).arg( (double) totalSelected * 100 / histogram->total, 0, 'f', 2 ) );
    }
    else
    {
        ClearSelectionInfoLabels( );
    }
}
