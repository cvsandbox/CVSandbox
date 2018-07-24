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

/* Algorithm:
 * --------------------------------------
 * Every pixel in destination image is set to
 * average value from the corresponding 3x3
 * square window in the source image (or
 * any available neighbours for edge pixels).
 * --------------------------------------
 */

// forward declaration ----
static void Mean3x3_8bitPerPlane( const ximage* src, ximage* dst );
// ------------------------

// 3x3 mean filter
XErrorCode Mean3x3( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->width < 3 ) || ( src->height < 3 ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( ( dst->width != src->width ) ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( src->format == XPixelFormatGrayscale8 ) ||
              ( src->format == XPixelFormatRGB24 ) ||
              ( src->format == XPixelFormatRGBA32 ) )
    {
        Mean3x3_8bitPerPlane( src, dst );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// 3x3 mean filter for images with 8bpp per plane (8bpp grayscale and 24/32 color)
void Mean3x3_8bitPerPlane( const ximage* src, ximage* dst )
{
    int width     = src->width;
    int height    = src->height;
    int widthM1   = width - 1;
    int heightM1  = height - 1;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int pixelSize = ( src->format == XPixelFormatGrayscale8 ) ? 1 :
                  ( ( src->format == XPixelFormatRGB24 ) ? 3 : 4 );
    int y;

    // some pre-calculate values
    int srcStridePps  =  srcStride + pixelSize;
    int srcStrideMps  =  srcStride - pixelSize;
    int mSrcStridePps = -srcStride + pixelSize;
    int mSrcStrideMps = -srcStride - pixelSize;
    int mSrcStride    = -srcStride;
    int mPixelSize    = -pixelSize;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthM1, pixelSize, srcStride, dstStride, srcStridePps, srcStrideMps, mSrcStridePps, mSrcStrideMps, mSrcStride, mPixelSize )
    for ( y = 1; y < heightM1; y++ )
    {
        int x, i;

        for ( i = 0; i < pixelSize; i++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride + i;
            uint8_t* dstRow = dstPtr + y * dstStride + i;

            // set the first pixel of the row
            *dstRow = (uint8_t) ( (
                (uint16_t) srcRow[mSrcStride] + srcRow[0] + srcRow[srcStride] +
                           srcRow[mSrcStridePps] + srcRow[pixelSize] + srcRow[srcStridePps]
                        ) / 6 );

            srcRow += pixelSize;
            dstRow += pixelSize;

            // process the rest
            for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
            {
                *dstRow = (uint8_t) ( (
                    (uint16_t) srcRow[mSrcStrideMps] + srcRow[mSrcStride] + srcRow[mSrcStridePps] +
                               srcRow[mPixelSize] + srcRow[ 0] + srcRow[ pixelSize] +
                               srcRow[srcStrideMps] + srcRow[srcStride] + srcRow[srcStridePps]
                        ) / 9 );
            }

            // set the last pixel of the row
            *dstRow = (uint8_t) ( (
                (uint16_t) srcRow[mSrcStride] + srcRow[0] + srcRow[srcStride] +
                           srcRow[mSrcStrideMps] + srcRow[mPixelSize] + srcRow[srcStrideMps]
                        ) / 6 );
        }
    }

    {
        int x, i;

        for ( i = 0; i < pixelSize; i++ )
        {
            // --- process the first row
            srcPtr  = src->data + i;
            dstPtr  = dst->data + i;

            // first pixel of the first row
            *dstPtr = (uint8_t) ( (
                (uint16_t) *srcPtr + srcPtr[pixelSize] + srcPtr[srcStride] + srcPtr[srcStridePps]
                        ) / 4 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the first row
            for ( x = 1; x < widthM1; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                *dstPtr = (uint8_t) ( (
                    (uint16_t) srcPtr[mPixelSize] + srcPtr[0] + srcPtr[ pixelSize] +
                               srcPtr[srcStrideMps] + srcPtr[srcStride] + srcPtr[srcStridePps]
                            ) / 6 );
            }

            // last pixel of the first row
            *dstPtr = (uint8_t) ( (
                (uint16_t) *srcPtr + srcPtr[mPixelSize] + srcPtr[srcStride] + srcPtr[srcStrideMps]
                        ) / 4 );

            // --- process the last row
            srcPtr  = src->data + srcStride * heightM1 + i;
            dstPtr  = dst->data + dstStride * heightM1 + i;

            // first pixel of the last row
            *dstPtr = (uint8_t) ( (
                (uint16_t) *srcPtr + srcPtr[pixelSize] + srcPtr[mSrcStride] + srcPtr[mSrcStridePps]
                        ) / 4 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the last row
            for ( x = 1; x < widthM1; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                *dstPtr = (uint8_t) ( (
                    (uint16_t) srcPtr[mPixelSize] + srcPtr[0] + srcPtr[ pixelSize] +
                               srcPtr[mSrcStrideMps] + srcPtr[mSrcStride] + srcPtr[mSrcStridePps]
                            ) / 6 );
            }

            // last pixel of the last row
            *dstPtr = (uint8_t) ( (
                (uint16_t) *srcPtr + srcPtr[mPixelSize] + srcPtr[mSrcStride] + srcPtr[mSrcStrideMps]
                        ) / 4 );
        }
    }
}
