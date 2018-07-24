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
static void ThresholdImage8bpp( uint8_t* ptr, int width, int height, int stride, int threshold );
static void ThresholdImage16bpp( uint8_t* ptr, int width, int height, int stride, int threshold );
// ------------------------

// Applies threshold to the image's data
XErrorCode ThresholdImage( ximage* src, uint16_t threshold )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( src->format )
        {
        case XPixelFormatGrayscale8:
            ThresholdImage8bpp( src->data, src->width, src->height, src->stride, threshold );
            break;

        case XPixelFormatGrayscale16:
            ThresholdImage16bpp( src->data, src->width, src->height, src->stride, threshold );
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }
    }

    return ret;
}

// 8 bpp grayscale images case
static void ThresholdImage8bpp( uint8_t* ptr, int width, int height, int stride, int threshold )
{
    int y;

    if ( IsSSE2( ) == false )
    {
        #pragma omp parallel for schedule(static) shared( ptr, width, stride, threshold )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            int x;

            for ( x = 0; x < width; x++, row++ )
            {
                *row = (uint8_t) ( ( *row >= threshold ) ? Max8bppPixelValue : MinPixelValue );
            }
        }
    }
    else
    {
        int packs = width / 16;
        int rem   = width % 16;

        __m128i hiBitMask = _mm_set1_epi8( 0x80 );
        __m128i coef;

        if ( threshold <= 128 )
        {
            coef = _mm_set1_epi8( 128 - (uint8_t) threshold );

            #pragma omp parallel for schedule(static) shared( ptr, stride, coef, hiBitMask, packs, rem )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row = ptr + y * stride;
                int      x;

                __m128i values, result;

                for ( x = 0; x < packs; x++, row += 16 )
                {
                    values = _mm_loadu_si128( (__m128i*) row );

                    // add required coefficient - all values higher than threshold will get value 128 or higher (high bit set)
                    result = _mm_adds_epu8( values, coef );
                    // keep the high bit only
                    result = _mm_and_si128( result, hiBitMask );
                    // compare value with the mask - those which are equal get to 255, other to 0
                    result = _mm_cmpeq_epi8( result, hiBitMask );

                    _mm_storeu_si128( (__m128i*) row, result );
                }
                for ( x = 0; x < rem; x++, row++ )
                {
                    *row = (uint8_t) ( ( *row >= threshold ) ? 255 : 0 );
                }
            }
        }
        else
        {
            coef = _mm_set1_epi8( (uint8_t) threshold - 128 );

            #pragma omp parallel for schedule(static) shared( ptr, stride, coef, hiBitMask, packs, rem )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row = ptr + y * stride;
                int      x;

                __m128i values, result;

                for ( x = 0; x < packs; x++, row += 16 )
                {
                    values = _mm_loadu_si128( (__m128i*) row );

                    // subtract required coefiecient - all values higher than threshold still stay 128 or above
                    result = _mm_subs_epu8( values, coef );

                    result = _mm_and_si128( result, hiBitMask );
                    result = _mm_cmpeq_epi8( result, hiBitMask );

                    _mm_storeu_si128( (__m128i*) row, result );
                }

                for ( x = 0; x < rem; x++, row++ )
                {
                    *row = (uint8_t) ( ( *row >= threshold ) ? 255 : 0 );
                }
            }
        }
    }
}

// 16 bpp grayscale images case
static void ThresholdImage16bpp( uint8_t* ptr, int width, int height, int stride, int threshold )
{
    int y;

    #pragma omp parallel for schedule(static) shared( ptr, width, stride, threshold )
    for ( y = 0; y < height; y++ )
    {
        uint16_t* row = (uint16_t*) ( ptr + y * stride );
        int x;

        for ( x = 0; x < width; x++, row++ )
        {
            *row = (uint16_t) ( ( *row >= threshold ) ? Max16bppPixelValue : MinPixelValue );
        }
    }
}
