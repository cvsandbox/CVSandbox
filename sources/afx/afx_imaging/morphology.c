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

#include <memory.h>
#include <math.h>
#include "ximaging.h"

// forward declaration ----
static void Erosion8bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize );
static void Erosion24bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize );
static void Dilatation8bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize );
static void Dilatation24bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize );
// ------------------------

// Applies erosion morphological operator to the specified image.
//
// seSize is in [3, 51] range and must be odd value
// se (structuring element) is array of seSize*seSize (1 is for element to use, 0 - to ignore)
//
XErrorCode Erosion( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( se == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( dst->width  != src->width )  ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( seSize < 3 ) || ( seSize > 51 ) || ( ( seSize & 1 ) == 0 ) )
    {
        // don't allow even kernels or too small/big kernels
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        if ( src->format == XPixelFormatGrayscale8 )
        {
            Erosion8bpp( src, dst, se, seSize );
        }
        else
        {
            Erosion24bpp( src, dst, se, seSize );
        }
    }

    return ret;
}

// Applies dilatation morphological operator to the specified image.
//
// seSize is in [3, 51] range and must be odd value
// se (structuring element) is array of seSize*seSize (1 is for element to use, 0 - to ignore)
//
XErrorCode Dilatation( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( se == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( dst->width  != src->width )  ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( seSize < 3 ) || ( seSize > 51 ) || ( ( seSize & 1 ) == 0 ) )
    {
        // don't allow even kernels or too small/big kernels
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        if ( src->format == XPixelFormatGrayscale8 )
        {
            Dilatation8bpp( src, dst, se, seSize );
        }
        else
        {
            Dilatation24bpp( src, dst, se, seSize );
        }
    }

    return ret;
}

// Applies hit-and-miss morphological operator to the specified image.
//
// seSize is in [3, 51] range and must be odd value
// se (structuring element) is array of seSize*seSize (1 is for foreground/object, 0 - background, -1 - to ignore)
// mode is the Hit-and-Miss to use
//
XErrorCode HitAndMiss( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize, XHitAndMissMode mode )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( se == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( dst->width  != src->width )  ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( seSize < 3 ) || ( seSize > 51 ) || ( ( seSize & 1 ) == 0 ) )
    {
        // don't allow even kernels or too small/big kernels
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        int width     = src->width;
        int height    = src->height;
        int srcStride = src->stride;
        int dstStride = dst->stride;
        int radius    = seSize >> 1;
        int modeIndex = (int) mode;
        int y;

        uint8_t* srcPtr  = src->data;
        uint8_t* dstPtr  = dst->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, se, modeIndex )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* dstRow = dstPtr + y * dstStride;
            uint8_t* srcRow = srcPtr + y * srcStride;
            uint8_t  v, dstValue;
            int      x, i, j;

            uint8_t hitValue[3]  = { 255, 0, 255 };
            uint8_t missValue[3] = { 0, 0, 0 };

            for ( x = 0; x < width; x++ )
            {
                int8_t* sePtr = se;
                int8_t  seValue;

                missValue[1] = missValue[2] = *srcRow;
                dstValue = 255;

                // for each structuring element's row
                for ( i = -radius; i <= radius; i++ )
                {
                    // for each structuring element's column
                    for ( j = -radius; j <= radius; j++, sePtr++ )
                    {
                        seValue = *sePtr;

                        // skip "don't care" value
                        if ( seValue == -1 )
                        {
                            continue;
                        }

                        // check, if we are outside
                        if ( ( y + i < 0 ) || ( y + i >= width  ) ||
                             ( x + j < 0 ) || ( x + j >= height ) )
                        {
                            // if it so, the result is zero, because it was required pixel
                            dstValue = 0;
                            break;
                        }

                        // get source image value
                        v = srcRow[i * srcStride + j];

                        if ( ( ( seValue != 0 ) || ( v != 0   ) ) &&
                             ( ( seValue != 1 ) || ( v != 255 ) ) )
                        {
                            // failed structuring element mutch
                            dstValue = 0;
                            break;
                        }
                    }

                    if ( dstValue == 0 )
                    {
                        break;
                    }
                }

                // result pixel
                *dstRow = ( dstValue == 255 ) ? hitValue[modeIndex] : missValue[modeIndex];

                srcRow++;
                dstRow++;
            }
        }
    }

    return ret;

}

// Erode horizontal edges in grayscale images - pixels, which are not connected to 3 neighbours above or below
XErrorCode ErodeHorizontalEdges( const ximage* src, ximage* dst )
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
    else if ( ( src->width < 3 ) || ( src->height < 3 ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( ( dst->format != src->format ) ||
              ( dst->width  != src->width  ) ||
              ( dst->height != src->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int width     = src->width;
        int height    = src->height;
        int widthM1   = width - 1;
        int heightM1  = height - 1;
        int srcStride = src->stride;
        int dstStride = dst->stride;
        int y;

        uint8_t* srcPtr = src->data;
        uint8_t* dstPtr = dst->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthM1, heightM1, srcStride, dstStride )
        for ( y = 1; y < heightM1; y++ )
        {
            uint8_t* dstRow  = dstPtr + y * dstStride;
            uint8_t* srcRow  = srcPtr + y * srcStride;
            uint8_t* srcRowA = srcRow - srcStride;
            uint8_t* srcRowB = srcRow + srcStride;
            int      x;

            // first pixel of the row
            if ( ( *srcRow == 0 ) ||
                 ( ( *srcRowA == 0 ) && ( srcRowA[1] == 0 ) ) ||
                 ( ( *srcRowB == 0 ) && ( srcRowB[1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }

            dstRow++; srcRow++; srcRowA++; srcRowB++;

            // process the rest of the row
            for ( x = 1; x < widthM1; x++, dstRow++, srcRow++, srcRowA++, srcRowB++ )
            {
                if ( ( *srcRow == 0 ) ||
                     ( ( srcRowA[-1] == 0 ) && ( *srcRowA == 0 ) && ( srcRowA[1] == 0 ) ) ||
                     ( ( srcRowB[-1] == 0 ) && ( *srcRowB == 0 ) && ( srcRowB[1] == 0 ) ) )
                {
                    *dstRow = 0;
                }
                else
                {
                    *dstRow = *srcRow;
                }
            }

            // last pixel of the row
            if ( ( *srcRow == 0 ) ||
                 ( ( *srcRowA == 0 ) && ( srcRowA[-1] == 0 ) ) ||
                 ( ( *srcRowB == 0 ) && ( srcRowB[-1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }
        }

        // process top row
        {
            uint8_t* dstRow  = dstPtr;
            uint8_t* srcRow  = srcPtr;
            uint8_t* srcRowB = srcRow + srcStride;
            int      x;

            // first pixel of the row
            if ( ( *srcRow == 0 ) || ( ( *srcRowB == 0 ) && ( srcRowB[1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }

            dstRow++; srcRow++; srcRowB++;

            // process the rest of the row
            for ( x = 1; x < widthM1; x++, dstRow++, srcRow++, srcRowB++ )
            {
                if ( ( *srcRow == 0 ) || ( ( srcRowB[-1] == 0 ) && ( *srcRowB == 0 ) && ( srcRowB[1] == 0 ) ) )
                {
                    *dstRow = 0;
                }
                else
                {
                    *dstRow = *srcRow;
                }
            }

            // last pixel of the row
            if ( ( *srcRow == 0 ) || ( ( *srcRowB == 0 ) && ( srcRowB[-1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }
        }

        // process bottom row
        {
            uint8_t* dstRow  = dstPtr + heightM1 * dstStride;
            uint8_t* srcRow  = srcPtr + heightM1 * srcStride;
            uint8_t* srcRowA = srcRow - srcStride;
            int      x;

            // first pixel of the row
            if ( ( *srcRow == 0 ) || ( ( *srcRowA == 0 ) && ( srcRowA[1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }

            dstRow++; srcRow++; srcRowA++;

            // process the rest of the row
            for ( x = 1; x < widthM1; x++, dstRow++, srcRow++, srcRowA++ )
            {
                if ( ( *srcRow == 0 ) || ( ( srcRowA[-1] == 0 ) && ( *srcRowA == 0 ) && ( srcRowA[1] == 0 ) ) )
                {
                    *dstRow = 0;
                }
                else
                {
                    *dstRow = *srcRow;
                }
            }

            // last pixel of the row
            if ( ( *srcRow == 0 ) || ( ( *srcRowA == 0 ) && ( srcRowA[-1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }
        }
    }

    return ret;
}

// Erode vertical edges in grayscale images - pixels, which are not connected to 3 neighbours on the left or right
XErrorCode ErodeVerticalEdges( const ximage* src, ximage* dst )
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
    else if ( ( src->width < 3 ) || ( src->height < 3 ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( ( dst->format != src->format ) ||
              ( dst->width  != src->width  ) ||
              ( dst->height != src->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int width     = src->width;
        int height    = src->height;
        int widthM1   = width - 1;
        int heightM1  = height - 1;
        int srcStride = src->stride;
        int dstStride = dst->stride;
        int y;

        uint8_t* srcPtr = src->data;
        uint8_t* dstPtr = dst->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthM1, heightM1, srcStride, dstStride )
        for ( y = 1; y < heightM1; y++ )
        {
            uint8_t* dstRow  = dstPtr + y * dstStride;
            uint8_t* srcRow  = srcPtr + y * srcStride;
            uint8_t* srcRowA = srcRow - srcStride;
            uint8_t* srcRowB = srcRow + srcStride;
            int      x;

            // first pixel of the row
            if ( ( *srcRow == 0 ) || ( ( srcRowA[1] == 0 ) && ( srcRow[1] == 0 ) && ( srcRowB[1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }

            dstRow++; srcRow++; srcRowA++; srcRowB++;

            // process the rest of the row
            for ( x = 1; x < widthM1; x++, dstRow++, srcRow++, srcRowA++, srcRowB++ )
            {
                if ( ( *srcRow == 0 ) ||
                     ( ( srcRowA[-1] == 0 ) && ( srcRow[-1] == 0 ) && ( srcRowB[-1] == 0 ) ) ||
                     ( ( srcRowA[ 1] == 0 ) && ( srcRow[ 1] == 0 ) && ( srcRowB[ 1] == 0 ) ) )
                {
                    *dstRow = 0;
                }
                else
                {
                    *dstRow = *srcRow;
                }
            }

            // last pixel of the row
            if ( ( *srcRow == 0 ) || ( ( srcRowA[-1] == 0 ) && ( srcRow[-1] == 0 ) && ( srcRowB[-1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }
        }

        // process top row
        {
            uint8_t* dstRow  = dstPtr;
            uint8_t* srcRow  = srcPtr;
            uint8_t* srcRowB = srcRow + srcStride;
            int      x;

            // first pixel of the row
            if ( ( *srcRow == 0 ) || ( ( srcRow[1] == 0 ) && ( srcRowB[1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }

            dstRow++; srcRow++; srcRowB++;

            // process the rest of the row
            for ( x = 1; x < widthM1; x++, dstRow++, srcRow++, srcRowB++ )
            {
                if ( ( *srcRow == 0 ) ||
                     ( ( srcRowB[-1] == 0 ) && ( srcRow[-1] == 0 ) ) ||
                     ( ( srcRowB[ 1] == 0 ) && ( srcRow[ 1] == 0 ) ) )
                {
                    *dstRow = 0;
                }
                else
                {
                    *dstRow = *srcRow;
                }
            }

            // last pixel of the row
            if ( ( *srcRow == 0 ) || ( ( srcRow[-1] == 0 ) && ( srcRowB[-1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }
        }

        // process bottom row
        {
            uint8_t* dstRow  = dstPtr + heightM1 * dstStride;
            uint8_t* srcRow  = srcPtr + heightM1 * srcStride;
            uint8_t* srcRowA = srcRow - srcStride;
            int      x;

            // first pixel of the row
            if ( ( *srcRow == 0 ) || ( ( srcRow[1] == 0 ) && ( srcRowA[1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }

            dstRow++; srcRow++; srcRowA++;

            // process the rest of the row
            for ( x = 1; x < widthM1; x++, dstRow++, srcRow++, srcRowA++ )
            {
                if ( ( *srcRow == 0 ) ||
                     ( ( srcRowA[-1] == 0 ) && ( srcRow[-1] == 0 ) ) ||
                     ( ( srcRowA[ 1] == 0 ) && ( srcRow[ 1] == 0 ) ) )
                {
                    *dstRow = 0;
                }
                else
                {
                    *dstRow = *srcRow;
                }
            }

            // last pixel of the row
            if ( ( *srcRow == 0 ) || ( ( srcRow[-1] == 0 ) && ( srcRowA[-1] == 0 ) ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = *srcRow;
            }
        }
    }

    return ret;
}

// Prepare structuring element of the specified type
XErrorCode FillMorphologicalStructuringElement( int8_t* se, uint32_t seSize, XStructuringElementType type )
{
    XErrorCode ret = SuccessCode;

    if ( se == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( seSize & 1 ) == 0 )
    {
        // don't allow even kernels
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        int size   = seSize;
        int radius = size >> 1;
        int i, j, x, y;

        switch ( type )
        {
        case SEType_Square:

            memset( se, 1, seSize * seSize );
            break;

        case SEType_Circle:
            {
                for ( i = 0; i < size; i++ )
                {
                    y = i - radius;

                    for ( j = 0; j < size; j++ )
                    {
                        x = j - radius;

                        *se = ( sqrt( x * x + y * y ) - radius < 0.5 ) ? 1 : 0;

                        se++;
                    }
                }
            }
            break;

        case SEType_Diamond:

            for ( i = 0; i < size; i++ )
            {
                y = abs( i - radius );

                for ( j = 0; j < size; j++ )
                {
                    x = abs( j - radius );

                    *se = ( x + y <= radius ) ? 1 : 0;

                    se++;
                }
            }
            break;

        default:
            ret = ErrorInvalidArgument;
            break;
        }
    }

    return ret;
}

// Applies erosion morphological operator to the specified 8bpp image
void Erosion8bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = seSize >> 1;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, se )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        int8_t*  sePtr;
        uint8_t  min, v;
        int      x, i, j, t;
        bool     foundSomething;

        for ( x = 0; x < width; x++ )
        {
            min            = 255;
            foundSomething = false;
            sePtr          = se;

            // for each structuring element row
            for ( i = -radius; i <= radius; i++ )
            {
                t = y + i;

                // skip row
                if ( t < 0 )
                {
                    sePtr += seSize;
                    continue;
                }
                // break
                if ( t >= height )
                {
                    break;
                }

                // source image row aligned to current kernel row
                srcRow2 = srcRow + i * srcStride - radius;

                // for each kernel column
                for ( j = -radius; j <= radius; j++, sePtr++, srcRow2++ )
                {
                    t = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( ( t < width ) && ( *sePtr == 1 ) )
                    {
                        foundSomething = true;

                        v = *srcRow2;
                        if ( v < min )
                        {
                            min = v;
                        }
                    }
                }
            }

            // result pixel
            *dstRow = ( foundSomething ) ? min : *srcRow;

            srcRow++;
            dstRow++;
        }
    }
}

// Applies erosion morphological operator to the specified 24/32 bpp image (alpha channel is kept as is)
void Erosion24bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = seSize >> 1;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, se, pixelSize )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        int8_t*  sePtr;
        uint8_t  minR, minG, minB, v;
        int      x, i, j, t;
        bool     foundSomething;

        for ( x = 0; x < width; x++ )
        {
            minR = minG = minB = 255;
            foundSomething = false;
            sePtr          = se;

            // for each structuring element row
            for ( i = -radius; i <= radius; i++ )
            {
                t = y + i;

                // skip row
                if ( t < 0 )
                {
                    sePtr += seSize;
                    continue;
                }
                // break
                if ( t >= height )
                {
                    break;
                }

                // source image row aligned to current kernel row
                srcRow2 = srcRow + i * srcStride - radius * pixelSize;

                // for each kernel column
                for ( j = -radius; j <= radius; j++, sePtr++, srcRow2 += pixelSize )
                {
                    t = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( ( t < width ) && ( *sePtr == 1 ) )
                    {
                        foundSomething = true;

                        // red
                        v = srcRow2[RedIndex];
                        if ( v < minR )
                        {
                            minR = v;
                        }
                        // green
                        v = srcRow2[GreenIndex];
                        if ( v < minG )
                        {
                            minG = v;
                        }
                        // blue
                        v = srcRow2[BlueIndex];
                        if ( v < minB )
                        {
                            minB = v;
                        }
                    }
                }
            }

            // result pixel
            if ( foundSomething == true )
            {
                dstRow[RedIndex  ] = minR;
                dstRow[GreenIndex] = minG;
                dstRow[BlueIndex ] = minB;
            }
            else
            {
                dstRow[RedIndex  ] = srcRow[RedIndex];
                dstRow[GreenIndex] = srcRow[GreenIndex];
                dstRow[BlueIndex ] = srcRow[BlueIndex];
            }

            if ( pixelSize == 4 )
            {
                // ignore alpha, just copy it
                dstRow[AlphaIndex] = srcRow[AlphaIndex];
            }

            srcRow += pixelSize;
            dstRow += pixelSize;
        }
    }
}

// Applies dilatation morphological operator to the specified 8bpp image
void Dilatation8bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = seSize >> 1;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, se )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        int8_t*  sePtr;
        uint8_t  max, v;
        int      x, i, j, t;
        bool     foundSomething;

        for ( x = 0; x < width; x++ )
        {
            max            = 0;
            foundSomething = false;
            sePtr          = se;

            // for each structuring element row
            for ( i = -radius; i <= radius; i++ )
            {
                t = y + i;

                // skip row
                if ( t < 0 )
                {
                    sePtr += seSize;
                    continue;
                }
                // break
                if ( t >= height )
                {
                    break;
                }

                // source image row aligned to current kernel row
                srcRow2 = srcRow + i * srcStride - radius;

                // for each kernel column
                for ( j = -radius; j <= radius; j++, sePtr++, srcRow2++ )
                {
                    t = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( ( t < width ) && ( *sePtr == 1 ) )
                    {
                        foundSomething = true;

                        v = *srcRow2;
                        if ( v > max )
                        {
                            max = v;
                        }
                    }
                }
            }

            // result pixel
            *dstRow = ( foundSomething == true ) ? max : *srcRow;

            srcRow++;
            dstRow++;
        }
    }
}

// Applies dilatation morphological operator to the specified 24/32 bpp image (alpha channel is kept as is)
void Dilatation24bpp( const ximage* src, ximage* dst, int8_t* se, uint32_t seSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = seSize >> 1;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, se, pixelSize )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        int8_t*  sePtr;
        uint8_t  maxR, maxG, maxB, v;
        int      x, i, j, t;
        bool     foundSomething;

        for ( x = 0; x < width; x++ )
        {
            maxR = maxG = maxB = 0;
            foundSomething = false;
            sePtr          = se;

            // for each structuring element row
            for ( i = -radius; i <= radius; i++ )
            {
                t = y + i;

                // skip row
                if ( t < 0 )
                {
                    sePtr += seSize;
                    continue;
                }
                // break
                if ( t >= height )
                {
                    break;
                }

                // source image row aligned to current kernel row
                srcRow2 = srcRow + i * srcStride - radius * pixelSize;

                // for each kernel column
                for ( j = -radius; j <= radius; j++, sePtr++, srcRow2 += pixelSize )
                {
                    t = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( ( t < width ) && ( *sePtr == 1 ) )
                    {
                        foundSomething = true;

                        // red
                        v = srcRow2[RedIndex];
                        if ( v > maxR )
                        {
                            maxR = v;
                        }
                        // green
                        v = srcRow2[GreenIndex];
                        if ( v > maxG )
                        {
                            maxG = v;
                        }
                        // blue
                        v = srcRow2[BlueIndex];
                        if ( v > maxB )
                        {
                            maxB = v;
                        }
                    }
                }
            }

            // result pixel
            if ( foundSomething == true )
            {
                dstRow[RedIndex  ] = maxR;
                dstRow[GreenIndex] = maxG;
                dstRow[BlueIndex ] = maxB;
            }
            else
            {
                dstRow[RedIndex  ] = srcRow[RedIndex];
                dstRow[GreenIndex] = srcRow[GreenIndex];
                dstRow[BlueIndex ] = srcRow[BlueIndex];
            }

            if ( pixelSize == 4 )
            {
                // ignore alpha, just copy it
                dstRow[AlphaIndex] = srcRow[AlphaIndex];
            }

            srcRow += pixelSize;
            dstRow += pixelSize;
        }
    }
}
