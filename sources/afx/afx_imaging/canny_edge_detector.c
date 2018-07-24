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

#include <math.h>
#include "ximaging.h"

// forward declaration ----
static void SeparableConvolutionBlur( const ximage* src, ximage* dst, ximage* tempImage, const float* kernel, uint32_t kernelSize );
static void CalculateGradientsAndOrientations( const ximage* blurredImage, ximage* gradients, ximage* orientations, float* pMaxGradient );
static void SuppressNonMaximumsAndScale( const ximage* gradients, const ximage* orientations, ximage* dst, float maxGradient );
static void EdgeHysteresis( const ximage* tempEdgesImage, ximage* dst, uint16_t lowThreshold, uint16_t highThreshold );
// ------------------------


XErrorCode CannyEdgeDetector( const ximage* src, ximage* dst, ximage* tempBlurImage1, ximage* tempBlurImage2, ximage* tempEdgesImage,
                              ximage* gradients, ximage* orientations, const float* blurKernel, uint32_t kernelSize,
                              uint16_t lowThreshold, uint16_t highThreshold )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) || ( tempBlurImage1 == 0 ) || ( tempBlurImage2 == 0 ) || ( tempEdgesImage == 0 ) ||
         ( gradients == 0 ) || ( orientations == 0 ) || ( blurKernel == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( dst->width  != src->width  ) ||
              ( dst->height != src->height ) ||
              ( dst->format != src->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( tempBlurImage1->width  != src->width  ) ||
              ( tempBlurImage1->height != src->height ) ||
              ( tempBlurImage2->width  != src->width  ) ||
              ( tempBlurImage2->height != src->height ) ||
              ( tempEdgesImage->width  != src->width  ) ||
              ( tempEdgesImage->height != src->height ) ||
              ( gradients->width       != src->width  ) ||
              ( gradients->height      != src->height ) ||
              ( orientations->width    != src->width  ) ||
              ( orientations->height   != src->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( tempBlurImage1->format != XPixelFormatGrayscaleR4 ) ||
              ( tempBlurImage2->format != XPixelFormatGrayscaleR4 ) ||
              ( tempEdgesImage->format != XPixelFormatGrayscale8  ) ||
              ( gradients->format      != XPixelFormatGrayscaleR4 ) ||
              ( orientations->format   != XPixelFormatGrayscale8  ) )
    {
        ret = ErrorInvalidArgument;
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
        float maxGradient;

        // Step 1 - Blur image with the specified 1D gaussian kernel
        SeparableConvolutionBlur( src, tempBlurImage2, tempBlurImage1, blurKernel, kernelSize );

        // Step 2 - Calculate gradients and edge orientations
        CalculateGradientsAndOrientations( tempBlurImage2, gradients, orientations, &maxGradient );

        // Step 3 - Suppress non maximum gradients and scale those who survive
        SuppressNonMaximumsAndScale( gradients, orientations, tempEdgesImage, maxGradient );

        lowThreshold  = (uint16_t) ( lowThreshold  * 255 / maxGradient );
        highThreshold = (uint16_t) ( highThreshold * 255 / maxGradient );

        // Step 4 - Edge tracking by hysteresis
        EdgeHysteresis( tempEdgesImage, dst, lowThreshold, highThreshold );
    }

    return ret;
}

// Blur image using the specified 1D kernel and seperable convolution
void SeparableConvolutionBlur( const ximage* src, ximage* dst, ximage* tempImage,
                               const float* kernel, uint32_t kernelSize )
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

    uint8_t* tmpPtr = tempImage->data;
    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;

    uint32_t j;
    float    kernelSum = 0;

    // get sum of kernel elements
    for ( j = 0; j < kernelSize; j++ )
    {
        kernelSum += kernel[j];
    }
    if ( kernelSum == 0 )
    {
        kernelSum = 1;
    }

    #pragma omp parallel for schedule(static) shared( srcPtr, tmpPtr, width, height, widthMr, srcStride, tmpStride, radius, kernel, kernelSum )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride;
        float*   tmpRow = (float*) ( tmpPtr + y * tmpStride );
        float    sum, div;
        int      x, i;

        const float* kernelPtr;

        // process pixels on the left edge, where entire kernel can not be used
        for ( x = 0; x < radius; x++, srcRow++, tmpRow++ )
        {
            sum       = 0.0f;
            div       = 0.0f;
            kernelPtr = &( kernel[radius - x] );

            for ( i = 0 - x; i <= radius; i++, kernelPtr++ )
            {
                sum += *kernelPtr * srcRow[i];
                div += *kernelPtr;
            }

            if ( div == 0 ) div = 1;

            *tmpRow = sum / div;
        }

        // process all pixels, where entire kernel can be used
        for ( x = radius; x < widthMr; x++, srcRow++, tmpRow++ )
        {
            sum       = 0.0f;
            kernelPtr = kernel;

            for ( i = -radius; i <= radius; i++, kernelPtr++ )
            {
                sum += *kernelPtr * srcRow[i];
            }

            *tmpRow = sum / kernelSum;
        }

        // process pixels on the right edge, where entire kernel can not be used
        for ( x = widthMr; x < width; x++, srcRow++, tmpRow++ )
        {
            int stop  = width - x - 1;

            sum       = 0.0f;
            div       = 0.0f;
            kernelPtr = kernel;

            // process the part of kernel we can
            for ( i = -radius; i <= stop; i++, kernelPtr++ )
            {
                sum += *kernelPtr * srcRow[i];
                div += *kernelPtr;
            }

            if ( div == 0 ) div = 1;

            *tmpRow = sum / div;
        }
    }

    // process all pixels, where entire vertical kernel can be used
    #pragma omp parallel for schedule(static) shared( dstPtr, tmpPtr, width, height, heightMr, dstStride, tmpStride, radius, kernel, kernelSum )
    for ( y = radius; y < heightMr; y++ )
    {
        float  * dstRow  = (float*) ( dstPtr + y * dstStride );
        uint8_t* tmpBase = tmpPtr + ( y - radius ) * tmpStride;
        uint8_t* tmpRow;
        float    sum;
        int      x, i;

        const float* kernelPtr;

        for ( x = 0; x < width; x++, dstRow++ )
        {
            sum       = 0.0f;
            kernelPtr = kernel;
            tmpRow    = tmpBase;

            for ( i = -radius; i <= radius; i++, kernelPtr++, tmpRow += tmpStride )
            {
                sum += *kernelPtr * ( (float*) tmpRow )[x];
            }

            *dstRow = sum / kernelSum;
        }
    }

    // process pixels on the top and bottom edges
    for ( y = 0; y < radius; y++ )
    {
        float*   dstRow   = (float*) ( dstPtr + y * dstStride );
        float*   dstRow2  = (float*) ( dstPtr + ( height - radius + y ) * dstStride );
        uint8_t* tmpBase  = tmpPtr;
        uint8_t* tmpBase2 = tmpPtr + ( height - radius - radius + y ) * tmpStride;;

        uint8_t* tmpRow;
        float    sum, div;
        int      x, i;
        int      stop = radius - y - 1;

        const float* kernelPtr;

        for ( x = 0; x < width; x++, dstRow++, dstRow2++ )
        {
            // top
            sum       = 0.0f;
            div       = 0.0f;
            kernelPtr = &( kernel[radius - y] );
            tmpRow    = tmpBase;

            for ( i = 0 - y; i <= radius; i++, kernelPtr++, tmpRow += tmpStride )
            {
                sum += *kernelPtr * ( (float*) tmpRow )[x];
                div += *kernelPtr;
            }

            if ( div == 0 ) div = 1;

            *dstRow = sum / div;

            // bottom
            sum       = 0.0f;
            div       = 0.0f;
            kernelPtr = kernel;
            tmpRow    = tmpBase2;

            for ( i = -radius; i <= stop; i++, kernelPtr++, tmpRow += tmpStride )
            {
                sum += *kernelPtr * ( (float*) tmpRow )[x];
                div += *kernelPtr;
            }

            if ( div == 0 ) div = 1;

            *dstRow2 = sum / div;
        }
    }
}

// Calculate pixels' gradients and edge orientations
void CalculateGradientsAndOrientations( const ximage* blurredImage, ximage* gradients, ximage* orientations, float* pMaxGradient )
{
    int width              = blurredImage->width;
    int height             = blurredImage->height;
    int blurredStride      = blurredImage->stride;
    int gradientsStride    = gradients->stride;
    int orientationsStride = orientations->stride;
    int widthM1            = width - 1;
    int heightM1           = height - 1;
    int y;

    uint8_t* blurredPtr      = blurredImage->data;
    uint8_t* gradientsPtr    = gradients->data;
    uint8_t* orientationsPtr = orientations->data;

    float toAngle = 180.0f / (float) XPI;

    #pragma omp parallel for schedule(static) shared( blurredPtr, gradientsPtr, orientationsPtr, widthM1, heightM1, blurredStride, gradientsStride, orientationsStride, toAngle )
    for ( y = 1; y < heightM1; y++ )
    {
        float*   blurredRow      = ( (float*) ( blurredPtr + y * blurredStride ) ) + 1;
        float*   blurredRowA     = ( (float*) ( blurredPtr + ( y - 1 ) * blurredStride ) ) + 1;
        float*   blurredRowB     = ( (float*) ( blurredPtr + ( y + 1 ) * blurredStride ) ) + 1;
        float*   gradientsRow    = ( (float*) ( gradientsPtr + y * gradientsStride ) ) + 1;
        uint8_t* orientationsRow = orientationsPtr + y * orientationsStride + 1;

        int      x;
        float    gx, gy, orientation, div, maxGradient = 0.0f;

        for ( x = 1; x < widthM1; x++, blurredRow++, blurredRowA++, blurredRowB++, gradientsRow++, orientationsRow++ )
        {
            gx = blurredRowA[1] + blurredRowB[1] - blurredRowA[-1] - blurredRowB[-1] +
                 2 * ( blurredRow[1] - blurredRow[-1] );

            gy = blurredRowA[-1] + blurredRowA[1] - blurredRowB[-1] - blurredRowB[1] +
                 2 * ( *blurredRowA - *blurredRowB );

            *gradientsRow = sqrtf( gx * gx + gy * gy );
            if ( *gradientsRow > maxGradient )
            {
                maxGradient = *gradientsRow;
            }

            // find orientation of the edge
            if ( gx == 0 )
            {
                orientation = ( gy == 0 ) ? 0.0f : 90.0f;
            }
            else
            {
                div = gy / gx;

                if ( div < 0 )
                {
                    orientation = 180 - atanf( -div ) * toAngle;
                }
                else
                {
                    orientation = atanf( div ) * toAngle;
                }

                // get closest angle from 0, 45, 90, 135 set
                if      ( orientation <  22.5f ) { orientation =   0.0f; }
                else if ( orientation <  67.5f ) { orientation =  45.0f; }
                else if ( orientation < 112.5f ) { orientation =  90.0f; }
                else if ( orientation < 157.5f ) { orientation = 135.0f; }
                else                             { orientation =   0.0f; }
            }

            *orientationsRow = (uint8_t) orientation;
        }

        // save max gradient of the row in its last value
        *gradientsRow = maxGradient;
    }

    // zero edges of the gradients image and find global maximum
    {
        float* gradientsPtr1 = (float*) gradientsPtr;
        float* gradientsPtr2 = (float*) ( gradientsPtr + heightM1 * gradientsStride );
        float  maxGradient = 0, g;
        int    x;

        for ( x = 0; x < width; x++ )
        {
            *gradientsPtr1 = 0;
            *gradientsPtr2 = 0;

            gradientsPtr1++;
            gradientsPtr2++;
        }

        gradientsPtr = gradients->data + gradientsStride;

        for ( y = 1; y < heightM1; y++ )
        {
            // first check for new global maximun
            g = ( (float*) gradientsPtr )[widthM1];
            if ( g > maxGradient )
            {
                maxGradient = g;
            }

            ( (float*) gradientsPtr )[0]       = 0;
            ( (float*) gradientsPtr )[widthM1] = 0;

            gradientsPtr += gradientsStride;
        }

        *pMaxGradient = maxGradient;
    }
}

// Perform suppression of non maximum gradients and scale remaining in the [0, 255] range
void SuppressNonMaximumsAndScale( const ximage* gradients, const ximage* orientations, ximage* dst, float maxGradient )
{
    int width              = gradients->width;
    int height             = gradients->height;
    int widthM1            = width - 1;
    int heightM1           = height - 1;
    int gradientsStride    = gradients->stride;
    int orientationsStride = orientations->stride;
    int dstStride          = dst->stride;
    int y;

    uint8_t* gradientsPtr    = gradients->data;
    uint8_t* orientationsPtr = orientations->data;
    uint8_t* dstPtr          = dst->data;

    float scalingFactor = 255.0f / maxGradient;

    #pragma omp parallel for schedule(static) shared( gradientsPtr, orientationsPtr, dstPtr, widthM1, heightM1, gradientsStride, orientationsStride, dstStride, scalingFactor )
    for ( y = 1; y < heightM1; y++ )
    {
        float*   gradientsRow    = ( (float*) ( gradientsPtr + y * gradientsStride ) ) + 1;
        float*   gradientsRowA   = ( (float*) ( gradientsPtr + ( y - 1 ) * gradientsStride ) ) + 1;
        float*   gradientsRowB   = ( (float*) ( gradientsPtr + ( y + 1 ) * gradientsStride ) ) + 1;
        uint8_t* orientationsRow = orientationsPtr + y * orientationsStride + 1;
        uint8_t* dstRow          = dstPtr + y * dstStride + 1;
        int      x;
        float    left, right;

        for ( x = 1; x < widthM1; x++, gradientsRow++, gradientsRowA++, gradientsRowB++, orientationsRow++, dstRow++ )
        {
            // get two adjacent pixels
            switch ( *orientationsRow )
            {
            case 45:
                left  = gradientsRowB[-1];;
                right = gradientsRowA[1];
                break;
            case 90:
                left  = *gradientsRowB;
                right = *gradientsRowA;
                break;
            case 135:
                left  = gradientsRowB[1];
                right = gradientsRowA[-1];
                break;
            default: // 0
                left  = gradientsRow[-1];
                right = gradientsRow[1];
                break;
            }

            // compare current pixels value with adjacent pixels
            if ( ( *gradientsRow < left ) || ( *gradientsRow < right ) )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = (uint8_t) ( *gradientsRow * scalingFactor );
            }
        }
    }

    // zero edges of the result image
    {
        uint8_t* dstPtr1 = dstPtr;
        uint8_t* dstPtr2 = dstPtr + heightM1 * dstStride;
        int x;

        for ( x = 0; x < width; x++ )
        {
            *dstPtr1 = 0;
            *dstPtr2 = 0;

            dstPtr1++;
            dstPtr2++;
        }

        dstPtr1 = dstPtr + dstStride;
        dstPtr2 = dstPtr + dstStride + widthM1;

        for ( y = 1; y < heightM1; y++ )
        {
            *dstPtr1 = 0;
            *dstPtr2 = 0;

            dstPtr1 += dstStride;
            dstPtr2 += dstStride;
        }
    }
}

// Edge tracking by hysteresis
void EdgeHysteresis( const ximage* tempEdgesImage, ximage* dst, uint16_t lowThreshold, uint16_t highThreshold )
{
    int width        = dst->width;
    int height       = dst->height;
    int widthM1      = width - 1;
    int heightM1     = height - 1;
    int srcStride    = tempEdgesImage->stride;
    int dstStride    = dst->stride;
    int srcStrideM1  = srcStride - 1;
    int srcStrideP1  = srcStride + 1;
    int srcMStride   = -srcStride;
    int srcMStrideM1 = srcMStride - 1;
    int srcMStrideP1 = srcMStride + 1;
    int y;

    uint8_t* srcPtr = tempEdgesImage->data;
    uint8_t* dstPtr = dst->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, dstPtr, widthM1, heightM1, srcStride, dstStride, srcStrideM1, srcStrideP1, srcMStride, srcMStrideM1, srcMStrideP1, lowThreshold, highThreshold )
    for ( y = 1; y < heightM1; y++ )
    {
        uint8_t* srcRow = srcPtr + y * srcStride + 1;
        uint8_t* dstRow = dstPtr + y * dstStride + 1;
        int      x;

        for ( x = 1; x < widthM1; x++, srcRow++, dstRow++ )
        {
            if ( *srcRow < highThreshold )
            {
                if ( *srcRow < lowThreshold )
                {
                    *dstRow = 0;
                }
                else
                {
                    if ( ( srcRow[-1]           < highThreshold ) &&
                         ( srcRow[1]            < highThreshold ) &&
                         ( srcRow[srcStride]    < highThreshold ) &&
                         ( srcRow[srcStrideM1]  < highThreshold ) &&
                         ( srcRow[srcStrideP1]  < highThreshold ) &&
                         ( srcRow[srcMStride]   < highThreshold ) &&
                         ( srcRow[srcMStrideM1] < highThreshold ) &&
                         ( srcRow[srcMStrideP1] < highThreshold ) )
                    {
                        *dstRow = 0;
                    }
                    else
                    {
                        *dstRow = 255;
                    }
                }
            }
            else
            {
                *dstRow = 255;
            }
        }
    }

    // zero edges of the result image
    {
        uint8_t* dstPtr1 = dstPtr;
        uint8_t* dstPtr2 = dstPtr + heightM1 * dstStride;
        int x;

        for ( x = 0; x < width; x++ )
        {
            *dstPtr1 = 0;
            *dstPtr2 = 0;

            dstPtr1++;
            dstPtr2++;
        }

        dstPtr1 = dstPtr + dstStride;
        dstPtr2 = dstPtr + dstStride + widthM1;

        for ( y = 1; y < heightM1; y++ )
        {
            *dstPtr1 = 0;
            *dstPtr2 = 0;

            dstPtr1 += dstStride;
            dstPtr2 += dstStride;
        }
    }
}
