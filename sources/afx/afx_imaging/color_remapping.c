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
#include <memory.h>

// ---> Heat gradient color map
static const uint8_t heatGradientRedMap[256] =
{
      0,   7,  14,  21,  28,  35,  42,  49,  56,  63,  70,  77,  84,  91,  98, 105,
    112, 119, 126, 133, 140, 147, 154, 161, 168, 175, 182, 189, 196, 203, 210, 217,
    224, 231, 239, 247, 255, 247, 239, 231, 224, 217, 210, 203, 196, 189, 182, 175,
    168, 161, 154, 147, 140, 133, 126, 119, 112, 105,  98,  91,  84,  77,  70,  63,
     56,  49,  42,  35,  28,  21,  14,   7,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   7,  14,  21,  28,  35,  42,  49,  56,  63,  70,  77,  84,  91,  98, 105,
    112, 119, 126, 133, 140, 147, 154, 161, 168, 175, 182, 189, 196, 203, 210, 217,
    224, 231, 239, 247, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
};
static const uint8_t heatGradientGreenMap[256] =
{
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   7,  14,  21,  28,  35,  42,  49,
     56,  63,  70,  77,  84,  91,  98, 105, 112, 119, 126, 133, 140, 147, 154, 161,
    168, 175, 182, 189, 196, 203, 210, 217, 224, 231, 239, 247, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 247, 239, 231, 224, 217, 210, 203, 196, 189, 182, 175,
    168, 161, 154, 147, 140, 133, 126, 119, 112, 105,  98,  91,  84,  77,  70,  63,
     56,  49,  42,  35,  28,  21,  14,   7,   0,   7,  14,  21,  28,  35,  42,  49,
     56,  63,  70,  77,  84,  91,  98, 105, 112, 119, 126, 133, 140, 147, 153, 159,
    165, 171, 177, 183, 189, 195, 201, 207, 213, 219, 225, 231, 237, 243, 249, 255
};
static const uint8_t heatGradientBlueMap[256] =
{
      0,   7,  14,  21,  28,  35,  42,  49,  56,  63,  70,  77,  84,  91,  98, 105,
    112, 119, 126, 133, 140, 147, 154, 161, 168, 175, 182, 189, 196, 203, 210, 217,
    224, 231, 239, 247, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 247, 239, 231,
    224, 217, 210, 203, 196, 189, 182, 175, 168, 161, 154, 147, 140, 133, 126, 119,
    112, 105,  98,  91,  84,  77,  70,  63,  56,  49,  42,  35,  28,  21,  14,   7,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   7,  14,  21,  28,  35,  42,  49,
     56,  63,  70,  77,  84,  91,  98, 105, 112, 119, 126, 133, 140, 147, 153, 159,
    165, 171, 177, 183, 189, 195, 201, 207, 213, 219, 225, 231, 237, 243, 249, 255
};
// <---


// Re-map values of RGB planes in 24/32 bpp color image
XErrorCode ColorRemapping( ximage* src, const uint8_t* redMap, const uint8_t* greenMap, const uint8_t* blueMap )
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
        int      width     = src->width;
        int      pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
        int      stride    = src->stride;
        int      y, height = src->height;
        uint8_t* ptr       = src->data;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride, pixelSize, redMap, greenMap, blueMap )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            int x;

            for ( x = 0; x < width; x++, row += pixelSize )
            {
                row[RedIndex]   = redMap  [row[RedIndex]];
                row[GreenIndex] = greenMap[row[GreenIndex]];
                row[BlueIndex]  = blueMap [row[BlueIndex]];
            }
        }
    }

    return ret;
}

// Re-map values of 8bpp grayscale image
XErrorCode GrayscaleRemapping( ximage* src, const uint8_t* map )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width     = src->width;
        int      stride    = src->stride;
        int      y, height = src->height;
        uint8_t* ptr       = src->data;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride, map )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            int x;

            for ( x = 0; x < width; x++, row++ )
            {
                *row = map[*row];
            }
        }
    }

    return ret;
}

// Re-map 8bpp grayscale color image to 24 bpp color image
XErrorCode GrayscaleRemappingToRGB( const ximage* src, ximage* dst, const uint8_t* redMap, const uint8_t* greenMap, const uint8_t* blueMap )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) ||
            ( ( dst->format != XPixelFormatRGB24 ) && ( dst->format != XPixelFormatRGBA32 ) ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int      width      = src->width;
        int      srcStride  = src->stride;
        int      dstStride  = dst->stride;
        int      y, height  = src->height;
        uint8_t* srcPtr     = src->data;
        uint8_t* dstPtr     = dst->data;

        if ( dst->format == XPixelFormatRGB24 )
        {
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, redMap, greenMap, blueMap )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* srcRow = srcPtr + y * srcStride;
                uint8_t* dstRow = dstPtr + y * dstStride;
                uint8_t  srcValue;
                int      x;

                for ( x = 0; x < width; x++ )
                {
                    srcValue = *srcRow;

                    dstRow[RedIndex]   = redMap[srcValue];
                    dstRow[GreenIndex] = greenMap[srcValue];
                    dstRow[BlueIndex]  = blueMap[srcValue];

                    srcRow++;
                    dstRow += 3;
                }
            }
        }
        else
        {
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, redMap, greenMap, blueMap )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* srcRow = srcPtr + y * srcStride;
                uint8_t* dstRow = dstPtr + y * dstStride;
                uint8_t  srcValue;
                int      x;

                for ( x = 0; x < width; x++ )
                {
                    srcValue = *srcRow;

                    dstRow[RedIndex]   = redMap[srcValue];
                    dstRow[GreenIndex] = greenMap[srcValue];
                    dstRow[BlueIndex]  = blueMap[srcValue];
                    dstRow[AlphaIndex] = 255;

                    srcRow++;
                    dstRow += 4;
                }
            }
        }
    }

    return ret;
}

// Re-color 8bpp grayscale image into 24 bpp color image by mapping grayscale values to gradient between the specified two colors
XErrorCode GradientGrayscaleReColoring( const ximage* src, ximage* dst, xargb startColor, xargb endColor )
{
    uint8_t redMap[256];
    uint8_t greenMap[256];
    uint8_t blueMap[256];

    CalculateLinearMap( redMap,   0, 255, startColor.components.r, endColor.components.r );
    CalculateLinearMap( greenMap, 0, 255, startColor.components.g, endColor.components.g );
    CalculateLinearMap( blueMap,  0, 255, startColor.components.b, endColor.components.b );

    return GrayscaleRemappingToRGB( src, dst, redMap, greenMap, blueMap );
}

// Re-color 8bpp grayscale image into 24 bpp color image by mapping grayscale values to two gradients between the specified three colors
XErrorCode GradientGrayscaleReColoring2( const ximage* src, ximage* dst, xargb startColor, xargb middleColor, xargb endColor, uint8_t threshold )
{
    uint8_t redMap[256];
    uint8_t greenMap[256];
    uint8_t blueMap[256];

    // 1st gradient
    CalculateLinearMapInInputRange( redMap,   0, threshold, startColor.components.r, middleColor.components.r );
    CalculateLinearMapInInputRange( greenMap, 0, threshold, startColor.components.g, middleColor.components.g );
    CalculateLinearMapInInputRange( blueMap,  0, threshold, startColor.components.b, middleColor.components.b );

    // 2nd gradient
    if ( threshold != 255 )
    {
        CalculateLinearMapInInputRange( redMap,   threshold, 255, middleColor.components.r, endColor.components.r );
        CalculateLinearMapInInputRange( greenMap, threshold, 255, middleColor.components.g, endColor.components.g );
        CalculateLinearMapInInputRange( blueMap,  threshold, 255, middleColor.components.b, endColor.components.b );
    }

    return GrayscaleRemappingToRGB( src, dst, redMap, greenMap, blueMap );
}

// Re-color 8bpp grayscale image into 24 bpp color image by mapping grayscale values to four gradients between the specified five colors
XErrorCode GradientGrayscaleReColoring4( const ximage* src, ximage* dst,
                xargb startColor, xargb middleColor1, xargb middleColor2, xargb middleColor3, xargb endColor,
                uint8_t threshold1, uint8_t threshold2, uint8_t threshold3 )
{
    uint8_t redMap[256];
    uint8_t greenMap[256];
    uint8_t blueMap[256];

    // 1st gradient
    CalculateLinearMapInInputRange( redMap,   0, threshold1, startColor.components.r, middleColor1.components.r );
    CalculateLinearMapInInputRange( greenMap, 0, threshold1, startColor.components.g, middleColor1.components.g );
    CalculateLinearMapInInputRange( blueMap,  0, threshold1, startColor.components.b, middleColor1.components.b );

    // 2nd gradient
    if ( ( threshold1 != 255 ) && ( threshold1 != threshold2 ) )
    {
        CalculateLinearMapInInputRange( redMap,   threshold1, threshold2, middleColor1.components.r, middleColor2.components.r );
        CalculateLinearMapInInputRange( greenMap, threshold1, threshold2, middleColor1.components.g, middleColor2.components.g );
        CalculateLinearMapInInputRange( blueMap,  threshold1, threshold2, middleColor1.components.b, middleColor2.components.b );
    }

    // 3rd gradient
    if ( ( threshold2 != 255 ) && ( threshold2 != threshold3 ) )
    {
        CalculateLinearMapInInputRange( redMap,   threshold2, threshold3, middleColor2.components.r, middleColor3.components.r );
        CalculateLinearMapInInputRange( greenMap, threshold2, threshold3, middleColor2.components.g, middleColor3.components.g );
        CalculateLinearMapInInputRange( blueMap,  threshold2, threshold3, middleColor2.components.b, middleColor3.components.b );
    }

    // 4th gradient
    if ( threshold3 != 255 )
    {
        CalculateLinearMapInInputRange( redMap,   threshold3, 255, middleColor3.components.r, endColor.components.r );
        CalculateLinearMapInInputRange( greenMap, threshold3, 255, middleColor3.components.g, endColor.components.g );
        CalculateLinearMapInInputRange( blueMap,  threshold3, 255, middleColor3.components.b, endColor.components.b );
    }

    return GrayscaleRemappingToRGB( src, dst, redMap, greenMap, blueMap );
}

// Convert grayscale image into color image by applying heat color map
XErrorCode GrayscaleToHeatGradient( const ximage* src, ximage* dst )
{
    return GrayscaleRemappingToRGB( src, dst, heatGradientRedMap, heatGradientGreenMap, heatGradientBlueMap );
}

// Calculate heat gradient color map
XErrorCode CalculateHeatGradientColorMap( uint8_t* redMap, uint8_t* greenMap, uint8_t* blueMap )
{
    XErrorCode ret = SuccessCode;

    if ( ( redMap == 0 ) || ( greenMap == 0 ) || ( blueMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        memcpy( redMap  , heatGradientRedMap  , sizeof( heatGradientRedMap   ) );
        memcpy( greenMap, heatGradientGreenMap, sizeof( heatGradientGreenMap ) );
        memcpy( blueMap , heatGradientBlueMap , sizeof( heatGradientBlueMap  ) );
    }

    return ret;
}
