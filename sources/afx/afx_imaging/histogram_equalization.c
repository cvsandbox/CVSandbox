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

#include <stdlib.h>
#include "ximaging.h"

/* Algorithm:
 * --------------------------------------
 * This method usually increases the global contrast of many images,
 * especially when the usable data of the image is represented by close
 * contrast values. Through this adjustment, the intensities can be
 * better distributed on the histogram. This allows for areas of lower
 * local contrast to gain a higher contrast. Histogram equalization
 * accomplishes this by effectively spreading out the most frequent
 * intensity values.
 * http://en.wikipedia.org/wiki/Histogram_equalization
 * --------------------------------------
 */

// forward declaration ----
static void HistogramEqualization8( ximage* src );
static void HistogramEqualization24( ximage* src );
// ------------------------

// Histogram equalization image processing filter
XErrorCode HistogramEqualization( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->format )
        {
        case XPixelFormatGrayscale8:
            HistogramEqualization8( src );
            break;

        case XPixelFormatRGB24:
        case XPixelFormatRGBA32:
            HistogramEqualization24( src );
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }
    }

    return ret;
}

// Calculate map for pixel values re-mapping after histogram equalization
static void EqualizeHistogram( const uint32_t* histogram, uint32_t pixelsCount, uint8_t* map )
{
    int i;
    double coef = 255.0f / pixelsCount;

    // calculate the first value
    double prev = histogram[0] * coef;
    map[0] = (uint8_t) prev;

    // calcualte the rest of values
    for ( i = 1; i < 256; i++ )
    {
        prev += histogram[i] * coef;
        map[i] = (uint8_t) prev;
    }
}

// Histogram equalization image processing filter for 8bpp grayscale images
void HistogramEqualization8( ximage* src )
{
    int width  = src->width;
    int height = src->height;
    int offset = src->stride - width;
    int x, y;

    uint8_t* ptr = src->data;

    uint32_t* histogram = (uint32_t*) calloc( 256, sizeof( uint32_t ) );
    uint8_t*  map = (uint8_t*) calloc( 256, sizeof( uint8_t ) );

    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            histogram[*ptr]++;
            ptr++;
        }

        ptr += offset;
    }

    EqualizeHistogram( histogram, src->width * src->height, map );
    GrayscaleRemapping( src, map );

    free( histogram );
    free( map );
}

// Histogram equalization image processing filter for 24/32bpp grayscale images
void HistogramEqualization24( ximage* src )
{
    int width     = src->width;
    int height    = src->height;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int offset    = src->stride - width * pixelSize;
    int x, y;

    uint32_t pixelsCount = src->width * src->height;
    uint8_t* ptr = src->data;

    uint32_t* histogramR = (uint32_t*) calloc( 256, sizeof( uint32_t ) );
    uint32_t* histogramG = (uint32_t*) calloc( 256, sizeof( uint32_t ) );
    uint32_t* histogramB = (uint32_t*) calloc( 256, sizeof( uint32_t ) );
    uint8_t*  mapR = (uint8_t*) calloc( 256, sizeof( uint8_t ) );
    uint8_t*  mapG = (uint8_t*) calloc( 256, sizeof( uint8_t ) );
    uint8_t*  mapB = (uint8_t*) calloc( 256, sizeof( uint8_t ) );

    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            histogramR[ptr[RedIndex]]++;
            histogramG[ptr[GreenIndex]]++;
            histogramB[ptr[BlueIndex]]++;

            ptr += pixelSize;
        }

        ptr += offset;
    }

    EqualizeHistogram( histogramR, pixelsCount, mapR );
    EqualizeHistogram( histogramG, pixelsCount, mapG );
    EqualizeHistogram( histogramB, pixelsCount, mapB );

    ColorRemapping( src, mapR, mapG, mapB );

    free( histogramR );
    free( histogramG );
    free( histogramB );
    free( mapR );
    free( mapG );
    free( mapB );
}
