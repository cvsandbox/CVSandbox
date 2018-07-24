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

// Algorithm:
// --------------------------------------
// 1) RGB -> YIQ
//  Y = 0.299 * R + 0.587 * G + 0.114 * B
//  I = 0.596 * R - 0.274 * G - 0.322 * B
//  Q = 0.212 * R - 0.523 * G + 0.311 * B
//
// 2) update
//  I = 51
//  Q = 0
//
// 3) YIQ -> RGB
//  R = 1.0 * Y + 0.956 * I + 0.621 * Q
//  G = 1.0 * Y - 0.272 * I - 0.647 * Q
//  B = 1.0 * Y - 1.105 * I + 1.702 * Q
// --------------------------------------

// Turn RGB color image into sepia colors (brown colors which create effect of an old picture)
XErrorCode MakeSepiaImage( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( ( src->format != XPixelFormatRGB24 ) &&
             ( src->format != XPixelFormatRGBA32 ) )
        {
            ret = ErrorUnsupportedPixelFormat;
        }
        else
        {
            uint8_t* ptr  = src->data;
            int width     = src->width;
            int height    = src->height;
            int stride    = src->stride;
            int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
            int y;

            #pragma omp parallel for schedule(static) shared( ptr, width, stride, pixelSize )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row = ptr + y * stride;
                int x, t;

                for ( x = 0; x < width; x++, row += pixelSize )
                {
                    t = (int) ( 0.299 * row[RedIndex] + 0.587 * row[GreenIndex] + 0.114 * row[BlueIndex] );

                    // red
                    row[RedIndex]   = (uint8_t) ( ( t > 206 ) ? 255 : t + 49 );
                    // green
                    row[GreenIndex] = (uint8_t) ( ( t < 14 ) ? 0 : t - 14 );
                    // blue
                    row[BlueIndex]  = (uint8_t) ( ( t < 56 ) ? 0 : t - 56 );
                }
            }
        }
    }

    return ret;
}
