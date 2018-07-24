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

// Perform ordered dithering of the specified 8 bpp grayscale image putting result into the specified 1 bpp binary image
// Note: destination image must be "clean" - black pixels only
XErrorCode BinaryOrderedDithering( const ximage* src, ximage* dst, const uint8_t* thresholdMatrix, uint32_t matrixOrder )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( thresholdMatrix == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) || ( dst->format != XPixelFormatBinary1 ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( matrixOrder == 0 )
    {
        ret = ErrorInvalidArgument;
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

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, thresholdMatrix, matrixOrder )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride;
            uint8_t* dstRow = dstPtr + y * dstStride;
            uint32_t x;
            uint32_t shift;
            uint8_t  bit;

            const uint8_t* matrixRow = thresholdMatrix + ( y % matrixOrder ) * matrixOrder;

            for ( x = 0; x < width; x++ )
            {
                bit = ( *srcRow >= matrixRow[x % matrixOrder] ) ? 1 : 0;
                shift = 7 - ( x & 7 );

                *dstRow |= ( bit << shift );

                if ( shift == 0 )
                {
                    dstRow++;
                }
                srcRow++;
            }
        }
    }

    return ret;
}

// Perform ordered dithering of the specified 8 bpp grayscale image using 2x2 threshold matrix
XErrorCode BinaryOrderedDithering2( const ximage* src, ximage* dst )
{
    static const uint8_t thresholdMatrix[] =
    {
         51, 153,
        204, 102,
    };

    return BinaryOrderedDithering( src, dst, thresholdMatrix, 2 );
}

// Perform ordered dithering of the specified 8 bpp grayscale image using 3x3 threshold matrix
XErrorCode BinaryOrderedDithering3( const ximage* src, ximage* dst )
{
    static const uint8_t thresholdMatrix[] =
    {
         77, 179, 102,
        153,  26, 230,
         51, 204, 128
    };

    return BinaryOrderedDithering( src, dst, thresholdMatrix, 3 );
}

// Perform ordered dithering of the specified 8 bpp grayscale image using 4x4 threshold matrix
XErrorCode BinaryOrderedDithering4( const ximage* src, ximage* dst )
{
    static const uint8_t thresholdMatrix[] =
    {
         15, 135,  45, 165,
        195,  75, 225, 105,
         60, 180,  30, 150,
        240, 120, 210,  90
    };

    return BinaryOrderedDithering( src, dst, thresholdMatrix, 4 );
}

// Perform ordered dithering of the specified 8 bpp grayscale image using 8x8 threshold matrix
XErrorCode BinaryOrderedDithering8( const ximage* src, ximage* dst )
{
    static const uint8_t thresholdMatrix[] =
    {
          4, 192,  51, 239,  16, 204,  63, 251,
        129,  67, 177, 114, 141,  78, 188, 126,
         35, 224,  20, 208,  47, 235,  31, 220,
        161,  98, 145,  82, 173, 110, 157,  94,
         12, 200,  59, 247,   8, 196,  55, 243,
        137,  75, 184, 122, 133,  71, 180, 118,
         43, 231,  27, 216,  39, 228,  24, 212,
        169, 106, 153,  90, 165, 102, 149,  86
    };

    return BinaryOrderedDithering( src, dst, thresholdMatrix, 8 );
}
