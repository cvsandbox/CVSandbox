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

// Performs simple posterization on the given image - replaces ranges of colors with a single value
XErrorCode SimplePosterization( ximage* src, uint8_t interval, XRangePoint fillType )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( interval < 2 ) || ( interval > 128 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        uint32_t    t, i;

        // calculate posterization offset
        uint32_t posterizationOffset = ( fillType == RangeMin ) ?
            0 : ( ( fillType == RangeMax ) ? interval - 1 : interval / 2 );

        // calculate mapping array
        uint8_t map[256];

        for ( i = 0; i < 256; i++ )
        {
            t = ( i / interval ) * interval + posterizationOffset;
            map[i] = (uint8_t) XMIN( 255,  t );
        }

        if ( src->format == XPixelFormatGrayscale8 )
        {
            GrayscaleRemapping( src, map );
        }
        else
        {
            ColorRemapping( src, map, map, map );
        }
    }

    return ret;
}
