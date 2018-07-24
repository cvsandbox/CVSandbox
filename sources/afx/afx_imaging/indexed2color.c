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

// forward declaration ----
static void Indexed1ToColor( const ximage* src, ximage* dst );
static void Indexed2ToColor( const ximage* src, ximage* dst );
static void Indexed4ToColor( const ximage* src, ximage* dst );
static void Indexed8ToColor( const ximage* src, ximage* dst );
// ------------------------

// Converts source indexed image to color 32 bpp image
XErrorCode IndexedToColor( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->palette == 0 ) || ( src->palette->colorsCount <= 0 ) || ( src->palette->values == 0 ) )
    {
        ret = ErrorImagePaletteIsMissing;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) || ( dst->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( src->format == XPixelFormatIndexed1 )
    {
        Indexed1ToColor( src, dst );
    }
    else if ( src->format == XPixelFormatIndexed2 )
    {
        Indexed2ToColor( src, dst );
    }
    else if ( src->format == XPixelFormatIndexed4 )
    {
        Indexed4ToColor( src, dst );
    }
    else if ( src->format == XPixelFormatIndexed8 )
    {
        Indexed8ToColor( src, dst );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// Converts 8 bpp indexed image to color 32 bpp image
void Indexed8ToColor( const ximage* src, ximage* dst )
{
    uint32_t width     = (uint32_t) src->width;
    int      height    = src->height;
    int      srcStride = src->stride;
    int      dstStride = dst->stride;
    int      y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;
    xargb*   colors = src->palette->values;
    uint32_t colorsCount = (uint32_t) src->palette->colorsCount;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, colors, colorsCount )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint32_t x;
        uint32_t colorIndex;
        xargb    color;

        for ( x = 0; x < width; x++ )
        {
            colorIndex = *srcRow;
            if ( colorIndex >= colorsCount )
            {
                colorIndex = 0;
            }

            color = colors[colorIndex];

            dstRow[RedIndex]   = color.components.r;
            dstRow[GreenIndex] = color.components.g;
            dstRow[BlueIndex]  = color.components.b;
            dstRow[AlphaIndex] = color.components.a;

            srcRow++;
            dstRow += 4;
        }
    }
}

// Converts 4 bpp indexed image to color 32 bpp image
void Indexed4ToColor( const ximage* src, ximage* dst )
{
    uint32_t width     = (uint32_t) src->width;
    int      height    = src->height;
    int      srcStride = src->stride;
    int      dstStride = dst->stride;
    int      y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;
    xargb*   colors = src->palette->values;
    uint32_t colorsCount = (uint32_t) src->palette->colorsCount;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, colors )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint32_t x;
        uint32_t isLowNibble;
        uint32_t colorIndex;
        xargb    color;

        for ( x = 0; x < width; x++ )
        {
            isLowNibble = x & 1;
            colorIndex = ( (*srcRow) >> ( ( 1 - isLowNibble ) << 2 ) ) & 0x0F;

            if ( colorIndex >= colorsCount )
            {
                colorIndex = 0;
            }

            color = colors[colorIndex];

            dstRow[RedIndex]   = color.components.r;
            dstRow[GreenIndex] = color.components.g;
            dstRow[BlueIndex]  = color.components.b;
            dstRow[AlphaIndex] = color.components.a;

            srcRow += isLowNibble;
            dstRow += 4;
        }
    }
}

// Converts 2 bpp indexed image to color 32 bpp image
void Indexed2ToColor( const ximage* src, ximage* dst )
{
    uint32_t width     = (uint32_t) src->width;
    int      height    = src->height;
    int      srcStride = src->stride;
    int      dstStride = dst->stride;
    int      y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;
    xargb*   colors = src->palette->values;
    uint32_t colorsCount = (uint32_t) src->palette->colorsCount;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, colors )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint32_t x;
        uint32_t shift;
        uint32_t colorIndex;
        xargb    color;

        for ( x = 0; x < width; x++ )
        {
            shift = ( 3 - ( x & 3 ) ) << 1;
            colorIndex = ( (*srcRow) >> shift ) & 3;

            if ( colorIndex >= colorsCount )
            {
                colorIndex = 0;
            }

            color = colors[colorIndex];

            dstRow[RedIndex]   = color.components.r;
            dstRow[GreenIndex] = color.components.g;
            dstRow[BlueIndex]  = color.components.b;
            dstRow[AlphaIndex] = color.components.a;

            if ( shift == 0 )
            {
                srcRow++;
            }
            dstRow += 4;
        }
    }
}

// Converts 1 bpp indexed image to color 32 bpp image
void Indexed1ToColor( const ximage* src, ximage* dst )
{
    uint32_t width     = (uint32_t) src->width;
    int      height    = src->height;
    int      srcStride = src->stride;
    int      dstStride = dst->stride;
    int      y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;
    xargb*   colors = src->palette->values;
    uint32_t colorsCount = (uint32_t) src->palette->colorsCount;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, colors )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint32_t x;
        uint32_t shift;
        uint32_t colorIndex;
        xargb    color;

        for ( x = 0; x < width; x++ )
        {
            shift = 7 - ( x & 7 );
            colorIndex = ( (*srcRow) >> shift ) & 1;

            if ( colorIndex >= colorsCount )
            {
                colorIndex = 0;
            }

            color = colors[colorIndex];

            dstRow[RedIndex]   = color.components.r;
            dstRow[GreenIndex] = color.components.g;
            dstRow[BlueIndex]  = color.components.b;
            dstRow[AlphaIndex] = color.components.a;

            if ( shift == 0 )
            {
                srcRow++;
            }
            dstRow += 4;
        }
    }
}
