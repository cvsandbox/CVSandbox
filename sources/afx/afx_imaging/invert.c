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
static void InvertImage8or24pp( uint8_t* ptr, int width, int height, int stride, int pixelSize );
static void InvertImage16pp( uint8_t* ptr, int width, int height, int stride );
static void InvertImage32pp( uint8_t* ptr, int width, int height, int stride );
static void InvertImage48pp( uint8_t* ptr, int width, int height, int stride, int pixelSize );
// ------------------------

// Inverts the specified image
XErrorCode InvertImage( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int width    = src->width;
        int height   = src->height;
        int stride   = src->stride;
        uint8_t* ptr = src->data;

        switch ( src->format )
        {
        case XPixelFormatGrayscale8:
        case XPixelFormatRGB24:
            InvertImage8or24pp( ptr, width, height, stride, ( src->format == XPixelFormatRGB24 ) ? 3 : 1 );
            break;

        case XPixelFormatRGBA32:
            InvertImage32pp( ptr, width, height, stride );
            break;

        case XPixelFormatGrayscale16:
            InvertImage16pp( ptr, width, height, stride );
            break;

        case XPixelFormatRGB48:
        case XPixelFormatRGBA64:
            InvertImage48pp( ptr, width, height, stride, ( src->format == XPixelFormatRGB48 ) ? 3 : 4 );
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }
    }

    return ret;
}

// 8 bpp grayscale and 24 bpp RGB images case
static void InvertImage8or24pp( uint8_t* ptr, int width, int height, int stride, int pixelSize )
{
    int y;
    int len   = width * pixelSize;
    int packs = len / 4;
    int rem   = len % 4;

    #pragma omp parallel for schedule(static) shared( ptr, stride, packs, rem )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* row = ptr + y * stride;
        int      x;

        for ( x = 0; x < packs; x++, row += 4 )
        {
            *( (uint32_t*) row ) = ~( *( (uint32_t*) row ) );
        }

        for ( x = 0; x < rem; x++, row++ )
        {
            *row = ~( *row );
        }
    }
}

// 16 bpp grayscale images case
static void InvertImage16pp( uint8_t* ptr, int width, int height, int stride )
{
    int y;

    #pragma omp parallel for schedule(static) shared( ptr, width, stride )
    for ( y = 0; y < height; y++ )
    {
        uint16_t* row = (uint16_t*) ( ptr + y * stride );
        int x;

        for ( x = 0; x < width; x++, row++ )
        {
            *row = ~( *row );
        }
    }
}

// 32 bpp color images case
static void InvertImage32pp( uint8_t* ptr, int width, int height, int stride )
{
    int y;

    #pragma omp parallel for schedule(static) shared( ptr, width, stride )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* row = ptr + y * stride;
        int x;

        for ( x = 0; x < width; x++, row += 4 )
        {
            row[RedIndex]   = ~row[RedIndex];
            row[GreenIndex] = ~row[GreenIndex];
            row[BlueIndex]  = ~row[BlueIndex];
        }
    }
}

// 48/64 bpp color images case
static void InvertImage48pp( uint8_t* ptr, int width, int height, int stride, int pixelSize )
{
    int y;

    #pragma omp parallel for schedule(static) shared( ptr, width, stride, pixelSize )
    for ( y = 0; y < height; y++ )
    {
        uint16_t* row = (uint16_t*) ( ptr + y * stride );
        int x;

        for ( x = 0; x < width; x++, row += pixelSize )
        {
            row[RedIndex]   = ~row[RedIndex];
            row[GreenIndex] = ~row[GreenIndex];
            row[BlueIndex]  = ~row[BlueIndex];
        }
    }
}
