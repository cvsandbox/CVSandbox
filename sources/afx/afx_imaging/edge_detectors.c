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

/* Algorithms:
 *
 * --------------------------------------
 * Difference edge detector
 * --------------------------------------
 * The image processng routine finds objects' edges by calculating
 * maximum difference between pixels in 4 directions around the
 * processing pixel
 *
 * Suppose 3x3 square element of the source image (x - is currently
 * processed pixel):
 * P1 P2 P3
 * P8  x P4
 * P7 P6 P5
 * The corresponding pixel of the result image is set to:
 * max( |P1-P5|, |P2-P6|, |P3-P7|, |P4-P8| )
 *
 * --------------------------------------
 * Homogenity edge detector
 * --------------------------------------
 * The image processing routine finds objects' edges by calculating
 * maximum difference between pixel being processed and its neighbours
 * in 8 directions.
 *
 * Suppose 3x3 square element of the source image (x - is currently
 * processed pixel):
 * P1 P2 P3
 * P8  x P4
 * P7 P6 P5
 * The corresponding pixel of the result image is set to:
 * max( |x-P1|, |x-P2|, |x-P3|, |x-P4|, |x-P5|, |x-P6|, |x-P7|, |x-P8| )
 *
 * --------------------------------------
 * Sobel edge detector
 * --------------------------------------
 * Each pixel of the result image is calculated as approximated absolute gradient
 * magnitude for corresponding pixel of the source image:
 * |G| = |Gx| + |Gy] ,
 * where Gx and Gy are calculate utilizing Sobel convolution kernels:
 *    Gx         Gy
 * -1 0 +1    +1 +2 +1
 * -2 0 +2     0  0  0
 * -1 0 +1    -1 -2 -1
 *
 * Using the above kernel the approximated magnitude for pixel X is calculate using
 * the next equation:
 * P1 P2 P3
 * P8  x P4
 * P7 P6 P5
 *
 * |G| = |P1 + 2P2 + P3 - P7 - 2P6 - P5| + |P3 + 2P4 + P5 - P1 - 2P8 - P7|
 *
 * --------------------------------------
 * Note: all edge pixels of the destination image are set to 0 for all algorithms.
 * --------------------------------------
 */

// Difference edge detector algorithm
XErrorCode EdgeDetector( const ximage* src, ximage* dst, XSimpleEdgeDetector type, bool scaleIntensity )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( dst->width  != src->width  ) ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( type > EdgeDetector_Sobel )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        int width     = src->width;
        int height    = src->height;
        int widthM1   = width  - 1;
        int heightM1  = height - 1;
        int srcStride = src->stride;
        int dstStride = dst->stride;
        int y;

        uint8_t* srcPtr    = src->data;
        uint8_t* dstPtr    = dst->data;
        uint8_t  globalMax = 0;

        // some pre-calculated values
        int srcStrideP1  =  srcStride + 1;
        int srcStrideM1  =  srcStride - 1;
        int mSrcStrideP1 = -srcStride + 1;
        int mSrcStrideM1 = -srcStride - 1;
        int mSrcStride   = -srcStride;

        if ( type == EdgeDetector_Difference )
        {
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthM1, srcStride, dstStride, srcStrideP1, srcStrideM1, mSrcStrideP1, mSrcStrideM1, mSrcStride )
            for ( y = 1; y < heightM1; y++ )
            {
                uint8_t* srcRow = srcPtr + y * srcStride + 1;
                uint8_t* dstRow = dstPtr + y * dstStride + 1;
                int      x , d, max, rowMax = 0;

                // process the rest
                for ( x = 1; x < widthM1; x++ )
                {
                    // left diagonal
                    max = (int) srcRow[mSrcStrideM1] - srcRow[srcStrideP1];
                    if ( max < 0 ) max = -max;

                    // right diagonal
                    d = (int) srcRow[mSrcStrideP1] - srcRow[srcStrideM1];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // vertical
                    d = (int) srcRow[mSrcStride] - srcRow[srcStride];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // horizontal
                    d = (int) srcRow[-1] - srcRow[1];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    *dstRow = (uint8_t) max;

                    if ( max > rowMax ) rowMax = max;

                    srcRow++;
                    dstRow++;
                }

                // since we run in parallel, put row maximum value into last pixel of the row
                *dstRow = (uint8_t) rowMax;
            }
        }
        else if ( type == EdgeDetector_Homogeneity )
        {
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthM1, srcStride, dstStride, srcStrideP1, srcStrideM1, mSrcStrideP1, mSrcStrideM1, mSrcStride )
            for ( y = 1; y < heightM1; y++ )
            {
                uint8_t* srcRow = srcPtr + y * srcStride + 1;
                uint8_t* dstRow = dstPtr + y * dstStride + 1;
                uint8_t  srcValue;
                int      x, d, max, rowMax = 0;

                // process the rest
                for ( x = 1; x < widthM1; x++ )
                {
                    srcValue = *srcRow;

                    // top-left
                    max = (int) srcValue - srcRow[mSrcStrideM1];
                    if ( max < 0 ) max = -max;

                    // top
                    d = (int) srcValue - srcRow[mSrcStride];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // top-right
                    d = (int) srcValue - srcRow[mSrcStrideP1];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // left
                    d = (int) srcValue - srcRow[-1];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // right
                    d = (int) srcValue - srcRow[1];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // bottom-left
                    d = (int) srcValue - srcRow[srcStrideM1];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // bottom
                    d = (int) srcValue - srcRow[srcStride];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    // bottom-right
                    d = (int) srcValue - srcRow[srcStrideP1];
                    if ( d < 0 ) d = -d;
                    if ( d > max ) max = d;

                    *dstRow = (uint8_t) max;

                    if ( max > rowMax ) rowMax = max;

                    srcRow++;
                    dstRow++;
                }

                // since we run in parallel, put row maximum value into last pixel of the row
                *dstRow = (uint8_t) rowMax;
            }
        }
        else // Sobel edge detector
        {
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthM1, srcStride, dstStride, srcStrideP1, srcStrideM1, mSrcStrideP1, mSrcStrideM1, mSrcStride )
            for ( y = 1; y < heightM1; y++ )
            {
                uint8_t* srcRow = srcPtr + y * srcStride + 1;
                uint8_t* dstRow = dstPtr + y * dstStride + 1;
                int      x , gx, gy, g, rowMax = 0;

                // process the rest
                for ( x = 1; x < widthM1; x++ )
                {
                    gy = (int) srcRow[mSrcStrideM1] + srcRow[mSrcStrideP1]
                             - srcRow[ srcStrideM1] - srcRow[ srcStrideP1]
                       + 2 * ( srcRow[mSrcStride  ] - srcRow[ srcStride  ] );

                    gx = (int) srcRow[mSrcStrideP1] + srcRow[srcStrideP1]
                             - srcRow[mSrcStrideM1] - srcRow[srcStrideM1]
                       + 2 * ( srcRow[1]            - srcRow[-1] );

                    if ( gx < 0 ) gx = -gx;
                    if ( gy < 0 ) gy = -gy;

                    g = gx + gy;

                    if ( g > 255 ) g = 255;
                    if ( g > rowMax ) rowMax = g;

                    *dstRow = (uint8_t) g;

                    srcRow++;
                    dstRow++;
                }

                // since we run in parallel, put row maximum value into last pixel of the row
                *dstRow = (uint8_t) rowMax;
            }
        }

        // zero edges of the destination image
        {
            uint8_t* dstPtr1 = dstPtr;
            uint8_t* dstPtr2 = dstPtr + heightM1 * dstStride;
            int x;

            for ( x = 0; x < width; x++ )
            {
                *dstPtr1 = 0;
                *dstPtr2 = 0;

                dstPtr1++;
                dstPtr2++;
            }

            dstPtr1 = dstPtr + dstStride;
            dstPtr2 = dstPtr + dstStride + widthM1;

            for ( y = 1; y < heightM1; y++ )
            {
                // first check for new global maximun
                if ( *dstPtr2 > globalMax ) globalMax = *dstPtr2;

                *dstPtr1 = 0;
                *dstPtr2 = 0;

                dstPtr1 += dstStride;
                dstPtr2 += dstStride;
            }
        }

        // scale intensities
        if ( ( scaleIntensity == true ) && ( globalMax != 255 ) && ( globalMax != 0 ) )
        {
            uint8_t map[256];
            float   factor = 255.0f / (float) globalMax;
            int     x, i;

            for ( i = 0; i <= globalMax; i++ )
            {
                map[i] = (uint8_t) (int) ( factor * i + 0.5f );
            }

            for ( y = 1; y < heightM1; y++ )
            {
                uint8_t* dstRow = dstPtr + y * dstStride + 1;

                // process the rest
                for ( x = 1; x < widthM1; x++, dstRow++ )
                {
                    *dstRow = map[*dstRow];
                }
            }
        }
    }

    return ret;
}
