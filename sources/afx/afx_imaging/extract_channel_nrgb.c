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

// forward declaration ----
static void Extract8bppChannel( uint8_t* src, int srcStride, int srcPixelSize,
                                uint8_t* dst, int dstStride,
                                int width, int height, int channelIndex );
// ------------------------

// Extracts specified nRGB channel from the source image
XErrorCode ExtractNRGBChannel( const ximage* src, ximage* dst, uint32_t channelIndex )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatRGB24 ) && ( src->format != XPixelFormatRGBA32 ) &&
              ( src->format != XPixelFormatRGB48 ) && ( src->format != XPixelFormatRGBA64 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( channelIndex > 2 )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        int width  = src->width;
        int height = src->height;

        if ( ( dst->width != width ) || ( dst->height != height ) )
        {
            ret = ErrorImageParametersMismatch;
        }
        else
        {
            if ( ( src->format == XPixelFormatRGB24 ) || ( src->format == XPixelFormatRGBA32 ) )
            {
                if ( dst->format == XPixelFormatGrayscale8 )
                {
                    Extract8bppChannel( src->data, src->stride, ( src->format == XPixelFormatRGB24 ) ? 3 : 4,
                                        dst->data, dst->stride, width, height, channelIndex );
                }
                else
                {
                    ret = ErrorImageParametersMismatch;
                }
            }
            else
            {
                if ( dst->format == XPixelFormatGrayscale16 )
                {
                    ret = ErrorNotImplemented;
                }
                else
                {
                    ret = ErrorImageParametersMismatch;
                }
            }
        }
    }

    return ret;
}

// Extract 8 bpp channel from 24/32 bpp color image
void Extract8bppChannel( uint8_t* src, int srcStride, int srcPixelSize,
                         uint8_t* dst, int dstStride, int width, int height, int channelIndex )
{
    int y;

    #pragma omp parallel for schedule(static) shared( src, dst, width, srcStride, dstStride, srcPixelSize, channelIndex )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = src + y * srcStride;
        uint8_t* dstRow = dst + y * dstStride;
        int x;
        int sum;

        for ( x = 0; x < width; x++, srcRow += srcPixelSize, dstRow++ )
        {
            sum = srcRow[RedIndex] + srcRow[GreenIndex] + srcRow[BlueIndex];

            *dstRow =  ( sum != 0 ) ? (uint8_t) ( 255 * srcRow[channelIndex] / sum ) : (uint8_t) 0;
        }
    }
}
