/*
    Imaging effects library of Computer Vision Sandbox

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

#include <memory.h>
#include "ximaging.h"

// Algorithm:
// --------------------------------------
// Processing source image the filter changes each pixels' value
// to the value of pixel with the most frequent intensity within window of the
// specified size. Going through the window the filters finds which intensity of
// pixels is the most frequent. Then it updates value of the pixel in the center
// of the window to the value with the most frequent intensity.
// --------------------------------------

// forward declaration ----
static void OilPainting8( const ximage* src, ximage* dst, uint8_t radius );
static void OilPainting24( const ximage* src, ximage* dst, uint8_t radius );
// ------------------------

// Applies simple oil painting effect to the specified image
XErrorCode OilPainting( const ximage* src, ximage* dst, uint8_t radius )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( dst->width  != src->width )  ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( radius < 1 ) || ( radius > 10 ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else if ( src->format == XPixelFormatGrayscale8 )
    {
        OilPainting8( src, dst, radius );
    }
    else if ( ( src->format == XPixelFormatRGB24 ) ||
              ( src->format == XPixelFormatRGBA32 ) )
    {
        OilPainting24( src, dst, radius );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// Applies simple oil painting effect to the specified 8 bpp grayscale image
void OilPainting8( const ximage* src, ximage* dst, uint8_t radius )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        uint16_t intensities[256];
        int      x, i, j, t, mostFrequentIntensity, maxPixels;

        for ( x = 0; x < width; x++ )
        {
            for ( i = 0; i < 256; i++ )
            {
                intensities[i] = 0;
            }

            // for each window's row
            for ( i = -radius; i <= radius; i++ )
            {
                t = y + i;

                // skip row
                if ( t < 0 )
                {
                    continue;
                }
                // break
                if ( t >= height )
                {
                    break;
                }

                // source image row aligned to current kernel row
                srcRow2 = srcRow + i * srcStride - radius;

                // for each window's column
                for ( j = -radius; j <= radius; j++, srcRow2++ )
                {
                    t = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( t < width )
                    {
                        intensities[*srcRow2]++;
                    }
                }
            }

            maxPixels = 0;
            mostFrequentIntensity = 0;

            for ( i = 0; i < 256; i++ )
            {
                if ( intensities[i] > maxPixels )
                {
                    maxPixels = intensities[i];
                    mostFrequentIntensity = i;
                }
            }

            *dstRow = (uint8_t) mostFrequentIntensity;

            srcRow++;
            dstRow++;
        }
    }
}

// Applies simple oil painting effect to the specified 24/32 bpp color image (alpha channel is ignored - just copied)
void OilPainting24( const ximage* src, ximage* dst, uint8_t radius )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, pixelSize )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        uint8_t  intensity;
        uint16_t intensities[256];
        uint32_t redFrequency[256];
        uint32_t greenFrequency[256];
        uint32_t blueFrequency[256];
        int      x, i, j, t, mostFrequentIntensity, maxPixels;

        for ( x = 0; x < width; x++ )
        {
            for ( i = 0; i < 256; i++ )
            {
                intensities[i]    = 0;
                redFrequency[i]   = 0;
                greenFrequency[i] = 0;
                blueFrequency[i]  = 0;
            }

            // for each window's row
            for ( i = -radius; i <= radius; i++ )
            {
                t = y + i;

                // skip row
                if ( t < 0 )
                {
                    continue;
                }
                // break
                if ( t >= height )
                {
                    break;
                }

                // source image row aligned to current kernel row
                srcRow2 = srcRow + i * srcStride - radius * pixelSize;

                // for each window's column
                for ( j = -radius; j <= radius; j++, srcRow2 += pixelSize )
                {
                    t = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( t < width )
                    {
                        intensity = (uint8_t) RGB_TO_GRAY( srcRow2[RedIndex],
                                                           srcRow2[GreenIndex],
                                                           srcRow2[BlueIndex] );
                        intensities[intensity]++;

                        redFrequency  [intensity] += srcRow2[RedIndex];
                        greenFrequency[intensity] += srcRow2[GreenIndex];
                        blueFrequency [intensity] += srcRow2[BlueIndex];
                    }
                }
            }

            maxPixels = 0;
            mostFrequentIntensity = 0;

            for ( i = 0; i < 256; i++ )
            {
                if ( intensities[i] > maxPixels )
                {
                    maxPixels = intensities[i];
                    mostFrequentIntensity = i;
                }
            }

            dstRow[RedIndex]   = (uint8_t) ( redFrequency[mostFrequentIntensity]   / maxPixels );
            dstRow[GreenIndex] = (uint8_t) ( greenFrequency[mostFrequentIntensity] / maxPixels );
            dstRow[BlueIndex]  = (uint8_t) ( blueFrequency[mostFrequentIntensity]  / maxPixels );

            if ( pixelSize == 4 )
            {
                dstRow[AlphaIndex] = srcRow[AlphaIndex];
            }

            srcRow += pixelSize;
            dstRow += pixelSize;
        }
    }
}
