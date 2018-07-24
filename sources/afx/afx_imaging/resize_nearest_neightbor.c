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

// Resize image using nearest neighbor "interpolation"
XErrorCode ResizeImageNearestNeighbor( const ximage* src, ximage* dst )
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
        int      dstWidth  = dst->width;
        int      dstHeight = dst->height;
        int      pixelSize = ( dst->format == XPixelFormatGrayscale8 ) ? 1 :
                             ( dst->format == XPixelFormatRGB24 ) ? 3 : 4;
        int      srcStride = src->stride;
        int      dstStride = dst->stride;
        float    xFactor   = (float) src->width  / dstWidth;
        float    yFactor   = (float) src->height / dstHeight;
        uint8_t* srcPtr    = src->data;
        uint8_t* dstPtr    = dst->data;
        int      y;

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, dstWidth, srcStride, dstStride, pixelSize, xFactor, yFactor )
        for ( y = 0; y < dstHeight; y++ )
        {
            uint8_t* dstRow = dstPtr + y * dstStride;
            uint8_t* srcRow = srcPtr + ( (int)( y * yFactor ) ) * srcStride;
            uint8_t* p;
            int      x, i;

            for ( x = 0; x < dstWidth; x++ )
            {
                p = srcRow + ( (int)( x * xFactor ) ) * pixelSize;

                for ( i = 0; i < pixelSize; i++ )
                {
                    *dstRow = *p;
                    dstRow++;
                    p++;
                }
            }
        }
    }
    return ret;
}
