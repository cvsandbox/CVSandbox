/*
    Imaging library of Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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

// Shift image in X/Y directions by the specified numbur of pixels
XErrorCode ShiftImage( ximage* src, int dx, int dy, bool fillOpenSpace, xargb fillColor )
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
    else if ( ( dx != 0 ) || ( dy != 0 ) )
    {
        int pixelSize   = ( src->format == XPixelFormatGrayscale8 ) ? 1 : ( ( src->format == XPixelFormatRGB24 ) ? 3 : 4 );
        int width       = src->width;
        int height      = src->height;
        int stride      = src->stride;
        int shiftWidth  = width * pixelSize;
        int shiftHeight = height;
        int shiftXstart = 0;
        int shiftYstart = 0;
        int loopDx      = 0;
        int loopDy      = 0;
        int xBytesShift = 0;
        int openXmin    = 0;
        int openXmax    = -1;
        int openYmin    = 0;
        int openYmax    = -1;
        int x, y;

        uint8_t* srcPtr;
        uint8_t* dstPtr;

        if ( dx > 0 )
        {
            shiftWidth -= dx * pixelSize;
            loopDx      = -1;
            shiftXstart = width - 1;
            xBytesShift = pixelSize - 1;

            openXmin    = 0;
            openXmax    = dx - 1;
        }
        else
        {
            shiftWidth += dx * pixelSize;
            loopDx      = 1;
            shiftXstart = 0;

            openXmin    = width + dx;
            openXmax    = width - 1;
        }

        if ( dy > 0 )
        {
            shiftHeight -= dy;
            loopDy       = -1;
            shiftYstart  = height - 1;

            openYmin     = 0;
            openYmax     = dy - 1;
        }
        else
        {
            shiftHeight += dy;
            loopDy       = 1;
            shiftYstart  = 0;

            openYmin     = height + dy;
            openYmax     = height - 1;
        }

        for ( y = 0; y < shiftHeight; y++ )
        {
            dstPtr = src->data + stride * ( shiftYstart + y * loopDy ) + shiftXstart * pixelSize + xBytesShift;
            srcPtr = dstPtr - stride * dy - pixelSize * dx;

            for ( x = 0; x < shiftWidth; x++ )
            {
                *dstPtr = *srcPtr;
                dstPtr += loopDx;
                srcPtr += loopDx;
            }
        }

        // fill open space if required
        if ( fillOpenSpace == true )
        {
            if ( openYmin <= openYmax )
            {
                XDrawingFillRectangle( src, 0, openYmin, width - 1, openYmax, fillColor );
            }

            if ( openXmin <= openXmax )
            {
                if ( openYmin <= openYmax )
                {
                    if ( openYmin == 0 )
                    {
                        // open space at the top
                        XDrawingFillRectangle( src, openXmin, openYmax + 1, openXmax, height - 1, fillColor );
                    }
                    else
                    {
                        // open space at the bottom
                        XDrawingFillRectangle( src, openXmin, 0, openXmax, openYmax - 1, fillColor );
                    }
                }
                else
                {
                    XDrawingFillRectangle( src, openXmin, 0, openXmax, height - 1, fillColor );
                }
            }
        }
    }

    return ret;
}
