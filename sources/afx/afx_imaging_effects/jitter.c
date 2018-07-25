/*
    Imaging effects library of Computer Vision Sandbox

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

#include <time.h>
#include "ximaging.h"

// forward declaration ----
static void ImageJitter8( ximage* src, uint8_t radius );
static void ImageJitter24( ximage* src, uint8_t radius );
// ------------------------

// Perform jittering on the image
XErrorCode ImageJitter( ximage* src, uint8_t radius )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format == XPixelFormatGrayscale8 )
    {
        ImageJitter8( src, radius );
    }
    else if ( ( src->format == XPixelFormatRGB24 ) || ( src->format == XPixelFormatRGBA32 ) )
    {
        ImageJitter24( src, radius );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// Performs jittering on 8 bpp grayscale images
void ImageJitter8( ximage* src, uint8_t radius )
{
    int width  = src->width;
    int height = src->height;
    int stride = src->stride;
    int x, y;
    int sx, sy;
    uint8_t* ptr = src->data;
    uint8_t* p;

    // maximum value for random number generator
    int maxRand = (int) radius * 2 + 1;

    for ( y = 0; y < height; y++ )
    {
        uint8_t* row = ptr + y * stride;

        for ( x = 0; x < width; x++ )
        {
            sx = x + ( rand( ) % maxRand ) - radius;
            sy = y + ( rand( ) % maxRand ) - radius;

            if ( ( sx >= 0 ) && ( sy >= 0 ) && ( sx < width ) && ( sy < height ) )
            {
                p = ptr + sy * stride + sx;
                *row = *p;
            }

            row++;
        }
    }
}

// Performs jittering on 24/32 bpp color images
void ImageJitter24( ximage* src, uint8_t radius )
{
    int width  = src->width;
    int height = src->height;
    int stride = src->stride;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int x, y, i;
    int sx, sy;
    uint8_t* ptr = src->data;
    uint8_t* p;

    // maximum value for random number generator
    int maxRand = (int) radius * 2 + 1;

    for ( y = 0; y < height; y++ )
    {
        uint8_t* row = ptr + y * stride;

        for ( x = 0; x < width; x++ )
        {
            sx = x + ( rand( ) % maxRand ) - radius;
            sy = y + ( rand( ) % maxRand ) - radius;

            if ( ( sx >= 0 ) && ( sy >= 0 ) && ( sx < width ) && ( sy < height ) )
            {
                p = ptr + sy * stride + sx * pixelSize;

                for ( i = 0; i < pixelSize; i++ )
                {
                    *row = *p;
                    row++;
                    p++;
                }
            }
            else
            {
                row += pixelSize;
            }
        }
    }
}
