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

// Rotates RGB components, r <- g <- b <- r
XErrorCode RotateRGBChannels( ximage* src )
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

            for ( y = 0; y < height; y++ )
            {
                uint8_t* row = ptr + y * stride;
                uint8_t t;
                int x;

                for ( x = 0; x < width; x++, row += pixelSize )
                {
                    t = row[RedIndex];

                    // red
                    row[RedIndex]   = row[GreenIndex];
                    // green
                    row[GreenIndex] = row[BlueIndex];
                    // blue
                    row[BlueIndex]  = t;
                }
            }
        }
    }

    return ret;
}
