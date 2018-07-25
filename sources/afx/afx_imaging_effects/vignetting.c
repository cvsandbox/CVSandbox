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

#include <math.h>
#include "ximaging_effects.h"

// forward declaration ----
static void MakeVignetteImage8( const ximage* src, float startWidthFactor, float endWidthFactor );
static void MakeVignetteImage24BrightnessOnly( const ximage* src, float startWidthFactor, float endWidthFactor );
static void MakeVignetteImage24IncludingSaturation( const ximage* src, float startWidthFactor, float endWidthFactor, bool decreaseBrightness );
// ------------------------

// Create vignetting effect on the specified image
XErrorCode MakeVignetteImage( ximage* src, float startWidthFactor, float endWidthFactor, bool decreaseBrightness, bool decreaseSaturation )
{
    XErrorCode ret = SuccessCode;

    if ( startWidthFactor > endWidthFactor )
    {
        endWidthFactor = startWidthFactor;
    }

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( ( decreaseBrightness ) || ( decreaseSaturation ) )
        {
            if ( src->format == XPixelFormatGrayscale8 )
            {
                // we care only about light in grayscale images, since those are desaturated by definition
                if ( decreaseBrightness )
                {
                    MakeVignetteImage8( src, startWidthFactor, endWidthFactor );
                }
            }
            else if ( ( src->format == XPixelFormatRGB24 ) ||
                      ( src->format == XPixelFormatRGBA32 ) )
            {
                if ( decreaseSaturation )
                {
                    // use the version which does RGB<->HSL conversion
                    MakeVignetteImage24IncludingSaturation( src, startWidthFactor, endWidthFactor, decreaseBrightness );
                }
                else
                {
                    // since saturation is not affected, use RGB version
                    MakeVignetteImage24BrightnessOnly( src, startWidthFactor, endWidthFactor );
                }
            }
            else
            {
                ret = ErrorUnsupportedPixelFormat;
            }
        }
    }

    return ret;
}

// Create vignetting effect on the specified 8bpp grayscale image
void MakeVignetteImage8( const ximage* src, float startWidthFactor, float endWidthFactor )
{
    int   width         = src->width;
    int   height        = src->height;
    int   srcStride     = src->stride;
    float halfWidth     = (float) ( width  - 1 ) / 2;
    float halfHeight    = (float) ( height - 1 ) / 2;
    float startRadius   = halfWidth * startWidthFactor;
    float startRadiusSq = startRadius * startRadius;
    float endRadius     = halfWidth * endWidthFactor;
    float endRadiusSq   = endRadius * endRadius;
    float radiusDelta   = endRadius - startRadius;
    int   y;

    uint8_t* srcPtr  = src->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, width, srcStride, halfWidth, halfHeight, startRadiusSq, endRadiusSq, startRadius, radiusDelta )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        float    dy     = (float) y - halfHeight;
        float    dy2    = dy * dy;

        float    dx, distance, distanceSq;
        int      x;

        for ( x = 0; x < width; x++ )
        {
            dx = (float) x - halfWidth;
            distanceSq = dx * dx + dy2;

            if ( distanceSq > startRadiusSq )
            {
                if ( distanceSq > endRadiusSq )
                {
                    *srcRow = 0;
                }
                else
                {
                    distance = (float) sqrt( distanceSq );

                    *srcRow = (uint8_t) ( ( 1.0f - ( distance - startRadius ) / radiusDelta ) * *srcRow );
                }
            }

            srcRow++;
        }
    }
}

// Create vignetting effect on the specified 24/32 color image - decrease light only
void MakeVignetteImage24BrightnessOnly( const ximage* src, float startWidthFactor, float endWidthFactor )
{
    int   width         = src->width;
    int   height        = src->height;
    int   srcStride     = src->stride;
    int   pixelSize     = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    float halfWidth     = (float) ( width  - 1 ) / 2;
    float halfHeight    = (float) ( height - 1 ) / 2;
    float startRadius   = halfWidth * startWidthFactor;
    float startRadiusSq = startRadius * startRadius;
    float endRadius     = halfWidth * endWidthFactor;
    float endRadiusSq   = endRadius * endRadius;
    float radiusDelta   = endRadius - startRadius;
    int   y;

    uint8_t* srcPtr  = src->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, width, srcStride, halfWidth, halfHeight, startRadiusSq, endRadiusSq, startRadius, radiusDelta )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        float    dy     = (float) y - halfHeight;
        float    dy2    = dy * dy;

        float    dx, distance, distanceSq, changeFactor;
        int      x;

        for ( x = 0; x < width; x++ )
        {
            dx = (float) x - halfWidth;
            distanceSq = dx * dx + dy2;

            if ( distanceSq > startRadiusSq )
            {
                if ( distanceSq > endRadiusSq )
                {
                    srcRow[RedIndex]   = 0;
                    srcRow[GreenIndex] = 0;
                    srcRow[BlueIndex]  = 0;
                }
                else
                {
                    distance = (float) sqrt( distanceSq );

                    changeFactor = 1.0f - ( distance - startRadius ) / radiusDelta;

                    srcRow[RedIndex]   = (uint8_t) ( changeFactor * srcRow[RedIndex]   );
                    srcRow[GreenIndex] = (uint8_t) ( changeFactor * srcRow[GreenIndex] );
                    srcRow[BlueIndex]  = (uint8_t) ( changeFactor * srcRow[BlueIndex]  );
                }
            }

            srcRow += pixelSize;
        }
    }
}

// Create vignetting effect on the specified 24/32 color image - saturation is always decreased
void MakeVignetteImage24IncludingSaturation( const ximage* src, float startWidthFactor, float endWidthFactor, bool decreaseBrightness )
{
    int   width         = src->width;
    int   height        = src->height;
    int   srcStride     = src->stride;
    int   pixelSize     = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    float halfWidth     = (float) ( width  - 1 ) / 2;
    float halfHeight    = (float) ( height - 1 ) / 2;
    float startRadius   = halfWidth * startWidthFactor;
    float startRadiusSq = startRadius * startRadius;
    float endRadius     = halfWidth * endWidthFactor;
    float endRadiusSq   = endRadius * endRadius;
    float radiusDelta   = endRadius - startRadius;
    int   y;

    uint8_t* srcPtr  = src->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, width, srcStride, halfWidth, halfHeight, startRadiusSq, endRadiusSq, startRadius, radiusDelta, decreaseBrightness )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        float    dy     = (float) y - halfHeight;
        float    dy2    = dy * dy;

        float    dx, distance, distanceSq, changeFactor;
        int      x;
        xargb    rgb;
        xhsv     hsv;

        for ( x = 0; x < width; x++ )
        {
            dx = (float) x - halfWidth;
            distanceSq = dx * dx + dy2;

            if ( distanceSq > startRadiusSq )
            {
                rgb.components.r = srcRow[RedIndex];
                rgb.components.g = srcRow[GreenIndex];
                rgb.components.b = srcRow[BlueIndex];

                Rgb2Hsv( &rgb, &hsv );

                if ( distanceSq > endRadiusSq )
                {
                    if ( decreaseBrightness )
                    {
                        hsv.Value = 0.0f;
                    }
                    hsv.Saturation = 0.0f;
                }
                else
                {
                    distance = (float) sqrt( distanceSq );

                    changeFactor = 1.0f - ( distance - startRadius ) / radiusDelta;

                    if ( decreaseBrightness )
                    {
                        hsv.Value *= changeFactor;
                    }
                    hsv.Saturation *= changeFactor;
                }

                Hsv2Rgb( &hsv, &rgb );

                srcRow[RedIndex]   = rgb.components.r;
                srcRow[GreenIndex] = rgb.components.g;
                srcRow[BlueIndex]  = rgb.components.b;
            }

            srcRow += pixelSize;
        }
    }
}
