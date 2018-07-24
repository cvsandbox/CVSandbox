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
 * bitwise OR of pixels from the corresponding
 * 3x3 square window in the source image
 * (edge pixels use only available neighbors).
 * --------------------------------------
 */

// 3x3 dilatation filter for grayscale images containing only black (0) and white (255) pixels
XErrorCode BinaryDilatation3x3( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
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
    else
    {
        int width     = src->width;
        int height    = src->height;
        int widthM1   = width - 1;
        int heightM1  = height - 1;
        int srcStride = src->stride;
        int dstStride = dst->stride;
        int y, tx;

        uint8_t* srcPtr  = src->data;
        uint8_t* dstPtr  = dst->data;
        uint8_t* srcPtr2 = srcPtr + srcStride * heightM1 + 1;
        uint8_t* dstPtr2 = dstPtr + dstStride * heightM1 + 1;

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride )
        for ( y = 1; y < heightM1; y++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride;
            uint8_t* dstRow = dstPtr + y * dstStride;
            int x;

            // process first pixel
            *dstRow = (uint8_t) ( *srcRow | srcRow[1] |
                srcRow[-srcStride] | srcRow[-srcStride + 1] |
                srcRow[ srcStride] | srcRow[ srcStride + 1] );

            srcRow++;
            dstRow++;

            // process the rest
            for ( x = 1; x < widthM1; x++, srcRow++, dstRow++ )
            {
                *dstRow = (uint8_t) ( *srcRow | srcRow[-1] | srcRow[1] |
                    srcRow[-srcStride] | srcRow[-srcStride - 1] | srcRow[-srcStride + 1] |
                    srcRow[ srcStride] | srcRow[ srcStride - 1] | srcRow[ srcStride + 1] );
            }

            // process last pixel
            *dstRow = (uint8_t) ( *srcRow | srcRow[-1] |
                srcRow[-srcStride] | srcRow[-srcStride - 1] |
                srcRow[ srcStride] | srcRow[ srcStride - 1] );
        }

        // process first and last rows
        *dstPtr = (uint8_t) ( *srcPtr | srcPtr[1] | srcPtr[srcStride] | srcPtr[srcStride + 1] );
        *dstPtr2 = (uint8_t) ( *srcPtr2 | srcPtr2[1] | srcPtr2[-srcStride] | srcPtr2[-srcStride + 1] );

        srcPtr++;
        dstPtr++;

        for ( tx = 1; tx < widthM1; tx++, dstPtr++, dstPtr2++, srcPtr++, srcPtr2++ )
        {
            *dstPtr = (uint8_t) ( *srcPtr | srcPtr[-1] | srcPtr[1] |
                srcPtr[srcStride] | srcPtr[srcStride - 1] | srcPtr[srcStride + 1] );

            *dstPtr2 = (uint8_t) ( *srcPtr2 | srcPtr2[-1] | srcPtr2[1] |
                srcPtr2[-srcStride] | srcPtr2[-srcStride - 1] | srcPtr2[-srcStride + 1] );
        }

        *dstPtr = (uint8_t) ( *srcPtr | srcPtr[-1] | srcPtr[srcStride] | srcPtr[srcStride - 1] );
        *dstPtr2 = (uint8_t) ( *srcPtr2 | srcPtr2[-1] | srcPtr2[-srcStride] | srcPtr2[-srcStride - 1] );
    }

    return ret;
}
