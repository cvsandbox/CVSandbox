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

// Fill horizontal gaps between objects in a thresholded grayscale image
XErrorCode HorizontalRunLengthSmoothing( ximage* src, uint16_t maxGap )
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
        int      width  = src->width;
        int      height = src->height;
        int      stride = src->stride;
        int      y;
        uint8_t* ptr    = src->data;
        uint8_t* row;
        uint8_t* rowEnd;
        uint8_t* gapStart;

        for ( y = 0; y < height; y++ )
        {
            row    = ptr + y * stride;
            rowEnd = row + width;

            // skip black pixels at the start of the line
            while ( ( row < rowEnd ) && ( *row == 0 ) )
            {
                row++;
            }

            while ( row < rowEnd )
            {
                // locate the first gap by skipping white pixels
                while ( ( row < rowEnd ) && ( *row != 0 ) )
                {
                    row++;
                }

                gapStart = row;

                // locate the next object by skipping black pixels
                while ( ( row < rowEnd ) && ( *row == 0 ) )
                {
                    row++;
                }

                if ( ( row < rowEnd ) && ( row - gapStart <= maxGap ) )
                {
                    // fill the gap
                    while ( gapStart != row )
                    {
                        *gapStart = 255;
                        gapStart++;
                    }
                }
            }
        }
    }

    return ret;
}

// Fill vertical gaps between objects in a thresholded grayscale image
XErrorCode VerticalRunLengthSmoothing( ximage* src, uint16_t maxGap )
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
        int      width  = src->width;
        int      height = src->height;
        int      stride = src->stride;
        int      x, gapSize;
        uint8_t* ptr    = src->data;
        uint8_t* column;
        uint8_t* columnEnd;
        uint8_t* gapStart;

        for ( x = 0; x < width; x++ )
        {
            column    = ptr + x;
            columnEnd = column + stride * height;

            // skip black pixels at the start of the line
            while ( ( column < columnEnd ) && ( *column == 0 ) )
            {
                column += stride;
            }

            while ( column < columnEnd )
            {
                // locate the first gap by skipping white pixels
                while ( ( column < columnEnd ) && ( *column != 0 ) )
                {
                    column += stride;
                }

                gapStart = column;
                gapSize  = 0;

                // locate the next object by skipping black pixels
                while ( ( column < columnEnd ) && ( *column == 0 ) )
                {
                    column += stride;
                    gapSize++;
                }

                if ( ( column < columnEnd ) && ( gapSize <= maxGap ) )
                {
                    // fill the gap
                    while ( gapStart != column )
                    {
                        *gapStart = 255;
                        gapStart += stride;
                    }
                }
            }
        }
    }

    return ret;
}
