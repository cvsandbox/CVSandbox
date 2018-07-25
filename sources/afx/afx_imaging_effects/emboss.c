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
 * The algorithm does "embossing" of an image.
 * Implementation is inspired by GIMP with some minor optimizations/changes.
 */

// Define the main computation of emboss value (just so we don't need to repeat same code for multiple cases)
#define COMPUTE_VALUE \
if ( ( dx == 0 ) && ( dy == 0 ) ) \
{ \
    value = bg; \
} \
else \
{ \
    dotL = dx * lx + dy * ly + nzlz; \
    if ( dotL < 0 ) \
    { \
        value = 0; \
    } \
    else \
    { \
        value = (int) ( dotL / sqrt( dx * dx + dy * dy + nz2 ) ); \
    } \
}

// Create image with emboss effect
XErrorCode EmbossImage( const ximage* src, ximage* dst, float azimuth, float elevation, float depth )
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
        double lz = sin( elevationRad ) * Max8bppPixelValue;
        int    bg = (int) lz;

        if ( src->format == XPixelFormatGrayscale8 )
        {
            // grayscale image

            double nz = ( 2.0 * Max8bppPixelValue ) / depth;
            double nz2 = nz * nz;
            double nzlz = nz * lz;

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

                value = XINRANGE( value, 0, 255 );
                *dstRow = (uint8_t) value;
            }
        }
        else
        {
            // color image
            int   pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;

            double nz = ( 6.0 * Max8bppPixelValue ) / depth;
            double nz2 = nz * nz;
            double nzlz = nz * lz;

            if ( src->format == XPixelFormatRGBA32 )
            {
                XImageFillPlane( dst, AlphaIndex, NotTransparent8bpp );
            }

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
                  + srcRow[1] + srcRow[-srcStride + 1] + srcRow[srcStride + 1]
                  + srcRow[2] + srcRow[-srcStride + 2] + srcRow[srcStride + 2]
                  - srcRow[ pixelSize    ] - srcRow[-srcStride + pixelSize    ] - srcRow[srcStride + pixelSize]
                  - srcRow[ pixelSize + 1] - srcRow[-srcStride + pixelSize + 1] - srcRow[srcStride + pixelSize + 1]
                  - srcRow[ pixelSize + 2] - srcRow[-srcStride + pixelSize + 2] - srcRow[srcStride + pixelSize + 2];

                dy = (int)
                    srcRow[ srcStride    ] * 2 + srcRow[ srcStride + pixelSize]
                  + srcRow[ srcStride + 1] * 2 + srcRow[ srcStride + pixelSize + 1]
                  + srcRow[ srcStride + 2] * 2 + srcRow[ srcStride + pixelSize + 2]
                  - srcRow[-srcStride    ] * 2 - srcRow[-srcStride + pixelSize]
                  - srcRow[-srcStride + 1] * 2 - srcRow[-srcStride + pixelSize + 1]
                  - srcRow[-srcStride + 2] * 2 - srcRow[-srcStride + pixelSize + 2];

                COMPUTE_VALUE;

                value = XINRANGE( value, 0, 255 );
                dstRow[0] = (uint8_t) value;
                dstRow[1] = (uint8_t) value;
                dstRow[2] = (uint8_t) value;

                srcRow += pixelSize;
                dstRow += pixelSize;

                for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
                {
                    dx = (int)
                        srcRow[-pixelSize    ] + srcRow[-srcStride - pixelSize    ] + srcRow[srcStride - pixelSize]
                      + srcRow[-pixelSize + 1] + srcRow[-srcStride - pixelSize + 1] + srcRow[srcStride - pixelSize + 1]
                      + srcRow[-pixelSize + 2] + srcRow[-srcStride - pixelSize + 2] + srcRow[srcStride - pixelSize + 2]
                      - srcRow[ pixelSize    ] - srcRow[-srcStride + pixelSize    ] - srcRow[srcStride + pixelSize]
                      - srcRow[ pixelSize + 1] - srcRow[-srcStride + pixelSize + 1] - srcRow[srcStride + pixelSize + 1]
                      - srcRow[ pixelSize + 2] - srcRow[-srcStride + pixelSize + 2] - srcRow[srcStride + pixelSize + 2];

                    dy = (int)
                        srcRow[ srcStride    ] + srcRow[ srcStride - pixelSize    ] + srcRow[ srcStride + pixelSize]
                      + srcRow[ srcStride + 1] + srcRow[ srcStride - pixelSize + 1] + srcRow[ srcStride + pixelSize + 1]
                      + srcRow[ srcStride + 2] + srcRow[ srcStride - pixelSize + 2] + srcRow[ srcStride + pixelSize + 2]
                      - srcRow[-srcStride    ] - srcRow[-srcStride - pixelSize    ] - srcRow[-srcStride + pixelSize]
                      - srcRow[-srcStride + 1] - srcRow[-srcStride - pixelSize + 1] - srcRow[-srcStride + pixelSize + 1]
                      - srcRow[-srcStride + 2] - srcRow[-srcStride - pixelSize + 2] - srcRow[-srcStride + pixelSize + 2];

                    COMPUTE_VALUE;

                    value = XINRANGE( value, 0, 255 );
                    dstRow[0] = (uint8_t) value;
                    dstRow[1] = (uint8_t) value;
                    dstRow[2] = (uint8_t) value;
                }

                // the last pixel
                dx = (int)
                    srcRow[-pixelSize    ] + srcRow[-srcStride - pixelSize    ] + srcRow[srcStride - pixelSize]
                  + srcRow[-pixelSize + 1] + srcRow[-srcStride - pixelSize + 1] + srcRow[srcStride - pixelSize + 1]
                  + srcRow[-pixelSize + 2] + srcRow[-srcStride - pixelSize + 2] + srcRow[srcStride - pixelSize + 2]
                  - srcRow[0] - srcRow[-srcStride    ] - srcRow[srcStride]
                  - srcRow[1] - srcRow[-srcStride + 1] - srcRow[srcStride + 1]
                  - srcRow[2] - srcRow[-srcStride + 2] - srcRow[srcStride + 2];

                dy = (int)
                    srcRow[ srcStride    ] * 2 + srcRow[ srcStride - pixelSize    ]
                  + srcRow[ srcStride + 1] * 2 + srcRow[ srcStride - pixelSize + 1]
                  + srcRow[ srcStride + 2] * 2 + srcRow[ srcStride - pixelSize + 2]
                  - srcRow[-srcStride    ] * 2 - srcRow[-srcStride - pixelSize    ]
                  - srcRow[-srcStride + 1] * 2 - srcRow[-srcStride - pixelSize + 1]
                  - srcRow[-srcStride + 2] * 2 - srcRow[-srcStride - pixelSize + 2];

                COMPUTE_VALUE;

                value = XINRANGE( value, 0, 255 );
                dstRow[0] = (uint8_t) value;
                dstRow[1] = (uint8_t) value;
                dstRow[2] = (uint8_t) value;
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
                  + srcRow[1] * 2 + srcRow[srcStride + 1]
                  + srcRow[2] * 2 + srcRow[srcStride + 2]
                  - srcRow[ pixelSize    ] * 2 - srcRow[srcStride + pixelSize]
                  - srcRow[ pixelSize + 1] * 2 - srcRow[srcStride + pixelSize + 1]
                  - srcRow[ pixelSize + 2] * 2 - srcRow[srcStride + pixelSize + 2];

                dy = (int)
                    srcRow[ srcStride    ] * 2 + srcRow[ srcStride + pixelSize]
                  + srcRow[ srcStride + 1] * 2 + srcRow[ srcStride + pixelSize + 1]
                  + srcRow[ srcStride + 2] * 2 + srcRow[ srcStride + pixelSize + 2]
                  - srcRow[0] * 2 - srcRow[pixelSize]
                  - srcRow[1] * 2 - srcRow[pixelSize + 1]
                  - srcRow[2] * 2 - srcRow[pixelSize + 2];

                COMPUTE_VALUE;

                value = XINRANGE( value, 0, 255 );
                dstRow[0] = (uint8_t) value;
                dstRow[1] = (uint8_t) value;
                dstRow[2] = (uint8_t) value;

                srcRow += pixelSize;
                dstRow += pixelSize;

                for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
                {
                    dx = (int)
                        srcRow[-pixelSize    ] * 2 + srcRow[srcStride - pixelSize]
                      + srcRow[-pixelSize + 1] * 2 + srcRow[srcStride - pixelSize + 1]
                      + srcRow[-pixelSize + 2] * 2 + srcRow[srcStride - pixelSize + 2]
                      - srcRow[ pixelSize    ] * 2 - srcRow[srcStride + pixelSize]
                      - srcRow[ pixelSize + 1] * 2 - srcRow[srcStride + pixelSize + 1]
                      - srcRow[ pixelSize + 2] * 2 - srcRow[srcStride + pixelSize + 2];

                    dy = (int)
                        srcRow[ srcStride    ] + srcRow[ srcStride - pixelSize    ] + srcRow[ srcStride + pixelSize]
                      + srcRow[ srcStride + 1] + srcRow[ srcStride - pixelSize + 1] + srcRow[ srcStride + pixelSize + 1]
                      + srcRow[ srcStride + 2] + srcRow[ srcStride - pixelSize + 2] + srcRow[ srcStride + pixelSize + 2]
                      - srcRow[0] - srcRow[-pixelSize    ] - srcRow[pixelSize]
                      - srcRow[1] - srcRow[-pixelSize + 1] - srcRow[pixelSize + 1]
                      - srcRow[2] - srcRow[-pixelSize + 2] - srcRow[pixelSize + 2];

                    COMPUTE_VALUE;

                    value = XINRANGE( value, 0, 255 );
                    dstRow[0] = (uint8_t) value;
                    dstRow[1] = (uint8_t) value;
                    dstRow[2] = (uint8_t) value;
                }

                // the last pixel
                dx = (int)
                    srcRow[-pixelSize    ] * 2 + srcRow[srcStride - pixelSize]
                  + srcRow[-pixelSize + 1] * 2 + srcRow[srcStride - pixelSize + 1]
                  + srcRow[-pixelSize + 2] * 2 + srcRow[srcStride - pixelSize + 2]
                  - srcRow[0] * 2 - srcRow[srcStride]
                  - srcRow[1] * 2 - srcRow[srcStride + 1]
                  - srcRow[2] * 2 - srcRow[srcStride + 2];

                dy = (int)
                    srcRow[ srcStride    ] * 2 + srcRow[ srcStride - pixelSize    ]
                  + srcRow[ srcStride + 1] * 2 + srcRow[ srcStride - pixelSize + 1]
                  + srcRow[ srcStride + 2] * 2 + srcRow[ srcStride - pixelSize + 2]
                  - srcRow[0] * 2 - srcRow[-pixelSize    ]
                  - srcRow[1] * 2 - srcRow[-pixelSize + 1]
                  - srcRow[2] * 2 - srcRow[-pixelSize + 2];

                COMPUTE_VALUE;

                value = XINRANGE( value, 0, 255 );
                dstRow[0] = (uint8_t) value;
                dstRow[1] = (uint8_t) value;
                dstRow[2] = (uint8_t) value;
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
                  + srcRow[1] * 2 + srcRow[-srcStride + 1]
                  + srcRow[2] * 2 + srcRow[-srcStride + 2]
                  - srcRow[pixelSize    ] * 2 - srcRow[-srcStride + pixelSize    ]
                  - srcRow[pixelSize + 1] * 2 - srcRow[-srcStride + pixelSize + 1]
                  - srcRow[pixelSize + 2] * 2 - srcRow[-srcStride + pixelSize + 2];

                dy = (int)
                    srcRow[0] * 2 + srcRow[pixelSize]
                  + srcRow[1] * 2 + srcRow[pixelSize + 1]
                  + srcRow[2] * 2 + srcRow[pixelSize + 2]
                  - srcRow[-srcStride    ] * 2 - srcRow[-srcStride + pixelSize]
                  - srcRow[-srcStride + 1] * 2 - srcRow[-srcStride + pixelSize + 1]
                  - srcRow[-srcStride + 2] * 2 - srcRow[-srcStride + pixelSize + 2];

                COMPUTE_VALUE;

                value = XINRANGE( value, 0, 255 );
                dstRow[0] = (uint8_t) value;
                dstRow[1] = (uint8_t) value;
                dstRow[2] = (uint8_t) value;

                srcRow += pixelSize;
                dstRow += pixelSize;

                for ( x = 1; x < widthM1; x++, srcRow += pixelSize, dstRow += pixelSize )
                {
                    dx = (int)
                        srcRow[-pixelSize    ] * 2 + srcRow[-srcStride - pixelSize    ]
                      + srcRow[-pixelSize + 1] * 2 + srcRow[-srcStride - pixelSize + 1]
                      + srcRow[-pixelSize + 2] * 2 + srcRow[-srcStride - pixelSize + 2]
                      - srcRow[ pixelSize    ] * 2 - srcRow[-srcStride + pixelSize    ]
                      - srcRow[ pixelSize + 1] * 2 - srcRow[-srcStride + pixelSize + 1]
                      - srcRow[ pixelSize + 2] * 2 - srcRow[-srcStride + pixelSize + 2];

                    dy = (int)
                        srcRow[0] + srcRow[-pixelSize    ] + srcRow[pixelSize]
                      + srcRow[1] + srcRow[-pixelSize + 1] + srcRow[pixelSize + 1]
                      + srcRow[2] + srcRow[-pixelSize + 2] + srcRow[pixelSize + 2]
                      - srcRow[-srcStride    ] - srcRow[-srcStride - pixelSize    ] - srcRow[-srcStride + pixelSize]
                      - srcRow[-srcStride + 1] - srcRow[-srcStride - pixelSize + 1] - srcRow[-srcStride + pixelSize + 1]
                      - srcRow[-srcStride + 2] - srcRow[-srcStride - pixelSize + 2] - srcRow[-srcStride + pixelSize + 2];

                    COMPUTE_VALUE;

                    value = XINRANGE( value, 0, 255 );
                    dstRow[0] = (uint8_t) value;
                    dstRow[1] = (uint8_t) value;
                    dstRow[2] = (uint8_t) value;
                }

                // the last pixel
                dx = (int)
                    srcRow[-pixelSize    ] * 2 + srcRow[-srcStride - pixelSize    ]
                  + srcRow[-pixelSize + 1] * 2 + srcRow[-srcStride - pixelSize + 1]
                  + srcRow[-pixelSize + 2] * 2 + srcRow[-srcStride - pixelSize + 2]
                  - srcRow[0] * 2 - srcRow[-srcStride    ]
                  - srcRow[1] * 2 - srcRow[-srcStride + 1]
                  - srcRow[2] * 2 - srcRow[-srcStride + 2];

                dy = (int)
                    srcRow[0] * 2 + srcRow[-pixelSize    ]
                  + srcRow[1] * 2 + srcRow[-pixelSize + 1]
                  + srcRow[2] * 2 + srcRow[-pixelSize + 2]
                  - srcRow[-srcStride    ] * 2 - srcRow[-srcStride - pixelSize    ]
                  - srcRow[-srcStride + 1] * 2 - srcRow[-srcStride - pixelSize + 1]
                  - srcRow[-srcStride + 2] * 2 - srcRow[-srcStride - pixelSize + 2];

                COMPUTE_VALUE;

                value = XINRANGE( value, 0, 255 );
                dstRow[0] = (uint8_t) value;
                dstRow[1] = (uint8_t) value;
                dstRow[2] = (uint8_t) value;
            }
        }
    }

    return ret;
}
