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

static void Replace8bppChannel( uint8_t* chan, int chanStride,
                                uint8_t* dst, int dstStride, int dstPixelSize,
                                int width, int height, int channelIndex );
// ------------------------

// Extracts specified ARGB channel from the source image
XErrorCode ExtractRGBChannel( const ximage* src, ximage* dst, uint32_t channelIndex )
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
    else if ( ( channelIndex > 3 ) ||
            ( ( src->format != XPixelFormatRGBA32 ) && ( src->format != XPixelFormatRGBA64 ) && ( channelIndex > 2 ) ) )
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

// Replaces specified ARGB channel in the destination image
XErrorCode ReplaceRGBChannel( ximage* dst, const ximage* channel, uint32_t channelIndex )
{
    XErrorCode ret = SuccessCode;

    if ( ( channel == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( dst->format != XPixelFormatRGB24 ) && ( dst->format != XPixelFormatRGBA32 ) &&
              ( dst->format != XPixelFormatRGB48 ) && ( dst->format != XPixelFormatRGBA64 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( channelIndex > 3 ) ||
            ( ( dst->format != XPixelFormatRGBA32 ) && ( dst->format != XPixelFormatRGBA64 ) && ( channelIndex > 2 ) ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        int width  = dst->width;
        int height = dst->height;

        if ( ( channel->width != width ) || ( channel->height != height ) )
        {
            ret = ErrorImageParametersMismatch;
        }
        else
        {
            if ( ( dst->format == XPixelFormatRGB24 ) || ( dst->format == XPixelFormatRGBA32 ) )
            {
                if ( channel->format == XPixelFormatGrayscale8 )
                {
                    Replace8bppChannel( channel->data, channel->stride, dst->data, dst->stride,
                                        ( dst->format == XPixelFormatRGB24 ) ? 3 : 4, width, height, channelIndex );
                }
                else
                {
                    ret = ErrorImageParametersMismatch;
                }
            }
            else
            {
                if ( channel->format == XPixelFormatGrayscale16 )
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
    int srcOffset = srcStride - width * srcPixelSize;
    int dstOffset = dstStride - width;
    int x, y;

    src += channelIndex;

    for ( y = 0; y < height; y++, src += srcOffset, dst += dstOffset )
    {
        for ( x = 0; x < width; x++, src += srcPixelSize, dst++ )
        {
            *dst = *src;
        }
    }
}

// Replace 8 bpp channel in 24/32 bpp color image
void Replace8bppChannel( uint8_t* chan, int chanStride,
                         uint8_t* dst, int dstStride, int dstPixelSize, int width, int height, int channelIndex )
{
    int chanOffset = chanStride - width;
    int dstOffset  = dstStride  - width * dstPixelSize;
    int x, y;

    dst += channelIndex;

    for ( y = 0; y < height; ++y, chan += chanOffset, dst += dstOffset )
    {
        for ( x = 0; x < width; ++x, ++chan, dst += dstPixelSize )
        {
            *dst = *chan;
        }
    }
}
