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
static void Convolution8bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize );
static void Convolution24bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize );
static void Convolution32bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize );

static void SeparableConvolutionImpl( const ximage* src, ximage* dst, ximage* tempImage,
                                      const float* hKernel, const float* vKernel, float hKernelSum, float vKernelSum, uint32_t kernelSize );

static void ConvolutionEx8bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, float divisor, float offset, XConvolutionBorderHandlingMode bhMode );
static void ConvolutionEx24bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, float divisor, float offset, XConvolutionBorderHandlingMode bhMode );
// ------------------------

// Perform convolution on the specified image.
//
// kernel       is an array of kernelSize*kernelSize size (pixel weights)
// kernelSize   is in [3, 51] range, must be odd value
// processAlpha specifies if alpha channel should be processed for 32 bpp images or not
//
XErrorCode Convolution( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, bool processAlpha )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( kernel == 0 ) )
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
    else if ( ( kernelSize < 3 ) || ( kernelSize > 51 ) || ( ( kernelSize & 1 ) == 0 ) )
    {
        // don't allow even kernels or too small/big kernels
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        if ( src->format == XPixelFormatGrayscale8 )
        {
            Convolution8bpp( src, dst, kernel, kernelSize );
        }
        else if ( ( src->format == XPixelFormatRGB24 ) ||
                ( ( src->format == XPixelFormatRGBA32 ) && ( processAlpha == false ) ) )
        {
            Convolution24bpp( src, dst, kernel, kernelSize );
        }
        else
        {
            Convolution32bpp( src, dst, kernel, kernelSize );
        }
    }

    return ret;
}

// Perform separable convolution on the specified image (first it does processing with horizontal kernel keeping result
// in temporary image and then it does processing with vertical kernel)
//
// hKernel      horizontal kernel is an array of kernelSize elements
// hKernel      vertical kernel -/-
// kernelSize   is in [3, 51] range, must be odd value
//
XErrorCode SeparableConvolution( const ximage* src, ximage* dst, ximage* tempImage,
                                 const float* hKernel, const float* vKernel, uint32_t kernelSize )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( tempImage == 0 ) || ( hKernel == 0 ) || ( vKernel == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( tempImage->format != XPixelFormatGrayscaleR4 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( dst->width  != src->width  ) ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) ||
              ( tempImage->width  != src->width ) ||
              ( tempImage->height != src->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( kernelSize < 3 ) || ( kernelSize > 51 ) || ( ( kernelSize & 1 ) == 0 ) )
    {
        // don't allow even kernels or too small/big kernels
        ret = ErrorArgumentOutOfRange;
    }
    else if ( ( src->width  < (int32_t) kernelSize ) ||
              ( src->height < (int32_t) kernelSize ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else
    {
        float    hSum = 0, vSum = 0;
        uint32_t i;

        for ( i = 0; i < kernelSize; i++ )
        {
            hSum += hKernel[i];
            vSum += vKernel[i];
        }

        if ( hSum == 0 ) hSum = 1;
        if ( vSum == 0 ) vSum = 1;

        SeparableConvolutionImpl( src, dst, tempImage, hKernel, vKernel, hSum, vSum, kernelSize );
    }

    return ret;
}

// Perform convolution on the specified image.
//
// kernel     is an array of kernelSize*kernelSize size (pixel weights)
// kernelSize is in [3, 51] range, must be odd value
// divisor    is the value used to divide weighted sum of pixels
// offset     is the value to add after dividing
// bhMode     is the mode of handling image borders
//
XErrorCode ConvolutionEx( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, float divisor, float offset, XConvolutionBorderHandlingMode bhMode )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( kernel == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( divisor == 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( kernelSize < 3 ) || ( kernelSize > 51 ) || ( ( kernelSize & 1 ) == 0 ) )
    {
        // don't allow even kernels or too small/big kernels
        ret = ErrorArgumentOutOfRange;
    }
    else if ( bhMode >= BHMode_Invalid )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( dst->format != src->format )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( bhMode == BHMode_Wrap ) &&
              ( ( src->width < ( (int32_t) kernelSize >> 1 ) ) || ( src->height < ( (int32_t) kernelSize >> 1 ) ) ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( ( bhMode == BHMode_Crop ) &&
              ( ( src->width < (int32_t) kernelSize ) || ( src->height < (int32_t) kernelSize ) ) )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( ( bhMode != BHMode_Crop ) &&
              ( ( dst->width != src->width  ) || ( dst->height != src->height ) ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( bhMode == BHMode_Crop ) &&
              ( ( dst->width != src->width - ( (int32_t) kernelSize - 1 ) ) || ( dst->height != src->height - ( (int32_t) kernelSize - 1 ) ) ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        if ( src->format == XPixelFormatGrayscale8 )
        {
            ConvolutionEx8bpp( src, dst, kernel, kernelSize, divisor, offset, bhMode );
        }
        else
        {
            ConvolutionEx24bpp( src, dst, kernel, kernelSize, divisor, offset, bhMode );
        }
    }

    return ret;
}

// Perform convolution on a 8 bpp grayscale image
void Convolution8bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = kernelSize >> 1;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, kernel )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        float    kernelValue;
        double   sum, div;
        int      x, i, j, t;

        const float* kernelPtr;

        for ( x = 0; x < width; x++ )
        {
            sum = div = 0.0f;
            kernelPtr = kernel;

            // for each kernel row
            for ( i = -radius; i <= radius; i++ )
            {
                t  = y + i;

                // skip row
                if ( t < 0 )
                {
                    kernelPtr += kernelSize;
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
                for ( j = -radius; j <= radius; j++, kernelPtr++, srcRow2++ )
                {
                    t = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( t < width )
                    {
                        kernelValue = *kernelPtr;

                        div += kernelValue;
                        sum += kernelValue * *srcRow2;
                    }
                }
            }

            // check divider
            if ( div != 0 )
            {
                sum /= div;
            }

            *dstRow = (uint8_t) ( ( sum > 255 ) ? 255 : ( ( sum < 0 ) ? 0 : sum ) );

            srcRow++;
            dstRow++;
        }
    }
}

// Perform convolution on a 24/32 bpp color image (alpha channel is ignored in the case of 32 bpp image)
void Convolution24bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = kernelSize >> 1;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, kernel, pixelSize )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        float    kernelValue;
        double   sumR, sumG, sumB, div;
        int      x, i, j, t;

        const float* kernelPtr;

        for ( x = 0; x < width; x++ )
        {
            sumR = sumG = sumB = div = 0.0f;
            kernelPtr = kernel;

            // for each kernel row
            for ( i = -radius; i <= radius; i++ )
            {
                t  = y + i;

                // skip row
                if ( t < 0 )
                {
                    kernelPtr += kernelSize;
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
                for ( j = -radius; j <= radius; j++, kernelPtr++, srcRow2 += pixelSize )
                {
                    t  = x + j;

                    // skip column
                    if ( t < 0 )
                    {
                        continue;
                    }

                    if ( t < width )
                    {
                        kernelValue = *kernelPtr;

                        div  += kernelValue;
                        sumR += kernelValue * srcRow2[RedIndex];
                        sumG += kernelValue * srcRow2[GreenIndex];
                        sumB += kernelValue * srcRow2[BlueIndex];
                    }
                }
            }

            // check divider
            if ( div != 0 )
            {
                sumR /= div;
                sumG /= div;
                sumB /= div;
            }

            dstRow[RedIndex  ] = (uint8_t) ( ( sumR > 255 ) ? 255 : ( ( sumR < 0 ) ? 0 : sumR ) );
            dstRow[GreenIndex] = (uint8_t) ( ( sumG > 255 ) ? 255 : ( ( sumG < 0 ) ? 0 : sumG ) );
            dstRow[BlueIndex ] = (uint8_t) ( ( sumB > 255 ) ? 255 : ( ( sumB < 0 ) ? 0 : sumB ) );

            // take care of alpha channel
            if ( pixelSize == 4 )
            {
                dstRow[AlphaIndex] = srcRow[AlphaIndex];
            }

            srcRow += pixelSize;
            dstRow += pixelSize;
        }
    }
}

// Perform convolution on a 32 bpp color image (alpha channel is also processed)
void Convolution32bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = kernelSize >> 1;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, kernel )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* dstRow = dstPtr + y * dstStride;
        uint8_t* srcRow = srcPtr + y * srcStride;
        uint8_t* srcRow2;
        float    kernelValue;
        double   sumR, sumG, sumB, sumA, div;
        int      x, i, j, t;

        const float* kernelPtr;

        for ( x = 0; x < width; x++ )
        {
            sumR = sumG = sumB = sumA = div = 0.0f;
            kernelPtr = kernel;

            // for each kernel row
            for ( i = -radius; i <= radius; i++ )
            {
                t = y + i;

                // skip row
                if ( t < 0 )
                {
                    kernelPtr += kernelSize;
                    continue;
                }
                // break
                if ( t >= height )
                    break;

                // source image row aligned to current kernel row
                srcRow2 = srcRow + i * srcStride - radius * 4;

                // for each kernel column
                for ( j = -radius; j <= radius; j++, kernelPtr++, srcRow2 += 4 )
                {
                    t  = x + j;

                    // skip column
                    if ( t < 0 )
                        continue;

                    if ( t < width )
                    {
                        kernelValue = *kernelPtr;

                        div  += kernelValue;
                        sumR += kernelValue * srcRow2[RedIndex];
                        sumG += kernelValue * srcRow2[GreenIndex];
                        sumB += kernelValue * srcRow2[BlueIndex];
                        sumA += kernelValue * srcRow2[AlphaIndex];
                    }
                }
            }

            // check divider
            if ( div != 0 )
            {
                sumR /= div;
                sumG /= div;
                sumB /= div;
                sumA /= div;
            }

            dstRow[RedIndex  ] = (uint8_t) ( ( sumR > 255 ) ? 255 : ( ( sumR < 0 ) ? 0 : sumR ) );
            dstRow[GreenIndex] = (uint8_t) ( ( sumG > 255 ) ? 255 : ( ( sumG < 0 ) ? 0 : sumG ) );
            dstRow[BlueIndex ] = (uint8_t) ( ( sumB > 255 ) ? 255 : ( ( sumB < 0 ) ? 0 : sumB ) );
            dstRow[AlphaIndex] = (uint8_t) ( ( sumA > 255 ) ? 255 : ( ( sumA < 0 ) ? 0 : sumA ) );

            srcRow += 4;
            dstRow += 4;
        }
    }
}

// Perform separable convolution on a 8 bpp grayscale or 24 bpp RGB image
static void SeparableConvolutionImpl( const ximage* src, ximage* dst, ximage* tempImage,
                                      const float* hKernel, const float* vKernel, float hKernelSum, float vKernelSum, uint32_t kernelSize )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int tmpStride = tempImage->stride;
    int radius    = kernelSize >> 1;
    int widthMr   = width - radius;
    int heightMr  = height - radius;
    int y;
    int p, planes = ( src->format == XPixelFormatGrayscale8 ) ? 1 : 3;

    uint8_t* tmpPtr = tempImage->data;

    for ( p = 0; p < planes; p++ )
    {
        uint8_t* srcPtr = src->data + p;
        uint8_t* dstPtr = dst->data + p;

        #pragma omp parallel for schedule(static) shared( srcPtr, tmpPtr, width, height, widthMr, srcStride, tmpStride, radius, hKernel, hKernelSum )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* srcRow = srcPtr + y * srcStride;
            float*   tmpRow = (float*) ( tmpPtr + y * tmpStride );
            float    sum, div;
            int      x, i;

            const float* kernelPtr;

            // process pixels on the left edge, where entire kernel can not be used
            for ( x = 0; x < radius; x++, srcRow += planes, tmpRow++ )
            {
                sum       = 0.0f;
                div       = 0.0f;
                kernelPtr = &( hKernel[radius - x] );

                for ( i = 0 - x; i <= radius; i++, kernelPtr++ )
                {
                    sum += *kernelPtr * srcRow[i * planes];
                    div += *kernelPtr;
                }

                if ( div == 0 ) div = 1;

                *tmpRow = sum / div;
            }

            // process all pixels, where entire kernel can be used
            for ( x = radius; x < widthMr; x++, srcRow += planes, tmpRow++ )
            {
                sum       = 0.0f;
                kernelPtr = hKernel;

                for ( i = -radius; i <= radius; i++, kernelPtr++ )
                {
                    sum += *kernelPtr * srcRow[i * planes];
                }

                *tmpRow = sum / hKernelSum;
            }

            // process pixels on the right edge, where entire kernel can not be used
            for ( x = widthMr; x < width; x++, srcRow += planes, tmpRow++ )
            {
                int stop  = width - x - 1;

                sum       = 0.0f;
                div       = 0.0f;
                kernelPtr = hKernel;

                // process the part of kernel we can
                for ( i = -radius; i <= stop; i++, kernelPtr++ )
                {
                    sum += *kernelPtr * srcRow[i * planes];
                    div += *kernelPtr;
                }

                if ( div == 0 ) div = 1;

                *tmpRow = sum / div;
            }
        }

        // process all pixels, where entire vertical kernel can be used
        #pragma omp parallel for schedule(static) shared( dstPtr, tmpPtr, width, height, heightMr, dstStride, tmpStride, radius, vKernel, vKernelSum )
        for ( y = radius; y < heightMr; y++ )
        {
            uint8_t* dstRow  = dstPtr + y * dstStride;
            uint8_t* tmpBase = tmpPtr + ( y - radius ) * tmpStride;
            uint8_t* tmpRow;
            float    sum;
            int      x, i;

            const float* kernelPtr;

            for ( x = 0; x < width; x++, dstRow += planes )
            {
                sum       = 0.0f;
                kernelPtr = vKernel;
                tmpRow    = tmpBase;

                for ( i = -radius; i <= radius; i++, kernelPtr++, tmpRow += tmpStride )
                {
                    sum += *kernelPtr * ( (float*) tmpRow )[x];
                }

                *dstRow = (uint8_t) ( sum / vKernelSum );
            }
        }

        // process pixels on the top and bottom edges
        for ( y = 0; y < radius; y++ )
        {
            uint8_t* dstRow   = dstPtr + y * dstStride;
            uint8_t* dstRow2  = dstPtr + ( height - radius + y ) * dstStride;
            uint8_t* tmpBase  = tmpPtr;
            uint8_t* tmpBase2 = tmpPtr + ( height - radius - radius + y ) * tmpStride;;

            uint8_t* tmpRow;
            float    sum, div;
            int      x, i;
            int      stop = radius - y - 1;

            const float* kernelPtr;

            for ( x = 0; x < width; x++, dstRow += planes, dstRow2 += planes )
            {
                // top
                sum       = 0.0f;
                div       = 0.0f;
                kernelPtr = &( vKernel[radius - y] );
                tmpRow    = tmpBase;

                for ( i = 0 - y; i <= radius; i++, kernelPtr++, tmpRow += tmpStride )
                {
                    sum += *kernelPtr * ( (float*) tmpRow )[x];
                    div += *kernelPtr;
                }

                if ( div == 0 ) div = 1;

                *dstRow = (uint8_t) ( sum / div );

                // bottom
                sum       = 0.0f;
                div       = 0.0f;
                kernelPtr = vKernel;
                tmpRow    = tmpBase2;

                for ( i = -radius; i <= stop; i++, kernelPtr++, tmpRow += tmpStride )
                {
                    sum += *kernelPtr * ( (float*) tmpRow )[x];
                    div += *kernelPtr;
                }

                if ( div == 0 ) div = 1;

                *dstRow2 = (uint8_t) ( sum / div );
            }
        }
    }
}

// Perform extended version of convolution on a 8 bpp grayscale image
void ConvolutionEx8bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, float divisor, float offset, XConvolutionBorderHandlingMode bhMode )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = kernelSize >> 1;
    int yStart    = 0;
    int yEnd      = height;
    int xStart    = 0;
    int xEnd      = width;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    // decrease the area to process for Crop mode
    if ( bhMode == BHMode_Crop )
    {
        yStart += radius;
        xStart += radius;
        yEnd   -= radius;
        xEnd   -= radius;
    }

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, kernel, divisor, offset, bhMode, xStart, xEnd )
    for ( y = yStart; y < yEnd; y++ )
    {
        // in the case if Crop mode is used the source pointer must be shifted to the first pixel of a row,
        // while destination pointer must pointer must stay few row behind
        uint8_t* dstRow = dstPtr + ( y - yStart ) * dstStride;
        uint8_t* srcRow;
        double   sum;
        int      x, i, j, t;

        const float* kernelPtr;

        for ( x = xStart; x < xEnd; x++ )
        {
            sum = 0.0;
            kernelPtr = kernel;

            // for each kernel row
            for ( i = -radius; i <= radius; i++ )
            {
                t  = y + i;

                // check if we crossed the top row
                if ( t < 0 )
                {
                    if ( bhMode == BHMode_Extend )
                    {
                        // use the top row for anything above it
                        srcRow = srcPtr;
                    }
                    else if ( bhMode == BHMode_Wrap )
                    {
                        // use one of the bottom rows
                        srcRow = srcPtr + ( t + height ) * srcStride;
                    }
                    else
                    {
                        // ignore the rows outside of the image
                        kernelPtr += kernelSize;
                        continue;
                    }
                }
                // check if we crossed the bottom row
                else if ( t >= height )
                {
                    if ( bhMode == BHMode_Extend )
                    {
                        // use the bottom row for anything below it
                        srcRow = srcPtr + ( height - 1 ) * srcStride;
                    }
                    else if ( bhMode == BHMode_Wrap )
                    {
                        // use one of the top rows
                        srcRow = srcPtr + ( t - height ) * srcStride;
                    }
                    else
                    {
                        // ignore the rows outside of the image
                        break;
                    }
                }
                else
                {
                    // source image row aligned to current kernel row
                    srcRow = srcPtr + t * srcStride;
                }

                // for each kernel column
                for ( j = -radius; j <= radius; j++, kernelPtr++ )
                {
                    t = x + j;

                    // check if we crossed the left edge
                    if ( t < 0 )
                    {
                        if ( bhMode == BHMode_Extend )
                        {
                            // use the first pixel
                            t = 0;
                        }
                        else if ( bhMode == BHMode_Wrap )
                        {
                            // use a pixel from the right side
                            t += width;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    // check if we crossed the right edge
                    else if ( t >= width )
                    {
                        if ( bhMode == BHMode_Extend )
                        {
                            // use the last pixel
                            t = width - 1;
                        }
                        else if ( bhMode == BHMode_Wrap )
                        {
                            // use a pixel from the left side
                            t -= width;
                        }
                        else
                        {
                            continue;
                        }

                    }

                    sum += *kernelPtr * srcRow[t];
                }
            }

            sum /= divisor;
            sum += offset;

            *dstRow = (uint8_t) ( ( sum > 255 ) ? 255 : ( ( sum < 0 ) ? 0 : sum ) );

            dstRow++;
        }
    }
}

// Perform extended version of convolution on a 24/32 bpp color image (alpha channel is ignored in the case of 32 bpp image)
void ConvolutionEx24bpp( const ximage* src, ximage* dst, const float* kernel, uint32_t kernelSize, float divisor, float offset, XConvolutionBorderHandlingMode bhMode )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int dstStride = dst->stride;
    int radius    = kernelSize >> 1;
    int pixelSize = ( src->format == XPixelFormatRGB24 ) ? 3 : 4;
    int yStart    = 0;
    int yEnd      = height;
    int xStart    = 0;
    int xEnd      = width;
    int y;

    uint8_t* srcPtr  = src->data;
    uint8_t* dstPtr  = dst->data;

    // decrease the area to process for Crop mode
    if ( bhMode == BHMode_Crop )
    {
        yStart += radius;
        xStart += radius;
        yEnd   -= radius;
        xEnd   -= radius;
    }

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, width, height, srcStride, dstStride, radius, kernel, divisor, offset, bhMode, pixelSize, xStart, xEnd )
    for ( y = yStart; y < yEnd; y++ )
    {
        // in the case if Crop mode is used the source pointer must be shifted to the first pixel of a row,
        // while destination pointer must pointer must stay few row behind
        uint8_t* dstRow = dstPtr + ( y - yStart ) * dstStride;
        uint8_t* srcRow;
        uint8_t* ptr;
        uint8_t* srcRowAlpha = srcPtr + y * srcStride + xStart * pixelSize + AlphaIndex;
        float    kernelValue;
        double   sumR, sumG, sumB;
        int      x, i, j, t;

        const float* kernelPtr;

        for ( x = xStart; x < xEnd; x++ )
        {
            sumR = sumG = sumB = 0.0;
            kernelPtr = kernel;

            // for each kernel row
            for ( i = -radius; i <= radius; i++ )
            {
                t  = y + i;

                // check if we crossed the top row
                if ( t < 0 )
                {
                    if ( bhMode == BHMode_Extend )
                    {
                        // use the top row for anything above it
                        srcRow = srcPtr;
                    }
                    else if ( bhMode == BHMode_Wrap )
                    {
                        // use one of the bottom rows
                        srcRow = srcPtr + ( t + height ) * srcStride;
                    }
                    else
                    {
                        // ignore the rows outside of the image
                        kernelPtr += kernelSize;
                        continue;
                    }
                }
                // check if we crossed the bottom row
                else if ( t >= height )
                {
                    if ( bhMode == BHMode_Extend )
                    {
                        // use the bottom row for anything below it
                        srcRow = srcPtr + ( height - 1 ) * srcStride;
                    }
                    else if ( bhMode == BHMode_Wrap )
                    {
                        // use one of the top rows
                        srcRow = srcPtr + ( t - height ) * srcStride;
                    }
                    else
                    {
                        // ignore the rows outside of the image
                        break;
                    }
                }
                else
                {
                    // source image row aligned to current kernel row
                    srcRow = srcPtr + t * srcStride;
                }

                // for each kernel column
                for ( j = -radius; j <= radius; j++, kernelPtr++ )
                {
                    t = x + j;

                    // check if we crossed the left edge
                    if ( t < 0 )
                    {
                        if ( bhMode == BHMode_Extend )
                        {
                            // use the first pixel
                            t = 0;
                        }
                        else if ( bhMode == BHMode_Wrap )
                        {
                            // use a pixel from the right side
                            t += width;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    // check if we crossed the right edge
                    else if ( t >= width )
                    {
                        if ( bhMode == BHMode_Extend )
                        {
                            // use the last pixel
                            t = width - 1;
                        }
                        else if ( bhMode == BHMode_Wrap )
                        {
                            // use a pixel from the left side
                            t -= width;
                        }
                        else
                        {
                            continue;
                        }

                    }

                    ptr         = srcRow + t * pixelSize;
                    kernelValue = *kernelPtr;


                    sumR += kernelValue * ptr[RedIndex];
                    sumG += kernelValue * ptr[GreenIndex];
                    sumB += kernelValue * ptr[BlueIndex];
                }
            }

            sumR /= divisor;
            sumG /= divisor;
            sumB /= divisor;
            sumR += offset;
            sumG += offset;
            sumB += offset;

            dstRow[RedIndex]   = (uint8_t) ( ( sumR > 255 ) ? 255 : ( ( sumR < 0 ) ? 0 : sumR ) );
            dstRow[GreenIndex] = (uint8_t) ( ( sumG > 255 ) ? 255 : ( ( sumG < 0 ) ? 0 : sumG ) );
            dstRow[BlueIndex]  = (uint8_t) ( ( sumB > 255 ) ? 255 : ( ( sumB < 0 ) ? 0 : sumB ) );

            // take care of alpha channel
            if ( pixelSize == 4 )
            {
                dstRow[AlphaIndex] = *srcRowAlpha;
            }

            dstRow += pixelSize;
            srcRowAlpha += pixelSize;
        }
    }
}
