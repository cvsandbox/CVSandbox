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
static void MirrorImage8X( ximage* src );
static void MirrorImage8Y( ximage* src );
static void MirrorImage8XY( ximage* src );

static void MirrorImage24X( ximage* src );
static void MirrorImage24Y( ximage* src );
static void MirrorImage24XY( ximage* src );

static void MirrorImage32X( ximage* src );
static void MirrorImage32Y( ximage* src );
static void MirrorImage32XY( ximage* src );
// ------------------------

// Mirror the specified image over X and/or Y axis
XErrorCode MirrorImage( ximage* src, bool xMirror, bool yMirror )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->format )
        {
        case XPixelFormatGrayscale8:
            if ( ( xMirror & yMirror ) != false )
            {
                MirrorImage8XY( src );
            }
            else if ( xMirror != false )
            {
                MirrorImage8X( src );
            }
            else if ( yMirror != false )
            {
                MirrorImage8Y( src );
            }

            break;

        case XPixelFormatRGB24:
            if ( ( xMirror & yMirror ) != false )
            {
                MirrorImage24XY( src );
            }
            else if ( xMirror != false )
            {
                MirrorImage24X( src );
            }
            else if ( yMirror != false )
            {
                MirrorImage24Y( src );
            }
            break;

        case XPixelFormatRGBA32:
            if ( ( xMirror & yMirror ) != false )
            {
                MirrorImage32XY( src );
            }
            else if ( xMirror != false )
            {
                MirrorImage32X( src );
            }
            else if ( yMirror != false )
            {
                MirrorImage32Y( src );
            }
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
        }
    }

    return ret;
}

// Mirror 8bpp grayscale image around Y axis
void MirrorImage8Y( ximage* src )
{
    int width     = src->width;
    int height    = src->height;
    int widthM1   = width - 1;
    int halfWidth = width / 2;
    int stride    = src->stride;
    int x, y;

    uint8_t* ptr = src->data;
    uint8_t* rowStart;
    uint8_t* rowEnd;
    uint8_t  temp;

    for ( y = 0; y < height; y++ )
    {
        rowStart = ptr + y * stride;
        rowEnd   = rowStart + widthM1;

        for ( x = 0; x < halfWidth; x++, rowStart++, rowEnd-- )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }
}

// Mirror 8bpp grayscale image around X axis
void MirrorImage8X( ximage* src )
{
    int width      = src->width;
    int height     = src->height;
    int heightM1   = height - 1;
    int halfHeight = height / 2;
    int stride     = src->stride;
    int x, y;

    uint8_t* ptr = src->data;
    uint8_t* rowStart;
    uint8_t* rowEnd;
    uint8_t  temp;

    for ( y = 0; y < halfHeight; y++ )
    {
        rowStart = ptr + y * stride;
        rowEnd   = ptr + ( heightM1 - y ) * stride;

        for ( x = 0; x < width; x++, rowStart++, rowEnd++ )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }
}

// Mirror 8bpp grayscale image around X and Y axes
void MirrorImage8XY( ximage* src )
{
    int width      = src->width;
    int height     = src->height;
    int widthM1    = width - 1;
    int heightM1   = height - 1;
    int halfWidth  = width / 2;
    int halfHeight = height / 2;
    int stride     = src->stride;
    int x, y;

    uint8_t* ptr = src->data;
    uint8_t* rowStart;
    uint8_t* rowEnd;
    uint8_t  temp;

    for ( y = 0; y < halfHeight; y++ )
    {
        rowStart = ptr + y * stride;
        rowEnd   = ptr + ( heightM1 - y ) * stride + widthM1;

        for ( x = 0; x < width; x++, rowStart++, rowEnd-- )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }

    if ( ( height & 1 ) != 0 )
    {
        rowStart = ptr + halfHeight * stride;
        rowEnd   = rowStart + widthM1;

        for ( x = 0; x < halfWidth; x++, rowStart++, rowEnd-- )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }
}

// Mirror 24bpp color image around Y axis
void MirrorImage24Y( ximage* src )
{
    int width     = src->width;
    int height    = src->height;
    int widthM1   = width - 1;
    int halfWidth = width / 2;
    int stride    = src->stride;
    int x, y;

    int widthM1x3 = widthM1 * 3;

    uint8_t* ptr = src->data;
    uint8_t* rowStart;
    uint8_t* rowEnd;

    uint8_t  temp;

    for ( y = 0; y < height; y++ )
    {
        rowStart = ptr + y * stride;
        rowEnd   = rowStart + widthM1x3;

        for ( x = 0; x < halfWidth; x++, rowStart += 3, rowEnd -= 3 )
        {
            temp        = *rowStart;
            *rowStart   = *rowEnd;
            *rowEnd     = temp;

            temp        = rowStart[1];
            rowStart[1] = rowEnd[1];
            rowEnd[1]   = temp;

            temp        = rowStart[2];
            rowStart[2] = rowEnd[2];
            rowEnd[2]   = temp;
        }
    }
}

// Mirror 24bpp color image around X axis
void MirrorImage24X( ximage* src )
{
    int width      = src->width;
    int height     = src->height;
    int heightM1   = height - 1;
    int halfHeight = height / 2;
    int stride     = src->stride;
    int x, y;

    uint8_t* ptr = src->data;
    uint8_t* rowStart;
    uint8_t* rowEnd;
    uint8_t  temp;

    for ( y = 0; y < halfHeight; y++ )
    {
        rowStart = ptr + y * stride;
        rowEnd   = ptr + ( heightM1 - y ) * stride;

        for ( x = 0; x < width; x++  )
        {
            // red
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;

            rowStart++;
            rowEnd++;

            // green
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;

            rowStart++;
            rowEnd++;

            // blue
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;

            rowStart++;
            rowEnd++;
        }
    }
}

// Mirror 24bpp color image around X and Y axes
void MirrorImage24XY( ximage* src )
{
    int width      = src->width;
    int height     = src->height;
    int widthM1    = width - 1;
    int heightM1   = height - 1;
    int halfWidth  = width / 2;
    int halfHeight = height / 2;
    int stride     = src->stride;
    int x, y;

    uint8_t* ptr = src->data;
    uint8_t* rowStart;
    uint8_t* rowEnd;
    uint8_t  temp;

    for ( y = 0; y < halfHeight; y++ )
    {
        rowStart = ptr + y * stride;
        rowEnd   = ptr + ( heightM1 - y ) * stride + widthM1 * 3;

        for ( x = 0; x < width; x++, rowStart += 3, rowEnd -= 3 )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;

            temp        = rowStart[1];
            rowStart[1] = rowEnd[1];
            rowEnd[1]   = temp;

            temp        = rowStart[2];
            rowStart[2] = rowEnd[2];
            rowEnd[2]   = temp;
        }
    }

    if ( ( height & 1 ) != 0 )
    {
        rowStart = ptr + halfHeight * stride;
        rowEnd   = rowStart + widthM1 * 3;

        for ( x = 0; x < halfWidth; x++, rowStart += 3, rowEnd -= 3 )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;

            temp        = rowStart[1];
            rowStart[1] = rowEnd[1];
            rowEnd[1]   = temp;

            temp        = rowStart[2];
            rowStart[2] = rowEnd[2];
            rowEnd[2]   = temp;
        }
    }
}

// Mirror 32bpp color image around Y axis
void MirrorImage32Y( ximage* src )
{
    int width     = src->width;
    int height    = src->height;
    int widthM1   = width - 1;
    int halfWidth = width / 2;
    int stride    = src->stride;
    int x, y;

    int widthM1x4 = widthM1 * 4;

    uint8_t*  ptr = src->data;
    uint32_t* rowStart;
    uint32_t* rowEnd;
    uint32_t  temp;

    for ( y = 0; y < height; y++ )
    {
        rowStart = (uint32_t*) ( ptr + y * stride );
        rowEnd   = (uint32_t*) ( ptr + y * stride + widthM1x4 );

        for ( x = 0; x < halfWidth; x++, rowStart++, rowEnd-- )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }
}

// Mirror 32bpp color image around X axis
void MirrorImage32X( ximage* src )
{
    int width      = src->width;
    int height     = src->height;
    int heightM1   = height - 1;
    int halfHeight = height / 2;
    int stride     = src->stride;
    int x, y;

    uint8_t*  ptr = src->data;
    uint32_t* rowStart;
    uint32_t* rowEnd;
    uint32_t  temp;

    for ( y = 0; y < halfHeight; y++ )
    {
        rowStart = (uint32_t*) ( ptr + y * stride );
        rowEnd   = (uint32_t*) ( ptr + ( heightM1 - y ) * stride );

        for ( x = 0; x < width; x++, rowStart++, rowEnd++ )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }
}

// Mirror 32bpp color image around X and Y axes
void MirrorImage32XY( ximage* src )
{
    int width      = src->width;
    int height     = src->height;
    int widthM1    = width - 1;
    int heightM1   = height - 1;
    int halfWidth  = width / 2;
    int halfHeight = height / 2;
    int stride     = src->stride;
    int x, y;

    int widthM1x4  = widthM1 * 4;

    uint8_t*  ptr = src->data;
    uint32_t* rowStart;
    uint32_t* rowEnd;
    uint32_t  temp;

    for ( y = 0; y < halfHeight; y++ )
    {
        rowStart = (uint32_t*) ( ptr + y * stride );
        rowEnd   = (uint32_t*) ( ptr + ( heightM1 - y ) * stride + widthM1x4 );

        for ( x = 0; x < width; x++, rowStart++, rowEnd-- )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }

    if ( ( height & 1 ) != 0 )
    {
        rowStart = (uint32_t*) ( ptr + halfHeight * stride );
        rowEnd   = (uint32_t*) ( ptr + halfHeight * stride + widthM1x4 );

        for ( x = 0; x < halfWidth; x++, rowStart++, rowEnd-- )
        {
            temp      = *rowStart;
            *rowStart = *rowEnd;
            *rowEnd   = temp;
        }
    }
}
