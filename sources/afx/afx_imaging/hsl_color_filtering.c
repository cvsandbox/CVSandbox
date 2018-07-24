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

// Remove colors outside/inside of the specified HSL range
XErrorCode HslColorFiltering( ximage* src, xhsl minValues, xhsl maxValues, bool fillOutside, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    if ( ( src->format != XPixelFormatRGB24 ) && ( src->format != XPixelFormatRGBA32 ) )
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

        uint16_t hueMin        = minValues.Hue % 360;
        uint16_t hueMax        = maxValues.Hue % 360;
        float    saturationMin = minValues.Saturation;
        float    saturationMax = maxValues.Saturation;
        float    luminanceMin  = minValues.Luminance;
        float    luminanceMax  = maxValues.Luminance;

        int y;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride, pixelSize, hueMin, hueMax, saturationMin, saturationMax, luminanceMin, luminanceMax, fillOutside, fillColor  )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            int      x;
            xargb    rgb;
            xhsl     hsl;
            bool     updatePixel;

            for ( x = 0; x < width; x++, row += pixelSize )
            {
                updatePixel = false;

                rgb.components.r = row[RedIndex];
                rgb.components.g = row[GreenIndex];
                rgb.components.b = row[BlueIndex];

                Rgb2Hsl( &rgb, &hsl );

                if (
                    ( hsl.Saturation >= saturationMin ) && ( hsl.Saturation <= saturationMax ) &&
                    ( hsl.Luminance >= luminanceMin ) && ( hsl.Luminance <= luminanceMax ) &&
                    (
                        ( ( hueMin < hueMax ) &&   ( hsl.Hue >= hueMin ) && ( hsl.Hue <= hueMax ) ) ||
                        ( ( hueMin > hueMax ) && ( ( hsl.Hue >= hueMin ) || ( hsl.Hue <= hueMax ) ) )
                    )
                   )
                {
                    updatePixel = ( fillOutside == false );
                }
                else
                {
                    updatePixel = ( fillOutside == true );
                }

                if ( updatePixel )
                {
                    row[RedIndex]   = fillColor.components.r;
                    row[GreenIndex] = fillColor.components.g;
                    row[BlueIndex]  = fillColor.components.b;

                    if ( pixelSize == 4 )
                    {
                        row[AlphaIndex] = fillColor.components.a;
                    }
                }
            }
        }
    }

    return ret;
}

// Remove colors outside/inside of the specified HSV range
XErrorCode HsvColorFiltering( ximage* src, xhsv minValues, xhsv maxValues, bool fillOutside, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    if ( ( src->format != XPixelFormatRGB24 ) && ( src->format != XPixelFormatRGBA32 ) )
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

        uint16_t hueMin        = minValues.Hue % 360;
        uint16_t hueMax        = maxValues.Hue % 360;
        float    saturationMin = minValues.Saturation;
        float    saturationMax = maxValues.Saturation;
        float    valueMin      = minValues.Value;
        float    valueMax      = maxValues.Value;

        int y;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride, pixelSize, hueMin, hueMax, saturationMin, saturationMax, valueMin, valueMax, fillOutside, fillColor  )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            int      x;
            xargb    rgb;
            xhsv     hsv;
            int      updatePixel;

            for ( x = 0; x < width; x++, row += pixelSize )
            {
                updatePixel = 0;

                rgb.components.r = row[RedIndex];
                rgb.components.g = row[GreenIndex];
                rgb.components.b = row[BlueIndex];

                Rgb2Hsv( &rgb, &hsv );

                if (
                    ( hsv.Saturation >= saturationMin ) && ( hsv.Saturation <= saturationMax ) &&
                    ( hsv.Value >= valueMin ) && ( hsv.Value <= valueMax ) &&
                    (
                        ( ( hueMin < hueMax ) &&   ( hsv.Hue >= hueMin ) && ( hsv.Hue <= hueMax ) ) ||
                        ( ( hueMin > hueMax ) && ( ( hsv.Hue >= hueMin ) || ( hsv.Hue <= hueMax ) ) )
                    )
                   )
                {
                    updatePixel = ( fillOutside == false );
                }
                else
                {
                    updatePixel = ( fillOutside == true );
                }

                if ( updatePixel )
                {
                    row[RedIndex]   = fillColor.components.r;
                    row[GreenIndex] = fillColor.components.g;
                    row[BlueIndex]  = fillColor.components.b;

                    if ( pixelSize == 4 )
                    {
                        row[AlphaIndex] = fillColor.components.a;
                    }
                }
            }
        }
    }

    return ret;
}
