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

#include "ximaging_effects.h"

// Shift hue of all pixels by the specified value, which looks like hue rotation
XErrorCode RotateImageHue( ximage* src, uint16_t hueAngle )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatRGB24 ) &&
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

        // make sure hue angle value is in the correct range
        hueAngle = hueAngle % 360;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride, pixelSize, hueAngle )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            int      x;
            xargb    rgb;
            xhsl     hsl;

            for ( x = 0; x < width; x++, row += pixelSize )
            {
                rgb.components.r = row[RedIndex];
                rgb.components.g = row[GreenIndex];
                rgb.components.b = row[BlueIndex];

                Rgb2Hsl( &rgb, &hsl );

                hsl.Hue += hueAngle;
                hsl.Hue %= 360;

                Hsl2Rgb( &hsl, &rgb );

                row[RedIndex]   = rgb.components.r;
                row[GreenIndex] = rgb.components.g;
                row[BlueIndex]  = rgb.components.b;
            }
        }
    }

    return ret;
}
