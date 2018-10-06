/*
    Standard image processing plug-ins of Computer Vision Sandbox

    Copyright (C) 2011-2018, cvsandbox
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

#include <ximaging.h>
#include "ImageStatisticsPlugin.hpp"

#include <stdio.h>

// Supported pixel formats of input/output images
const XPixelFormat ImageStatisticsPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

namespace Private
{
    class ImageStatisticsPluginData
    {
    public:
        ImageStatisticsPluginData( ) :
            RangeToFind( 95.0f ),
            TotalRedInteresting( 0 ), TotalGreenInteresting( 0 ), TotalBlueInteresting( 0 ), TotalGrayInteresting( 0 ),
            RedHistogram( nullptr ), GreenHistogram( nullptr ), BlueHistogram( nullptr ), GrayHistogram( nullptr )
        {
            XHistogramCreate( 256, &RedHistogram );
            XHistogramCreate( 256, &GreenHistogram );
            XHistogramCreate( 256, &BlueHistogram );
            XHistogramCreate( 256, &GrayHistogram );
        }

        ~ImageStatisticsPluginData( )
        {
            XHistogramFree( &RedHistogram );
            XHistogramFree( &GreenHistogram );
            XHistogramFree( &BlueHistogram );
            XHistogramFree( &GrayHistogram );
        }

    public:
        float       RangeToFind;
        uint32_t    TotalRedInteresting;
        uint32_t    TotalGreenInteresting;
        uint32_t    TotalBlueInteresting;
        uint32_t    TotalGrayInteresting;
        xrange      RedInterestingRange;
        xrange      GreenInterestingRange;
        xrange      BlueInterestingRange;
        xrange      GrayInterestingRange;
        xhistogram* RedHistogram;
        xhistogram* GreenHistogram;
        xhistogram* BlueHistogram;
        xhistogram* GrayHistogram;
    };
}


ImageStatisticsPlugin::ImageStatisticsPlugin( ) :
    mData( new Private::ImageStatisticsPluginData( ) )
{
}

void ImageStatisticsPlugin::Dispose( )
{
    delete mData;
    delete this;
}

// Provide supported pixel formats
XErrorCode ImageStatisticsPlugin::GetSupportedPixelFormats( XPixelFormat* formats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedFormats, XARRAY_SIZE( supportedFormats ), formats, count );
}

// Find range which contains specified percentage of histogram value around its median
static uint32_t CalculateRange( const xhistogram* hist, float range, xrange* rangeFound )
{
    uint32_t pixelsInRange = hist->total;

    rangeFound->min = hist->min;
    rangeFound->max = hist->max;

    if ( range <= 100 )
    {
        uint32_t pixelsToRemove = static_cast<uint32_t>( ( 100.0f - range ) / 100 * hist->total );
        uint32_t halfToRemove   = pixelsToRemove / 2;
        uint32_t pixelsRemoved  = 0;

        // remove on the MIN side
        while ( pixelsRemoved < halfToRemove )
        {
            pixelsRemoved += hist->values[rangeFound->min];

            // really remove only if still under the limit - we don't want to cross it
            if ( pixelsRemoved < halfToRemove )
            {
                pixelsInRange -= hist->values[rangeFound->min];
                rangeFound->min++;
            }
        }

        // remove on the max side
        pixelsRemoved = 0;

        while ( pixelsRemoved < halfToRemove )
        {
            pixelsRemoved += hist->values[rangeFound->max];

            if ( pixelsRemoved < halfToRemove )
            {
                pixelsInRange -= hist->values[rangeFound->max];
                rangeFound->max--;
            }
        }
    }

    return pixelsInRange;
}

// Process the specified source image and return new as a result
XErrorCode ImageStatisticsPlugin::ProcessImage( const ximage* image )
{
    XErrorCode ret = SuccessCode;

    XHistogramClear( mData->RedHistogram );
    XHistogramClear( mData->GreenHistogram );
    XHistogramClear( mData->BlueHistogram );
    XHistogramClear( mData->GrayHistogram );

    mData->TotalRedInteresting = mData->TotalGreenInteresting = mData->TotalBlueInteresting = mData->TotalGrayInteresting = 0;
    mData->RedInterestingRange = mData->GreenInterestingRange = mData->BlueInterestingRange = mData->GrayInterestingRange = { 0, 0 };

    if ( image == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format == XPixelFormatRGB24 ) || ( image->format == XPixelFormatRGBA32 ) )
    {
        ret = GetColorImageHistograms( image, mData->RedHistogram, mData->GreenHistogram, mData->BlueHistogram );

        if ( ret == SuccessCode )
        {
            mData->TotalRedInteresting   = CalculateRange( mData->RedHistogram,   mData->RangeToFind, &mData->RedInterestingRange );
            mData->TotalGreenInteresting = CalculateRange( mData->GreenHistogram, mData->RangeToFind, &mData->GreenInterestingRange );
            mData->TotalBlueInteresting  = CalculateRange( mData->BlueHistogram,  mData->RangeToFind, &mData->BlueInterestingRange );
            }
    }
    else if ( image->format == XPixelFormatGrayscale8 )
    {

        ret = GetGrayscaleImageHistogram( image, mData->GrayHistogram );

        if ( ret == SuccessCode )
        {
            mData->TotalGrayInteresting = CalculateRange( mData->GrayHistogram, mData->RangeToFind, &mData->GrayInterestingRange );
        }
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// Get the specified property value of the plug-in
XErrorCode ImageStatisticsPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = mData->RangeToFind;
        break;

    case 1:
    case 2:
    case 3:
    case 4:
        {
            xhistogram* hist = ( ( id == 1 ) ? mData->RedHistogram :
                               ( ( id == 2 ) ? mData->GreenHistogram :
                               ( ( id == 3 ) ? mData->BlueHistogram : mData->GrayHistogram ) ) );

            xarray* array = nullptr;

            ret = XArrayAllocate( &array, XVT_U4, 256 );

            if ( ret == SuccessCode )
            {
                xvariant v;

                v.type = XVT_U4;

                for ( uint32_t i = 0; i < 256; i++ )
                {
                    v.value.uiVal = hist->values[i];
                    XArraySet( array, i, &v );
                }

                value->type = XVT_U4 | XVT_Array;
                value->value.arrayVal = array;
            }
        }

        break;

    // Red channel statistics

    case 5:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->RedHistogram->min );
        value->value.rangeVal.max = static_cast<int32_t>( mData->RedHistogram->max );
        break;

    case 6:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->RedHistogram->minEx0 );
        value->value.rangeVal.max = static_cast<int32_t>( mData->RedHistogram->max );
        break;

    case 7:
        value->type = XVT_Range;
        value->value.rangeVal = mData->RedInterestingRange;
        break;

    case 8:
        value->type = XVT_R4;
        value->value.fVal = mData->RedHistogram->mean;
        break;

    case 9:
        value->type = XVT_R4;
        value->value.fVal = mData->RedHistogram->stddev;
        break;

    case 10:
        value->type = XVT_R4;
        value->value.fVal = mData->RedHistogram->meanEx0;
        break;

    case 11:
        value->type = XVT_R4;
        value->value.fVal = mData->RedHistogram->stddevEx0;
        break;

    case 12:
        value->type = XVT_U4;
        value->value.uiVal = mData->RedHistogram->totalEx0;
        break;

    case 13:
        value->type = XVT_U4;
        value->value.uiVal = mData->TotalRedInteresting;
        break;

    // Green channel statistics

    case 14:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->GreenHistogram->min );
        value->value.rangeVal.max = static_cast<int32_t>( mData->GreenHistogram->max );
        break;

    case 15:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->GreenHistogram->minEx0 );
        value->value.rangeVal.max = static_cast<int32_t>( mData->GreenHistogram->max );
        break;

    case 16:
        value->type = XVT_Range;
        value->value.rangeVal = mData->GreenInterestingRange;
        break;

    case 17:
        value->type = XVT_R4;
        value->value.fVal = mData->GreenHistogram->mean;
        break;

    case 18:
        value->type = XVT_R4;
        value->value.fVal = mData->GreenHistogram->stddev;
        break;

    case 19:
        value->type = XVT_R4;
        value->value.fVal = mData->GreenHistogram->meanEx0;
        break;

    case 20:
        value->type = XVT_R4;
        value->value.fVal = mData->GreenHistogram->stddevEx0;
        break;

    case 21:
        value->type = XVT_U4;
        value->value.uiVal = mData->GreenHistogram->totalEx0;
        break;

    case 22:
        value->type = XVT_U4;
        value->value.uiVal = mData->TotalGreenInteresting;
        break;

    // Blue channel statistics

    case 23:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->BlueHistogram->min );
        value->value.rangeVal.max = static_cast<int32_t>( mData->BlueHistogram->max );
        break;

    case 24:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->BlueHistogram->minEx0 );
        value->value.rangeVal.max = static_cast<int32_t>( mData->BlueHistogram->max );
        break;

    case 25:
        value->type = XVT_Range;
        value->value.rangeVal = mData->BlueInterestingRange;
        break;

    case 26:
        value->type = XVT_R4;
        value->value.fVal = mData->BlueHistogram->mean;
        break;

    case 27:
        value->type = XVT_R4;
        value->value.fVal = mData->BlueHistogram->stddev;
        break;

    case 28:
        value->type = XVT_R4;
        value->value.fVal = mData->BlueHistogram->meanEx0;
        break;

    case 29:
        value->type = XVT_R4;
        value->value.fVal = mData->BlueHistogram->stddevEx0;
        break;

    case 30:
        value->type = XVT_U4;
        value->value.uiVal = mData->BlueHistogram->totalEx0;
        break;

    case 31:
        value->type = XVT_U4;
        value->value.uiVal = mData->TotalBlueInteresting;
        break;

    // Intensity statistics

    case 32:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->GrayHistogram->min );
        value->value.rangeVal.max = static_cast<int32_t>( mData->GrayHistogram->max );
        break;

    case 33:
        value->type = XVT_Range;
        value->value.rangeVal.min = static_cast<int32_t>( mData->GrayHistogram->minEx0 );
        value->value.rangeVal.max = static_cast<int32_t>( mData->GrayHistogram->max );
        break;

    case 34:
        value->type = XVT_Range;
        value->value.rangeVal = mData->GrayInterestingRange;
        break;

    case 35:
        value->type = XVT_R4;
        value->value.fVal = mData->GrayHistogram->mean;
        break;

    case 36:
        value->type = XVT_R4;
        value->value.fVal = mData->GrayHistogram->stddev;
        break;

    case 37:
        value->type = XVT_R4;
        value->value.fVal = mData->GrayHistogram->meanEx0;
        break;

    case 38:
        value->type = XVT_R4;
        value->value.fVal = mData->GrayHistogram->stddevEx0;
        break;

    case 39:
        value->type = XVT_U4;
        value->value.uiVal = mData->GrayHistogram->totalEx0;
        break;

    case 40:
        value->type = XVT_U4;
        value->value.uiVal = mData->TotalGrayInteresting;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode ImageStatisticsPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    if ( id == 0 )
    {
        xvariant convertedValue;
        XVariantInit( &convertedValue );

        ret = XVariantChangeType( value, &convertedValue, XVT_R4 );

        if ( ret == SuccessCode )
        {
            mData->RangeToFind = XINRANGE( convertedValue.value.fVal, 5.0f, 100.0f );
        }

        XVariantClear( &convertedValue );
    }
    else if ( ( id >= 1 ) && ( id < 41 ) )
    {
        ret = ErrorReadOnlyProperty;
    }
    else
    {
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Get individual values of histograms
XErrorCode ImageStatisticsPlugin::GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
{
    XErrorCode  ret  = SuccessCode;
    xhistogram* hist = nullptr;

    switch ( id )
    {
    case 1:
        hist = mData->RedHistogram;
        break;
    case 2:
        hist = mData->GreenHistogram;
        break;
    case 3:
        hist = mData->BlueHistogram;
        break;
    case 4:
        hist = mData->GrayHistogram;
        break;
    default:
        ret = ( ( id >= 0 ) && ( id < 41 ) ) ? ErrorNotIndexedProperty : ErrorInvalidProperty;
        break;
    }

    if ( ret == SuccessCode )
    {
        if ( index > 255 )
        {
            ret = ErrorIndexOutOfBounds;
        }
        else
        {
            value->type = XVT_U4;
            value->value.uiVal = hist->values[index];
        }
    }

    return ret;
}
