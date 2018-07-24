/*
    Imaging library of Computer Vision Sandbox

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

#include <string.h>
#include "ximaging.h"

// Calculate RGB histogram for 24/32 bpp color image
XErrorCode GetColorImageHistograms( const ximage* image, xhistogram* redHistogram, xhistogram* greenHistogram, xhistogram* blueHistogram )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) ||
         ( redHistogram == 0 ) || ( greenHistogram == 0 ) || ( blueHistogram == 0 ) ||
         ( redHistogram->values == 0 ) || ( greenHistogram->values == 0 ) || ( blueHistogram->values == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( redHistogram->length != 256 ) || ( greenHistogram->length != 256 ) || ( blueHistogram->length != 256 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( image->format != XPixelFormatRGB24 ) && ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width     = image->width;
        int height    = image->height;
        int stride    = image->stride;
        int pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;
        int x, y;

        uint8_t* ptr = image->data;
        uint8_t* rowPtr;

        uint32_t* redHistogramValues   = redHistogram->values;
        uint32_t* greenHistogramValues = greenHistogram->values;
        uint32_t* blueHistogramValues  = blueHistogram->values;

        memset( redHistogramValues,   0, sizeof( uint32_t ) * 256 );
        memset( greenHistogramValues, 0, sizeof( uint32_t ) * 256 );
        memset( blueHistogramValues,  0, sizeof( uint32_t ) * 256 );

        for ( y = 0; y < height; y++ )
        {
            rowPtr = ptr + stride * y;

            for ( x = 0; x < width; x++ )
            {
                redHistogramValues  [rowPtr[RedIndex  ]]++;
                greenHistogramValues[rowPtr[GreenIndex]]++;
                blueHistogramValues [rowPtr[BlueIndex ]]++;

                rowPtr += pixelSize;
            }
        }

        XHistogramUpdate( redHistogram );
        XHistogramUpdate( greenHistogram );
        XHistogramUpdate( blueHistogram );
    }

    return ret;
}

// Calculate intensity histogram for 8 bpp grayscale image
XErrorCode GetGrayscaleImageHistogram( const ximage* image, xhistogram* histogram )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( histogram == 0 ) || ( histogram->values == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( histogram->length != 256 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( image->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width  = image->width;
        int height = image->height;
        int stride = image->stride;
        int x, y;

        uint8_t* ptr    = image->data;
        uint8_t* rowPtr;

        uint32_t* histogramValues = histogram->values;

        memset( histogramValues, 0, sizeof( uint32_t ) * 256 );

        for ( y = 0; y < height; y++ )
        {
            rowPtr = ptr + stride * y;

            for ( x = 0; x < width; x++ )
            {
                histogramValues[*rowPtr]++;
                rowPtr += 1;
            }
        }

        XHistogramUpdate( histogram );
    }

    return ret;
}
