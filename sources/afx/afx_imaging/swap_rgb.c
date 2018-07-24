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
static void SwapRedBlue24pp( uint8_t* ptr, int width, int height, int stride, int pixelSize );
static void SwapRedBlue24ppCopy( uint8_t* src, uint8_t* dst, int width, int height, int srcStride, int dstStride );
static void SwapRedBlue32ppCopy( uint8_t* src, uint8_t* dst, int width, int height, int srcStride, int dstStride );
static void SwapRedBlue24to32ppCopy( uint8_t* src, uint8_t* dst, int width, int height, int srcStride, int dstStride );
// ------------------------

// Swap specified color channels in a RGB(A) image
XErrorCode SwapRGBChannels( ximage* src, uint32_t channel1, uint32_t channel2 )
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
    else if ( ( ( channel1 != RedIndex ) && ( channel1 != GreenIndex ) && ( channel1 != BlueIndex ) ) ||
              ( ( channel2 != RedIndex ) && ( channel2 != GreenIndex ) && ( channel2 != BlueIndex ) ) ||
              ( channel1 == channel2 )  )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        int width  = src->width;
        int height = src->height;
        int stride = src->stride;
        int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
        int y;

        uint8_t* ptr = src->data;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride, pixelSize )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            uint8_t  swap;
            int x;

            for ( x = 0; x < width; x++, row += pixelSize )
            {
                swap          = row[channel1];
                row[channel1] = row[channel2];
                row[channel2] = swap;
            }
        }
    }

    return ret;
}

// Swap Red and Blue components in RGB (RGBA) image
XErrorCode SwapRedBlue( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int width  = src->width;
        int height = src->height;
        int stride = src->stride;
        uint8_t* ptr    = src->data;

        if ( ( src->format == XPixelFormatRGB24 ) || ( src->format == XPixelFormatRGBA32 ) )
        {
            SwapRedBlue24pp( ptr, width, height, stride, ( src->format == XPixelFormatRGB24 ) ? 3 : 4 );
        }
        else if ( ( src->format == XPixelFormatRGB48 ) || ( src->format == XPixelFormatRGBA64 ) )
        {
            ret = ErrorNotImplemented;
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }
    }

    return ret;
}

// Swap Red and Blue components in RGB (RGBA) image - put result to destination image
XErrorCode SwapRedBlueCopy( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) ||
              (
              ( src->format != dst->format ) && ( ( src->format != XPixelFormatRGB24 ) || ( dst->format != XPixelFormatRGBA32 ) )
              ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int width       = src->width;
        int height      = src->height;
        int srcStride   = src->stride;
        int dstStride   = dst->stride;
        uint8_t* srcPtr = src->data;
        uint8_t* dstPtr = dst->data;

        switch ( src->format )
        {
            case XPixelFormatRGB24:
                if ( dst->format == XPixelFormatRGB24 )
                {
                    SwapRedBlue24ppCopy( srcPtr, dstPtr, width, height, srcStride, dstStride );
                }
                else
                {
                    SwapRedBlue24to32ppCopy( srcPtr, dstPtr, width, height, srcStride, dstStride );
                }
                break;
            case XPixelFormatRGBA32:
                SwapRedBlue32ppCopy( srcPtr, dstPtr, width, height, srcStride, dstStride );
                break;
            case XPixelFormatRGB48:
            case XPixelFormatRGBA64:
                ret = ErrorNotImplemented;
                break;
            default:
                ret = ErrorUnsupportedPixelFormat;
                break;
        }
    }

    return ret;
}

// 24/32 bpp color images case
static void SwapRedBlue24pp( uint8_t* ptr, int width, int height, int stride, int pixelSize )
{
    int y;

    if ( IsSSSE3( ) == false )
    {
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            uint8_t  swap;
            int x;

            for ( x = 0; x < width; x++, row += pixelSize )
            {
                swap           = row[RedIndex];
                row[RedIndex]  = row[BlueIndex];
                row[BlueIndex] = swap;
            }
        }
    }
    else
    {
        int packs = width / 16;
        int rem   = width % 16;

        // swap indeces for 3 chunks of 16 bytes each
        __m128i swapIndeces0 = _mm_set_epi8( -1, 12, 13, 14,  9, 10, 11,  6,  7,  8,  3,  4,  5,  0,  1,  2 );
        __m128i swapIndeces1 = _mm_set_epi8( 15, -1, 11, 12, 13,  8,  9, 10,  5,  6,  7,  2,  3,  4, -1,  0 );
        __m128i swapIndeces2 = _mm_set_epi8( 13, 14, 15, 10, 11, 12,  7,  8,  9,  4,  5,  6,  1,  2,  3, -1 );

        // remaining values for chunk 0 from chunk 1
        __m128i chunk0RemIndeces1 = _mm_set_epi8(  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
        // remaining values for chunk 2 from chunk 1
        __m128i chunk2RemIndeces1 = _mm_set_epi8( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14 );
        // remaining values for chunk 1 from chunks 0 and 2
        __m128i chunk1RemIndeces0 = _mm_set_epi8( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1 );
        __m128i chunk1RemIndeces2 = _mm_set_epi8( -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );

        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            uint8_t  swap;
            int      x;

            __m128i chunk0, chunk1, chunk2;

            for ( x = 0; x < packs; x++, row += 48 )
            {
                chunk0 = _mm_loadu_si128( (__m128i*)   row );
                chunk1 = _mm_loadu_si128( (__m128i*) ( row + 16 ) );
                chunk2 = _mm_loadu_si128( (__m128i*) ( row + 32 ) );

                _mm_storeu_si128( (__m128i*) row,
                    _mm_or_si128( _mm_shuffle_epi8( chunk0, swapIndeces0 ),
                                  _mm_shuffle_epi8( chunk1, chunk0RemIndeces1 ) ) );

                _mm_storeu_si128( (__m128i*) ( row + 16 ),
                    _mm_or_si128(
                    _mm_or_si128( _mm_shuffle_epi8( chunk1, swapIndeces1 ),
                                  _mm_shuffle_epi8( chunk0, chunk1RemIndeces0 ) ),
                                  _mm_shuffle_epi8( chunk2, chunk1RemIndeces2 ) ) );

                _mm_storeu_si128( (__m128i*) ( row + 32 ),
                    _mm_or_si128( _mm_shuffle_epi8( chunk2, swapIndeces2 ),
                                  _mm_shuffle_epi8( chunk1, chunk2RemIndeces1 ) ) );
            }

            for ( x = 0; x < rem; x++, row += 3 )
            {
                swap           = row[RedIndex];
                row[RedIndex]  = row[BlueIndex];
                row[BlueIndex] = swap;
            }
        }
    }
}

// 24 bpp color image case with copying
static void SwapRedBlue24ppCopy( uint8_t* src, uint8_t* dst, int width, int height, int srcStride, int dstStride )
{
    int y;

    if ( IsSSSE3( ) == false )
    {
        for ( y = 0; y < height; y++ )
        {
            uint8_t* srcRow = src + y * srcStride;
            uint8_t* dstRow = dst + y * dstStride;
            int x;

            for ( x = 0; x < width; x++, srcRow += 3, dstRow += 3 )
            {
                dstRow[RedIndex]   = srcRow[BlueIndex];
                dstRow[GreenIndex] = srcRow[GreenIndex];
                dstRow[BlueIndex]  = srcRow[RedIndex];
            }
        }
    }
    else
    {
        int packs = width / 16;
        int rem   = width % 16;

        // swap indeces for 3 chunks of 16 bytes each
        __m128i swapIndeces0 = _mm_set_epi8( -1, 12, 13, 14,  9, 10, 11,  6,  7,  8,  3,  4,  5,  0,  1,  2 );
        __m128i swapIndeces1 = _mm_set_epi8( 15, -1, 11, 12, 13,  8,  9, 10,  5,  6,  7,  2,  3,  4, -1,  0 );
        __m128i swapIndeces2 = _mm_set_epi8( 13, 14, 15, 10, 11, 12,  7,  8,  9,  4,  5,  6,  1,  2,  3, -1 );

        // remaining values for chunk 0 from chunk 1
        __m128i chunk0RemIndeces1 = _mm_set_epi8(  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
        // remaining values for chunk 2 from chunk 1
        __m128i chunk2RemIndeces1 = _mm_set_epi8( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14 );
        // remaining values for chunk 1 from chunks 0 and 2
        __m128i chunk1RemIndeces0 = _mm_set_epi8( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, -1 );
        __m128i chunk1RemIndeces2 = _mm_set_epi8( -1,  0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );

        for ( y = 0; y < height; y++ )
        {
            const uint8_t*  srcRow = src + y * srcStride;
            uint8_t*        dstRow = dst + y * dstStride;
            int             x;

            __m128i chunk0, chunk1, chunk2;

            for ( x = 0; x < packs; x++, srcRow += 48, dstRow += 48 )
            {
                chunk0 = _mm_loadu_si128( (__m128i*)   srcRow );
                chunk1 = _mm_loadu_si128( (__m128i*) ( srcRow + 16 ) );
                chunk2 = _mm_loadu_si128( (__m128i*) ( srcRow + 32 ) );

                _mm_storeu_si128( (__m128i*) dstRow,
                    _mm_or_si128( _mm_shuffle_epi8( chunk0, swapIndeces0 ),
                                  _mm_shuffle_epi8( chunk1, chunk0RemIndeces1 ) ) );

                _mm_storeu_si128( (__m128i*) ( dstRow + 16 ),
                    _mm_or_si128(
                    _mm_or_si128( _mm_shuffle_epi8( chunk1, swapIndeces1 ),
                                  _mm_shuffle_epi8( chunk0, chunk1RemIndeces0 ) ),
                                  _mm_shuffle_epi8( chunk2, chunk1RemIndeces2 ) ) );

                _mm_storeu_si128( (__m128i*) ( dstRow + 32 ),
                    _mm_or_si128( _mm_shuffle_epi8( chunk2, swapIndeces2 ),
                                  _mm_shuffle_epi8( chunk1, chunk2RemIndeces1 ) ) );
            }

            for ( x = 0; x < rem; x++, srcRow += 3, dstRow += 3 )
            {
                dstRow[RedIndex]   = srcRow[BlueIndex];
                dstRow[GreenIndex] = srcRow[GreenIndex];
                dstRow[BlueIndex]  = srcRow[RedIndex];
            }
        }
    }
}

// 32 bpp color image case with copying
static void SwapRedBlue32ppCopy( uint8_t* src, uint8_t* dst, int width, int height, int srcStride, int dstStride )
{
    int y;

    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = src + y * srcStride;
        uint8_t* dstRow = dst + y * dstStride;
        int x;

        for ( x = 0; x < width; x++, srcRow += 4, dstRow += 4 )
        {
            dstRow[RedIndex]   = srcRow[BlueIndex];
            dstRow[GreenIndex] = srcRow[GreenIndex];
            dstRow[BlueIndex]  = srcRow[RedIndex];
            dstRow[AlphaIndex] = srcRow[AlphaIndex];
        }
    }
}

// Copy RGB data from 24 to 32 bpp image while swapping Red and Blue channels (alpha is kept as is; supposed to be initlized)
static void SwapRedBlue24to32ppCopy( uint8_t* src, uint8_t* dst, int width, int height, int srcStride, int dstStride )
{
    int y;

    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = src + y * srcStride;
        uint8_t* dstRow = dst + y * dstStride;
        int x;

        for ( x = 0; x < width; x++, srcRow += 3, dstRow += 4 )
        {
            dstRow[RedIndex]   = srcRow[BlueIndex];
            dstRow[GreenIndex] = srcRow[GreenIndex];
            dstRow[BlueIndex]  = srcRow[RedIndex];
        }
    }
}
