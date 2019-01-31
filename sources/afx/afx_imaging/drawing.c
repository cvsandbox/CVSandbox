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

// Draw a horizontal line
static void XDrawingHLine( ximage* image, int32_t x1, int32_t x2, int32_t y, xargb color )
{
    if ( ( y >= 0 ) && ( y < image->height ) && ( color.components.a != Transparent ) )
    {
        int stride = image->stride;
        int left   = XMIN( x1, x2 );
        int right  = XMAX( x1, x2 );
        int x;

        uint8_t fillA       = color.components.a;
        float   fillAlpha   = 0;
        float   fill1mAlpha = 0;

        if ( fillA != NotTransparent8bpp )
        {
            fillAlpha   = ( float ) fillA / 255.0f;
            fill1mAlpha = 1.0f - fillAlpha;
        }

        uint8_t* ptr = image->data + y * stride;

        left   = XMAX( 0, left );
        right  = XMIN( image->width - 1, right );

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) RGB_TO_GRAY( color.components.r, color.components.g, color.components.b );

            ptr += left;

            if ( fillA == NotTransparent8bpp )
            {
                // the fill color has no transparency, so just fill
                for ( x = left; x <= right; x++ )
                {
                    *ptr = fillValue;
                    ptr++;
                }
            }
            else
            {
                float fillValueA = fillAlpha * fillValue;

                // do alpha blending
                for ( x = left; x <= right; x++ )
                {
                    *ptr = (uint8_t) ( fillValueA + ( *ptr * fill1mAlpha ) );
                    ptr++;
                }

            }
        }
        else
        {
            uint8_t fillR     = color.components.r;
            uint8_t fillG     = color.components.g;
            uint8_t fillB     = color.components.b;
            int     pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

            ptr += left * pixelSize;

            // NOTE: for 32 bpp images we leave their alpha as is and don't take it into account for alpha blending

            if ( fillA == NotTransparent8bpp )
            {
                // the fill color has no transparency, so just fill
                for ( x = left; x <= right; x++ )
                {
                    ptr[RedIndex]   = fillR;
                    ptr[GreenIndex] = fillG;
                    ptr[BlueIndex]  = fillB;
                    ptr += pixelSize;
                }
            }
            else
            {
                float fillRA = fillAlpha * fillR;
                float fillGA = fillAlpha * fillG;
                float fillBA = fillAlpha * fillB;

                // do alpha blending
                for ( x = left; x <= right; x++ )
                {
                    ptr[RedIndex]   = (uint8_t) ( fillRA + ( ptr[RedIndex]   * fill1mAlpha ) );
                    ptr[GreenIndex] = (uint8_t) ( fillGA + ( ptr[GreenIndex] * fill1mAlpha ) );
                    ptr[BlueIndex]  = (uint8_t) ( fillBA + ( ptr[BlueIndex]  * fill1mAlpha ) );
                    ptr += pixelSize;
                }
            }
        }
    }
}

// Draw a vertical line
static void XDrawingVLine( ximage* image, int32_t y1, int32_t y2, int32_t x, xargb color )
{
    if ( ( x >= 0 ) && ( x < image->width ) )
    {
        int stride = image->stride;
        int top    = XMIN( y1, y2 );
        int bottom = XMAX( y1, y2 );
        int y;

        uint8_t fillA       = color.components.a;
        float   fillAlpha   = 0;
        float   fill1mAlpha = 0;

        if ( fillA != NotTransparent8bpp )
        {
            fillAlpha   = ( float ) fillA / 255.0f;
            fill1mAlpha = 1.0f - fillAlpha;
        }

        uint8_t* ptr = image->data;

        top    = XMAX( 0, top );
        bottom = XMIN( image->height - 1, bottom );

        ptr += top * stride;

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) RGB_TO_GRAY( color.components.r, color.components.g, color.components.b );

            ptr += x;

            if ( fillA == NotTransparent8bpp )
            {
                // the fill color has no transparency, so just fill
                for ( y = top; y <= bottom; y++ )
                {
                    *ptr = fillValue;
                    ptr += stride;
                }
            }
            else
            {
                float   fillValueA = fillAlpha * fillValue;

                // do alpha blending
                for ( y = top; y <= bottom; y++ )
                {
                    *ptr = ( uint8_t ) ( fillValueA + ( *ptr * fill1mAlpha ) );
                    ptr += stride;
                }
            }
        }
        else
        {
            uint8_t fillR     = color.components.r;
            uint8_t fillG     = color.components.g;
            uint8_t fillB     = color.components.b;
            int     pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

            ptr += x * pixelSize;

            // NOTE: for 32 bpp images we leave their alpha as is and don't take it into account for alpha blending

            if ( fillA == NotTransparent8bpp )
            {
                // the fill color has no transparency, so just fill
                for ( y = top; y <= bottom; y++ )
                {
                    ptr[RedIndex]   = fillR;
                    ptr[GreenIndex] = fillG;
                    ptr[BlueIndex]  = fillB;
                    ptr += stride;
                }
            }
            else
            {
                float fillRA = fillAlpha * fillR;
                float fillGA = fillAlpha * fillG;
                float fillBA = fillAlpha * fillB;

                // do alpha blending
                for ( y = top; y <= bottom; y++ )
                {
                    ptr[RedIndex]   = (uint8_t) ( fillRA + ( ptr[RedIndex]   * fill1mAlpha ) );
                    ptr[GreenIndex] = (uint8_t) ( fillGA + ( ptr[GreenIndex] * fill1mAlpha ) );
                    ptr[BlueIndex]  = (uint8_t) ( fillBA + ( ptr[BlueIndex]  * fill1mAlpha ) );
                    ptr += stride;
                }
            }
        }
    }
}

// Draw line between the specified points (all coordinates are inclusive)
XErrorCode XDrawingLine( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        if ( y1 == y2 )
        {
            XDrawingHLine( image, x1, x2, y1, color );
        }
        else if ( x1 == x2 )
        {
            XDrawingVLine( image, y1, y2, x1, color );
        }
        else
        {
            int dx  = x2 - x1;
            int dy  = y2 - y1;
            int sx  = ( x1 < x2 ) ? 1 : -1;
            int sy  = ( y1 < y2 ) ? 1 : -1;
            int err = 0, et ;

            int stride = image->stride;
            int width  = image->width;
            int height = image->height;

            uint8_t fillA       = color.components.a;
            float   fillAlpha   = 0;
            float   fill1mAlpha = 0;

            uint8_t* ptr = image->data;
            uint8_t* p;
            int32_t  x   = x1;
            int32_t  y   = y1;

            // correct deltas and calculate initial error
            if ( dx < 0 ) { dx = -dx; }
            if ( dy < 0 ) { dy = -dy; }
            err = ( ( dx > dy ) ? dx : -dy ) / 2;

            // check transparency
            if ( fillA != NotTransparent8bpp )
            {
                fillAlpha   = ( float ) fillA / 255.0f;
                fill1mAlpha = 1.0f - fillAlpha;
            }

            // define the common part of line drawing algorithm.
            // then only pixel setting should be defined for different image formats
#define DRAW_LINE_ALGORITHM                                                         \
            for ( ; ; )                                                             \
            {                                                                       \
                if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) )  \
                {                                                                   \
                    DRAW_PIXEL( x, y )                                              \
                }                                                                   \
                                                                                    \
                if ( ( x == x2 ) && ( y == y2 ) )                                   \
                {                                                                   \
                    break;                                                          \
                }                                                                   \
                                                                                    \
                et = err;                                                           \
                                                                                    \
                if ( et > -dx )                                                     \
                {                                                                   \
                    err -= dy;                                                      \
                    x   += sx;                                                      \
                }                                                                   \
                if ( et <  dy )                                                     \
                {                                                                   \
                    err += dx;                                                      \
                    y   += sy;                                                      \
                }                                                                   \
            }

            if ( image->format == XPixelFormatGrayscale8 )
            {
                uint8_t fillValue = (uint8_t) RGB_TO_GRAY( color.components.r, color.components.g, color.components.b );

                if ( fillA == NotTransparent8bpp )
                {
                    #define DRAW_PIXEL(x, y)  p = ptr + y * stride + x; *p = fillValue;

                    DRAW_LINE_ALGORITHM

                    #undef DRAW_PIXEL
                }
                else
                {
                    float fillValueA = fillAlpha * fillValue;

                    #define DRAW_PIXEL(x, y) p  = ptr + y * stride + x; *p = (uint8_t) ( fillValueA + ( *p * fill1mAlpha ) );

                    DRAW_LINE_ALGORITHM

                    #undef DRAW_PIXEL
                }
            }
            else
            {
                uint8_t fillR     = color.components.r;
                uint8_t fillG     = color.components.g;
                uint8_t fillB     = color.components.b;
                int     pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

                // NOTE: for 32 bpp images we leave their alpha as is and don't take it into account for alpha blending
                if ( fillA == NotTransparent8bpp )
                {
                    // the fill color has no transparency, so just fill
                    #define DRAW_PIXEL(x, y)  p = ptr + y * stride + x * pixelSize; p[RedIndex] = fillR; p[GreenIndex] = fillG; p[BlueIndex] = fillB;

                    DRAW_LINE_ALGORITHM

                    #undef DRAW_PIXEL
                }
                else
                {
                    float fillRA = fillAlpha * fillR;
                    float fillGA = fillAlpha * fillG;
                    float fillBA = fillAlpha * fillB;

                    // do alpha blending
                    #define DRAW_PIXEL(x, y)  p = ptr + y * stride + x * pixelSize;                 \
                            p[RedIndex]   = (uint8_t) ( fillRA + ( p[RedIndex]   * fill1mAlpha ) ); \
                            p[GreenIndex] = (uint8_t) ( fillGA + ( p[GreenIndex] * fill1mAlpha ) ); \
                            p[BlueIndex]  = (uint8_t) ( fillBA + ( p[BlueIndex]  * fill1mAlpha ) );

                    DRAW_LINE_ALGORITHM

                    #undef DRAW_PIXEL
                }
            }

            #undef DRAW_LINE_ALGORITHM
        }
    }

    return ret;
}

// Draw rectangle in the specified image with the specfied color (all coordinates are inclusive)
XErrorCode XDrawingRectanle( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        XDrawingHLine( image, x1, x2, y1, color );
        XDrawingHLine( image, x1, x2, y2, color );

        XDrawingVLine( image, y1 + 1, y2 - 1, x1, color );
        XDrawingVLine( image, y1 + 1, y2 - 1, x2, color );
    }

    return ret;
}

// Draw circle
XErrorCode XDrawingCircle( ximage* image, int32_t xc, int32_t yc, int32_t r, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int stride = image->stride;
        int width  = image->width;
        int height = image->height;
        int x      = 0;
        int y      = r;
        int dp     = 1 - r;
        int xt;
        int yt;

        uint8_t fillA       = color.components.a;
        float   fillAlpha   = 0;
        float   fill1mAlpha = 0;

        uint8_t* ptr = image->data;
        uint8_t* p;

        // check transparency
        if ( fillA != NotTransparent8bpp )
        {
            fillAlpha   = ( float ) fillA / 255.0f;
            fill1mAlpha = 1.0f - fillAlpha;
        }

        // Define the common part of circle drawing algorithm.
        // Then only pixel setting should be defined for different image formats.

        // Midpoint Circle Algorithm (http://www.jcomputers.us/vol8/jcp0801-09.pdf)
#define DRAW_CIRCLE_ALGORITHM                               \
        /* right/left and top/bottom pixels */              \
        xt = xc + r; DRAW_PIXEL( xt, yc )                   \
        xt = xc - r; DRAW_PIXEL( xt, yc )                   \
        yt = yc - r; DRAW_PIXEL( xc, yt )                   \
        yt = yc + r; DRAW_PIXEL( xc, yt )                   \
        for ( ; ; )                                         \
        {                                                   \
            if ( dp < 0 )                                   \
            {                                               \
                dp = dp + 2 * ( ++x ) + 1;                  \
            }                                               \
            else                                            \
            {                                               \
                dp = dp + 2 * ( ++x ) - 2 * ( --y ) + 1;    \
            }                                               \
                                                            \
            if ( x >= y ) break;                            \
                                                            \
            xt = xc + x; yt = yc + y; DRAW_PIXEL( xt, yt )  \
            xt = xc - x;              DRAW_PIXEL( xt, yt )  \
            xt = xc + x; yt = yc - y; DRAW_PIXEL( xt, yt )  \
            xt = xc - x;              DRAW_PIXEL( xt, yt )  \
            xt = xc + y; yt = yc + x; DRAW_PIXEL( xt, yt )  \
            xt = xc - y;              DRAW_PIXEL( xt, yt )  \
            xt = xc + y; yt = yc - x; DRAW_PIXEL( xt, yt )  \
            xt = xc - y;              DRAW_PIXEL( xt, yt )  \
        }                                                   \
        /* complete circle if required */                   \
        if ( x == y )                                       \
        {                                                   \
            xt = xc + x; yt = yc + y; DRAW_PIXEL( xt, yt )  \
            xt = xc - x;              DRAW_PIXEL( xt, yt )  \
            xt = xc + x; yt = yc - y; DRAW_PIXEL( xt, yt )  \
            xt = xc - x;              DRAW_PIXEL( xt, yt )  \
        }

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) RGB_TO_GRAY( color.components.r, color.components.g, color.components.b );

            if ( fillA == NotTransparent8bpp )
            {
                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x; *p = fillValue; }

                DRAW_CIRCLE_ALGORITHM

                #undef DRAW_PIXEL
            }
            else
            {
                float   fillValueA = fillAlpha * fillValue;

                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x; *p = (uint8_t) ( fillValueA + ( *p * fill1mAlpha ) ); }

                DRAW_CIRCLE_ALGORITHM

                #undef DRAW_PIXEL
            }
        }
        else
        {
            uint8_t fillR = color.components.r;
            uint8_t fillG = color.components.g;
            uint8_t fillB = color.components.b;
            int     pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

            // NOTE: for 32 bpp images we leave their alpha as is and don't take it into account for alpha blending

            if ( fillA == NotTransparent8bpp )
            {
                // the fill color has no transparency, so just fill
                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x * pixelSize; p[RedIndex] = fillR; p[GreenIndex] = fillG; p[BlueIndex] = fillB; }

                DRAW_CIRCLE_ALGORITHM

                #undef DRAW_PIXEL
            }
            else
            {
                float fillRA = fillAlpha * fillR;
                float fillGA = fillAlpha * fillG;
                float fillBA = fillAlpha * fillB;

                // do alpha blending
                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x * pixelSize; \
                                            p[RedIndex]   = ( uint8_t ) ( fillRA + ( p[RedIndex]   * fill1mAlpha ) );   \
                                            p[GreenIndex] = ( uint8_t ) ( fillGA + ( p[GreenIndex] * fill1mAlpha ) );   \
                                            p[BlueIndex]  = ( uint8_t ) ( fillBA + ( p[BlueIndex]  * fill1mAlpha ) );   }

                DRAW_CIRCLE_ALGORITHM

                #undef DRAW_PIXEL
            }
        }

        #undef DRAW_CIRCLE_ALGORITHM
    }

    return ret;
}

// Draw ellipse
XErrorCode XDrawingEllipse( ximage* image, int32_t xc, int32_t yc, int32_t rx, int32_t ry, xargb color )
{
        XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int stride  = image->stride;
        int width   = image->width;
        int height  = image->height;
        int a2      = rx * rx;
        int b2      = ry * ry;
        int twoa2   = 2 * a2;
        int twob2   = 2 * b2;
        int x       = 0;
        int y       = ry;
        int px      = 0;
        int py      = twoa2 * y;
        int xt, yt, dp;

        uint8_t fillA       = color.components.a;
        float   fillAlpha   = 0;
        float   fill1mAlpha = 0;

        uint8_t* ptr = image->data;
        uint8_t* p;

        // check transparency
        if ( fillA != NotTransparent8bpp )
        {
            fillAlpha   = ( float ) fillA / 255.0f;
            fill1mAlpha = 1.0f - fillAlpha;
        }

        // Define the common part of ellipse drawing algorithm.
        // Then only pixel setting should be defined for different image formats.

        // Midpoint Ellipse Algorithm (http://www.jcomputers.us/vol8/jcp0801-09.pdf)

#define DRAW_ELLIPSE_ALGORITHM                                      \
        /* right/left and top/bottom pixels */                      \
        xt = xc + rx; DRAW_PIXEL( xt, yc )                          \
        xt = xc - rx; DRAW_PIXEL( xt, yc )                          \
        yt = yc - ry; DRAW_PIXEL( xc, yt )                          \
        yt = yc + ry; DRAW_PIXEL( xc, yt )                          \
                                                                    \
        /* 1st segment */                                           \
        dp = ( int ) ( b2 - ( a2 * ry ) + ( 0.25 * a2 ) + 0.5 );    \
        while ( px < py )                                           \
        {                                                           \
            x++;                                                    \
            px += twob2;                                            \
            if ( dp < 0 )                                           \
            {                                                       \
                dp += b2 + px;                                      \
            }                                                       \
            else                                                    \
            {                                                       \
                y--;                                                \
                py -= twoa2;                                        \
                dp += b2 + px - py;                                 \
            }                                                       \
                                                                    \
            xt = xc + x; yt = yc + y; DRAW_PIXEL( xt, yt )          \
            xt = xc - x; yt = yc + y; DRAW_PIXEL( xt, yt )          \
            xt = xc + x; yt = yc - y; DRAW_PIXEL( xt, yt )          \
            xt = xc - x; yt = yc - y; DRAW_PIXEL( xt, yt )          \
        }                                                           \
                                                                    \
        /* 2nd segment */                                           \
        dp = (int) ( b2 * ( x + 0.5 ) * ( x + 0.5 ) +               \
                     a2 * ( y -   1 ) * ( y -   1 ) -               \
                     a2 * b2 + 0.5 );                               \
        while ( y > 1 )                                             \
        {                                                           \
            y--;                                                    \
            py -= twoa2;                                            \
            if ( dp > 0 )                                           \
            {                                                       \
                dp += a2 - py;                                      \
            }                                                       \
            else                                                    \
            {                                                       \
                x++;                                                \
                px += twob2;                                        \
                dp += a2 - py + px;                                 \
            }                                                       \
                                                                    \
            xt = xc + x; yt = yc + y; DRAW_PIXEL( xt, yt )          \
            xt = xc - x; yt = yc + y; DRAW_PIXEL( xt, yt )          \
            xt = xc + x; yt = yc - y; DRAW_PIXEL( xt, yt )          \
            xt = xc - x; yt = yc - y; DRAW_PIXEL( xt, yt )          \
        }

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) RGB_TO_GRAY( color.components.r, color.components.g, color.components.b );

            if ( fillA == NotTransparent8bpp )
            {
                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x; *p = fillValue; }

                DRAW_ELLIPSE_ALGORITHM

                #undef DRAW_PIXEL
            }
            else
            {
                float   fillValueA = fillAlpha * fillValue;

                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x; *p = (uint8_t) ( fillValueA + ( *p * fill1mAlpha ) ); }

                DRAW_ELLIPSE_ALGORITHM

                #undef DRAW_PIXEL
            }
        }
        else
        {
            uint8_t fillR = color.components.r;
            uint8_t fillG = color.components.g;
            uint8_t fillB = color.components.b;
            int     pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

            // NOTE: for 32 bpp images we leave their alpha as is and don't take it into account for alpha blending

            if ( fillA == NotTransparent8bpp )
            {
                // the fill color has no transparency, so just fill
                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x * pixelSize; p[RedIndex] = fillR; p[GreenIndex] = fillG; p[BlueIndex] = fillB; }

                DRAW_ELLIPSE_ALGORITHM

                #undef DRAW_PIXEL
            }
            else
            {
                float fillRA = fillAlpha * fillR;
                float fillGA = fillAlpha * fillG;
                float fillBA = fillAlpha * fillB;

                // do alpha blending
                #define DRAW_PIXEL(x, y) if ( ( x >= 0 ) && ( y >= 0 ) && ( x < width ) && ( y < height ) ) { p = ptr + y * stride + x * pixelSize; \
                                            p[RedIndex]   = ( uint8_t ) ( fillRA + ( p[RedIndex]   * fill1mAlpha ) );   \
                                            p[GreenIndex] = ( uint8_t ) ( fillGA + ( p[GreenIndex] * fill1mAlpha ) );   \
                                            p[BlueIndex]  = ( uint8_t ) ( fillBA + ( p[BlueIndex]  * fill1mAlpha ) );   }

                DRAW_ELLIPSE_ALGORITHM

                #undef DRAW_PIXEL
            }
        }

        #undef DRAW_ELLIPSE_ALGORITHM
    }

    return ret;

}

// Fill rectangle (with alpha blending) in the specified image with the specfied color (all coordinates are inclusive)
XErrorCode XDrawingBlendRectanle( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width    = image->width;
        int height   = image->height;
        int widthM1  = width - 1;
        int heightM1 = height - 1;

        int left     = XMIN( x1, x2 );
        int right    = XMAX( x1, x2 );
        int top      = XMIN( y1, y2 );
        int bottom   = XMAX( y1, y2 );

        left   = XMAX( 0, left );
        top    = XMAX( 0, top );
        right  = XMIN( widthM1, right );
        bottom = XMIN( heightM1, bottom );

        // fill rectangle only if it is inside of the image
        if ( ( right >= left ) && ( bottom >= top ) )
        {
            int      stride = image->stride;
            uint8_t* ptr    = image->data;
            uint8_t* row;
            int      x, y;

            uint8_t fillA       = color.components.a;
            float   fillAlpha   = 0;
            float   fill1mAlpha = 0;

            if ( fillA != NotTransparent8bpp )
            {
                fillAlpha   = (float) fillA / 255.0f;
                fill1mAlpha = 1.0f - fillAlpha;
            }

            if ( image->format == XPixelFormatGrayscale8 )
            {
                uint8_t fillValue = (uint8_t) RGB_TO_GRAY( color.components.r, color.components.g, color.components.b );

                if ( fillA == NotTransparent8bpp )
                {
                    for ( y = top; y <= bottom; y++ )
                    {
                        row = ptr + y * stride + left;

                        for ( x = left; x <= right; x++ )
                        {
                            *row = fillValue;
                            row++;
                        }
                    }
                }
                else
                {
                    float fillValueA = fillAlpha * fillValue;

                    // do alpha blending
                    for ( y = top; y <= bottom; y++ )
                    {
                        row = ptr + y * stride + left;

                        for ( x = left; x <= right; x++ )
                        {
                            *row = (uint8_t) ( fillValueA + ( *row * fill1mAlpha ) );
                            row++;
                        }
                    }
                }
            }
            else
            {
                // fill values
                uint8_t fillR     = color.components.r;
                uint8_t fillG     = color.components.g;
                uint8_t fillB     = color.components.b;
                int     pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

                // NOTE: for 32 bpp images we leave their alpha as is and don't take it into account for alpha blending

                if ( fillA == NotTransparent8bpp )
                {
                    for ( y = top; y <= bottom; y++ )
                    {
                        row = ptr + y * stride + left * pixelSize;

                        for ( x = left; x <= right; x++ )
                        {
                            row[RedIndex]   = fillR;
                            row[GreenIndex] = fillG;
                            row[BlueIndex]  = fillB;

                            row += pixelSize;
                        }
                    }
                }
                else
                {
                    float fillRA = fillAlpha * fillR;
                    float fillGA = fillAlpha * fillG;
                    float fillBA = fillAlpha * fillB;

                    // do alpha blending
                    for ( y = top; y <= bottom; y++ )
                    {
                        row = ptr + y * stride + left * pixelSize;

                        for ( x = left; x <= right; x++ )
                        {
                            row[RedIndex]   = (uint8_t) ( fillRA + ( row[RedIndex]   * fill1mAlpha ) );
                            row[GreenIndex] = (uint8_t) ( fillGA + ( row[GreenIndex] * fill1mAlpha ) );
                            row[BlueIndex]  = (uint8_t) ( fillBA + ( row[BlueIndex]  * fill1mAlpha ) );

                            row += pixelSize;
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// Fill frame with alpha blending (all coordinates are inclusive)
XErrorCode XDrawingBlendFrame( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t xThickness, uint32_t yThickness, xargb color )
{
    XErrorCode ret = SuccessCode;

    int xc = ( x2 - x1 + 1 ) / 2 + x1;
    int yc = ( y2 - y1 + 1 ) / 2 + y1;

    int x1r = x1 + xThickness - 1;
    int y1b = y1 + yThickness - 1;

    int x2l = x2 - xThickness + 1;
    int y2t = y2 - yThickness + 1;

    if ( x1r >= xc )
    {
        x1r = xc - 1;
    }
    if ( y1b >= yc )
    {
        y1b = yc - 1;
    }
    if ( x2l < xc )
    {
        x2l = xc;
    }
    if ( y2t < yc )
    {
        y2t = yc;
    }

    if ( ( ( ret = XDrawingBlendRectanle( image, x1, y1, x2, y1b, color ) ) == SuccessCode ) &&
         ( ( ret = XDrawingBlendRectanle( image, x1, y1b + 1, x1r, y2t - 1, color ) ) == SuccessCode ) &&
         ( ( ret = XDrawingBlendRectanle( image, x2l, y1b + 1, x2, y2t - 1, color ) ) == SuccessCode ) &&
         ( ( ret = XDrawingBlendRectanle( image, x1, y2t, x2, y2, color ) ) == SuccessCode ) )
    {
        // Good then
    }

    return ret;
}


// Fill circle (with alpha blending) in the specified image with the specfied color
XErrorCode XDrawingBlendCircle( ximage* image, int32_t xc, int32_t yc, int32_t r, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int x       = 0;
        int y       = r;
        int dp      = 1 - r;

        for ( ; ; )
        {
            if ( dp < 0 )
            {
                if ( x != 0 )
                {
                    XDrawingHLine( image, xc - y, xc + y, yc + x, color );
                    XDrawingHLine( image, xc - y, xc + y, yc - x, color );
                }
                else
                {
                    XDrawingHLine( image, xc - y, xc + y, yc, color );
                }

                dp = dp + 2 * ( ++x ) + 1;
            }
            else
            {
                XDrawingHLine( image, xc - x, xc + x, yc + y, color );
                XDrawingHLine( image, xc - x, xc + x, yc - y, color );

                XDrawingHLine( image, xc - y, xc + y, yc + x, color );
                XDrawingHLine( image, xc - y, xc + y, yc - x, color );

                dp = dp + 2 * ( ++x ) - 2 * ( --y ) + 1;
            }

            if ( x >= y ) break;
        }

        if ( x == y )
        {
            XDrawingHLine( image, xc - x, xc + x, yc + y, color );
            XDrawingHLine( image, xc - x, xc + x, yc - y, color );
        }
    }

    return ret;
}

// Helper to calculate X coordinate of innter circle
static int GetInnerXforOuterX( int outerX, int innerR )
{
    int x  = 0;
    int y  = innerR;
    int dp = 1 - innerR;


    while ( y > outerX )
    {
        if ( dp < 0 )
        {
            dp = dp + 2 * ( ++x ) + 1;
        }
        else
        {
            dp = dp + 2 * ( ++x ) - 2 * ( --y ) + 1;
        }

        if ( x >= y )
        {
            break;
        }
    }

    return x;
}

// Fill ring (with alpha blending) in the specified image with the specfied color
XErrorCode XDrawingBlendRing( ximage* image, int32_t xc, int32_t yc, int32_t r1, int32_t r2, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int rmax = XMAX( r1, r2 );
        int rmin = XMIN( r1, r2 );

        int x    = 0;
        int y    = rmax;
        int dp   = 1 - rmax;

        // maintain two sets of coordinates for different parts of the inner circle
        int ix1   = 0;
        int iy1   = rmin;
        int idp1  = 1 - rmin;

        int ix2   = 0;
        int iy2   = rmin;
        int idp2  = 1 - rmin;

        int inner1Done = 0;
        int inner2Done = 0;

        for ( ; ; )
        {
            if ( dp < 0 )
            {
                if ( x != 0 )
                {
                    if ( x >= rmin )
                    {
                        // these lines are above/below inner circle, so fill them from edge to another
                        XDrawingHLine( image, xc - y, xc + y, yc + x, color );
                        XDrawingHLine( image, xc - y, xc + y, yc - x, color );
                    }
                    else
                    {
                        // inner circle is on the way, each line splits into two
                        int innerX = ( inner1Done == 0 ) ? iy1 : GetInnerXforOuterX( x, rmin );

                        XDrawingHLine( image, xc - y, xc - innerX, yc + x, color );
                        XDrawingHLine( image, xc + innerX, xc + y, yc + x, color );

                        XDrawingHLine( image, xc - y, xc - innerX, yc - x, color );
                        XDrawingHLine( image, xc + innerX, xc + y, yc - x, color );
                    }
                }
                else
                {
                    if ( x >= rmin )
                    {
                        XDrawingHLine( image, xc - y, xc + y, yc, color );
                    }
                    else
                    {
                        XDrawingHLine( image, xc - y , xc - iy1, yc, color );
                        XDrawingHLine( image, xc + iy1, xc + y , yc, color );
                    }
                }

                dp = dp + 2 * ( ++x ) + 1;
            }
            else
            {
                if ( y >= rmin )
                {
                    // lines from edge to edge
                    XDrawingHLine( image, xc - x, xc + x, yc + y, color );
                    XDrawingHLine( image, xc - x, xc + x, yc - y, color );
                }
                else
                {
                    // split into two
                    XDrawingHLine( image, xc - x  , xc - ix2, yc + y, color );
                    XDrawingHLine( image, xc + ix2, xc + x  , yc + y, color );

                    XDrawingHLine( image, xc - x  , xc - ix2, yc - y, color );
                    XDrawingHLine( image, xc + ix2, xc + x  , yc - y, color );
                }

                if ( x >= rmin )
                {
                    // lines from edge to edge
                    XDrawingHLine( image, xc - y, xc + y, yc + x, color );
                    XDrawingHLine( image, xc - y, xc + y, yc - x, color );
                }
                else
                {
                    // split into two
                    int innerX = ( inner1Done == 0 ) ? iy1 : GetInnerXforOuterX( x, rmin );

                    XDrawingHLine( image, xc - y, xc - innerX, yc + x, color );
                    XDrawingHLine( image, xc + innerX, xc + y, yc + x, color );

                    XDrawingHLine( image, xc - y, xc - innerX, yc - x, color );
                    XDrawingHLine( image, xc + innerX, xc + y, yc - x, color );
                }

                dp = dp + 2 * ( ++x ) - 2 * ( --y ) + 1;
            }

            // update inner circles
            if ( inner1Done == 0 )
            {
                if ( idp1 < 0 )
                {
                    idp1 = idp1 + 2 * ( ++ix1 ) + 1;
                }
                else
                {
                    idp1 = idp1 + 2 * ( ++ix1 ) - 2 * ( --iy1 ) + 1;
                }

                if ( ix1 >= iy1 )
                {
                    inner1Done = 1;
                }
            }

            if ( inner2Done == 0 )
            {
                while ( y < iy2 )
                {
                    if ( idp2 < 0 )
                    {
                        idp2 = idp2 + 2 * ( ++ix2 ) + 1;
                    }
                    else
                    {
                        idp2 = idp2 + 2 * ( ++ix2 ) - 2 * ( --iy2 ) + 1;
                    }

                    if ( ix2 >= iy2 )
                    {
                        inner2Done = 1;
                        break;
                    }
                }
            }

            if ( x >= y ) break;
        }

        if ( x == y )
        {
            if ( y >= rmin )
            {
                XDrawingHLine( image, xc - x, xc + x, yc + y, color );
                XDrawingHLine( image, xc - x, xc + x, yc - y, color );
            }
            else
            {
                int innerX = GetInnerXforOuterX( x, rmin );

                XDrawingHLine( image, xc - x, xc - innerX, yc + y, color );
                XDrawingHLine( image, xc + innerX, xc + x, yc + y, color );

                XDrawingHLine( image, xc - x, xc - innerX, yc - y, color );
                XDrawingHLine( image, xc + innerX, xc + x, yc - y, color );
            }
        }
    }

    return ret;
}

// Fill pie (with alpha blending) between the specified start/end angles in the specified image
XErrorCode XDrawingBlendPie( ximage* image, int32_t xc, int32_t yc, int32_t r1, int32_t r2, float angleStart, float angleEnd,
                             xargb color, int32_t* buffer, uint32_t* bufferSize )
{
    XErrorCode ret           = SuccessCode;
    int        rmax          = XMAX( r1, r2 );
    int        rmin          = XMIN( r1, r2 );
    double     angleStartMod = fmod( angleStart, 360 );
    double     angleEndMod   = fmod( angleEnd, 360 );

    if ( angleStartMod < 0 )
    {
        angleStartMod += 360;
    }
    if ( angleEndMod < 0 )
    {
        angleEndMod += 360;
    }

    if ( angleStartMod == angleEndMod )
    {
        if ( rmin == 0 )
        {
            XDrawingBlendCircle( image, xc, yc, rmax, color );
        }
        else
        {
            XDrawingBlendRing( image, xc, yc, rmax, rmin, color );
        }
    }

    if ( ( image == 0 ) || ( buffer == 0 ) || ( bufferSize == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      diameter         = rmax * 2 + 1;
        uint32_t bufferSizeNeeded = diameter * 10;

        if ( *bufferSize < bufferSizeNeeded )
        {
            *bufferSize = bufferSizeNeeded;
            ret = ErrorTooSmallBuffer;
        }
        else
        {
            int x  = 0;
            int y  = rmax;
            int dp = 1 - rmax;
            int i;

            // We may have up to 3 horizontal lines with the same Y coordinate.
            // Initially we find lines, which would fill the circle.
            // Then the inner circle is calculated, which splits some of the lines into 2 segments.
            // Finally when cutting the ring, one of the lines may get cut again into two (in the
            // case if cut out segment is narraw).

            // IMPORTANT: don't change the order below; buffers are allocated within provided memory space
            int32_t* line1x1 = buffer;
            int32_t* line2x1 = line1x1 + diameter;
            int32_t* line3x1 = line2x1 + diameter;
            int32_t* line1x2 = line3x1 + diameter;
            int32_t* line2x2 = line1x2 + diameter;
            int32_t* line3x2 = line2x2 + diameter;

            // coordinates of start/end cutting lines
            int32_t* startCutLineX1 = line3x2 + diameter;
            int32_t* startCutLineX2 = startCutLineX1 + diameter;
            int32_t* endCutLineX1   = startCutLineX2 + diameter;
            int32_t* endCutLineX2   = endCutLineX1 + diameter;

            // initially set 2nd and 3rd segments as not existing (rendering and other calculations
            // suppose X2 coordinate of lines is greater than X1 coordinate)
            for ( i = 0; i < diameter; i++ )
            {
                line2x1[i] = -1;
                line2x2[i] = -2;

                line3x1[i] = -1;
                line3x2[i] = -2;

                startCutLineX1[i] = -1;
                startCutLineX2[i] = -2;

                endCutLineX1[i] = -1;
                endCutLineX2[i] = -2;
            }

            // outer circle - prepare lines which would fill a circle
            for ( ; ; )
            {
                if ( dp < 0 )
                {
                    if ( x != 0 )
                    {
                        line1x1[rmax + x] = xc - y;
                        line1x2[rmax + x] = xc + y;

                        line1x1[rmax - x] = xc - y;
                        line1x2[rmax - x] = xc + y;
                    }
                    else
                    {
                        line1x1[rmax] = xc - y;
                        line1x2[rmax] = xc + y;
                    }

                    dp = dp + 2 * ( ++x ) + 1;
                }
                else
                {
                    line1x1[rmax + y] = xc - x;
                    line1x2[rmax + y] = xc + x;

                    line1x1[rmax - y] = xc - x;
                    line1x2[rmax - y] = xc + x;

                    line1x1[rmax + x] = xc - y;
                    line1x2[rmax + x] = xc + y;

                    line1x1[rmax - x] = xc - y;
                    line1x2[rmax - x] = xc + y;

                    dp = dp + 2 * ( ++x ) - 2 * ( --y ) + 1;
                }

                if ( x >= y )
                {
                    if ( x == y )
                    {
                        line1x1[rmax + y] = xc - x;
                        line1x2[rmax + y] = xc + x;

                        line1x1[rmax - y] = xc - x;
                        line1x2[rmax - y] = xc + x;
                    }

                    break;
                }
            }

            // inner circle
            x  = 0;
            y  = rmin;
            dp = 1 - rmin;

            for ( ; ; )
            {
                if ( dp < 0 )
                {
                    if ( x != 0 )
                    {
                        line2x1[rmax + x] = xc + y;
                        line2x2[rmax + x] = line1x2[rmax + x];
                        line1x2[rmax + x] = xc - y;

                        line2x1[rmax - x] = xc + y;
                        line2x2[rmax - x] = line1x2[rmax - x];
                        line1x2[rmax - x] = xc - y;
                    }
                    else
                    {
                        line2x1[rmax] = xc + y;
                        line2x2[rmax] = line1x2[rmax];
                        line1x2[rmax] = xc - y;
                    }

                    dp = dp + 2 * ( ++x ) + 1;
                }
                else
                {
                    line2x1[rmax + x] = xc + y;
                    line2x2[rmax + x] = line1x2[rmax + x];
                    line1x2[rmax + x] = xc - y;

                    if ( x != 0 )
                    {
                        line2x1[rmax - x] = xc + y;
                        line2x2[rmax - x] = line1x2[rmax - x];
                        line1x2[rmax - x] = xc - y;
                    }

                    dp = dp + 2 * ( ++x ) - 2 * ( --y ) + 1;

                    if ( x <= y )
                    {
                        line2x1[rmax + y] = xc + x;
                        line2x2[rmax + y] = line1x2[rmax + y];
                        line1x2[rmax + y] = xc - x;

                        if ( y != 0 )
                        {
                            line2x1[rmax - y] = xc + x;
                            line2x2[rmax - y] = line1x2[rmax - y];
                            line1x2[rmax - y] = xc - x;
                        }
                    }
                }

                if ( x >= y ) break;
            }

            // cut the pie
            {
                double startAngleRad = XPI * angleStartMod / 180.0;
                double endAngleRad   = XPI * angleEndMod   / 180.0;

                int useSegment3ForSingleLine = 0;
                int useSegment3ForDoubleLine = 0;

                int32_t startCutX1 = xc;
                int32_t startCutY1 = yc;
                int32_t startCutX2 = (int32_t) ( xc + cos( startAngleRad ) * rmax + 0.5 );
                int32_t startCutY2 = (int32_t) ( yc - sin( startAngleRad ) * rmax + 0.5 );

                int32_t endCutX1 = xc;
                int32_t endCutY1 = yc;
                int32_t endCutX2 = (int32_t) ( xc + cos( endAngleRad ) * rmax + 0.5 );
                int32_t endCutY2 = (int32_t) ( yc - sin( endAngleRad ) * rmax + 0.5 );

                // check if 3rd segment is required at all
                if ( angleEndMod < angleStartMod )
                {
                    useSegment3ForSingleLine = 1;

                    // when we have 2 segments, we need to take extra care about them only when
                    // both angles are in the same quadrant
                    if ( ( (int) angleStartMod / 90 == (int) angleEndMod / 90 ) &&
                         ( fmod( angleStartMod, 90 ) != 0 ) && ( fmod( angleEndMod, 90 ) != 0 ) )
                    {
                        useSegment3ForDoubleLine = 1;
                    }
                }

                // render the line correponding to the start angle
                {
                    int dx  = startCutX2 - startCutX1;
                    int dy  = startCutY2 - startCutY1;
                    int sx  = ( startCutX1 < startCutX2 ) ? 1 : -1;
                    int sy  = ( startCutY1 < startCutY2 ) ? 1 : -1;
                    int err = 0, et;

                    int32_t  tx = startCutX1;
                    int32_t  ty = startCutY1;

                    // correct deltas and calculate initial error
                    if ( dx < 0 ) { dx = -dx; }
                    if ( dy < 0 ) { dy = -dy; }
                    err = ( ( dx > dy ) ? dx : -dy ) / 2;

                    for ( ; ; )
                    {
                        i = rmax - ( yc - ty );

                        if ( startCutLineX1[i] > startCutLineX2[i] )
                        {
                            startCutLineX1[i] = tx;
                            startCutLineX2[i] = tx;
                        }
                        else
                        {
                            if ( tx < startCutLineX1[i] )
                            {
                                startCutLineX1[i] = tx;
                            }
                            if ( tx > startCutLineX2[i] )
                            {
                                startCutLineX2[i] = tx;
                            }
                        }

                        if ( ( tx == startCutX2 ) && ( ty == startCutY2 ) )
                        {
                            break;
                        }

                        et = err;

                        if ( et > -dx )
                        {
                            err -= dy;
                            tx  += sx;
                        }
                        if ( et <  dy )
                        {
                            err += dx;
                            ty  += sy;
                        }
                    }
                }

                // render the line correponding to the end angle
                {
                    int dx  = endCutX2 - endCutX1;
                    int dy  = endCutY2 - endCutY1;
                    int sx  = ( endCutX1 < endCutX2 ) ? 1 : -1;
                    int sy  = ( endCutY1 < endCutY2 ) ? 1 : -1;
                    int err = 0, et;

                    int32_t  tx = endCutX1;
                    int32_t  ty = endCutY1;

                    // correct deltas and calculate initial error
                    if ( dx < 0 ) { dx = -dx; }
                    if ( dy < 0 ) { dy = -dy; }
                    err = ( ( dx > dy ) ? dx : -dy ) / 2;

                    for ( ; ; )
                    {
                        i = rmax - ( yc - ty );

                        if ( endCutLineX1[i] > endCutLineX2[i] )
                        {
                            endCutLineX1[i] = tx;
                            endCutLineX2[i] = tx;
                        }
                        else
                        {
                            if ( tx < endCutLineX1[i] )
                            {
                                endCutLineX1[i] = tx;
                            }
                            if ( tx > endCutLineX2[i] )
                            {
                                endCutLineX2[i] = tx;
                            }
                        }

                        if ( ( tx == endCutX2 ) && ( ty == endCutY2 ) )
                        {
                            break;
                        }

                        et = err;

                        if ( et > -dx )
                        {
                            err -= dy;
                            tx  += sx;
                        }
                        if ( et <  dy )
                        {
                            err += dx;
                            ty  += sy;
                        }
                    }
                }

                // perform segments cutting at the start angle
                {
                    int32_t  tx;
                    int32_t* xCoords = startCutLineX2;

                    if ( ( angleStartMod >= 180 ) && ( angleStartMod < 360 ) )
                    {
                        xCoords = startCutLineX1;
                    }

                    for ( i = 0; i < diameter; i++ )
                    {
                        // check the cut line exists at this hight
                        if ( startCutLineX1[i] <= startCutLineX2[i] )
                        {
                            tx = xCoords[i];

                            // the X coordinate must belong to 1st or 2nd segment
                            if ( ( ( tx >= line1x1[i] ) && ( tx <= line1x2[i] ) ) ||
                                 ( ( tx >= line2x1[i] ) && ( tx <= line2x2[i] ) ) )
                            {
                                if ( ( angleStartMod >= 180 ) && ( angleStartMod < 360 ) )
                                {
                                    if ( line2x1[i] > line2x2[i] )
                                    {
                                        // we have only one segment, so cut it
                                        if ( useSegment3ForSingleLine )
                                        {
                                            line3x1[i] = line1x1[i];
                                            line3x2[i] = tx;
                                        }
                                        line1x1[i] = tx;
                                    }
                                    else
                                    {
                                        // while cutting one of the line segment, create 3rd segment
                                        // (it will be removed if not need later)
                                        if ( angleStartMod < 270 )
                                        {
                                            if ( useSegment3ForDoubleLine == 1 )
                                            {
                                                line3x1[i] = line1x1[i];
                                                line3x2[i] = tx;
                                            }
                                            line1x1[i] = tx;
                                        }
                                        else
                                        {
                                            if ( useSegment3ForDoubleLine == 1 )
                                            {
                                                line3x1[i] = line2x1[i];
                                                line3x2[i] = tx;
                                            }
                                            line2x1[i] = tx;
                                        }
                                    }
                                }
                                else
                                {
                                    if ( line2x1[i] > line2x2[i] )
                                    {
                                        // we have only one segment, so cut it
                                        if ( useSegment3ForSingleLine )
                                        {
                                            line3x1[i] = tx;
                                            line3x2[i] = line1x2[i];
                                        }
                                        line1x2[i] = tx;
                                    }
                                    else
                                    {
                                        if ( angleStartMod < 90 )
                                        {
                                            if ( useSegment3ForDoubleLine == 1 )
                                            {
                                                line3x1[i] = tx;
                                                line3x2[i] = line2x2[i];
                                            }
                                            line2x2[i] = tx;
                                        }
                                        else
                                        {
                                            if ( useSegment3ForDoubleLine == 1 )
                                            {
                                                line3x1[i] = tx;
                                                line3x2[i] = line1x2[i];
                                            }
                                            line1x2[i] = tx;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // perform segments cutting at the end angle
                {
                    int32_t  tx;
                    int32_t* xCoords = endCutLineX2;

                    if ( ( angleEndMod > 0 ) && ( angleEndMod <= 180 ) )
                    {
                        xCoords = endCutLineX1;
                    }

                    for ( i = 0; i < diameter; i++ )
                    {
                        // check the cut line exists at this hight
                        if ( endCutLineX1[i] <= endCutLineX2[i] )
                        {
                            tx = xCoords[i];

                            // the X coordinate must belong to one of the segments
                            if ( ( ( tx >= line1x1[i] ) && ( tx <= line1x2[i] ) ) ||
                                 ( ( tx >= line2x1[i] ) && ( tx <= line2x2[i] ) ) ||
                                 ( ( tx >= line3x1[i] ) && ( tx <= line3x2[i] ) ) )
                            {
                                if ( ( angleEndMod > 0 ) && ( angleEndMod <= 180 ) )
                                {
                                    if ( line2x1[i] > line2x2[i] )
                                    {
                                        // we have only one segment, so cut it
                                        if ( line3x1[i] > line3x2[i] )
                                        {
                                            line1x1[i] = tx;
                                        }
                                        else
                                        {
                                            line3x1[i] = tx;
                                        }
                                    }
                                    else
                                    {
                                        if ( angleEndMod < 90 )
                                        {
                                            if ( line3x1[i] > line3x2[i] )
                                            {
                                                line2x1[i] = tx;
                                            }
                                            else
                                            {
                                                line3x1[i] = tx;
                                            }
                                        }
                                        else
                                        {
                                            if ( line3x1[i] > line3x2[i] )
                                            {
                                                line1x1[i] = tx;
                                            }
                                            else
                                            {
                                                line3x1[i] = tx;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if ( line2x1[i] > line2x2[i] )
                                    {
                                        // we have only one segment, so cut it
                                        if ( line3x1[i] > line3x2[i] )
                                        {
                                            line1x2[i] = tx;
                                        }
                                        else
                                        {
                                            line3x2[i] = tx;
                                        }
                                    }
                                    else
                                    {
                                        if ( ( angleEndMod >= 270 ) || ( angleEndMod == 0 ) )
                                        {
                                            if ( line3x1[i] > line3x2[i] )
                                            {
                                                line2x2[i] = tx;
                                            }
                                            else
                                            {
                                                line3x2[i] = tx;
                                            }
                                        }
                                        else
                                        {
                                            if ( line3x1[i] > line3x2[i] )
                                            {
                                                line1x2[i] = tx;
                                            }
                                            else
                                            {
                                                line3x2[i] = tx;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                // remove segments outside of the specified angles range
                int32_t* linesX1 = line1x1;
                int32_t* linesX2 = line1x2;

                for ( i = 0; i < diameter * 3; i++ )
                {
                    int j = i % diameter;
                    int minLength = ( i >= diameter * 2 ) ? 1 : 0;

                    if ( linesX2[i] - linesX1[i] >= minLength )
                    {
                        if ( ( i >= diameter * 2 )
                            ||
                             (
                               (
                                 ( ( startCutLineX1[j] > startCutLineX2[j] ) ||
                                   ( ( linesX1[i] != startCutLineX1[j] ) && ( linesX1[i] != startCutLineX2[j] ) &&
                                     ( linesX2[i] != startCutLineX1[j] ) && ( linesX2[i] != startCutLineX2[j] ) ) )
                                &&
                                 ( ( endCutLineX1[j] > endCutLineX2[j] ) ||
                                   ( ( linesX1[i] != endCutLineX1[j] ) && ( linesX1[i] != endCutLineX2[j] ) &&
                                     ( linesX2[i] != endCutLineX1[j] ) && ( linesX2[i] != endCutLineX2[j] ) ) )
                               )
                             )
                           )
                        {
                            // remove segments if their mid point is outside of the angles ranges
                            double a = atan2( rmax - j, ( linesX1[i] - xc ) + ( linesX2[i] - linesX1[i] ) / 2.0 ) * 180.0 / XPI;

                            if ( a < 0 ) a += 360;

                            if ( ( ( angleEndMod > angleStartMod ) && ( ( a < angleStartMod ) || ( a > angleEndMod ) ) ) ||
                                 ( ( angleEndMod < angleStartMod ) && ( ( a < angleStartMod ) && ( a > angleEndMod ) ) ) )
                            {
                                // remove it
                                linesX1[i] = -1;
                                linesX2[i] = -2;
                            }
                        }
                    }
                }
            }

            // render the pie
            for ( i = 0; i < diameter; i++ )
            {
                if ( line1x2[i] >= line1x1[i] )
                {
                    XDrawingHLine( image, line1x1[i], line1x2[i], yc - rmax + i, color );
                }

                if ( line2x2[i] >= line2x1[i] )
                {
                    XDrawingHLine( image, line2x1[i], line2x2[i], yc - rmax + i, color );
                }

                if ( line3x2[i] >= line3x1[i] )
                {
                    XDrawingHLine( image, line3x1[i], line3x2[i], yc - rmax + i, color );
                }
            }
        }
    }

    return ret;
}

// Fill ellipse (with alpha blending) in the specified image with the specfied color
XErrorCode XDrawingBlendEllipse( ximage* image, int32_t xc, int32_t yc, int32_t rx, int32_t ry, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int a2      = rx * rx;
        int b2      = ry * ry;
        int twoa2   = 2 * a2;
        int twob2   = 2 * b2;
        int x       = 0;
        int y       = ry;
        int px      = 0;
        int py      = twoa2 * y;
        int dp;

        /* 1st segment */
        dp = ( int ) ( b2 - ( a2 * ry ) + ( 0.25 * a2 ) + 0.5 );
        while ( px < py )
        {
            px += twob2;
            if ( dp < 0 )
            {
                dp += b2 + px;
            }
            else
            {
                XDrawingHLine( image, xc - x, xc + x, yc - y, color );
                XDrawingHLine( image, xc - x, xc + x, yc + y, color );

                y--;
                py -= twoa2;
                dp += b2 + px - py;
            }

            x++;
        }

        XDrawingHLine( image, xc - x, xc + x, yc - y, color );
        XDrawingHLine( image, xc - x, xc + x, yc + y, color );

        /* 2nd segment */
        dp = (int) ( b2 * ( x + 0.5 ) * ( x + 0.5 ) +
                     a2 * ( y -   1 ) * ( y -   1 ) -
                     a2 * b2 + 0.5 );
        while ( y > 1 )
        {
            y--;
            py -= twoa2;
            if ( dp > 0 )
            {
                dp += a2 - py;
            }
            else
            {
                x++;
                px += twob2;
                dp += a2 - py + px;
            }

            XDrawingHLine( image, xc - x, xc + x, yc - y, color );
            XDrawingHLine( image, xc - x, xc + x, yc + y, color );
        }

        // last line at Y center
        XDrawingHLine( image, xc - x, xc + x, yc, color );
    }

    return ret;
}

// Fill convex polygon (with alpha blending)
XErrorCode XDrawingBlendConvexPolygon( ximage* image, const xpoint* points, uint32_t pointsCount, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( points == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( pointsCount < 3 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        int32_t  yMin, yMax;
        uint32_t i, topPoint = 0, bottomPoint = 0;
        uint32_t currentLeftPoint, currentRightPoint;
        uint32_t nextLeftPoint, nextRightPoint;

        // find most top/botttom/left/right points
        yMin = yMax = points[0].y;
        for ( i = 1; i < pointsCount; i++ )
        {
            if ( points[i].y < yMin )
            {
                yMin = points[i].y;
                topPoint = i;
            }
            if ( points[i].y > yMax )
            {
                yMax = points[i].y;
                bottomPoint = i;
            }
        }

        currentLeftPoint = currentRightPoint = topPoint;
        nextLeftPoint    = ( ( topPoint == 0 ) ? pointsCount : topPoint ) - 1;
        nextRightPoint   = ( topPoint + 1 ) % pointsCount;

        {
            int32_t line1x1 = 0;
            int32_t line1y1 = 0;
            int32_t line1x2 = 0;
            int32_t line1y2 = 0;

            int32_t line2x1 = 0;
            int32_t line2y1 = 0;
            int32_t line2x2 = 0;
            int32_t line2y2 = 0;

            int32_t leftEdgeMinX  = points[topPoint].x;
            int32_t leftEdgeMaxX  = leftEdgeMinX;
            int32_t rightEdgeMinX = leftEdgeMinX;
            int32_t rightEdgeMaxX = leftEdgeMinX;

            int32_t leftEdgeMinXPrepared  = 0;
            int32_t leftEdgeMaxXPrepared  = 0;
            int32_t rightEdgeMinXPrepared = 0;
            int32_t rightEdgeMaxXPrepared = 0;

            int32_t x1 = 0, y1 = 0, dx1 = 0, dy1 = 0, sx1 = 0, sy1 = 0, err1 = 0;
            int32_t x2 = 0, y2 = 0, dx2 = 0, dy2 = 0, sx2 = 0, sy2 = 0, err2 = 0;
            int32_t et;

            uint8_t needNewLeftLine = 1, needNewRightLine = 1;
            uint8_t leftPointReady  = 0, rightPointReady  = 0;
            uint8_t leftLineDone    = 0, rightLineDone    = 0;

            for ( ;; )
            {
                if ( needNewLeftLine == 1 )
                {
                    needNewLeftLine = 0;

                    line1x1 = points[currentLeftPoint].x;
                    line1y1 = points[currentLeftPoint].y;
                    line1x2 = points[nextLeftPoint].x;
                    line1y2 = points[nextLeftPoint].y;

                    leftEdgeMinX = XMIN( leftEdgeMinX, line1x1 );
                    leftEdgeMaxX = XMAX( leftEdgeMaxX, line1x1 );

                    // horizontal line on the left
                    while ( ( line1y1 == line1y2 ) && ( nextLeftPoint != bottomPoint ) )
                    {
                        currentLeftPoint = nextLeftPoint;
                        nextLeftPoint    = ( ( currentLeftPoint == 0 ) ? pointsCount : currentLeftPoint ) - 1;

                        line1x1 = points[currentLeftPoint].x;
                        line1y1 = points[currentLeftPoint].y;
                        line1x2 = points[nextLeftPoint].x;
                        line1y2 = points[nextLeftPoint].y;

                        leftEdgeMinX = XMIN( leftEdgeMinX, line1x1 );
                        leftEdgeMaxX = XMAX( leftEdgeMaxX, line1x1 );
                    }

                    x1 = line1x1;
                    y1 = line1y1;

                    if ( line1y1 != line1y2 )
                    {
                        dx1  = line1x2 - line1x1;
                        dy1  = line1y2 - line1y1;
                        sx1  = ( line1x1 < line1x2 ) ? 1 : -1;
                        sy1  = ( line1y1 < line1y2 ) ? 1 : -1;
                        err1 = 0;

                        if ( dx1 < 0 ) { dx1 = -dx1; }
                        if ( dy1 < 0 ) { dy1 = -dy1; }
                        err1 = ( ( dx1 > dy1 ) ? dx1 : -dy1 ) / 2;
                    }
                    else
                    {
                        leftLineDone = 1;

                        leftEdgeMinX = XMIN( leftEdgeMinX, line1x2 );
                        leftEdgeMaxX = XMAX( leftEdgeMaxX, line1x2 );
                    }
                }

                if ( needNewRightLine == 1 )
                {
                    needNewRightLine = 0;

                    line2x1 = points[currentRightPoint].x;
                    line2y1 = points[currentRightPoint].y;
                    line2x2 = points[nextRightPoint].x;
                    line2y2 = points[nextRightPoint].y;

                    rightEdgeMinX = XMIN( rightEdgeMinX, line2x1 );
                    rightEdgeMaxX = XMAX( rightEdgeMaxX, line2x1 );

                    // horizontal line on the right
                    while ( ( line2y1 == line2y2 ) && ( nextRightPoint != bottomPoint ) )
                    {
                        currentRightPoint = nextRightPoint;
                        nextRightPoint    = ( currentRightPoint + 1 ) % pointsCount;

                        line2x1 = points[currentRightPoint].x;
                        line2y1 = points[currentRightPoint].y;
                        line2x2 = points[nextRightPoint].x;
                        line2y2 = points[nextRightPoint].y;

                        rightEdgeMinX = XMIN( rightEdgeMinX, line2x1 );
                        rightEdgeMaxX = XMAX( rightEdgeMaxX, line2x1 );
                    }

                    x2 = line2x1;
                    y2 = line2y1;

                    if ( line2y1 != line2y2 )
                    {
                        dx2  = line2x2 - line2x1;
                        dy2  = line2y2 - line2y1;
                        sx2  = ( line2x1 < line2x2 ) ? 1 : -1;
                        sy2  = ( line2y1 < line2y2 ) ? 1 : -1;
                        err2 = 0;

                        if ( dx2 < 0 ) { dx2 = -dx2; }
                        if ( dy2 < 0 ) { dy2 = -dy2; }
                        err2 = ( ( dx2 > dy2 ) ? dx2 : -dy2 ) / 2;
                    }
                    else
                    {
                        rightLineDone = 1;

                        rightEdgeMinX = XMIN( rightEdgeMinX, line2x2 );
                        rightEdgeMaxX = XMAX( rightEdgeMaxX, line2x2 );
                    }
                }

                // get new X of the line 1
                while ( ( leftPointReady == 0 ) && ( leftLineDone == 0 ) )
                {
                    if ( ( x1 == line1x2 ) && ( y1 == line1y2 ) )
                    {
                        if ( nextLeftPoint != bottomPoint )
                        {
                            needNewLeftLine  = 1;
                            currentLeftPoint = nextLeftPoint;
                            nextLeftPoint    = ( ( currentLeftPoint == 0 ) ? pointsCount : currentLeftPoint ) - 1;
                        }
                        else
                        {
                            leftLineDone = 1;
                        }
                        break;
                    }

                    if ( err1 < dy1 )
                    {
                        leftPointReady       = 1;
                        leftEdgeMinXPrepared = leftEdgeMinX;
                        leftEdgeMaxXPrepared = leftEdgeMaxX;
                    }

                    et = err1;
                    if ( et > -dx1 )
                    {
                        err1 -= dy1;
                        x1   += sx1;

                        leftEdgeMinX = XMIN( leftEdgeMinX, x1 );
                        leftEdgeMaxX = XMAX( leftEdgeMaxX, x1 );
                    }
                    if ( et < dy1 )
                    {
                        err1 += dx1;
                        y1   += sy1;

                        leftEdgeMinX = leftEdgeMaxX = x1;
                    }
                }

                // get new X of the line 2
                while ( ( rightPointReady == 0 ) && ( rightLineDone == 0 ) )
                {
                    if ( ( x2 == line2x2 ) && ( y2 == line2y2 ) )
                    {
                        if ( nextRightPoint != bottomPoint )
                        {
                            needNewRightLine  = 1;
                            currentRightPoint = nextRightPoint;
                            nextRightPoint    = ( currentRightPoint + 1 ) % pointsCount;
                        }
                        else
                        {
                            rightLineDone = 1;
                        }
                        break;
                    }

                    if ( err2 < dy2 )
                    {
                        rightPointReady       = 1;
                        rightEdgeMinXPrepared = rightEdgeMinX;
                        rightEdgeMaxXPrepared = rightEdgeMaxX;
                    }

                    et = err2;

                    if ( et > -dx2 )
                    {
                        err2 -= dy2;
                        x2   += sx2;

                        rightEdgeMinX = XMIN( rightEdgeMinX, x2 );
                        rightEdgeMaxX = XMAX( rightEdgeMaxX, x2 );
                    }
                    if ( et < dy2 )
                    {
                        err2 += dx2;
                        y2   += sy2;

                        rightEdgeMinX = rightEdgeMaxX = x2;
                    }
                }

                if ( ( leftPointReady == 1 ) && ( rightPointReady == 1 ) )
                {
                    int minX = XMIN( XMIN( leftEdgeMinXPrepared, leftEdgeMaxXPrepared ), XMIN( rightEdgeMinXPrepared, rightEdgeMaxXPrepared ) );
                    int maxX = XMAX( XMAX( leftEdgeMinXPrepared, leftEdgeMaxXPrepared ), XMAX( rightEdgeMinXPrepared, rightEdgeMaxXPrepared ) );

                    XDrawingHLine( image, minX, maxX, y1 - 1, color );

                    leftPointReady = rightPointReady = 0;
                }

                if ( ( leftLineDone == 1 ) || ( rightLineDone == 1 ) )
                {
                    int minX = XMIN( XMIN( leftEdgeMinX, leftEdgeMaxX ), XMIN( rightEdgeMinX, rightEdgeMaxX ) );
                    int maxX = XMAX( XMAX( leftEdgeMinX, leftEdgeMaxX ), XMAX( rightEdgeMinX, rightEdgeMaxX ) );

                    XDrawingHLine( image, minX, maxX, y1, color );

                    break;
                }
            }
        }
    }

    return ret;
}

// Fill rectangle (no alpha blending) in the specified image with the specfied color (all coordinates are inclusive)
XErrorCode XDrawingFillRectanle( ximage* image, int32_t x1, int32_t y1, int32_t x2, int32_t y2, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width    = image->width;
        int height   = image->height;
        int widthM1  = width - 1;
        int heightM1 = height - 1;

        int left     = XMIN( x1, x2 );
        int right    = XMAX( x1, x2 );
        int top      = XMIN( y1, y2 );
        int bottom   = XMAX( y1, y2 );

        left   = XMAX( 0, left );
        top    = XMAX( 0, top );
        right  = XMIN( widthM1, right );
        bottom = XMIN( heightM1, bottom );

        // fill rectangle only if it is inside of the image
        if ( ( right >= left ) && ( bottom >= top ) )
        {
            int      stride = image->stride;
            int      x, y;
            uint8_t* ptr = image->data;
            uint8_t* row;

            if ( image->format == XPixelFormatGrayscale8 )
            {
                uint8_t fillValue = (uint8_t) ( RGB_TO_GRAY( color.components.r, color.components.g, color.components.b ) * color.components.a / 255 );

                for ( y = top; y <= bottom; y++ )
                {
                    row = ptr + y * stride + left;

                    for ( x = left; x <= right; x++ )
                    {
                        *row = fillValue;
                        row++;
                    }
                }
            }
            else if ( image->format == XPixelFormatRGB24 )
            {
                // fill values
                uint8_t fillR = (uint8_t) ( color.components.r * color.components.a / 255 );
                uint8_t fillG = (uint8_t) ( color.components.g * color.components.a / 255 );
                uint8_t fillB = (uint8_t) ( color.components.b * color.components.a / 255 );

                for ( y = top; y <= bottom; y++ )
                {
                    row = ptr + y * stride + left * 3;

                    for ( x = left; x <= right; x++ )
                    {
                        row[RedIndex]   = fillR;
                        row[GreenIndex] = fillG;
                        row[BlueIndex]  = fillB;
                        row += 3;
                    }
                }
            }
            else
            {
                // fill values
                uint8_t fillR = color.components.r;
                uint8_t fillG = color.components.g;
                uint8_t fillB = color.components.b;
                uint8_t fillA = color.components.a;

                for ( y = top; y <= bottom; y++ )
                {
                    row = ptr + y * stride + left * 4;

                    for ( x = left; x <= right; x++ )
                    {
                        row[RedIndex]   = fillR;
                        row[GreenIndex] = fillG;
                        row[BlueIndex]  = fillB;
                        row[AlphaIndex] = fillA;
                        row += 4;
                    }
                }
            }
        }
    }

    return ret;
}

// Fill entire image with the specfied color
XErrorCode XDrawingFillImage( ximage* image, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( ( image->width != 0 ) && ( image->height != 0 ) )
        {
            ret = XDrawingFillRectanle( image, 0, 0, image->width - 1, image->height - 1, color );
        }
    }

    return ret;
}

// Fill image with the specified color according to the mask image
XErrorCode XDrawingMaskedFill( ximage* image, const ximage* mask, int32_t maskX, int32_t maskY, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( mask == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( mask->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorInvalidArgument;
    }
    // check that mask image overlaps the image
    else if ( ( maskX + mask->width > 0 ) &&
              ( maskY + mask->height > 0 ) &&
              ( maskX < image->width ) &&
              ( maskY < image->height ) )
    {
        int startX = maskX;
        int startY = maskY;
        int stopX  = XMIN( image->width,  maskX + mask->width );
        int stopY  = XMIN( image->height, maskY + mask->height );
        int stride = image->stride;
        int maskStride = mask->stride;
        int y;

        uint8_t* imagePtr = image->data;
        uint8_t* maskPtr  = mask->data;

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) RGB_TO_GRAY( fillColor.components.r,
                                                       fillColor.components.g,
                                                       fillColor.components.b );

            #pragma omp parallel for schedule(static) shared( startX, stopX, imagePtr, maskPtr, stride, maskStride, fillValue )
            for ( y = startY; y < stopY; y++ )
            {
                uint8_t* imageRow = imagePtr + y * stride + startX;
                uint8_t* maskRow  = maskPtr + y * maskStride + startX;
                float    fillCoef, imageCoef;
                int      x;

                for ( x = startX; x < stopX; x++ )
                {
                    fillCoef  = (float)( *maskRow ) / 255.0f;
                    imageCoef = 1.0f - fillCoef;

                    *imageRow = (uint8_t) ( fillCoef  * fillValue +
                                           imageCoef * *imageRow );

                    imageRow++;
                    maskRow++;
                }
            }
        }
        else
        {
            int pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

            #pragma omp parallel for schedule(static) shared( startX, stopX, imagePtr, maskPtr, stride, maskStride, pixelSize )
            for ( y = startY; y < stopY; y++ )
            {
                uint8_t* imageRow = imagePtr + y * stride + startX * pixelSize;
                uint8_t* maskRow  = maskPtr + y * maskStride + startX;
                float    fillCoef, imageCoef;
                int      x;

                for ( x = startX; x < stopX; x++ )
                {
                    fillCoef  = (float)( *maskRow ) / 255.0f;
                    imageCoef = 1.0f - fillCoef;

                    imageRow[RedIndex]   = (uint8_t)( fillCoef  * fillColor.components.r +
                                                      imageCoef * imageRow[RedIndex] );
                    imageRow[GreenIndex] = (uint8_t)( fillCoef  * fillColor.components.g +
                                                      imageCoef * imageRow[GreenIndex] );
                    imageRow[BlueIndex]  = (uint8_t)( fillCoef  * fillColor.components.b +
                                                      imageCoef * imageRow[BlueIndex] );

                    if ( pixelSize == 4 )
                    {
                        imageRow[AlphaIndex]  = (uint8_t)( fillCoef  * fillColor.components.a +
                                                           imageCoef * imageRow[AlphaIndex] );
                    }

                    imageRow += pixelSize;
                    maskRow++;
                }
            }
        }
    }

    return ret;
}

// Fill image with the specified image according to the mask image (both images mush have same pixel format)
XErrorCode XDrawingMaskedImageFill( ximage* image, const ximage* fillImage, const ximage* mask, int32_t maskX, int32_t maskY )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( fillImage == 0 ) || ( mask == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( image->format != fillImage->format )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( mask->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorInvalidArgument;
    }
    // check that mask image overlaps the image
    else if ( ( maskX + mask->width > 0 ) &&
              ( maskY + mask->height > 0 ) &&
              ( maskX < image->width ) &&
              ( maskY < image->height ) )
    {
        int startX = maskX;
        int startY = maskY;
        // fill image starts at (0, 0) of the image to fill, i.e. its not moving as mask moves
        // so changing position of mask will:
        // 1) fill different part of the source image;
        // 2) use different pixels from the fill image
        int stopX  = XMIN3( image->width,  maskX + mask->width,  fillImage->width );
        int stopY  = XMIN3( image->height, maskY + mask->height, fillImage->height );
        int stride = image->stride;
        int fillStride = fillImage->stride;
        int maskStride = mask->stride;
        int y;

        uint8_t* imagePtr = image->data;
        uint8_t* fillPtr  = fillImage->data;
        uint8_t* maskPtr  = mask->data;

        if ( image->format == XPixelFormatGrayscale8 )
        {
            #pragma omp parallel for schedule(static) shared( startX, stopX, imagePtr, fillPtr, maskPtr, stride, fillStride, maskStride )
            for ( y = startY; y < stopY; y++ )
            {
                uint8_t* imageRow = imagePtr + y * stride + startX;
                uint8_t* fillRow  = fillPtr + y * fillStride + startX;
                uint8_t* maskRow  = maskPtr + y * maskStride + startX;
                float    fillCoef, imageCoef;
                int      x;

                for ( x = startX; x < stopX; x++ )
                {
                    fillCoef  = (float)( *maskRow ) / 255.0f;
                    imageCoef = 1.0f - fillCoef;

                    *imageRow = (uint8_t)( fillCoef  * *fillRow + imageCoef * *imageRow );

                    imageRow++;
                    fillRow++;
                    maskRow++;
                }
            }
        }
        else
        {
            int pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;

            #pragma omp parallel for schedule(static) shared( startX, stopX, imagePtr, fillPtr, maskPtr, stride, fillStride, maskStride, pixelSize )
            for ( y = startY; y < stopY; y++ )
            {
                uint8_t* imageRow = imagePtr + y * stride + startX * pixelSize;
                uint8_t* fillRow  = fillPtr + y * fillStride + startX * pixelSize;
                uint8_t* maskRow  = maskPtr + y * maskStride + startX;
                float    fillCoef, imageCoef;
                int      x;

                for ( x = startX; x < stopX; x++ )
                {
                    fillCoef  = (float)( *maskRow ) / 255.0f;
                    imageCoef = 1.0f - fillCoef;

                    imageRow[RedIndex]   = (uint8_t)( fillCoef  * fillRow[RedIndex] +
                                                      imageCoef * imageRow[RedIndex] );
                    imageRow[GreenIndex] = (uint8_t)( fillCoef  * fillRow[GreenIndex] +
                                                      imageCoef * imageRow[GreenIndex] );
                    imageRow[BlueIndex]  = (uint8_t)( fillCoef  * fillRow[BlueIndex] +
                                                      imageCoef * imageRow[BlueIndex] );

                    if ( pixelSize == 4 )
                    {
                        imageRow[AlphaIndex]  = (uint8_t)( fillCoef  * fillRow[AlphaIndex] +
                                                           imageCoef * imageRow[AlphaIndex] );
                    }

                    imageRow += pixelSize;
                    fillRow  += pixelSize;
                    maskRow++;
                }
            }
        }
    }

    return ret;
}

// Draw one image on another at the specified location
XErrorCode XDrawingImage( ximage* image, const ximage* imageToDraw, int32_t x, int32_t y )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( imageToDraw == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( image->format == imageToDraw->format )
    {
        ret = XImagePutImage( image, imageToDraw, x, y );
    }
    else if ( ( ( image->format != XPixelFormatGrayscale8 ) && ( image->format != XPixelFormatRGB24 ) && ( image->format != XPixelFormatRGBA32 ) ) ||
              ( ( imageToDraw->format != XPixelFormatGrayscale8 ) && ( imageToDraw->format != XPixelFormatRGB24 ) && ( imageToDraw->format != XPixelFormatRGBA32 ) ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int32_t targetX = x;
        int32_t targetY = y;
        int32_t srcSubX = 0;
        int32_t srcSubY = 0;
        int32_t srcSubW = imageToDraw->width;
        int32_t srcSubH = imageToDraw->height;

        if ( targetX < 0 )
        {
            srcSubX -= targetX;
            srcSubW += targetX;
            targetX = 0;
        }
        if ( targetY < 0 )
        {
            srcSubY -= targetY;
            srcSubH += targetY;
            targetY = 0;
        }

        // make sure destination rectangle still overlaps the destination image
        if ( ( srcSubW > 0 ) && ( srcSubH > 0 ) && ( targetX < image->width ) && ( targetY < image->height ) )
        {
            int srcStride    = imageToDraw->stride;
            int dstStride    = image->stride;
            int srcPixelSize = ( imageToDraw->format == XPixelFormatGrayscale8 ) ? 1 : ( ( imageToDraw->format == XPixelFormatRGB24 ) ? 3 : 4 );
            int dstPixelSize = ( image->format == XPixelFormatGrayscale8 ) ? 1 : ( ( image->format == XPixelFormatRGB24 ) ? 3 : 4 );
            int srcOffset    = 0;
            int dstOffset    = 0;
            uint8_t* srcPtr  = imageToDraw->data + srcSubY * srcStride + srcSubX * srcPixelSize;
            uint8_t* dstPtr  = image->data + targetY * dstStride + targetX * dstPixelSize;
            int tx, ty;

            // make further clipping if required
            if ( targetX + srcSubW > image->width )
            {
                srcSubW = image->width - targetX;
            }
            if ( targetY + srcSubH > image->height )
            {
                srcSubH = image->height - targetY;
            }

            // calculate offsets now
            srcOffset = srcStride - srcSubW * srcPixelSize;
            dstOffset = dstStride - srcSubW * dstPixelSize;

            if ( ( dstPixelSize == 1 ) && ( srcPixelSize == 3 ) )
            {
                // draw RGB image on Grayscale
                for ( ty = 0; ty < srcSubH; ty++ )
                {
                    for ( tx = 0; tx < srcSubW; tx++, dstPtr++, srcPtr += 3 )
                    {
                        *dstPtr = (uint8_t) RGB_TO_GRAY( srcPtr[RedIndex], srcPtr[GreenIndex], srcPtr[BlueIndex] );
                    }
                    srcPtr += srcOffset;
                    dstPtr += dstOffset;
                }
            }
            else if ( ( ( dstPixelSize == 3 ) || ( dstPixelSize == 4 ) ) && ( srcPixelSize == 1 ) )
            {
                // draw Grayscale image on RGB
                for ( ty = 0; ty < srcSubH; ty++ )
                {
                    for ( tx = 0; tx < srcSubW; tx++, srcPtr++, dstPtr += dstPixelSize )
                    {
                        dstPtr[RedIndex]   = *srcPtr;
                        dstPtr[GreenIndex] = *srcPtr;
                        dstPtr[BlueIndex]  = *srcPtr;
                    }
                    srcPtr += srcOffset;
                    dstPtr += dstOffset;
                }
            }
            else if ( ( dstPixelSize == 4 ) && ( srcPixelSize == 3 ) )
            {
                // draw Grayscale image on RGB
                for ( ty = 0; ty < srcSubH; ty++ )
                {
                    for ( tx = 0; tx < srcSubW; tx++, srcPtr += 3, dstPtr += 4 )
                    {
                        dstPtr[RedIndex]   = srcPtr[RedIndex];
                        dstPtr[GreenIndex] = srcPtr[GreenIndex];
                        dstPtr[BlueIndex]  = srcPtr[BlueIndex];
                    }
                    srcPtr += srcOffset;
                    dstPtr += dstOffset;
                }
            }
            else if ( ( dstPixelSize == 3 ) && ( srcPixelSize == 4 ) )
            {
                // draw Grayscale image on RGB
                for ( ty = 0; ty < srcSubH; ty++ )
                {
                    for ( tx = 0; tx < srcSubW; tx++, srcPtr += 4, dstPtr += 3 )
                    {
                        dstPtr[RedIndex]   = (uint8_t)( ( srcPtr[RedIndex]   * srcPtr[AlphaIndex] + dstPtr[RedIndex]   * ( 255 - srcPtr[AlphaIndex] ) ) / 255 );
                        dstPtr[GreenIndex] = (uint8_t)( ( srcPtr[GreenIndex] * srcPtr[AlphaIndex] + dstPtr[GreenIndex] * ( 255 - srcPtr[AlphaIndex] ) ) / 255 );
                        dstPtr[BlueIndex]  = (uint8_t)( ( srcPtr[BlueIndex]  * srcPtr[AlphaIndex] + dstPtr[BlueIndex]  * ( 255 - srcPtr[AlphaIndex] ) ) / 255 );
                    }
                    srcPtr += srcOffset;
                    dstPtr += dstOffset;
                }
            }
            else if ( ( dstPixelSize == 1 ) && ( srcPixelSize == 4 ) )
            {
                // draw Grayscale image on RGB
                for ( ty = 0; ty < srcSubH; ty++ )
                {
                    for ( tx = 0; tx < srcSubW; tx++, srcPtr += 4, dstPtr++ )
                    {
                        *dstPtr = (uint8_t)( ( RGB_TO_GRAY( srcPtr[RedIndex], srcPtr[GreenIndex], srcPtr[BlueIndex] ) * srcPtr[AlphaIndex] + *dstPtr * ( 255 - srcPtr[AlphaIndex] ) ) / 255 );
                    }
                    srcPtr += srcOffset;
                    dstPtr += dstOffset;
                }
            }
        }
    }

    return ret;
}
