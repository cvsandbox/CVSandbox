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

#include <time.h>
#include "ximaging.h"

// forward declaration ----
static void ImagePixellate8( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight );
static void ImagePixellate24( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight );
static void ImagePixellate32( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight );
// ------------------------

// Perform pixellation of the specified image
XErrorCode ImagePixellate( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format == XPixelFormatGrayscale8 )
    {
        ImagePixellate8( src, pixelWidth, pixelHeight );
    }
    else if ( src->format == XPixelFormatRGB24 )
    {
        ImagePixellate24( src, pixelWidth, pixelHeight );
    }
    else if ( src->format == XPixelFormatRGBA32 )
    {
        ImagePixellate32( src, pixelWidth, pixelHeight );
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }

    return ret;
}

// Perform pixellation of a 8bpp grayscale image
static void ImagePixellate8( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight )
{
    int width  = src->width;
    int height = src->height;
    int offset = src->stride - width;
    int x, y1, y2;
    int i;      // loops' indexes
    int t1, t2; // temp variables
    uint8_t* psrc = src->data;
    uint8_t* pdst = psrc;

    // line length to process
    int len = (int) ( ( width - 1 ) / pixelWidth ) + 1;
    int lenM1 = len - 1;
    // reminder
    int rem = ( ( width - 1 ) % pixelWidth ) + 1;

    // allocate buffer for pixels' accumulation
    uint32_t* tmp = (uint32_t*) malloc( len * sizeof( uint32_t ) );

    for ( y1 = 0, y2 = 0; y1 < height; )
    {
        // clear the temporary buffer
        for ( i = 0; i < len; i++ )
        {
            tmp[i] = 0;
        }

        // calculate sums for each "pixel"
        for ( i = 0; ( i < pixelHeight ) && ( y1 < height ); i++, y1++ )
        {
            // for each pixel
            for ( x = 0; x < width; x++, psrc++ )
            {
                tmp[x / pixelWidth] += (int) *psrc;
            }

            psrc += offset;
        }

        // get average values
        t1 = i * pixelWidth;
        t2 = i * rem;

        for ( i = 0; i < lenM1; i++ )
        {
            tmp[i] /= t1;
        }
        tmp[i] /= t2;

        // save average value to destination image
        for ( i = 0; ( i < pixelHeight ) && ( y2 < height ); i++, y2++ )
        {
            // for each pixel
            for ( x = 0; x < width; x++, pdst++ )
            {
                *pdst = (uint8_t) tmp[x / pixelWidth];
            }
            pdst += offset;
        }
    }

    // free allocated memory
    free( tmp );
}

// Perform pixellation of a 24 color image
static void ImagePixellate24( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight )
{
    int width  = src->width;
    int height = src->height;
    int offset = src->stride - width * 3;
    int x, y1, y2;
    int i;                  // loops' indexes
    int t1, t2, pixelIndex; // temp variables
    uint8_t* psrc = src->data;
    uint8_t* pdst = psrc;

    // line length to process
    int len = (int) ( ( width - 1 ) / pixelWidth ) + 1;
    int lenM1 = len - 1;
    // reminder
    int rem = ( ( width - 1 ) % pixelWidth ) + 1;

    // allocate buffers for pixels' accumulation
    uint32_t* tmpR = (uint32_t*) malloc( len * sizeof( uint32_t ) );
    uint32_t* tmpG = (uint32_t*) malloc( len * sizeof( uint32_t ) );
    uint32_t* tmpB = (uint32_t*) malloc( len * sizeof( uint32_t ) );

    for ( y1 = 0, y2 = 0; y1 < height; )
    {
        // clear the temporary buffer
        for ( i = 0; i < len; i++ )
        {
            tmpR[i] = 0;
            tmpG[i] = 0;
            tmpB[i] = 0;
        }

        // calculate sums for each "pixel"
        for ( i = 0; ( i < pixelHeight ) && ( y1 < height ); i++, y1++ )
        {
            // for each pixel
            for ( x = 0; x < width; x++, psrc += 3 )
            {
                pixelIndex = x / pixelWidth;
                tmpR[pixelIndex] += (int) psrc[RedIndex];
                tmpG[pixelIndex] += (int) psrc[GreenIndex];
                tmpB[pixelIndex] += (int) psrc[BlueIndex];
            }

            psrc += offset;
        }

        // get average values
        t1 = i * pixelWidth;
        t2 = i * rem;

        for ( i = 0; i < lenM1; i++ )
        {
            tmpR[i] /= t1;
            tmpG[i] /= t1;
            tmpB[i] /= t1;
        }
        tmpR[i] /= t2;
        tmpG[i] /= t2;
        tmpB[i] /= t2;

        // save average value to destination image
        for ( i = 0; ( i < pixelHeight ) && ( y2 < height ); i++, y2++ )
        {
            // for each pixel
            for ( x = 0; x < width; x++, pdst += 3 )
            {
                pixelIndex = x / pixelWidth;
                pdst[RedIndex]   = (uint8_t) tmpR[pixelIndex];
                pdst[GreenIndex] = (uint8_t) tmpG[pixelIndex];
                pdst[BlueIndex]  = (uint8_t) tmpB[pixelIndex];
            }
            pdst += offset;
        }
    }

    // free allocated memory
    free( tmpR );
    free( tmpG );
    free( tmpB );
}

// Perform pixellation of a 32 color image
static void ImagePixellate32( ximage* src, uint8_t pixelWidth, uint8_t pixelHeight )
{
    int width  = src->width;
    int height = src->height;
    int offset = src->stride - width * 4;
    int x, y1, y2;
    int i;                  // loops' indexes
    int t1, t2, pixelIndex; // temp variables
    uint8_t* psrc = src->data;
    uint8_t* pdst = psrc;

    // line length to process
    int len = (int) ( ( width - 1 ) / pixelWidth ) + 1;
    int lenM1 = len - 1;
    // reminder
    int rem = ( ( width - 1 ) % pixelWidth ) + 1;

    // allocate buffers for pixels' accumulation
    uint32_t* tmpR = (uint32_t*) malloc( len * sizeof( uint32_t ) );
    uint32_t* tmpG = (uint32_t*) malloc( len * sizeof( uint32_t ) );
    uint32_t* tmpB = (uint32_t*) malloc( len * sizeof( uint32_t ) );
    uint32_t* tmpA = (uint32_t*) malloc( len * sizeof( uint32_t ) );

    for ( y1 = 0, y2 = 0; y1 < height; )
    {
        // clear the temporary buffer
        for ( i = 0; i < len; i++ )
        {
            tmpR[i] = 0;
            tmpG[i] = 0;
            tmpB[i] = 0;
            tmpA[i] = 0;
        }

        // calculate sums for each "pixel"
        for ( i = 0; ( i < pixelHeight ) && ( y1 < height ); i++, y1++ )
        {
            // for each pixel
            for ( x = 0; x < width; x++, psrc += 4)
            {
                pixelIndex = x / pixelWidth;
                tmpR[pixelIndex] += (int) psrc[RedIndex];
                tmpG[pixelIndex] += (int) psrc[GreenIndex];
                tmpB[pixelIndex] += (int) psrc[BlueIndex];
                tmpA[pixelIndex] += (int) psrc[AlphaIndex];
            }

            psrc += offset;
        }

        // get average values
        t1 = i * pixelWidth;
        t2 = i * rem;

        for ( i = 0; i < lenM1; i++ )
        {
            tmpR[i] /= t1;
            tmpG[i] /= t1;
            tmpB[i] /= t1;
            tmpA[i] /= t1;
        }
        tmpR[i] /= t2;
        tmpG[i] /= t2;
        tmpB[i] /= t2;
        tmpA[i] /= t2;

        // save average value to destination image
        for ( i = 0; ( i < pixelHeight ) && ( y2 < height ); i++, y2++ )
        {
            // for each pixel
            for ( x = 0; x < width; x++, pdst += 4 )
            {
                pixelIndex = x / pixelWidth;
                pdst[RedIndex]   = (uint8_t) tmpR[pixelIndex];
                pdst[GreenIndex] = (uint8_t) tmpG[pixelIndex];
                pdst[BlueIndex]  = (uint8_t) tmpB[pixelIndex];
                pdst[AlphaIndex] = (uint8_t) tmpA[pixelIndex];
            }
            pdst += offset;
        }
    }

    // free allocated memory
    free( tmpR );
    free( tmpG );
    free( tmpB );
    free( tmpA );
}
