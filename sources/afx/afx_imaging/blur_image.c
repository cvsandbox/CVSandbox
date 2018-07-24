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
 * Performs image blurring by applying convolution
 * with 5x5 kernel below:
 *  | 1  2  3  2  1 |
 *  | 2  4  5  4  2 |
 *  | 3  5  6  5  3 |
 *  | 2  4  5  4  2 |
 *  | 1  2  3  2  1 |
 * --------------------------------------
 */

// forward declaration ----
static void BlurImage_8bitPerPlane( const ximage* src, ximage* dst );
// ------------------------

// Blurs image using 5x5 kernel
XErrorCode BlurImage( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->width < 5 ) || ( src->height < 5 ) )
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
        BlurImage_8bitPerPlane( src, dst );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// Blurs image using 5x5 kernel - works with 8 bit per plane images
void BlurImage_8bitPerPlane( const ximage* src, ximage* dst )
{
    int width     = src->width;
    int height    = src->height;
    int widthM2   = width - 2;
    int heightM2  = height - 2;
    int heightM1  = height - 1;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int pixelSize = ( src->format == XPixelFormatGrayscale8 ) ? 1 :
                  ( ( src->format == XPixelFormatRGB24 ) ? 3 : 4 );
    int y;

    // some pre-calculate values
    int srcStridePps    = srcStride + pixelSize;
    int srcStrideMps    = srcStride - pixelSize;
    int srcStrideP2ps   = srcStride + pixelSize * 2;
    int srcStrideM2ps   = srcStride - pixelSize * 2;

    int mSrcStridePps   = -srcStride + pixelSize;
    int mSrcStrideMps   = -srcStride - pixelSize;
    int mSrcStrideP2ps  = -srcStride + pixelSize * 2;
    int mSrcStrideM2ps  = -srcStride - pixelSize * 2;

    int mSrcStride      = -srcStride;
    int srcStride2      =  srcStride * 2;
    int mSrcStride2     = -srcStride * 2;

    int pixelSize2      =  pixelSize * 2;
    int mPixelSize      = -pixelSize;
    int mPixelSize2     = -pixelSize * 2;

    int srcStride2P2ps  =  srcStride * 2 + pixelSize * 2;
    int srcStride2M2ps  =  srcStride * 2 - pixelSize * 2;
    int mSrcStride2P2ps = -srcStride * 2 + pixelSize * 2;
    int mSrcStride2M2ps = -srcStride * 2 - pixelSize * 2;

    int srcStride2Pps   =  srcStride * 2 + pixelSize;
    int srcStride2Mps   =  srcStride * 2 - pixelSize;
    int mSrcStride2Pps  = -srcStride * 2 + pixelSize;
    int mSrcStride2Mps  = -srcStride * 2 - pixelSize;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, pixelSize )
    for ( y = 2; y < heightM2; y++ )
    {
        int x, i;

        for ( i = 0; i < pixelSize; i++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride + i;
            uint8_t* dstRow = dstPtr + y * dstStride + i;

            // set the first pixel of the row
            *dstRow = (uint8_t) ( (
                // 2 corners (*1)
                (uint16_t) srcRow[mSrcStride2P2ps] +
                           srcRow[ srcStride2P2ps] +
                // 4 edge values (*2)
                (uint16_t) 2 * (
                            srcRow[mSrcStride2Pps] +
                            srcRow[ srcStride2Pps] +
                            srcRow[mSrcStrideP2ps] +
                            srcRow[ srcStrideP2ps] ) +
                // 3 mid edge values (*3)
                (uint16_t) 3 * (
                            srcRow[mSrcStride2] +
                            srcRow[ srcStride2] +
                            srcRow[ pixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcRow[mSrcStridePps] +
                            srcRow[ srcStridePps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcRow[mSrcStride] +
                            srcRow[ srcStride] +
                            srcRow[ pixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcRow )
                    ) / 48 );

            srcRow += pixelSize;
            dstRow += pixelSize;

            // set the second pixel of the row
            *dstRow = (uint8_t) ( (
                // 2 corners (*1)
                (uint16_t) srcRow[mSrcStride2P2ps] +
                           srcRow[ srcStride2P2ps] +
                // 6 edge values (*2)
                (uint16_t) 2 * (
                            srcRow[mSrcStride2Mps] +
                            srcRow[mSrcStride2Pps] +
                            srcRow[ srcStride2Mps] +
                            srcRow[ srcStride2Pps] +
                            srcRow[mSrcStrideP2ps] +
                            srcRow[ srcStrideP2ps] ) +
                // 3 mid edge values (*3)
                (uint16_t) 3 * (
                            srcRow[mSrcStride2] +
                            srcRow[ srcStride2] +
                            srcRow[ pixelSize2] ) +
                // 4 inner corners (*4)
                (uint16_t) 4 * (
                            srcRow[mSrcStrideMps] +
                            srcRow[mSrcStridePps] +
                            srcRow[ srcStrideMps] +
                            srcRow[ srcStridePps] ) +
                // 4 inner mid value (*5)
                (uint16_t) 5 * (
                            srcRow[mSrcStride] +
                            srcRow[ srcStride] +
                            srcRow[ pixelSize] +
                            srcRow[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcRow )
                    ) / 65 );

            srcRow += pixelSize;
            dstRow += pixelSize;

            // process the rest
            for ( x = 2; x < widthM2; x++, srcRow += pixelSize, dstRow += pixelSize )
            {
                *dstRow = (uint8_t) ( (
                    // 4 corners (*1)
                    (uint16_t) srcRow[mSrcStride2M2ps] +
                               srcRow[mSrcStride2P2ps] +
                               srcRow[ srcStride2M2ps] +
                               srcRow[ srcStride2P2ps] +
                    // 8 edge values (*2)
                    (uint16_t) 2 * (
                               srcRow[mSrcStride2Mps] +
                               srcRow[mSrcStride2Pps] +
                               srcRow[ srcStride2Mps] +
                               srcRow[ srcStride2Pps] +
                               srcRow[mSrcStrideM2ps] +
                               srcRow[mSrcStrideP2ps] +
                               srcRow[ srcStrideM2ps] +
                               srcRow[ srcStrideP2ps] ) +
                    // 4 mid edge values (*3)
                    (uint16_t) 3 * (
                               srcRow[mSrcStride2] +
                               srcRow[ srcStride2] +
                               srcRow[ pixelSize2] +
                               srcRow[mPixelSize2] ) +
                    // 4 inner corners (*4)
                    (uint16_t) 4 * (
                               srcRow[mSrcStrideMps] +
                               srcRow[mSrcStridePps] +
                               srcRow[ srcStrideMps] +
                               srcRow[ srcStridePps] ) +
                    // 4 inner mid value (*5)
                    (uint16_t) 5 * (
                               srcRow[mSrcStride] +
                               srcRow[ srcStride] +
                               srcRow[ pixelSize] +
                               srcRow[mPixelSize] ) +
                    // center value
                    (uint16_t) 6 * ( *srcRow )
                        ) / 74 );
            }

            // set the pre-last pixel of the row
            *dstRow = (uint8_t) ( (
                // 2 corners (*1)
                (uint16_t) srcRow[mSrcStride2M2ps] +
                            srcRow[ srcStride2M2ps] +
                // 6 edge values (*2)
                (uint16_t) 2 * (
                            srcRow[mSrcStride2Mps] +
                            srcRow[mSrcStride2Pps] +
                            srcRow[ srcStride2Mps] +
                            srcRow[ srcStride2Pps] +
                            srcRow[mSrcStrideM2ps] +
                            srcRow[ srcStrideM2ps] ) +
                // 3 mid edge values (*3)
                (uint16_t) 3 * (
                            srcRow[mSrcStride2] +
                            srcRow[ srcStride2] +
                            srcRow[mPixelSize2] ) +
                // 4 inner corners (*4)
                (uint16_t) 4 * (
                            srcRow[mSrcStrideMps] +
                            srcRow[mSrcStridePps] +
                            srcRow[ srcStrideMps] +
                            srcRow[ srcStridePps] ) +
                // 4 inner mid value (*5)
                (uint16_t) 5 * (
                            srcRow[mSrcStride] +
                            srcRow[ srcStride] +
                            srcRow[ pixelSize] +
                            srcRow[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcRow )
                    ) / 65 );

            srcRow += pixelSize;
            dstRow += pixelSize;

            // set the last pixel of the row
            *dstRow = (uint8_t) ( (
                // 2 corners (*1)
                (uint16_t) srcRow[mSrcStride2M2ps] +
                            srcRow[ srcStride2M2ps] +
                // 4 edge values (*2)
                (uint16_t) 2 * (
                            srcRow[mSrcStride2Mps] +
                            srcRow[ srcStride2Mps] +
                            srcRow[mSrcStrideM2ps] +
                            srcRow[ srcStrideM2ps] ) +
                // 3 mid edge values (*3)
                (uint16_t) 3 * (
                            srcRow[mSrcStride2] +
                            srcRow[ srcStride2] +
                            srcRow[mPixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcRow[mSrcStrideMps] +
                            srcRow[ srcStrideMps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcRow[mSrcStride] +
                            srcRow[ srcStride] +
                            srcRow[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcRow )
                    ) / 48 );
        }
    }

    // process top/bottom edge pixels
    {
        int x, i;

        for ( i = 0; i < pixelSize; i++ )
        {
            // ====================================================================
            // --- process the first row
            srcPtr  = src->data + i;
            dstPtr  = dst->data + i;

            // first pixel of the first row
            *dstPtr = (uint8_t) ( (
                // 1 corner (*1)
                (uint16_t) srcPtr[ srcStride2P2ps] +
                // 2 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Pps] +
                            srcPtr[ srcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 1 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[ srcStridePps] ) +
                // 2 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 31 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // second pixel of the first row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[ srcStride2P2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Mps] +
                            srcPtr[ srcStride2Pps] +
                            srcPtr[ srcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[ srcStrideMps] +
                            srcPtr[ srcStridePps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the first row
            for ( x = 2; x < widthM2; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                *dstPtr = (uint8_t) ( (
                    // 2 corners (*1)
                    (uint16_t) srcPtr[ srcStride2M2ps] +
                               srcPtr[ srcStride2P2ps] +
                    // 4 edge values (*2)
                    (uint16_t) 2 * (
                               srcPtr[ srcStride2Mps] +
                               srcPtr[ srcStride2Pps] +
                               srcPtr[ srcStrideM2ps] +
                               srcPtr[ srcStrideP2ps] ) +
                    // 3 mid edge values (*3)
                    (uint16_t) 3 * (
                               srcPtr[ srcStride2] +
                               srcPtr[ pixelSize2] +
                               srcPtr[mPixelSize2] ) +
                    // 2 inner corners (*4)
                    (uint16_t) 4 * (
                               srcPtr[ srcStrideMps] +
                               srcPtr[ srcStridePps] ) +
                    // 3 inner mid value (*5)
                    (uint16_t) 5 * (
                               srcPtr[ srcStride] +
                               srcPtr[ pixelSize] +
                               srcPtr[mPixelSize] ) +
                    // center value
                    (uint16_t) 6 * ( *srcPtr )
                        ) / 48 );
            }

            // set the pre-last pixel of the first row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[ srcStride2M2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Mps] +
                            srcPtr[ srcStride2Pps] +
                            srcPtr[ srcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[ srcStrideMps] +
                            srcPtr[ srcStridePps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // set the last pixel of the first row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[ srcStride2M2ps] +
                // 2 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Mps] +
                            srcPtr[ srcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 1 inner corner (*4)
                (uint16_t) 4 * (
                            srcPtr[ srcStrideMps] ) +
                // 2 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[ srcStride] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 31 );

            // ====================================================================
            // --- process the second row
            srcPtr  = src->data + srcStride + i;
            dstPtr  = dst->data + dstStride + i;

            // first pixel of the second row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[ srcStride2P2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Pps] +
                            srcPtr[mSrcStrideP2ps] +
                            srcPtr[ srcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStridePps] +
                            srcPtr[ srcStridePps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // second pixel of the second row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[ srcStride2P2ps] +
                // 4 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Mps] +
                            srcPtr[ srcStride2Pps] +
                            srcPtr[mSrcStrideP2ps] +
                            srcPtr[ srcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 4 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[mSrcStridePps] +
                            srcPtr[ srcStrideMps] +
                            srcPtr[ srcStridePps] ) +
                // 4 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 57 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the second row
            for ( x = 2; x < widthM2; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                *dstPtr = (uint8_t) ( (
                    // 2 corners (*1)
                    (uint16_t) srcPtr[ srcStride2M2ps] +
                               srcPtr[ srcStride2P2ps] +
                    // 6 edge values (*2)
                    (uint16_t) 2 * (
                               srcPtr[ srcStride2Mps] +
                               srcPtr[ srcStride2Pps] +
                               srcPtr[mSrcStrideM2ps] +
                               srcPtr[mSrcStrideP2ps] +
                               srcPtr[ srcStrideM2ps] +
                               srcPtr[ srcStrideP2ps] ) +
                    // 3 mid edge values (*3)
                    (uint16_t) 3 * (
                               srcPtr[ srcStride2] +
                               srcPtr[ pixelSize2] +
                               srcPtr[mPixelSize2] ) +
                    // 4 inner corners (*4)
                    (uint16_t) 4 * (
                               srcPtr[mSrcStrideMps] +
                               srcPtr[mSrcStridePps] +
                               srcPtr[ srcStrideMps] +
                               srcPtr[ srcStridePps] ) +
                    // 4 inner mid value (*5)
                    (uint16_t) 5 * (
                               srcPtr[mSrcStride] +
                               srcPtr[ srcStride] +
                               srcPtr[ pixelSize] +
                               srcPtr[mPixelSize] ) +
                    // center value
                    (uint16_t) 6 * ( *srcPtr )
                        ) / 65 );
            }

            // set the pre-last pixel of the second row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[ srcStride2M2ps] +
                // 4 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Mps] +
                            srcPtr[ srcStride2Pps] +
                            srcPtr[mSrcStrideM2ps] +
                            srcPtr[ srcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 4 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[mSrcStridePps] +
                            srcPtr[ srcStrideMps] +
                            srcPtr[ srcStridePps] ) +
                // 4 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 57 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // set the last pixel of the second row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[ srcStride2M2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[ srcStride2Mps] +
                            srcPtr[mSrcStrideM2ps] +
                            srcPtr[ srcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[ srcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[ srcStrideMps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            // ====================================================================
            // --- process the pre-last row
            srcPtr  = src->data + srcStride * heightM2 + i;
            dstPtr  = dst->data + dstStride * heightM2 + i;

            // first pixel of the pre-last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t)  srcPtr[mSrcStride2P2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Pps] +
                            srcPtr[mSrcStrideP2ps] +
                            srcPtr[ srcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStridePps] +
                            srcPtr[ srcStridePps] ) +
                // 1 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // second pixel of the pre-last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t)  srcPtr[mSrcStride2P2ps] +
                // 4 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Mps] +
                            srcPtr[mSrcStride2Pps] +
                            srcPtr[mSrcStrideP2ps] +
                            srcPtr[ srcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 4 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[mSrcStridePps] +
                            srcPtr[ srcStrideMps] +
                            srcPtr[ srcStridePps] ) +
                // 4 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 57 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the pre-last row
            for ( x = 2; x < widthM2; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                *dstPtr = (uint8_t) ( (
                    // 2 corners (*1)
                    (uint16_t) srcPtr[mSrcStride2M2ps] +
                               srcPtr[mSrcStride2P2ps] +
                    // 6 edge values (*2)
                    (uint16_t) 2 * (
                               srcPtr[mSrcStride2Mps] +
                               srcPtr[mSrcStride2Pps] +
                               srcPtr[mSrcStrideM2ps] +
                               srcPtr[mSrcStrideP2ps] +
                               srcPtr[ srcStrideM2ps] +
                               srcPtr[ srcStrideP2ps] ) +
                    // 3 mid edge values (*3)
                    (uint16_t) 3 * (
                               srcPtr[mSrcStride2] +
                               srcPtr[ pixelSize2] +
                               srcPtr[mPixelSize2] ) +
                    // 4 inner corners (*4)
                    (uint16_t) 4 * (
                               srcPtr[mSrcStrideMps] +
                               srcPtr[mSrcStridePps] +
                               srcPtr[ srcStrideMps] +
                               srcPtr[ srcStridePps] ) +
                    // 4 inner mid value (*5)
                    (uint16_t) 5 * (
                               srcPtr[mSrcStride] +
                               srcPtr[ srcStride] +
                               srcPtr[ pixelSize] +
                               srcPtr[mPixelSize] ) +
                    // center value
                    (uint16_t) 6 * ( *srcPtr )
                        ) / 65 );
            }

            // set the pre-last pixel of the pre-last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[mSrcStride2M2ps] +
                // 4 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Mps] +
                            srcPtr[mSrcStride2Pps] +
                            srcPtr[mSrcStrideM2ps] +
                            srcPtr[ srcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 4 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[mSrcStridePps] +
                            srcPtr[ srcStrideMps] +
                            srcPtr[ srcStridePps] ) +
                // 4 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 57 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // set the last pixel of the pre-last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[mSrcStride2M2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Mps] +
                            srcPtr[mSrcStrideM2ps] +
                            srcPtr[ srcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[ srcStrideMps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ srcStride] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            // ====================================================================
            // --- process the last row
            srcPtr  = src->data + srcStride * heightM1 + i;
            dstPtr  = dst->data + dstStride * heightM1 + i;

            // first pixel of the last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[mSrcStride2P2ps] +
                // 2 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Pps] +
                            srcPtr[mSrcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 1 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStridePps] ) +
                // 2 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ pixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 31 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // second pixel of the last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[mSrcStride2P2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Mps] +
                            srcPtr[mSrcStride2Pps] +
                            srcPtr[mSrcStrideP2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[ pixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[mSrcStridePps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // process the rest of the last row
            for ( x = 2; x < widthM2; x++, srcPtr += pixelSize, dstPtr += pixelSize )
            {
                *dstPtr = (uint8_t) ( (
                    // 2 corners (*1)
                    (uint16_t) srcPtr[mSrcStride2M2ps] +
                               srcPtr[mSrcStride2P2ps] +
                    // 4 edge values (*2)
                    (uint16_t) 2 * (
                               srcPtr[mSrcStride2Mps] +
                               srcPtr[mSrcStride2Pps] +
                               srcPtr[mSrcStrideM2ps] +
                               srcPtr[mSrcStrideP2ps] ) +
                    // 3 mid edge values (*3)
                    (uint16_t) 3 * (
                               srcPtr[mSrcStride2] +
                               srcPtr[ pixelSize2] +
                               srcPtr[mPixelSize2] ) +
                    // 2 inner corners (*4)
                    (uint16_t) 4 * (
                               srcPtr[mSrcStrideMps] +
                               srcPtr[mSrcStridePps] ) +
                    // 3 inner mid value (*5)
                    (uint16_t) 5 * (
                               srcPtr[mSrcStride] +
                               srcPtr[ pixelSize] +
                               srcPtr[mPixelSize] ) +
                    // center value
                    (uint16_t) 6 * ( *srcPtr )
                        ) / 48 );
            }

            // set the pre-last pixel of the last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[mSrcStride2M2ps] +
                // 3 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Mps] +
                            srcPtr[mSrcStride2Pps] +
                            srcPtr[mSrcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 2 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] +
                            srcPtr[mSrcStridePps] ) +
                // 3 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[ pixelSize] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 42 );

            srcPtr += pixelSize;
            dstPtr += pixelSize;

            // set the last pixel of the last row
            *dstPtr = (uint8_t) ( (
                // 1 corners (*1)
                (uint16_t) srcPtr[mSrcStride2M2ps] +
                // 2 edge values (*2)
                (uint16_t) 2 * (
                            srcPtr[mSrcStride2Mps] +
                            srcPtr[mSrcStrideM2ps] ) +
                // 2 mid edge values (*3)
                (uint16_t) 3 * (
                            srcPtr[mSrcStride2] +
                            srcPtr[mPixelSize2] ) +
                // 1 inner corners (*4)
                (uint16_t) 4 * (
                            srcPtr[mSrcStrideMps] ) +
                // 2 inner mid value (*5)
                (uint16_t) 5 * (
                            srcPtr[mSrcStride] +
                            srcPtr[mPixelSize] ) +
                // center value
                (uint16_t) 6 * ( *srcPtr )
                    ) / 31 );
        }
    }
}