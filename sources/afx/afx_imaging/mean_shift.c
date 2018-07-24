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

#include "ximaging.h"

// forward declaration ----
static void MeanShift8bpp( const ximage* src, ximage* dst, int radius, int colorDistance );
static void MeanShift24bpp( const ximage* src, ximage* dst, int radius, int colorDistance );
// ------------------------

// Calculates mean value of window with size radius * 2 + 1. However, it takes only certain pixels
// for averaging - those, which color is withing the specified distance from the color of the
// window's ceter pixel.
XErrorCode MeanShift( const ximage* src, ximage* dst, uint16_t radius, uint16_t colorDistance )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( dst->width  != src->width  ) || ( dst->height != src->height ) || ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( radius < 1 ) || ( radius > 20 ) )
    {
        // don't allow too big radius
        ret = ErrorArgumentOutOfRange;
    }
    else if ( ( src->width / 2 < radius ) || ( src->height / 2 < radius ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( src->format == XPixelFormatGrayscale8 )
    {
        MeanShift8bpp( src, dst, radius, colorDistance );
    }
    else if ( ( src->format == XPixelFormatRGB24 ) || ( src->format == XPixelFormatRGBA32 ) )
    {
        MeanShift24bpp( src, dst, radius, colorDistance );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// Mean shift version for 8bpp grayscale images
void MeanShift8bpp( const ximage* src, ximage* dst, int radius, int colorDistance )
{
    int width      = src->width;
    int height     = src->height;
    int srcStride  = src->stride;
    int dstStride  = dst->stride;
    int widthTail  = XMAX( radius, width - radius );
    int heightTail = XMAX( radius, height - radius );
    int y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthTail, heightTail, width, height, srcStride, dstStride, radius, colorDistance )
    for ( y = 0; y < height; y++ )
    {
        int      windowStartY = ( y < radius ) ? -y : -radius;
        int      windowEndY   = ( y >= heightTail ) ? ( radius - ( y - heightTail + 1 ) ) : radius;
        int      windowEndX;
        uint8_t* srcRow       = srcPtr + y * srcStride;
        uint8_t* dstRow       = dstPtr + y * dstStride;
        uint8_t* srcBase      = srcPtr + ( y + windowStartY ) * srcStride;
        uint8_t* srcTemp;
        int      x, i, j;
        int      sum, count, value, diff;

        // left edge pixels
        for ( x = 0; x < radius; x++, srcRow++, dstRow++, srcBase++ )
        {
            sum     = 0;
            count   = 0;
            value   = *srcRow;
            srcTemp = srcBase;

            for ( i = windowStartY; i <= windowEndY; i++, srcTemp += srcStride )
            {
                for ( j = -x; j <= radius; j++ )
                {
                    diff = value - srcTemp[j];
                    if ( diff < 0 ) { diff = -diff; }

                    if ( diff <= colorDistance )
                    {
                        sum += srcTemp[j];
                        count++;
                    }
                }
            }

            *dstRow = (uint8_t) ( sum / count );
        }

        // pixels which have the X window fitting into the image
        for ( x = radius; x < widthTail; x++, srcRow++, dstRow++, srcBase++ )
        {
            sum     = 0;
            count   = 0;
            value   = *srcRow;
            srcTemp = srcBase;

            for ( i = windowStartY; i <= windowEndY; i++, srcTemp += srcStride )
            {
                for ( j = -radius; j <= radius; j++ )
                {
                    diff = value - srcTemp[j];
                    if ( diff < 0 ) { diff = -diff; }

                    if ( diff <= colorDistance )
                    {
                        sum += srcTemp[j];
                        count++;
                    }
                }
            }

            *dstRow = (uint8_t) ( sum / count );
        }

        // right edge pixels
        for ( x = widthTail; x < width; x++, srcRow++, dstRow++, srcBase++ )
        {
            sum        = 0;
            count      = 0;
            value      = *srcRow;
            srcTemp    = srcBase;
            windowEndX = radius - ( x - widthTail + 1 );

            for ( i = windowStartY; i <= windowEndY; i++, srcTemp += srcStride )
            {
                for ( j = -radius; j <= windowEndX; j++ )
                {
                    diff = value - srcTemp[j];
                    if ( diff < 0 ) { diff = -diff; }

                    if ( diff <= colorDistance )
                    {
                        sum += srcTemp[j];
                        count++;
                    }
                }
            }

            *dstRow = (uint8_t) ( sum / count );
        }
    }
}

// Mean shift version for 24/32bpp color images
void MeanShift24bpp( const ximage* src, ximage* dst, int radius, int colorDistance )
{
    int width          = src->width;
    int height         = src->height;
    int srcStride      = src->stride;
    int dstStride      = dst->stride;
    int widthTail      = XMAX( radius, width - radius );
    int heightTail     = XMAX( radius, height - radius );
    int pixelSize      = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int colorDistance2 = colorDistance * colorDistance;
    int y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthTail, heightTail, width, height, srcStride, dstStride, radius, colorDistance2 )
    for ( y = 0; y < height; y++ )
    {
        int      windowStartY = ( y < radius ) ? -y : -radius;
        int      windowEndY   = ( y >= heightTail ) ? ( radius - ( y - heightTail + 1 ) ) : radius;
        int      windowEndX;
        uint8_t* srcRow       = srcPtr + y * srcStride;
        uint8_t* dstRow       = dstPtr + y * dstStride;
        uint8_t* srcBase      = srcPtr + ( y + windowStartY ) * srcStride;
        uint8_t* srcTemp;
        uint8_t* srcTempRow;
        int      x, i, j;
        int      sumR, sumG, sumB, count;
        int      valueR, valueG, valueB, diffR, diffG, diffB;

        // left edge pixels
        for ( x = 0; x < radius; x++, srcRow += pixelSize, dstRow += pixelSize, srcBase += pixelSize )
        {
            sumR = sumG = sumB = 0;
            count   = 0;
            valueR  = srcRow[RedIndex];
            valueG  = srcRow[GreenIndex];
            valueB  = srcRow[BlueIndex];
            srcTemp = srcBase;

            for ( i = windowStartY; i <= windowEndY; i++, srcTemp += srcStride )
            {
                srcTempRow = &srcTemp[-x * pixelSize];

                for ( j = -x; j <= radius; j++, srcTempRow += pixelSize )
                {
                    diffR = valueR - srcTempRow[RedIndex];
                    diffG = valueG - srcTempRow[GreenIndex];
                    diffB = valueB - srcTempRow[BlueIndex];

                    if ( diffR * diffR + diffG * diffG + diffB * diffB <= colorDistance2 )
                    {
                        sumR += srcTempRow[RedIndex];
                        sumG += srcTempRow[GreenIndex];
                        sumB += srcTempRow[BlueIndex];
                        count++;
                    }
                }
            }

            dstRow[RedIndex]   = (uint8_t) ( sumR / count );
            dstRow[GreenIndex] = (uint8_t) ( sumG / count );
            dstRow[BlueIndex]  = (uint8_t) ( sumB / count );
        }

        // pixels which have the X window fitting into the image
        for ( x = radius; x < widthTail; x++, srcRow += pixelSize, dstRow += pixelSize, srcBase += pixelSize )
        {
            sumR = sumG = sumB = 0;
            count   = 0;
            valueR  = srcRow[RedIndex];
            valueG  = srcRow[GreenIndex];
            valueB  = srcRow[BlueIndex];
            srcTemp = srcBase;

            for ( i = windowStartY; i <= windowEndY; i++, srcTemp += srcStride )
            {
                srcTempRow = &srcTemp[-radius * pixelSize];

                for ( j = -radius; j <= radius; j++, srcTempRow += pixelSize )
                {
                    diffR = valueR - srcTempRow[RedIndex];
                    diffG = valueG - srcTempRow[GreenIndex];
                    diffB = valueB - srcTempRow[BlueIndex];

                    if ( diffR * diffR + diffG * diffG + diffB * diffB <= colorDistance2 )
                    {
                        sumR += srcTempRow[RedIndex];
                        sumG += srcTempRow[GreenIndex];
                        sumB += srcTempRow[BlueIndex];
                        count++;
                    }
                }
            }

            dstRow[RedIndex]   = (uint8_t) ( sumR / count );
            dstRow[GreenIndex] = (uint8_t) ( sumG / count );
            dstRow[BlueIndex]  = (uint8_t) ( sumB / count );
        }

        // right edge pixels
        for ( x = widthTail; x < width; x++, srcRow += pixelSize, dstRow += pixelSize, srcBase += pixelSize  )
        {
            sumR = sumG = sumB = 0;
            count      = 0;
            valueR     = srcRow[RedIndex];
            valueG     = srcRow[GreenIndex];
            valueB     = srcRow[BlueIndex];
            srcTemp    = srcBase;
            windowEndX = radius - ( x - widthTail + 1 );

            for ( i = windowStartY; i <= windowEndY; i++, srcTemp += srcStride )
            {
                srcTempRow = &srcTemp[-radius * pixelSize];

                for ( j = -radius; j <= windowEndX; j++, srcTempRow += pixelSize )
                {
                    diffR = valueR - srcTempRow[RedIndex];
                    diffG = valueG - srcTempRow[GreenIndex];
                    diffB = valueB - srcTempRow[BlueIndex];

                    if ( diffR * diffR + diffG * diffG + diffB * diffB <= colorDistance2 )
                    {
                        sumR += srcTempRow[RedIndex];
                        sumG += srcTempRow[GreenIndex];
                        sumB += srcTempRow[BlueIndex];
                        count++;
                    }
                }
            }

            dstRow[RedIndex]   = (uint8_t) ( sumR / count );
            dstRow[GreenIndex] = (uint8_t) ( sumG / count );
            dstRow[BlueIndex]  = (uint8_t) ( sumB / count );
        }
    }
}
