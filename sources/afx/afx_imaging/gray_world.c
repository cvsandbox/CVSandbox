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

// Performs gray world normalization filter. It calculate first mean values of each RGB channel and a global mean.
// Then it updates all pixel value multiplying them with next coefficients: mean/redMean, mean/greenMean, mean/blueMean
XErrorCode GrayWorldNormalization( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatRGB24 ) && ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width     = src->width;
        int height    = src->height;
        int area      = width * height;
        int stride    = src->stride;
        int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
        int x, y, i;

        uint8_t* base = src->data;

        double rmean, gmean, bmean, mean;
        float  kr, kg, kb;

        uint8_t redMap[256];
        uint8_t greenMap[256];
        uint8_t blueMap[256];

        if ( area < 0xFFFFFF )
        {
            uint32_t rsum = 0;
            uint32_t gsum = 0;
            uint32_t bsum = 0;

            for ( y = 0; y < height; y++ )
            {
                uint8_t* ptr = base + y * stride;

                for ( x = 0; x < width; x++, ptr += pixelSize )
                {
                    rsum += ptr[RedIndex];
                    gsum += ptr[GreenIndex];
                    bsum += ptr[BlueIndex];
                }
            }

            rmean = (double) rsum / area;
            gmean = (double) gsum / area;
            bmean = (double) bsum / area;
        }
        else
        {
            double rsum = 0;
            double gsum = 0;
            double bsum = 0;

            for ( y = 0; y < height; y++ )
            {
                uint8_t* ptr = base + y * stride;

                for ( x = 0; x < width; x++, ptr += pixelSize )
                {
                    rsum += ptr[RedIndex];
                    gsum += ptr[GreenIndex];
                    bsum += ptr[BlueIndex];
                }
            }

            rmean = rsum / area;
            gmean = gsum / area;
            bmean = bsum / area;
        }

        mean = ( rmean + gmean + bmean ) / 3;

        kr = (float) ( mean / rmean );
        kg = (float) ( mean / gmean );
        kb = (float) ( mean / bmean );

        // calculate color maps
        for ( i = 0; i < 256; i++ )
        {
            int r = (int) ( kr * i );
            int g = (int) ( kg * i );
            int b = (int) ( kb * i );

            redMap[i]   = ( r > 255 ) ? 255 : (uint8_t) r;
            greenMap[i] = ( g > 255 ) ? 255 : (uint8_t) g;
            blueMap[i]  = ( b > 255 ) ? 255 : (uint8_t) b;
        }

        // finally do the remapping
        ret = ColorRemapping( src, redMap, greenMap, blueMap );
    }

    return ret;
}
