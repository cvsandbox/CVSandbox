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

// Resize image using bilinear interpolation
XErrorCode ResizeImageBilinear( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( src->format != dst->format )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( dst->width == src->width ) && ( dst->height == src->height ) )
    {
        ret = XImageCopyData( src, dst );
    }
    else
    {
        int      srcWidth    = src->width;
        int      srcHeight   = src->height;
        int      srcWidthM1  = srcWidth - 1;
        int      srcHeightM1 = srcHeight - 1;
        int      dstWidth    = dst->width;
        int      dstHeight   = dst->height;
        int      pixelSize   = ( dst->format == XPixelFormatGrayscale8 ) ? 1 :
                               ( dst->format == XPixelFormatRGB24 ) ? 3 : 4;
        int      srcStride   = src->stride;
        int      dstStride   = dst->stride;
        float    xFactor     = (float) src->width  / dstWidth;
        float    yFactor     = (float) src->height / dstHeight;
        uint8_t* srcPtr      = src->data;
        uint8_t* dstPtr      = dst->data;
        int      y;

        if ( ( xFactor <= 1.0f ) && ( yFactor <= 1.0f ) )
        {
            // increase both width and height

            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, dstWidth, srcWidthM1, srcHeightM1, srcStride, dstStride, pixelSize, xFactor, yFactor )
            for ( y = 0; y < dstHeight; y++ )
            {
                float sy     = yFactor * y;
                int   sy1    = (int) sy;
                int   sy2    = ( sy1 == srcHeightM1 ) ? sy1 : sy1 + 1;
                float yCoef2 = sy - sy1;
                float yCoef1 = 1.0f - yCoef2;
                float sx, xcoef1, xcoef2;
                int   x, i, sx1, sx2;

                uint8_t* dstRow = dstPtr + y * dstStride;
                uint8_t* srcRow1 = srcPtr + sy1 * srcStride;
                uint8_t* srcRow2 = srcPtr + sy2 * srcStride;
                uint8_t  *sp1, *sp2, *sp3, *sp4;

                for ( x = 0; x < dstWidth; x++ )
                {
                    sx     = xFactor * x;
                    sx1    = (int) sx;
                    sx2    = ( sx1 == srcWidthM1 ) ? sx1 : sx1 + 1;
                    xcoef2 = sx - sx1;
                    xcoef1 = 1.0f - xcoef2;

                    sp1 = srcRow1 + sx1 * pixelSize;
                    sp2 = srcRow1 + sx2 * pixelSize;
                    sp3 = srcRow2 + sx1 * pixelSize;
                    sp4 = srcRow2 + sx2 * pixelSize;

                    for ( i = 0; i < pixelSize; i++ )
                    {
                        *dstRow = (uint8_t) (
                            yCoef1 * ( xcoef1 * ( *sp1 ) + xcoef2 * ( *sp2 ) ) +
                            yCoef2 * ( xcoef1 * ( *sp3 ) + xcoef2 * ( *sp4 ) ) );

                        dstRow++;
                        sp1++;
                        sp2++;
                        sp3++;
                        sp4++;
                    }
                }
            }
        }
        else if ( ( xFactor > 1.0f ) && ( yFactor > 1.0f ) )
        {
            // decrease both width and height

            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, dstWidth, srcWidth, srcHeight, srcStride, dstStride, pixelSize, xFactor, yFactor )
            for ( y = 0; y < dstHeight; y++ )
            {
                float sy1        = yFactor * y;
                float sy2        = yFactor * ( y + 1 );
                int   startY     = (int) sy1;
                int   endY       = (int) sy2;
                float startYcoef = 1.0f - ( sy1 - startY );
                float endYcoef   = 1.0f;
                float sx1, sx2, startXcoef, endXcoef, ycoef, xcoef, coef;
                int   x, i, tx, ty, startX, endX;

                float pixelSums[4];
                float pixelCoef;

                uint8_t* dstRow = dstPtr + y * dstStride;
                uint8_t* srcRow;

                // increase end row by 1 if 2nd Y coordinate is not a whole number
                if ( endY != sy2 )
                {
                    endYcoef = sy2 - endY;
                    endY++;
                }
                // boundary check
                if ( endY > srcHeight )
                {
                    endYcoef = 1.0f;
                    endY = srcHeight;
                }

                // decrement it to make inner loops simpler
                endY--;

                for ( x = 0; x < dstWidth; x++ )
                {
                    sx1        = xFactor * x;
                    sx2        = xFactor * ( x + 1 );
                    startX     = (int) sx1;
                    endX       = (int) sx2;
                    startXcoef = 1.0f - ( sx1 - startX );
                    endXcoef   = 1.0f;

                    // increase end column by 1 if if 2nd X coordinate is not a whole number
                    if ( endX != sx2 )
                    {
                        endXcoef = sx2 - endX;
                        endX++;
                    }
                    // boundary check
                    if ( endX > srcWidth )
                    {
                        endXcoef = 1.0f;
                        endX = srcWidth;
                    }

                    // decrement it to make inner loops simpler
                    endX--;

                    // reset pixels' weighted sums and coeffients' sum
                    pixelSums[0] = 0;
                    pixelSums[1] = 0;
                    pixelSums[2] = 0;
                    pixelSums[3] = 0;

                    pixelCoef = 0;

                    for ( ty = startY; ty <= endY; ty++ )
                    {
                        srcRow = srcPtr + ty * srcStride + startX * pixelSize;
                        ycoef = ( ty == startY ) ? startYcoef : ( ( ty == endY ) ? endYcoef  : 1.0f );

                        for ( tx = startX; tx <= endX; tx++ )
                        {
                            xcoef = ( tx == startX ) ? startXcoef : ( ( tx == endX ) ? endXcoef  : 1.0f );
                            coef  = xcoef * ycoef;

                            for ( i = 0; i < pixelSize; i++ )
                            {
                                pixelSums[i] += coef * srcRow[i];
                            }

                            pixelCoef += coef;
                            srcRow += pixelSize;
                        }
                    }

                    for ( i = 0; i < pixelSize; i++ )
                    {
                        *dstRow = (uint8_t) ( pixelSums[i] / pixelCoef );
                        dstRow++;
                    }
                }
            }
        }
        else if ( xFactor <= 1.0f )
        {
            // increase width, but decrease height
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, dstWidth, srcWidthM1, srcHeight, srcStride, dstStride, pixelSize, xFactor, yFactor )
            for ( y = 0; y < dstHeight; y++ )
            {
                float sy1        = yFactor * y;
                float sy2        = yFactor * ( y + 1 );
                int   startY     = (int) sy1;
                int   endY       = (int) sy2;
                float startYcoef = 1.0f - ( sy1 - startY );
                float endYcoef   = 1.0f;
                float ycoef, xcoef1, xcoef2, coef1, coef2, sx;
                int   x, i, ty, sx1, sx2;

                float pixelSums[4];
                float pixelCoef;

                uint8_t* dstRow = dstPtr + y * dstStride;
                uint8_t* srcRow;
                uint8_t* srcRow1;
                uint8_t* srcRow2;

                // increase end row by 1 if 2nd Y coordinate is not a whole number
                if ( endY != sy2 )
                {
                    endYcoef = sy2 - endY;
                    endY++;
                }
                // boundary check
                if ( endY > srcHeight )
                {
                    endYcoef = 1.0f;
                    endY = srcHeight;
                }

                // decrement it to make inner loops simpler
                endY--;

                for ( x = 0; x < dstWidth; x++ )
                {
                    sx     = xFactor * x;
                    sx1    = (int) sx;
                    sx2    = ( sx1 == srcWidthM1 ) ? sx1 : sx1 + 1;
                    xcoef2 = sx - sx1;
                    xcoef1 = 1.0f - xcoef2;

                    // reset pixels' weighted sums and coeffients' sum
                    pixelSums[0] = 0;
                    pixelSums[1] = 0;
                    pixelSums[2] = 0;
                    pixelSums[3] = 0;

                    pixelCoef = 0;

                    for ( ty = startY; ty <= endY; ty++ )
                    {
                        srcRow  = srcPtr + ty * srcStride;
                        srcRow1 = srcRow + sx1 * pixelSize;
                        srcRow2 = srcRow + sx2 * pixelSize;
                        ycoef   = ( ty == startY ) ? startYcoef : ( ( ty == endY ) ? endYcoef  : 1.0f );
                        coef1   = xcoef1 * ycoef;
                        coef2   = xcoef2 * ycoef;

                        for ( i = 0; i < pixelSize; i++ )
                        {
                            pixelSums[i] += coef1 * srcRow1[i];
                            pixelSums[i] += coef2 * srcRow2[i];
                        }

                        pixelCoef += coef1;
                        pixelCoef += coef2;
                    }

                    for ( i = 0; i < pixelSize; i++ )
                    {
                        *dstRow = (uint8_t) ( pixelSums[i] / pixelCoef );
                        dstRow++;
                    }
                }
            }
        }
        else
        {
            // decrease width, but increase height
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, dstWidth, srcWidth, srcHeightM1, srcStride, dstStride, pixelSize, xFactor, yFactor )
            for ( y = 0; y < dstHeight; y++ )
            {
                float sy     = yFactor * y;
                int   sy1    = (int) sy;
                int   sy2    = ( sy1 == srcHeightM1 ) ? sy1 : sy1 + 1;
                float ycoef2 = sy - sy1;
                float ycoef1 = 1.0f - ycoef2;
                float sx1, sx2, startXcoef, endXcoef, xcoef, coef1, coef2;
                int   x, i, startX, endX, tx;

                float pixelSums[4];
                float pixelCoef;

                uint8_t* dstRow  = dstPtr + y * dstStride;
                uint8_t* srcRow1 = srcPtr + sy1 * srcStride;
                uint8_t* srcRow2 = srcPtr + sy2 * srcStride;
                uint8_t* srcPtr1;
                uint8_t* srcPtr2;

                for ( x = 0; x < dstWidth; x++ )
                {
                    sx1        = xFactor * x;
                    sx2        = xFactor * ( x + 1 );
                    startX     = (int) sx1;
                    endX       = (int) sx2;
                    startXcoef = 1.0f - ( sx1 - startX );
                    endXcoef   = 1.0f;

                    srcPtr1 = srcRow1 + startX * pixelSize;
                    srcPtr2 = srcRow2 + startX * pixelSize;

                    // increase end column by 1 if if 2nd X coordinate is not a whole number
                    if ( endX != sx2 )
                    {
                        endXcoef = sx2 - endX;
                        endX++;
                    }
                    // boundary check
                    if ( endX > srcWidth )
                    {
                        endXcoef = 1.0f;
                        endX = srcWidth;
                    }

                    // decrement it to make inner loops simpler
                    endX--;

                    // reset pixels' weighted sums and coeffients' sum
                    pixelSums[0] = 0;
                    pixelSums[1] = 0;
                    pixelSums[2] = 0;
                    pixelSums[3] = 0;

                    pixelCoef = 0;

                    for ( tx = startX; tx <= endX; tx++ )
                    {
                        xcoef = ( tx == startX ) ? startXcoef : ( ( tx == endX ) ? endXcoef  : 1.0f );
                        coef1 = xcoef * ycoef1;
                        coef2 = xcoef * ycoef2;

                        for ( i = 0; i < pixelSize; i++ )
                        {
                            pixelSums[i] += coef1 * srcPtr1[i];
                            pixelSums[i] += coef2 * srcPtr2[i];
                        }

                        pixelCoef += coef1 + coef2;
                        srcPtr1 += pixelSize;
                        srcPtr2 += pixelSize;
                    }

                    for ( i = 0; i < pixelSize; i++ )
                    {
                        *dstRow = (uint8_t) ( pixelSums[i] / pixelCoef );
                        dstRow++;
                    }
                }
            }
        }
    }

    return ret;
}
