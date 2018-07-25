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
#include "ximaging.h"

/*
 * The algorithm is similar to the one produced by EmbossImage(), but
 * instead of embossing on a solid background it just increases/decreases pixels' brightness.
 */

// Define the main computation of emboss value (just so we don't need to repeat same code for multiple cases)
#define COMPUTE_VALUE \
if ( ( dx == 0 ) && ( dy == 0 ) ) \
{ \
    value = 0; \
} \
else \
{ \
    dotL = dx * lx + dy * ly; \
    value = (int) ( dotL / sqrt( dx * dx + dy * dy + nz2 ) ); \
}

// Create image with effect of light dropped at the image surface from the specified direction
XErrorCode ImageDropLight( const ximage* src, ximage* dst, float azimuth, float elevation, float depth )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 )  &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( src->width < 3 ) || ( src->height < 3 ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( ( dst->width  != src->width ) ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
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

        uint8_t* srcPtr  = src->data;
        uint8_t* dstPtr  = dst->data;

        double  azimuthRad   = azimuth   * XPI / 180.0;
        double  elevationRad = elevation * XPI / 180.0;

        double lx = cos( azimuthRad ) * cos ( elevationRad ) * Max8bppPixelValue;
        double ly = sin( azimuthRad ) * cos ( elevationRad ) * Max8bppPixelValue;

        double nz = ( 2.0 * Max8bppPixelValue ) / depth;
        double nz2 = nz * nz;


        if ( src->format == XPixelFormatGrayscale8 )
        {
            // grayscale image
            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride )
            for ( y = 1; y < heightM1; y++ )
            {
                uint8_t* srcRow = srcPtr + y * srcStride;
                uint8_t* dstRow = dstPtr + y * dstStride;
                int      x;
                int      dx, dy, value;
                double   dotL;

                // the  first pixel
                dx = (int)
                    srcRow[0] + srcRow[-srcStride    ] + srcRow[srcStride]
                  - srcRow[1] - srcRow[-srcStride + 1] - srcRow[srcStride + 1];
                dy = (int)
                    srcRow[ srcStride] * 2  + srcRow[ srcStride + 1]
                  - srcRow[-srcStride] * 2  - srcRow[-srcStride + 1];

                COMPUTE_VALUE;

                value = value + *srcRow;
                value = XINRANGE( value, 0, 255 );
                *dstRow = (uint8_t) value;

                srcRow++;
                dstRow++;

                for ( x = 1; x < widthM1; x++, srcRow++, dstRow++ )
                {
                    dx = (int)
                        srcRow[-1] + srcRow[-srcStride - 1] + srcRow[srcStride - 1]
                      - srcRow[ 1] - srcRow[-srcStride + 1] - srcRow[srcStride + 1];
                    dy = (int)
                        srcRow[ srcStride] + srcRow[ srcStride - 1] + srcRow[ srcStride + 1]
                      - srcRow[-srcStride] - srcRow[-srcStride - 1] - srcRow[-srcStride + 1];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;
                }

                // the last pixel
                dx = (int)
                    srcRow[-1] + srcRow[-srcStride - 1] + srcRow[srcStride - 1]
                  - srcRow[ 0] - srcRow[-srcStride    ] - srcRow[srcStride];
                dy = (int)
                    srcRow[ srcStride] * 2 + srcRow[ srcStride - 1]
                  - srcRow[-srcStride] * 2 - srcRow[-srcStride - 1];

                COMPUTE_VALUE;

                value = value + *srcRow;
                value = XINRANGE( value, 0, 255 );
                *dstRow = (uint8_t) value;
            }

            // the first row
            {
                uint8_t* srcRow = srcPtr;
                uint8_t* dstRow = dstPtr;
                int      x;
                int      dx, dy, value;
                double   dotL;

                // the  first pixel
                dx = (int)
                    srcRow[0] * 2 + srcRow[srcStride]
                  - srcRow[1] * 2 - srcRow[srcStride + 1];
                dy = (int)
                    srcRow[srcStride] * 2  + srcRow[srcStride + 1]
                  - srcRow[0] * 2 - srcRow[1];

                COMPUTE_VALUE;

                value = value + *srcRow;
                value = XINRANGE( value, 0, 255 );
                *dstRow = (uint8_t) value;

                srcRow++;
                dstRow++;

                for ( x = 1; x < widthM1; x++, srcRow++, dstRow++ )
                {
                    dx = (int)
                        srcRow[-1] * 2 + srcRow[srcStride - 1]
                      - srcRow[ 1] * 2 - srcRow[srcStride + 1];
                    dy = (int)
                        srcRow[srcStride] + srcRow[srcStride - 1] + srcRow[srcStride + 1]
                      - srcRow[0] - srcRow[-1] - srcRow[1];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;
                }

                // the last pixel
                dx = (int)
                    srcRow[-1] * 2 + srcRow[srcStride - 1]
                  - srcRow[ 0] * 2 - srcRow[srcStride];
                dy = (int)
                    srcRow[srcStride] * 2 + srcRow[srcStride - 1]
                  - srcRow[0] * 2 - srcRow[-1];

                COMPUTE_VALUE;

                value = value + *srcRow;
                value = XINRANGE( value, 0, 255 );
                *dstRow = (uint8_t) value;
            }

            // the last row
            {
                uint8_t* srcRow = srcPtr + heightM1 * srcStride;
                uint8_t* dstRow = dstPtr + heightM1 * dstStride;
                int      x;
                int      dx, dy, value;
                double   dotL;

                // the  first pixel
                dx = (int)
                    srcRow[0] * 2 + srcRow[-srcStride    ]
                  - srcRow[1] * 2 - srcRow[-srcStride + 1];
                dy = (int)
                    srcRow[0] * 2  + srcRow[1]
                  - srcRow[-srcStride] * 2  - srcRow[-srcStride + 1];

                COMPUTE_VALUE;

                value = value + *srcRow;
                value = XINRANGE( value, 0, 255 );
                *dstRow = (uint8_t) value;

                srcRow++;
                dstRow++;

                for ( x = 1; x < widthM1; x++, srcRow++, dstRow++ )
                {
                    dx = (int)
                        srcRow[-1] * 2 + srcRow[-srcStride - 1]
                      - srcRow[ 1] * 2 - srcRow[-srcStride + 1];
                    dy = (int)
                        srcRow[0] + srcRow[-1] + srcRow[1]
                      - srcRow[-srcStride] - srcRow[-srcStride - 1] - srcRow[-srcStride + 1];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;
                }

                // the last pixel
                dx = (int)
                    srcRow[-1] * 2 + srcRow[-srcStride - 1]
                  - srcRow[ 0] * 2 - srcRow[-srcStride    ];
                dy = (int)
                    srcRow[0] * 2 + srcRow[-1]
                  - srcRow[-srcStride] * 2 - srcRow[-srcStride - 1];

                COMPUTE_VALUE;

                value = value + *srcRow;
                value = XINRANGE( value, 0, 255 );
                *dstRow = (uint8_t) value;
            }
        }
        else
        {
            // color image
            int   pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;

            #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, srcStride, dstStride, pixelSize )
            for ( y = 1; y < heightM1; y++ )
            {
                int      x, i;
                int      dx, dy, value;
                double   dotL;
                uint8_t* srcRow;
                uint8_t* dstRow;

                if  ( pixelSize == 4 )
                {
                    srcRow = srcPtr + y * srcStride + 3;
                    dstRow = dstPtr + y * dstStride + 3;

                    for ( x = 0; x < width; x++, srcRow += pixelSize, dstRow += pixelSize )
                    {
                        *dstRow = *srcRow;
                    }
                }

                for ( i = 0; i < 3; i++ )
                {
                    srcRow = srcPtr + y * srcStride + i;
                    dstRow = dstPtr + y * dstStride + i;

                    // the  first pixel
                    dx = (int)
                        srcRow[0] + srcRow[-srcStride] + srcRow[srcStride]
                      - srcRow[pixelSize] - srcRow[-srcStride + pixelSize] - srcRow[srcStride + pixelSize];
                    dy = (int)
                        srcRow[ srcStride] * 2  + srcRow[ srcStride + pixelSize]
                      - srcRow[-srcStride] * 2  - srcRow[-srcStride + pixelSize];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;

                    srcRow += pixelSize;
                    dstRow += pixelSize;

                    for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
                    {
                        dx = (int)
                              srcRow[-pixelSize] + srcRow[-srcStride - pixelSize] + srcRow[srcStride - pixelSize]
                            - srcRow[ pixelSize] - srcRow[-srcStride + pixelSize] - srcRow[srcStride + pixelSize];
                        dy = (int)
                              srcRow[ srcStride] + srcRow[ srcStride - pixelSize] + srcRow[ srcStride + pixelSize]
                            - srcRow[-srcStride] - srcRow[-srcStride - pixelSize] - srcRow[-srcStride + pixelSize];

                        COMPUTE_VALUE;

                        value = value + *srcRow;
                        value = XINRANGE( value, 0, 255 );
                        *dstRow = (uint8_t) value;
                    }

                    // the last pixel
                    dx = (int)
                        srcRow[-pixelSize] + srcRow[-srcStride - pixelSize] + srcRow[srcStride - pixelSize]
                      - srcRow[0] - srcRow[-srcStride] - srcRow[srcStride];
                    dy = (int)
                        srcRow[ srcStride] * 2 + srcRow[ srcStride - pixelSize]
                      - srcRow[-srcStride] * 2 - srcRow[-srcStride - pixelSize];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;
                }
            }

            {
                int i;

                // the first row
                for ( i = 0; i < 3; i++ )
                {
                    uint8_t* srcRow = srcPtr + i;
                    uint8_t* dstRow = dstPtr + i;
                    int      x;
                    int      dx, dy, value;
                    double   dotL;

                    // the  first pixel
                    dx = (int)
                          srcRow[0] * 2 + srcRow[srcStride]
                        - srcRow[pixelSize] * 2 - srcRow[srcStride + pixelSize];
                    dy = (int)
                          srcRow[srcStride] * 2  + srcRow[srcStride + pixelSize]
                        - srcRow[0] * 2 - srcRow[pixelSize];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;

                    srcRow += pixelSize;
                    dstRow += pixelSize;

                    for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
                    {
                        dx = (int)
                              srcRow[-pixelSize] * 2 + srcRow[srcStride - pixelSize]
                            - srcRow[ pixelSize] * 2 - srcRow[srcStride + pixelSize];
                        dy = (int)
                              srcRow[srcStride] + srcRow[srcStride - pixelSize] + srcRow[srcStride + pixelSize]
                            - srcRow[0] - srcRow[-pixelSize] - srcRow[pixelSize];

                        COMPUTE_VALUE;

                        value = value + *srcRow;
                        value = XINRANGE( value, 0, 255 );
                        *dstRow = (uint8_t) value;
                    }

                    // the last pixel
                    dx = (int)
                          srcRow[-pixelSize] * 2 + srcRow[srcStride - pixelSize]
                        - srcRow[ 0] * 2 - srcRow[srcStride];
                    dy = (int)
                          srcRow[srcStride] * 2 + srcRow[srcStride - pixelSize]
                        - srcRow[0] * 2 - srcRow[-pixelSize];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;
                }

                // the last row
                for ( i = 0; i < 3; i++ )
                {
                    uint8_t* srcRow = srcPtr + heightM1 * srcStride + i;
                    uint8_t* dstRow = dstPtr + heightM1 * dstStride + i;
                    int      x;
                    int      dx, dy, value;
                    double   dotL;

                    // the  first pixel
                    dx = (int)
                        srcRow[0] * 2 + srcRow[-srcStride]
                      - srcRow[pixelSize] * 2 - srcRow[-srcStride + pixelSize];
                    dy = (int)
                        srcRow[0] * 2  + srcRow[pixelSize]
                      - srcRow[-srcStride] * 2  - srcRow[-srcStride + pixelSize];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;

                    srcRow += pixelSize;
                    dstRow += pixelSize;

                    for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
                    {
                        dx = (int)
                            srcRow[-pixelSize] * 2 + srcRow[-srcStride - pixelSize]
                          - srcRow[ pixelSize] * 2 - srcRow[-srcStride + pixelSize];
                        dy = (int)
                            srcRow[0] + srcRow[-pixelSize] + srcRow[pixelSize]
                          - srcRow[-srcStride] - srcRow[-srcStride - pixelSize] - srcRow[-srcStride + pixelSize];

                        COMPUTE_VALUE;

                        value = value + *srcRow;
                        value = XINRANGE( value, 0, 255 );
                        *dstRow = (uint8_t) value;
                    }

                    // the last pixel
                    dx = (int)
                        srcRow[-pixelSize] * 2 + srcRow[-srcStride - pixelSize]
                      - srcRow[ 0] * 2 - srcRow[-srcStride    ];
                    dy = (int)
                        srcRow[0] * 2 + srcRow[-pixelSize]
                      - srcRow[-srcStride] * 2 - srcRow[-srcStride - pixelSize];

                    COMPUTE_VALUE;

                    value = value + *srcRow;
                    value = XINRANGE( value, 0, 255 );
                    *dstRow = (uint8_t) value;
                }

                if ( pixelSize == 4 )
                {
                    uint8_t* srcRow1 = srcPtr + 3;
                    uint8_t* dstRow1 = dstPtr + 3;
                    uint8_t* srcRow2 = srcPtr + heightM1 * srcStride + 3;
                    uint8_t* dstRow2 = dstPtr + heightM1 * dstStride + 3;
                    int      x;

                    for ( x = 0; x < width; x++, srcRow1 += pixelSize, srcRow2 += pixelSize, dstRow1 += pixelSize, dstRow2 += pixelSize )
                    {
                        *dstRow1 = *srcRow1;
                        *dstRow2 = *srcRow2;
                    }
                }
            }
        }
    }

    return ret;
}
