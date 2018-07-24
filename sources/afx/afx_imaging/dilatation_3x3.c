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
 * MAX value from the corresponding 3x3 square
 * window in the source image (or any available
 * neighbors for edge pixels).
 * --------------------------------------
 */

// forward declaration ----
static void Dilatation3x3_8bitPerPlane( const ximage* src, ximage* dst );
// ------------------------

// 3x3 dilatation filter with square structuring element
XErrorCode Dilatation3x3( const ximage* src, ximage* dst )
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
        Dilatation3x3_8bitPerPlane( src, dst );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// 3x3 dilatation filter for images with 8bpp per plane (8bpp grayscale and 24/32 color)
void Dilatation3x3_8bitPerPlane( const ximage* src, ximage* dst )
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
    int srcStridePps  = srcStride + pixelSize;
    int srcStrideMps  = srcStride - pixelSize;
    int mSrcStridePps = -srcStride + pixelSize;
    int mSrcStrideMps = -srcStride - pixelSize;
    int mSrcStride    = -srcStride;
    int mPixelSize    = -pixelSize;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride )
    for ( y = 1; y < heightM1; y++ )
    {
        uint8_t v1, v2, v3, v4, v5, v6, v7, v8, v9;
        uint8_t min1, min2, min3;
        int x, i;

        for ( i = 0; i < pixelSize; i++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride + i;
            uint8_t* dstRow = dstPtr + y * dstStride + i;

            // set the first pixel of the row
            v1 = srcRow[mSrcStride];
            v2 = srcRow[0];
            v3 = srcRow[srcStride];

            v4 = srcRow[mSrcStridePps];
            v5 = srcRow[pixelSize];
            v6 = srcRow[srcStridePps];

            min1 = XMAX3( v1, v2, v3 );
            min2 = XMAX3( v4, v5, v6 );

            *dstRow = XMAX( min1, min2 );

            srcRow += pixelSize;
            dstRow += pixelSize;

            // process the rest
            for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
            {
                v1 = srcRow[mSrcStrideMps];
                v2 = srcRow[mSrcStride];
                v3 = srcRow[mSrcStridePps];

                v4 = srcRow[mPixelSize];
                v5 = srcRow[ 0];
                v6 = srcRow[ pixelSize];

                v7 = srcRow[srcStrideMps];
                v8 = srcRow[srcStride];
                v9 = srcRow[srcStridePps];

                min1 = XMAX3( v1, v2, v3 );
                min2 = XMAX3( v4, v5, v6 );
                min3 = XMAX3( v7, v8, v9 );

                *dstRow = XMAX3( min1, min2, min3 );
            }

            // set the last pixel of the row
            v1 = srcRow[mSrcStride];
            v2 = srcRow[0];
            v3 = srcRow[srcStride];

            v4 = srcRow[mSrcStrideMps];
            v5 = srcRow[mPixelSize];
            v6 = srcRow[srcStrideMps];

            min1 = XMAX3( v1, v2, v3 );
            min2 = XMAX3( v4, v5, v6 );

            *dstRow = XMAX( min1, min2 );
        }
    }

    {
        uint8_t v1, v2, v3, v4, v5, v6;
        uint8_t min1, min2;
        int x, i;

        for ( i = 0; i < pixelSize; i++ )
        {
            // --- process the first row
            srcPtr  = src->data + i;
            dstPtr  = dst->data + i;

            // first pixel of the first row
            min1 = XMAX3( srcPtr[pixelSize], srcPtr[srcStride], srcPtr[srcStridePps] );
            *dstPtr = XMAX( *srcPtr, min1 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the first row
            for ( x = 1; x < widthM1; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                v1 = srcPtr[mPixelSize];
                v2 = srcPtr[ 0];
                v3 = srcPtr[ pixelSize];

                v4 = srcPtr[srcStrideMps];
                v5 = srcPtr[srcStride];
                v6 = srcPtr[srcStridePps];

                min1 = XMAX3( v1, v2, v3 );
                min2 = XMAX3( v4, v5, v6 );

                *dstPtr = XMAX( min1, min2 );
            }

            // last pixel of the first row
            min1 = XMAX3( srcPtr[mPixelSize], srcPtr[srcStride], srcPtr[srcStrideMps] );
            *dstPtr = XMAX( *srcPtr, min1 );

            // --- process the last row
            srcPtr  = src->data + srcStride * heightM1 + i;
            dstPtr  = dst->data + dstStride * heightM1 + i;

            // first pixel of the last row
            min1 = XMAX3( srcPtr[pixelSize], srcPtr[mSrcStride], srcPtr[mSrcStridePps] );
            *dstPtr = XMAX( *srcPtr, min1);

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the last row
            for ( x = 1; x < widthM1; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                v1 = srcPtr[mPixelSize];
                v2 = srcPtr[ 0];
                v3 = srcPtr[ pixelSize];

                v4 = srcPtr[mSrcStrideMps];
                v5 = srcPtr[mSrcStride];
                v6 = srcPtr[mSrcStridePps];

                min1 = XMAX3( v1, v2, v3 );
                min2 = XMAX3( v4, v5, v6 );

                *dstPtr = XMAX( min1, min2 );
            }

            // last pixel of the last row
            min1 = XMAX3( srcPtr[mPixelSize], srcPtr[mSrcStride], srcPtr[mSrcStrideMps] );
            *dstPtr = XMAX( *srcPtr, min1 );
        }
    }
}
