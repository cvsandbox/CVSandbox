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
 * Contrast stretching (or as it is often called normalization)
 * is a simple image enhancement technique that attempts to improve
 * the contrast in an image by 'stretching' the range of intensity
 * values it contains to span a desired range of values, e.g. the
 * full range of pixel values that the image type concerned allows.
 * It differs from the more sophisticated histogram equalization
 * histogram equalization in that it can only apply a linear scaling
 * function to the image pixel values.
 * --------------------------------------
 */

// forward declaration ----
static void ContrastStretching8( ximage* src );
static void ContrastStretching24( ximage* src );
// ------------------------

// Contrast stretching image processing filter
XErrorCode ContrastStretching( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->format )
        {
        case XPixelFormatGrayscale8:
            ContrastStretching8( src );
            break;

        case XPixelFormatRGB24:
        case XPixelFormatRGBA32:
            ContrastStretching24( src );
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }
    }

    return ret;
}

// Contrast stretching image processing filter for 8 bpp grayscale images
void ContrastStretching8( ximage* src )
{
    int width  = src->width;
    int height = src->height;
    int offset = src->stride - width;
    int x, y;

    uint8_t  min = 255;
    uint8_t  max = 0;
    uint8_t  value;
    uint8_t* ptr = src->data;

    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++, ptr++ )
        {
            value = *ptr;

            if ( value < min )
            {
                min = value;
            }
            else if ( value > max )
            {
                max = value;
            }
        }

        ptr += offset;
    }

    if ( ( min != 0 ) || ( max != 255 ) )
    {
        LevelsLinearGrayscale( src, min, max, 0, 255 );
    }
}

// Contrast stretching image processing filter for 24/32 bpp colour images
void ContrastStretching24( ximage* src )
{
    int width     = src->width;
    int height    = src->height;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int offset    = src->stride - width * pixelSize;
    int x, y;

    uint8_t minR = 255;
    uint8_t maxR = 0;
    uint8_t minG = 255;
    uint8_t maxG = 0;
    uint8_t minB = 255;
    uint8_t maxB = 0;
    uint8_t value;

    uint8_t* ptr = src->data;

    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++ )
        {
            // red component
            value = ptr[RedIndex];

            if ( value < minR )
            {
                minR = value;
            }
            else if ( value > maxR )
            {
                maxR = value;
            }

            // green component
            value = ptr[GreenIndex];

            if ( value < minG )
            {
                minG = value;
            }
            else if ( value > maxG )
            {
                maxG = value;
            }

            // blue component
            value = ptr[BlueIndex];

            if ( value < minB )
            {
                minB = value;
            }
            else if ( value > maxB )
            {
                maxB = value;
            }

            ptr += pixelSize;
        }

        ptr += offset;
    }

    if ( ( minR != 0 ) || ( maxR != 255 ) || ( minG != 0 ) || ( maxG != 255 ) || ( minB != 0 ) || ( maxB != 255 ) )
    {
        LevelsLinear( src, minR, maxR, 0, 255, minG, maxG, 0, 255, minB, maxB, 0, 255 );
    }
}
