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
#include <math.h>

// forward declaration ----
static void RotateImageBilinear8 ( const ximage* src, ximage* dst, float angle, xargb fillColor );
static void RotateImageBilinear24( const ximage* src, ximage* dst, float angle, xargb fillColor );
static void RotateImageBilinear32( const ximage* src, ximage* dst, float angle, xargb fillColor );
// ------------------------

// Resize image using bilinear interpolation
XErrorCode RotateImageBilinear( const ximage* src, ximage* dst, float angle, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( src->format != dst->format )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( dst->width == src->width ) && ( dst->height == src->height ) && ( angle == 0 ) )
    {
        ret = XImageCopyData( src, dst );
    }
    else
    {
        if ( src->format == XPixelFormatGrayscale8 )
        {
            RotateImageBilinear8( src, dst, angle, fillColor );
        }
        else if ( src->format == XPixelFormatRGB24 )
        {
            RotateImageBilinear24( src, dst, angle, fillColor );
        }
        else
        {
            RotateImageBilinear32( src, dst, angle, fillColor );
        }
    }

    return ret;
}

// Rotate grayscale 8 bpp image
void RotateImageBilinear8( const ximage* src, ximage* dst, float angle, xargb fillColor )
{
    int      srcWidth    = src->width;
    int      srcHeight   = src->height;
    int      srcWidthM1  = src->width - 1;
    int      srcHeightM1 = src->height - 1;
    int      dstWidth    = dst->width;
    int      dstHeight   = dst->height;
    int      srcStride   = src->stride;
    int      dstStride   = dst->stride;
    uint8_t* srcPtr      = src->data;
    uint8_t* dstPtr      = dst->data;
    int      y;

    // images' radiuses
    double srcXradius = (double) ( srcWidth  - 1 ) / 2;
    double srcYradius = (double) ( srcHeight - 1 ) / 2;
    double dstXradius = (double) ( dstWidth  - 1 ) / 2;
    double dstYradius = (double) ( dstHeight - 1 ) / 2;

    // angle's sine and cosine
    double angleRad = -angle * XPI / 180;
    double angleCos = cos( angleRad );
    double angleSin = sin( angleRad );

    // fill values
    uint8_t fillValue = (uint8_t) ( RGB_TO_GRAY( fillColor.components.r, fillColor.components.g, fillColor.components.b ) * fillColor.components.a / 255 );

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, srcWidth, srcHeight, dstWidth, srcWidthM1, srcHeightM1, srcStride, dstStride, \
                                                      angleCos, angleSin, fillValue, srcXradius, srcYradius, dstXradius, dstYradius )
    for ( y = 0; y < dstHeight; y++ )
    {
        int         x;
        double      ox, oy, dx1, dy1, dx2, dy2;  // source point coordinates
        int         ox1, oy1, ox2, oy2;
        uint8_t*    dstRow = dstPtr + y * dstStride;
        uint8_t*    p1;
        uint8_t*    p2;
        uint8_t*    p3;
        uint8_t*    p4;

        // destination pixel's coordinate relative to image center
        double cx = -dstXradius;
        double cy = -dstYradius + y;

        // do some pre-calculations of source points' coordinates
        // (calculate the part which depends on y-loop, but does not depend on x-loop)
        double tx = angleSin * cy + srcXradius;
        double ty = angleCos * cy + srcYradius;

        for ( x = 0; x < dstWidth; x++ )
        {
            // get the source point coordinates
            ox = tx + angleCos * cx;
            oy = ty - angleSin * cx;

            // top-left coordinate
            ox1 = (int) ox;
            oy1 = (int) oy;

            // validate source pixel's coordinates
            if ( ( ox1 < 0 ) || ( oy1 < 0 ) || ( ox1 >= srcWidth ) || ( oy1 >= srcHeight ) )
            {
                // fill destination image with filler
                *dstRow = fillValue;
            }
            else
            {
                // bottom-right coordinate
                ox2 = ( ox1 == srcWidthM1 )  ? ox1 : ox1 + 1;
                oy2 = ( oy1 == srcHeightM1 ) ? oy1 : oy1 + 1;

                if ( ( dx1 = ox - ox1 ) < 0 )
                    dx1 = 0;
                dx2 = 1.0f - dx1;

                if ( ( dy1 = oy - oy1 ) < 0 )
                    dy1 = 0;
                dy2 = 1.0f - dy1;

                // get four points
                p1 = p2 = srcPtr + oy1 * srcStride;
                p1 += ox1;
                p2 += ox2;

                p3 = p4 = srcPtr + oy2 * srcStride;
                p3 += ox1;
                p4 += ox2;

                // interpolate using 4 points

                *dstRow = (uint8_t) (
                    dy2 * ( dx2 * *p1 + dx1 * *p2 ) +
                    dy1 * ( dx2 * *p3 + dx1 * *p4 ) );
            }

            cx++;
            dstRow++;
        }
    }
}

// Rotate color 24 bpp RGB image
void RotateImageBilinear24( const ximage* src, ximage* dst, float angle, xargb fillColor )
{
    int      srcWidth    = src->width;
    int      srcHeight   = src->height;
    int      srcWidthM1  = src->width - 1;
    int      srcHeightM1 = src->height - 1;
    int      dstWidth    = dst->width;
    int      dstHeight   = dst->height;
    int      srcStride   = src->stride;
    int      dstStride   = dst->stride;
    uint8_t* srcPtr      = src->data;
    uint8_t* dstPtr      = dst->data;
    int      y;

    // images' radiuses
    double srcXradius = (double) ( srcWidth  - 1 ) / 2;
    double srcYradius = (double) ( srcHeight - 1 ) / 2;
    double dstXradius = (double) ( dstWidth  - 1 ) / 2;
    double dstYradius = (double) ( dstHeight - 1 ) / 2;

    // angle's sine and cosine
    double angleRad = -angle * XPI / 180;
    double angleCos = cos( angleRad );
    double angleSin = sin( angleRad );

    // fill values
    uint8_t fillR = (uint8_t) ( fillColor.components.r * fillColor.components.a / 255 );
    uint8_t fillG = (uint8_t) ( fillColor.components.g * fillColor.components.a / 255 );
    uint8_t fillB = (uint8_t) ( fillColor.components.b * fillColor.components.a / 255 );

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, srcWidth, srcHeight, dstWidth, srcWidthM1, srcHeightM1, srcStride, dstStride, \
                                                      angleCos, angleSin, fillR, fillG, fillB, srcXradius, srcYradius, dstXradius, dstYradius )
    for ( y = 0; y < dstHeight; y++ )
    {
        int         x;
        double      ox, oy, dx1, dy1, dx2, dy2;  // source point coordinates
        int         ox1, oy1, ox2, oy2;
        uint8_t*    dstRow = dstPtr + y * dstStride;
        uint8_t*    p1;
        uint8_t*    p2;
        uint8_t*    p3;
        uint8_t*    p4;

        // destination pixel's coordinate relative to image center
        double cx = -dstXradius;
        double cy = -dstYradius + y;

        // do some pre-calculations of source points' coordinates
        // (calculate the part which depends on y-loop, but does not depend on x-loop)
        double tx = angleSin * cy + srcXradius;
        double ty = angleCos * cy + srcYradius;

        for ( x = 0; x < dstWidth; x++ )
        {
            // get the source point coordinates
            ox = tx + angleCos * cx;
            oy = ty - angleSin * cx;

            // top-left coordinate
            ox1 = (int) ox;
            oy1 = (int) oy;

            // validate source pixel's coordinates
            if ( ( ox1 < 0 ) || ( oy1 < 0 ) || ( ox1 >= srcWidth ) || ( oy1 >= srcHeight ) )
            {
                // fill destination image with filler
                dstRow[RedIndex]   = fillR;
                dstRow[GreenIndex] = fillG;
                dstRow[BlueIndex]  = fillB;
            }
            else
            {
                // bottom-right coordinate
                ox2 = ( ox1 == srcWidthM1 )  ? ox1 : ox1 + 1;
                oy2 = ( oy1 == srcHeightM1 ) ? oy1 : oy1 + 1;

                if ( ( dx1 = ox - ox1 ) < 0 )
                    dx1 = 0;
                dx2 = 1.0f - dx1;

                if ( ( dy1 = oy - oy1 ) < 0 )
                    dy1 = 0;
                dy2 = 1.0f - dy1;

                // get four points
                p1 = p2 = srcPtr + oy1 * srcStride;
                p1 += ox1 * 3;
                p2 += ox2 * 3;

                p3 = p4 = srcPtr + oy2 * srcStride;
                p3 += ox1 * 3;
                p4 += ox2 * 3;

                // interpolate using 4 points

                // red
                dstRow[RedIndex] = (uint8_t) (
                    dy2 * ( dx2 * p1[RedIndex] + dx1 * p2[RedIndex] ) +
                    dy1 * ( dx2 * p3[RedIndex] + dx1 * p4[RedIndex] ) );

                // green
                dstRow[GreenIndex] = (uint8_t) (
                    dy2 * ( dx2 * p1[GreenIndex] + dx1 * p2[GreenIndex] ) +
                    dy1 * ( dx2 * p3[GreenIndex] + dx1 * p4[GreenIndex] ) );

                // blue
                dstRow[BlueIndex] = (uint8_t) (
                    dy2 * ( dx2 * p1[BlueIndex] + dx1 * p2[BlueIndex] ) +
                    dy1 * ( dx2 * p3[BlueIndex] + dx1 * p4[BlueIndex] ) );
            }

            cx++;
            dstRow += 3;
        }
    }
}

// Rotate color 32 bpp RGBA image
void RotateImageBilinear32( const ximage* src, ximage* dst, float angle, xargb fillColor )
{
    int      srcWidth    = src->width;
    int      srcHeight   = src->height;
    int      srcWidthM1  = src->width - 1;
    int      srcHeightM1 = src->height - 1;
    int      dstWidth    = dst->width;
    int      dstHeight   = dst->height;
    int      srcStride   = src->stride;
    int      dstStride   = dst->stride;
    uint8_t* srcPtr      = src->data;
    uint8_t* dstPtr      = dst->data;
    int      y;

    // images' radiuses
    double srcXradius = (double) ( srcWidth  - 1 ) / 2;
    double srcYradius = (double) ( srcHeight - 1 ) / 2;
    double dstXradius = (double) ( dstWidth  - 1 ) / 2;
    double dstYradius = (double) ( dstHeight - 1 ) / 2;

    // angle's sine and cosine
    double angleRad = -angle * XPI / 180;
    double angleCos = cos( angleRad );
    double angleSin = sin( angleRad );

    // fill values
    uint8_t fillR = fillColor.components.r;
    uint8_t fillG = fillColor.components.g;
    uint8_t fillB = fillColor.components.b;
    uint8_t fillA = fillColor.components.a;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, srcWidth, srcHeight, dstWidth, srcWidthM1, srcHeightM1, srcStride, dstStride, \
                                                      angleCos, angleSin, fillR, fillG, fillB, fillA, srcXradius, srcYradius, dstXradius, dstYradius )
    for ( y = 0; y < dstHeight; y++ )
    {
        int         x;
        double      ox, oy, dx1, dy1, dx2, dy2;  // source point coordinates
        int         ox1, oy1, ox2, oy2;
        uint8_t*    dstRow = dstPtr + y * dstStride;
        uint8_t*    p1;
        uint8_t*    p2;
        uint8_t*    p3;
        uint8_t*    p4;

        // destination pixel's coordinate relative to image center
        double cx = -dstXradius;
        double cy = -dstYradius + y;

        // do some pre-calculations of source points' coordinates
        // (calculate the part which depends on y-loop, but does not depend on x-loop)
        double tx = angleSin * cy + srcXradius;
        double ty = angleCos * cy + srcYradius;

        for ( x = 0; x < dstWidth; x++ )
        {
            // get the source point coordinates
            ox = tx + angleCos * cx;
            oy = ty - angleSin * cx;

            // top-left coordinate
            ox1 = (int) ox;
            oy1 = (int) oy;

            // validate source pixel's coordinates
            if ( ( ox1 < 0 ) || ( oy1 < 0 ) || ( ox1 >= srcWidth ) || ( oy1 >= srcHeight ) )
            {
                // fill destination image with filler
                dstRow[RedIndex]   = fillR;
                dstRow[GreenIndex] = fillG;
                dstRow[BlueIndex]  = fillB;
                dstRow[AlphaIndex] = fillA;
            }
            else
            {
                // bottom-right coordinate
                ox2 = ( ox1 == srcWidthM1 )  ? ox1 : ox1 + 1;
                oy2 = ( oy1 == srcHeightM1 ) ? oy1 : oy1 + 1;

                if ( ( dx1 = ox - ox1 ) < 0 )
                    dx1 = 0;
                dx2 = 1.0f - dx1;

                if ( ( dy1 = oy - oy1 ) < 0 )
                    dy1 = 0;
                dy2 = 1.0f - dy1;

                // get four points
                p1 = p2 = srcPtr + oy1 * srcStride;
                p1 += ox1 * 4;
                p2 += ox2 * 4;

                p3 = p4 = srcPtr + oy2 * srcStride;
                p3 += ox1 * 4;
                p4 += ox2 * 4;

                // interpolate using 4 points

                // red
                dstRow[RedIndex] = (uint8_t) (
                    dy2 * ( dx2 * p1[RedIndex] + dx1 * p2[RedIndex] ) +
                    dy1 * ( dx2 * p3[RedIndex] + dx1 * p4[RedIndex] ) );

                // green
                dstRow[GreenIndex] = (uint8_t) (
                    dy2 * ( dx2 * p1[GreenIndex] + dx1 * p2[GreenIndex] ) +
                    dy1 * ( dx2 * p3[GreenIndex] + dx1 * p4[GreenIndex] ) );

                // blue
                dstRow[BlueIndex] = (uint8_t) (
                    dy2 * ( dx2 * p1[BlueIndex] + dx1 * p2[BlueIndex] ) +
                    dy1 * ( dx2 * p3[BlueIndex] + dx1 * p4[BlueIndex] ) );

                // alpha
                dstRow[AlphaIndex] = (uint8_t) (
                    dy2 * ( dx2 * p1[AlphaIndex] + dx1 * p2[AlphaIndex] ) +
                    dy1 * ( dx2 * p3[AlphaIndex] + dx1 * p4[AlphaIndex] ) );
            }

            cx++;
            dstRow += 4;
        }
    }
}

// Calculate image size for a rotated image, so it fit into the new size
XErrorCode CalculateRotatedImageSize( int32_t width, int32_t height, float angle, int32_t* newWidth, int32_t* newHeight )
{
    XErrorCode ret = SuccessCode;

    if ( ( newWidth == 0 ) || ( newHeight == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    if ( ( width < 1 ) || ( height < 1 ) )
    {
        ret = ErrorInvalidImageSize;
    }
    else
    {
        // angle's sine and cosine
        double angleRad = -angle * XPI / 180;
        double angleCos = cos( angleRad );
        double angleSin = sin( angleRad );

        // calculate half size
        double halfWidth  = (double) width  / 2;
        double halfHeight = (double) height / 2;

        // rotate corners
        double cx1 = halfWidth * angleCos;
        double cy1 = halfWidth * angleSin;

        double cx2 = halfWidth * angleCos - halfHeight * angleSin;
        double cy2 = halfWidth * angleSin + halfHeight * angleCos;

        double cx3 = -halfHeight * angleSin;
        double cy3 =  halfHeight * angleCos;

        double cx4 = 0;
        double cy4 = 0;

        // recalculate image size
        halfWidth  = XMAX( XMAX( cx1, cx2 ), XMAX( cx3, cx4 ) ) - XMIN( XMIN( cx1, cx2 ), XMIN( cx3, cx4 ) );
        halfHeight = XMAX( XMAX( cy1, cy2 ), XMAX( cy3, cy4 ) ) - XMIN( XMIN( cy1, cy2 ), XMIN( cy3, cy4 ) );

        *newWidth  = (int32_t) ( halfWidth  * 2 + 0.5 );
        *newHeight = (int32_t) ( halfHeight * 2 + 0.5 );
    }

    return ret;
}