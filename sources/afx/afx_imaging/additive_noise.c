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

// Add uniform additive noise to the specified image
XErrorCode UniformAdditiveNoise( ximage* src, uint32_t seed, uint8_t amplitude )
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
        int32_t     noiseAmplitude = amplitude;
        int32_t     randMax = noiseAmplitude * 2 + 1;
        int32_t     value;
        int         width  = src->width;
        int         height = src->height;
        int         stride = src->stride;
        int         x, y;
        uint8_t*    ptr = src->data;
        uint8_t*    row;

        // initialize random number generator
        srand( seed );

        if ( src->format == XPixelFormatGrayscale8 )
        {
            for ( y = 0; y < height; y++ )
            {
                row = ptr + y * stride;

                for ( x = 0; x < width; x++ )
                {
                    value = ( rand( ) % randMax ) - noiseAmplitude + *row;
                    *row = (uint8_t) XINRANGE( value, 0, 255 );
                    row++;
                }
            }
        }
        else
        {
            int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;

            for ( y = 0; y < height; y++ )
            {
                row = ptr + y * stride;

                for ( x = 0; x < width; x++ )
                {
                    // red
                    value = ( rand( ) % randMax ) - noiseAmplitude + row[RedIndex];
                    row[RedIndex] = (uint8_t) XINRANGE( value, 0, 255 );
                    // green
                    value = ( rand( ) % randMax ) - noiseAmplitude + row[GreenIndex];
                    row[GreenIndex] = (uint8_t) XINRANGE( value, 0, 255 );
                    // blue
                    value = ( rand( ) % randMax ) - noiseAmplitude + row[BlueIndex];
                    row[BlueIndex] = (uint8_t) XINRANGE( value, 0, 255 );

                    row += pixelSize;
                }
            }
        }
    }

    return ret;
}
