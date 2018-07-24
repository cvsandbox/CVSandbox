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

// Converts source grayscale image to 24/32 bpp color image
XErrorCode GrayscaleToColor( const ximage* src, ximage* dst )
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
    else if ( ( dst->width != src->width ) || ( dst->height != src->height ) ||
              ( ( dst->format != XPixelFormatRGB24 ) && ( dst->format != XPixelFormatRGBA32 ) ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int width        = src->width;
        int height       = src->height;
        int srcStride    = src->stride;
        int dstStride    = dst->stride;
        int dstPixelSize = ( dst->format == XPixelFormatRGB24 ) ? 3 : 4;
        int y;

        uint8_t* srcPtr = src->data;
        uint8_t* dstPtr = dst->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, dstPixelSize )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride;
            uint8_t* dstRow = dstPtr + y * dstStride;
            uint8_t  value;
            int x;

            for ( x = 0; x < width; x++, srcRow++, dstRow += dstPixelSize )
            {
                value = *srcRow;

                dstRow[RedIndex]   = value;
                dstRow[GreenIndex] = value;
                dstRow[BlueIndex]  = value;

                if ( dstPixelSize == 4 )
                {
                    dstRow[AlphaIndex]  = NotTransparent8bpp;
                }
            }
        }
    }

    return ret;
}
