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

// Converts source 1 bpp binary image into grayscale
XErrorCode BinaryToGrayscale( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatBinary1 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) || ( dst->format != XPixelFormatGrayscale8 ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        uint32_t width     = (uint32_t) src->width;
        int      height    = src->height;
        int      srcStride = src->stride;
        int      dstStride = dst->stride;
        int      y;

        uint8_t* srcPtr = src->data;
        uint8_t* dstPtr = dst->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride;
            uint8_t* dstRow = dstPtr + y * dstStride;
            uint8_t  mask;
            uint32_t x;

            for ( x = 0; x < width; x++ )
            {
                mask  = (uint8_t) ( 0x80 >> ( x & 0x07 ) );

                *dstRow = ( (*srcRow) & mask ) ? (uint8_t) Max8bppPixelValue : (uint8_t) MinPixelValue;
                dstRow++;

                if ( mask == 1 )
                {
                    srcRow++;
                }
            }
        }
    }

    return ret;
}
