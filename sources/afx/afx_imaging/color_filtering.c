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
static void ColorFiltering24( ximage* src, uint8_t minRed, uint8_t maxRed, uint8_t minGreen, uint8_t maxGreen,
                              uint8_t minBlue, uint8_t maxBlue, bool fillOutside, xargb fillColor );
static void ColorFiltering32( ximage* src, uint8_t minRed, uint8_t maxRed, uint8_t minGreen, uint8_t maxGreen,
                              uint8_t minBlue, uint8_t maxBlue, bool fillOutside, xargb fillColor );
// ------------------------

// Remove colors outside/inside of the specified range
XErrorCode ColorFiltering( ximage* src, uint8_t minRed, uint8_t maxRed, uint8_t minGreen, uint8_t maxGreen,
                           uint8_t minBlue, uint8_t maxBlue, bool fillOutside, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( src->format == XPixelFormatRGB24 )
        {
            ColorFiltering24( src, minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue, fillOutside, fillColor );
        }
        else if ( src->format == XPixelFormatRGBA32 )
        {
            ColorFiltering32( src, minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue, fillOutside, fillColor );
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }
    }

    return ret;
}

// Filters colors outside/inside certain distance from the sample color (0 - Euclidean, 1 - Manhattan)
XErrorCode ColorFilteringByDistance( ximage* src, xargb sampleColor, uint16_t maxDistance, uint8_t distanceType, bool fillOutside, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( src->format != XPixelFormatRGB24 )
        {
            ret = ErrorUnsupportedPixelFormat;
        }
        else
        {
            int      width  = src->width;
            int      stride = src->stride;
            uint8_t* ptr    = src->data;

            int y, height    = src->height;
            int maxDistance2 = maxDistance * maxDistance;

            int sampleR = sampleColor.components.r;
            int sampleG = sampleColor.components.g;
            int sampleB = sampleColor.components.b;

            // fill values
            uint8_t fillR = (uint8_t) ( fillColor.components.r * fillColor.components.a / 255 );
            uint8_t fillG = (uint8_t) ( fillColor.components.g * fillColor.components.a / 255 );
            uint8_t fillB = (uint8_t) ( fillColor.components.b * fillColor.components.a / 255 );

            if ( fillOutside == true )
            {
                if ( distanceType == 0 )
                {
                    #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, sampleR, sampleG, sampleB, maxDistance2 )
                    for ( y = 0; y < height; y++ )
                    {
                        uint8_t* row = ptr + y * stride;
                        int      dr, dg, db, x;

                        for ( x = 0; x < width; x++, row += 3 )
                        {
                            dr = sampleR - row[RedIndex];
                            dg = sampleG - row[GreenIndex];
                            db = sampleB - row[BlueIndex];

                            if ( dr * dr + dg * dg + db * db > maxDistance2 )
                            {
                                row[RedIndex]   = fillR;
                                row[GreenIndex] = fillG;
                                row[BlueIndex]  = fillB;
                            }
                        }
                    }
                }
                else
                {
                    #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, sampleR, sampleG, sampleB, maxDistance )
                    for ( y = 0; y < height; y++ )
                    {
                        uint8_t* row = ptr + y * stride;
                        int      dr, dg, db, x;

                        for ( x = 0; x < width; x++, row += 3 )
                        {
                            dr = sampleR - row[RedIndex];
                            dg = sampleG - row[GreenIndex];
                            db = sampleB - row[BlueIndex];

                            if ( dr < 0 ) dr = -dr;
                            if ( dg < 0 ) dg = -dg;
                            if ( db < 0 ) db = -db;

                            if ( dr + dg + db > maxDistance )
                            {
                                row[RedIndex]   = fillR;
                                row[GreenIndex] = fillG;
                                row[BlueIndex]  = fillB;
                            }
                        }
                    }
                }
            }
            else
            {
                if ( distanceType == 0 )
                {
                    #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, sampleR, sampleG, sampleB, maxDistance2 )
                    for ( y = 0; y < height; y++ )
                    {
                        uint8_t* row = ptr + y * stride;
                        int      dr, dg, db, x;

                        for ( x = 0; x < width; x++, row += 3 )
                        {
                            dr = sampleR - row[RedIndex];
                            dg = sampleG - row[GreenIndex];
                            db = sampleB - row[BlueIndex];

                            if ( dr * dr + dg * dg + db * db <= maxDistance2 )
                            {
                                row[RedIndex]   = fillR;
                                row[GreenIndex] = fillG;
                                row[BlueIndex]  = fillB;
                            }
                        }
                    }
                }
                else
                {
                    #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, sampleR, sampleG, sampleB, maxDistance )
                    for ( y = 0; y < height; y++ )
                    {
                        uint8_t* row = ptr + y * stride;
                        int      dr, dg, db, x;

                        for ( x = 0; x < width; x++, row += 3 )
                        {
                            dr = sampleR - row[RedIndex];
                            dg = sampleG - row[GreenIndex];
                            db = sampleB - row[BlueIndex];

                            if ( dr < 0 ) dr = -dr;
                            if ( dg < 0 ) dg = -dg;
                            if ( db < 0 ) db = -db;

                            if ( dr + dg + db <= maxDistance )
                            {
                                row[RedIndex]   = fillR;
                                row[GreenIndex] = fillG;
                                row[BlueIndex]  = fillB;
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// Remove colors outside/inside of the specified range in 24 bpp image
void ColorFiltering24( ximage* src, uint8_t minRed, uint8_t maxRed, uint8_t minGreen, uint8_t maxGreen,
                       uint8_t minBlue, uint8_t maxBlue, bool fillOutside, xargb fillColor )
{
    int      width  = src->width;
    int      stride = src->stride;
    uint8_t* ptr    = src->data;

    int y, height = src->height;

    // fill values
    uint8_t fillR = (uint8_t) ( fillColor.components.r * fillColor.components.a / 255 );
    uint8_t fillG = (uint8_t) ( fillColor.components.g * fillColor.components.a / 255 );
    uint8_t fillB = (uint8_t) ( fillColor.components.b * fillColor.components.a / 255 );

    if ( fillOutside == true )
    {
        #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, \
                minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            uint8_t  r, g, b;
            int x;

            for ( x = 0; x < width; x++, row += 3 )
            {
                r = row[RedIndex];
                g = row[GreenIndex];
                b = row[BlueIndex];

                if ( ( r < minRed   ) || ( r > maxRed   ) ||
                     ( g < minGreen ) || ( g > maxGreen ) ||
                     ( b < minBlue  ) || ( b > maxBlue  ) )
                {
                    row[RedIndex]   = fillR;
                    row[GreenIndex] = fillG;
                    row[BlueIndex]  = fillB;
                }
            }
        }
    }
    else
    {
        #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, \
                minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            uint8_t  r, g, b;
            int x;

            for ( x = 0; x < width; x++, row += 3 )
            {
                r = row[RedIndex];
                g = row[GreenIndex];
                b = row[BlueIndex];

                if ( ( r >= minRed   ) && ( r <= maxRed   ) &&
                     ( g >= minGreen ) && ( g <= maxGreen ) &&
                     ( b >= minBlue  ) && ( b <= maxBlue  ) )
                {
                    row[RedIndex]   = fillR;
                    row[GreenIndex] = fillG;
                    row[BlueIndex]  = fillB;
                }
            }
        }
    }
}

// Remove colors outside/inside of the specified range in 32 bpp image
void ColorFiltering32( ximage* src, uint8_t minRed, uint8_t maxRed, uint8_t minGreen, uint8_t maxGreen,
                       uint8_t minBlue, uint8_t maxBlue, bool fillOutside, xargb fillColor )
{
    int width  = src->width;
    int stride = src->stride;
    uint8_t* ptr = src->data;

    int y, height = src->height;

    uint8_t fillR = fillColor.components.r;
    uint8_t fillG = fillColor.components.g;
    uint8_t fillB = fillColor.components.b;
    uint8_t fillA = fillColor.components.a;

    if ( fillOutside == true )
    {
        #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, fillA, \
                minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            uint8_t  r, g, b;
            int x;

            for ( x = 0; x < width; x++, row += 4 )
            {
                r = row[RedIndex];
                g = row[GreenIndex];
                b = row[BlueIndex];

                if ( ( r < minRed   ) || ( r > maxRed   ) ||
                     ( g < minGreen ) || ( g > maxGreen ) ||
                     ( b < minBlue  ) || ( b > maxBlue  ) )
                {
                    row[RedIndex]   = fillR;
                    row[GreenIndex] = fillG;
                    row[BlueIndex]  = fillB;
                    row[AlphaIndex] = fillA;
                }
            }
        }
    }
    else
    {
        #pragma omp parallel for schedule(static) shared( ptr, width, stride, fillR, fillG, fillB, fillA, \
                minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            uint8_t  r, g, b;
            int x;

            for ( x = 0; x < width; x++, row += 4 )
            {
                r = row[RedIndex];
                g = row[GreenIndex];
                b = row[BlueIndex];

                if ( ( r >= minRed   ) && ( r <= maxRed   ) &&
                     ( g >= minGreen ) && ( g <= maxGreen ) &&
                     ( b >= minBlue  ) && ( b <= maxBlue  ) )
                {
                    row[RedIndex]   = fillR;
                    row[GreenIndex] = fillG;
                    row[BlueIndex]  = fillB;
                    row[AlphaIndex] = fillA;
                }
            }
        }
    }
}
