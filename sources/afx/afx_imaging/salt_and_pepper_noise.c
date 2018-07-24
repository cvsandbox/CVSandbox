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

#include <time.h>
#include "ximaging.h"

// Add salt-and-pepper noise to the specified image
XErrorCode SaltAndPepperNoise( ximage* src, uint32_t seed, float noiseAmount, uint8_t pepperValue, uint8_t saltValue )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int         noiseThreshold = (int) ( XINRANGE( noiseAmount, 0.0f, 100.0f ) * 100 );
        int         width  = src->width;
        int         height = src->height;
        int         stride = src->stride;
        int         x, y;
        uint8_t*    ptr = src->data;
        uint8_t*    row;

        uint8_t     noiseValues[2];

        noiseValues[0] = pepperValue;
        noiseValues[1] = saltValue;

        // initialize random number generator
        srand( seed );

        if ( src->format == XPixelFormatGrayscale8 )
        {
            for ( y = 0; y < height; y++ )
            {
                row = ptr + y * stride;

                for ( x = 0; x < width; x++ )
                {
                    if ( ( rand( ) % 10000 ) < noiseThreshold )
                    {
                        *row = noiseValues[rand( ) & 1];
                    }

                    row++;
                }
            }
        }
        else
        {
            uint32_t pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;

            for ( y = 0; y < height; y++ )
            {
                row = ptr + y * stride;

                for ( x = 0; x < width; x++ )
                {
                    if ( ( rand( ) % 10000 ) < noiseThreshold )
                    {
                        row[rand( ) % 3] = noiseValues[rand( ) & 1];
                    }

                    row += pixelSize;
                }
            }
        }
    }

    return ret;
}
