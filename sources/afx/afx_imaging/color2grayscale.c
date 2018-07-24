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
#include "xcpuid.h"

// SSE intrinsics
#ifdef _MSC_VER
    #include <intrin.h>
#elif __GNUC__
    #include <x86intrin.h>
#endif

// forward declaration ----
static void ColorToGrayscale24bpp( uint8_t* src, int srcStride, int srcPixelSize, uint8_t* dst, int dstStride, int width, int height );
// ------------------------

// Converts source color image into grayscale
XErrorCode ColorToGrayscale( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int width  = src->width;
        int height = src->height;

        if ( ( dst->width != width ) || ( dst->height != height ) )
        {
            ret = ErrorImageParametersMismatch;
        }
        else
        {
            if ( ( src->format == XPixelFormatRGB24 ) || ( src->format == XPixelFormatRGBA32 ) )
            {
                if ( dst->format == XPixelFormatGrayscale8 )
                {
                    ColorToGrayscale24bpp( src->data, src->stride, ( src->format == XPixelFormatRGB24 ) ? 3 : 4,
                                           dst->data, dst->stride, width, height );
                }
                else
                {
                    ret = ErrorImageParametersMismatch;
                }
            }
            else if ( ( src->format == XPixelFormatRGB48 ) || ( src->format == XPixelFormatRGBA64 ) )
            {
                if ( dst->format == XPixelFormatGrayscale16 )
                {
                    ret = ErrorNotImplemented;
                }
                else
                {
                    ret = ErrorImageParametersMismatch;
                }
            }
            else
            {
                ret = ErrorUnsupportedPixelFormat;
            }
        }
    }

    return ret;
}

// Convert 24/32 bpp color image to grayscale
void ColorToGrayscale24bpp( uint8_t* src, int srcStride, int srcPixelSize, uint8_t* dst, int dstStride, int width, int height )
{
    int y;

    if ( ( srcPixelSize == 4 ) || ( IsSSSE3( ) == false ) )
    {
        #pragma omp parallel for schedule(static) shared( src, dst, width, srcStride, dstStride, srcPixelSize )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* srcRow = src + y * srcStride;
            uint8_t* dstRow = dst + y * dstStride;
            int x;

            for ( x = 0; x < width; x++, srcRow += srcPixelSize, dstRow++ )
            {
                *dstRow = (uint8_t) RGB_TO_GRAY( srcRow[RedIndex], srcRow[GreenIndex], srcRow[BlueIndex] );
            }
        }
    }
    else
    {
        int packs = width / 16;
        int rem   = width % 16;

        // we process blocks of 16 pixels (48 bytes), and so we need indeces of RGB values from each 16 bytes block
        __m128i redIndeces0   = _mm_set_epi8( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12,  9,  6,  3,  0 );
        __m128i redIndeces1   = _mm_set_epi8( -1, -1, -1, -1, -1, 14, 11,  8,  5,  2, -1, -1, -1, -1, -1, -1 );
        __m128i redIndeces2   = _mm_set_epi8( 13, 10,  7,  4,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );

        __m128i greenIndeces0 = _mm_set_epi8( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10,  7,  4,  1 );
        __m128i greenIndeces1 = _mm_set_epi8( -1, -1, -1, -1, -1, 15, 12,  9,  6,  3,  0, -1, -1, -1, -1, -1 );
        __m128i greenIndeces2 = _mm_set_epi8( 14, 11,  8,  5,  2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );

        __m128i blueIndeces0  = _mm_set_epi8( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11,  8,  5,  2 );
        __m128i blueIndeces1  = _mm_set_epi8( -1, -1, -1, -1, -1, -1, 13, 10,  7,  4,  1, -1, -1, -1, -1, -1 );
        __m128i blueIndeces2  = _mm_set_epi8( 15, 12,  9,  6,  3,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );

        // BT709 Grayscale coefficients multiplied by 256
        __m128i redCoef       = _mm_set_epi8( 0, GRAY_COEF_RED8,   0, GRAY_COEF_RED8,   0, GRAY_COEF_RED8,   0, GRAY_COEF_RED8,
                                              0, GRAY_COEF_RED8,   0, GRAY_COEF_RED8,   0, GRAY_COEF_RED8,   0, GRAY_COEF_RED8 );
        __m128i greenCoef     = _mm_set_epi8( 0, GRAY_COEF_GREEN8, 0, GRAY_COEF_GREEN8, 0, GRAY_COEF_GREEN8, 0, GRAY_COEF_GREEN8,
                                              0, GRAY_COEF_GREEN8, 0, GRAY_COEF_GREEN8, 0, GRAY_COEF_GREEN8, 0, GRAY_COEF_GREEN8 );
        __m128i blueCoef      = _mm_set_epi8( 0, GRAY_COEF_BLUE8,  0, GRAY_COEF_BLUE8,  0, GRAY_COEF_BLUE8,  0, GRAY_COEF_BLUE8,
                                              0, GRAY_COEF_BLUE8,  0, GRAY_COEF_BLUE8,  0, GRAY_COEF_BLUE8,  0, GRAY_COEF_BLUE8 );

        __m128i zero          = _mm_setzero_si128( );

        #pragma omp parallel for schedule(static) shared( src, dst, srcStride, dstStride, packs, rem, redIndeces0, redIndeces1, redIndeces2, \
                greenIndeces0, greenIndeces1, greenIndeces2, blueIndeces0, blueIndeces1, blueIndeces2, redCoef, greenCoef, blueCoef, zero )
        for ( y = 0; y < height; y++ )
        {
            const uint8_t*  srcRow = src + y * srcStride;
            uint8_t*        dstRow = dst + y * dstStride;
            int             x;

            __m128i chunk0, chunk1, chunk2;

            __m128i red, green, blue;
            __m128i red1, red2, green1, green2, blue1, blue2;

            __m128i red1Mul, red2Mul, green1Mul, green2Mul, blue1Mul, blue2Mul;

            __m128i gray, gray1, gray2;

            for ( x = 0; x < packs; x++, srcRow += 48, dstRow += 16 )
            {
                // load 48 bytes of RGB data - 3 blocks of 16 bytes each
                chunk0 = _mm_loadu_si128( (__m128i*) srcRow );
                chunk1 = _mm_loadu_si128( (__m128i*) ( srcRow + 16 ) );
                chunk2 = _mm_loadu_si128( (__m128i*) ( srcRow + 32 ) );

                // reshuffle RGB components so we have 3 blocks of data each containing only required component
                red   = _mm_or_si128( _mm_or_si128( _mm_shuffle_epi8( chunk0, redIndeces0 ),
                                                    _mm_shuffle_epi8( chunk1, redIndeces1 ) ),
                                                    _mm_shuffle_epi8( chunk2, redIndeces2 ) );

                green = _mm_or_si128( _mm_or_si128( _mm_shuffle_epi8( chunk0, greenIndeces0 ),
                                                    _mm_shuffle_epi8( chunk1, greenIndeces1 ) ),
                                                    _mm_shuffle_epi8( chunk2, greenIndeces2 ) );

                blue  = _mm_or_si128( _mm_or_si128( _mm_shuffle_epi8( chunk0, blueIndeces0 ),
                                                    _mm_shuffle_epi8( chunk1, blueIndeces1 ) ),
                                                    _mm_shuffle_epi8( chunk2, blueIndeces2 ) );

                // extend each 8 bit value to 16 bit value
                red1   = _mm_unpacklo_epi8( red, zero );
                red2   = _mm_unpackhi_epi8( red, zero );

                green1 = _mm_unpacklo_epi8( green, zero );
                green2 = _mm_unpackhi_epi8( green, zero );

                blue1  = _mm_unpacklo_epi8( blue, zero );
                blue2  = _mm_unpackhi_epi8( blue, zero );

                // multiply 16 bit RGB components with appropriate coefficients
                red1Mul   = _mm_mullo_epi16( red1, redCoef );
                red2Mul   = _mm_mullo_epi16( red2, redCoef );

                green1Mul = _mm_mullo_epi16( green1, greenCoef );
                green2Mul = _mm_mullo_epi16( green2, greenCoef );

                blue1Mul  = _mm_mullo_epi16( blue1, blueCoef );
                blue2Mul  = _mm_mullo_epi16( blue2, blueCoef );

                // add up gray values
                gray1 = _mm_adds_epu16( red1Mul, _mm_adds_epu16( green1Mul, blue1Mul ) );
                gray2 = _mm_adds_epu16( red2Mul, _mm_adds_epu16( green2Mul, blue2Mul ) );

                // shift gray values 8 bits right (dividing by 256)
                gray1 = _mm_srli_epi16( gray1, 8 );
                gray2 = _mm_srli_epi16( gray2, 8 );

                // pack 16 bit gray values back to 8 bit
                gray = _mm_packus_epi16( gray1, gray2 );

                // store result in the grayscale image
                _mm_storeu_si128( (__m128i*) dstRow, gray );
            }

            for ( x = 0; x < rem; x++, srcRow += 3, dstRow++ )
            {
                *dstRow = (uint8_t) RGB_TO_GRAY8( srcRow[RedIndex], srcRow[GreenIndex], srcRow[BlueIndex] );
            }
        }
    }
}

// Changes color image to grayscale but keeps original pixel format (24/32 bpp RGB(A))
XErrorCode DesaturateColorImage( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatRGB24 ) && ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width  = src->width;
        int height = src->height;
        int stride = src->stride;
        int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
        int y;

        uint8_t* srcPtr = src->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, width, stride, pixelSize )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = srcPtr + y * stride;
            uint8_t  value;
            int x;

            for ( x = 0; x < width; x++ )
            {
                value = (uint8_t) RGB_TO_GRAY( row[RedIndex], row[GreenIndex], row[BlueIndex] );

                row[RedIndex]   = value;
                row[GreenIndex] = value;
                row[BlueIndex]  = value;

                row += pixelSize;
            }
        }
    }

    return ret;
}
