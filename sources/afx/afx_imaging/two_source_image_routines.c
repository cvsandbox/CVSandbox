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

// Make sure provided images are valid
static XErrorCode CheckImages( ximage* image1, const ximage* image2 )
{
    XErrorCode ret = SuccessCode;

    if ( ( image1 == 0 ) || ( image2 == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image1->format != XPixelFormatGrayscale8 ) &&
              ( image1->format != XPixelFormatRGB24 ) &&
              ( image1->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image1->format != image2->format ) ||
              ( image1->width  != image2->width  ) ||
              ( image1->height != image2->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }

    return ret;
}

// Apply mask to an image by setting its pixels to fill color if corresponding pixels of the mask have 0 value
// and fillOnZero is set to true. If fillOnZero is set to false, then filling happens if mask has non zero value.
XErrorCode MaskImage( ximage* image, const ximage* mask, xargb fillColor, bool fillOnZero )
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
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->width  != mask->width ) ||
              ( image->height != mask->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int width       = image->width;
        int height      = image->height;
        int imageStride = image->stride;
        int maskStride  = mask->stride;
        int y;

        uint8_t* imagePtr = image->data;
        uint8_t* maskPtr  = mask->data;

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) ( RGB_TO_GRAY( fillColor.components.r, fillColor.components.g, fillColor.components.b ) * fillColor.components.a / 255 );

            if ( fillOnZero == true )
            {
                #pragma omp parallel for schedule(static) shared( imagePtr, maskPtr, width, imageStride, maskStride, fillValue )
                for ( y = 0; y < height; y++ )
                {
                    uint8_t* imageRow = imagePtr + y * imageStride;
                    uint8_t* maskRow  = maskPtr  + y * maskStride;
                    int x;

                    for ( x = 0; x < width; ++x, ++maskRow, ++imageRow )
                    {
                        if ( *maskRow == 0 )
                        {
                            *imageRow = fillValue;
                        }
                    }
                }
            }
            else
            {
                #pragma omp parallel for schedule(static) shared( imagePtr, maskPtr, width, imageStride, maskStride, fillValue )
                for ( y = 0; y < height; y++ )
                {
                    uint8_t* imageRow = imagePtr + y * imageStride;
                    uint8_t* maskRow  = maskPtr  + y * maskStride;
                    int x;

                    for ( x = 0; x < width; ++x, ++maskRow, ++imageRow )
                    {
                        if ( *maskRow != 0 )
                        {
                            *imageRow = fillValue;
                        }
                    }
                }
            }
        }
        else if ( image->format == XPixelFormatRGB24 )
        {
            uint8_t fillR = (uint8_t) ( fillColor.components.r * fillColor.components.a / 255 );
            uint8_t fillG = (uint8_t) ( fillColor.components.g * fillColor.components.a / 255 );
            uint8_t fillB = (uint8_t) ( fillColor.components.b * fillColor.components.a / 255 );

            if ( fillOnZero == true )
            {
                #pragma omp parallel for schedule(static) shared( imagePtr, maskPtr, width, imageStride, maskStride, fillR, fillG, fillB )
                for ( y = 0; y < height; y++ )
                {
                    uint8_t* imageRow = imagePtr + y * imageStride;
                    uint8_t* maskRow  = maskPtr  + y * maskStride;
                    int x;

                    for ( x = 0; x < width; ++x, ++maskRow, imageRow += 3 )
                    {
                        if ( *maskRow == 0 )
                        {
                            imageRow[RedIndex]   = fillR;
                            imageRow[GreenIndex] = fillG;
                            imageRow[BlueIndex]  = fillB;
                        }
                    }
                }
            }
            else
            {
                #pragma omp parallel for schedule(static) shared( imagePtr, maskPtr, width, imageStride, maskStride, fillR, fillG, fillB )
                for ( y = 0; y < height; y++ )
                {
                    uint8_t* imageRow = imagePtr + y * imageStride;
                    uint8_t* maskRow  = maskPtr  + y * maskStride;
                    int x;

                    for ( x = 0; x < width; ++x, ++maskRow, imageRow += 3 )
                    {
                        if ( *maskRow != 0 )
                        {
                            imageRow[RedIndex]   = fillR;
                            imageRow[GreenIndex] = fillG;
                            imageRow[BlueIndex]  = fillB;
                        }
                    }
                }

            }
        }
        else
        {
            if ( fillOnZero == true )
            {
                #pragma omp parallel for schedule(static) shared( imagePtr, maskPtr, width, imageStride, maskStride, fillColor )
                for ( y = 0; y < height; y++ )
                {
                    uint8_t* imageRow = imagePtr + y * imageStride;
                    uint8_t* maskRow  = maskPtr  + y * maskStride;
                    int x;

                    for ( x = 0; x < width; ++x, ++maskRow, imageRow += 4 )
                    {
                        if ( *maskRow == 0 )
                        {
                            imageRow[RedIndex]   = fillColor.components.r;
                            imageRow[GreenIndex] = fillColor.components.g;
                            imageRow[BlueIndex]  = fillColor.components.b;
                            imageRow[AlphaIndex] = fillColor.components.a;
                        }
                    }
                }
            }
            else
            {
                #pragma omp parallel for schedule(static) shared( imagePtr, maskPtr, width, imageStride, maskStride, fillColor )
                for ( y = 0; y < height; y++ )
                {
                    uint8_t* imageRow = imagePtr + y * imageStride;
                    uint8_t* maskRow  = maskPtr  + y * maskStride;
                    int x;

                    for ( x = 0; x < width; ++x, ++maskRow, imageRow += 4 )
                    {
                        if ( *maskRow != 0 )
                        {
                            imageRow[RedIndex]   = fillColor.components.r;
                            imageRow[GreenIndex] = fillColor.components.g;
                            imageRow[BlueIndex]  = fillColor.components.b;
                            imageRow[AlphaIndex] = fillColor.components.a;
                        }
                    }
                }

            }
        }
    }

    return ret;
}

// Merge two images by applying MAX operator for every pair of pixels (result is put back to image1)
XErrorCode MergeImages( ximage* image1, const ximage* image2 )
{
    XErrorCode ret = CheckImages( image1, image2 );

    if ( ret == SuccessCode )
    {
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int pixelSize = ( image1->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image1->format == XPixelFormatRGB24 ) ? 3 : 4;
        int lineSize  = pixelSize * image1->width;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2 )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row1 = ptr1 + y * stride1;
            uint8_t* row2 = ptr2 + y * stride2;
            int x;

            // for 24/32 bpp image the loop can be unrolled a bit to increase performance.
            // however it is left like this for now because 8 bpp grayscale images are the main aim.
            for ( x = 0; x < lineSize; x++ )
            {
                if ( *row2 > *row1 )
                {
                    *row1 = *row2;
                }

                row1++;
                row2++;
            }
        }
    }

    return ret;
}

// Intersect two images by applying MIN operator for every pair of pixels (result is put back to image1)
XErrorCode IntersectImages( ximage* image1, const ximage* image2 )
{
    XErrorCode ret = CheckImages( image1, image2 );

    if ( ret == SuccessCode )
    {
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int pixelSize = ( image1->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image1->format == XPixelFormatRGB24 ) ? 3 : 4;
        int lineSize  = pixelSize * image1->width;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2 )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row1 = ptr1 + y * stride1;
            uint8_t* row2 = ptr2 + y * stride2;
            int x;

            // for 24/32 bpp image the loop can be unrolled a bit to increase performance.
            // however it is left like this for now because 8 bpp grayscale images are the main aim.
            for ( x = 0; x < lineSize; x++ )
            {
                if ( *row2 < *row1 )
                {
                    *row1 = *row2;
                }

                row1++;
                row2++;
            }
        }
    }

    return ret;
}

// Move image1 towards image2 by the specified step size
XErrorCode MoveTowardsImages( ximage* image1, const ximage* image2, uint8_t stepSize )
{
    XErrorCode ret = CheckImages( image1, image2 );

    if ( ret == SuccessCode )
    {
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int pixelSize = ( image1->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image1->format == XPixelFormatRGB24 ) ? 3 : 4;
        int lineSize  = pixelSize * image1->width;
        int step      = stepSize;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2, step )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row1 = ptr1 + y * stride1;
            uint8_t* row2 = ptr2 + y * stride2;
            int      x, diff;

            // for 24/32 bpp image the loop can be unrolled a bit to increase performance
            for ( x = 0; x < lineSize; x++ )
            {
                diff = (int) *row2 - *row1;

                if ( diff > 0 )
                {
                    *row1 += (uint8_t) ( ( step < diff ) ? step : diff );
                }
                else
                {
                    diff = -diff;
                    *row1 -= (uint8_t) ( ( step < diff ) ? step : diff );
                }

                row1++;
                row2++;
            }
        }
    }

    return ret;
}


/* NOTE:
 *
 * The below functions (FadeImages(), AddImages() and SubtractImages()) can be implemeted by a single common
 * function. But left 3 implementations for now to avoid some if-statements they are not required.
 */

// Fade one image into another by calculating: image1 * factor + image2 * (1.0 - factor), where factor is in [0, 1] range (result is put back to image1)
XErrorCode FadeImages( ximage* image1, const ximage* image2, float factor )
{
    XErrorCode ret = CheckImages( image1, image2 );

    factor = XINRANGE( factor, 0.0f, 1.0f );

    if ( ret == SuccessCode )
    {
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int pixelSize = ( image1->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image1->format == XPixelFormatRGB24 ) ? 3 : 4;
        int lineSize  = pixelSize * image1->width;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        float    factor2 = 1.0f - factor;

        #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2, factor, factor2 )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row1 = ptr1 + y * stride1;
            uint8_t* row2 = ptr2 + y * stride2;
            int x;

            // for 24/32 bpp image the loop can be unrolled a bit to increase performance
            for ( x = 0; x < lineSize; x++ )
            {
                *row1 = (uint8_t) ( factor * *row1 + factor2 * *row2 );

                row1++;
                row2++;
            }
        }
    }

    return ret;
}

// Add two images; image2 is multiplied by the specified factor which is in [0, 1] range (result is put back to image1)
XErrorCode AddImages( ximage* image1, const ximage* image2, float factor2 )
{
    XErrorCode ret = CheckImages( image1, image2 );

    factor2 = XINRANGE( factor2, 0.0f, 1.0f );

    if ( ret == SuccessCode )
    {
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int pixelSize = ( image1->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image1->format == XPixelFormatRGB24 ) ? 3 : 4;
        int lineSize  = pixelSize * image1->width;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2, factor2 )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row1 = ptr1 + y * stride1;
            uint8_t* row2 = ptr2 + y * stride2;
            int      x;
            float    v;

            // for 24/32 bpp image the loop can be unrolled a bit to increase performance
            for ( x = 0; x < lineSize; x++ )
            {
                v = factor2 * *row2 + *row1;

                if ( v > 255 )
                {
                    v = 255;
                }

                *row1 = (uint8_t) v;

                row1++;
                row2++;
            }
        }
    }

    return ret;
}

// Subtract two images; image2 is multiplied by the specified factor which is in [0, 1] range and then
// subtracted from image1 (result is put back to image1)
XErrorCode SubtractImages( ximage* image1, const ximage* image2, float factor2 )
{
    XErrorCode ret = CheckImages( image1, image2 );

    factor2 = XINRANGE( factor2, 0.0f, 1.0f );
    factor2 = -factor2;

    if ( ret == SuccessCode )
    {
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int pixelSize = ( image1->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image1->format == XPixelFormatRGB24 ) ? 3 : 4;
        int lineSize  = pixelSize * image1->width;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2, factor2 )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row1 = ptr1 + y * stride1;
            uint8_t* row2 = ptr2 + y * stride2;
            int      x;
            float    v;

            // for 24/32 bpp image the loop can be unrolled a bit to increase performance
            for ( x = 0; x < lineSize; x++ )
            {
                v = factor2 * *row2 + *row1;

                if ( v < 0 )
                {
                    v = 0;
                }

                *row1 = (uint8_t) v;

                row1++;
                row2++;
            }
        }
    }

    return ret;
}

// Calculates images difference: image1 = abs( image1 - image2 )
XErrorCode DiffImages( ximage* image1, const ximage* image2 )
{
    XErrorCode ret = CheckImages( image1, image2 );

// ---> define to make code more compact
#define COMPUTE_VALUE               \
    diff = (int16_t) *row1 - *row2; \
    if ( diff < 0 ) diff = -diff;   \
    *row1 = (uint8_t) diff;         \
    row1++; row2++;
// <--- end of define

    if ( ret == SuccessCode )
    {
        int width     = image1->width;
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        if ( image1->format == XPixelFormatGrayscale8 )
        {
            // grayscale version
            #pragma omp parallel for schedule(static) shared( ptr1, ptr2, width, stride1, stride2 )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row1 = ptr1 + y * stride1;
                uint8_t* row2 = ptr2 + y * stride2;
                int      x;
                int16_t  diff;

                for ( x = 0; x < width; x++ )
                {
                    COMPUTE_VALUE
                }
            }
        }
        else
        {
            // color version (Alpha channel is ignored for 32 bpp images)

            int is32bpp = ( image1->format == XPixelFormatRGB24 ) ? 0 : 1;

            #pragma omp parallel for schedule(static) shared( ptr1, ptr2, width, stride1, stride2, is32bpp )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row1 = ptr1 + y * stride1;
                uint8_t* row2 = ptr2 + y * stride2;
                int      x;
                int16_t  diff;

                for ( x = 0; x < width; x++ )
                {
                    // process 3 color channels
                    COMPUTE_VALUE
                    COMPUTE_VALUE
                    COMPUTE_VALUE

                    // increase pointers by 1 in case we deal with 32 bpp images
                    row1 += is32bpp;
                    row2 += is32bpp;
                }
            }
        }
    }

#undef COMPUTE_VALUE

    return ret;
}

// Calculates thresholded images difference: image1 = ( abs( image1 - image2 ) >= threshold ) ? hiColor : lowColor
XErrorCode DiffImagesThresholded( ximage* image1, const ximage* image2, int16_t threshold, uint32_t* diffPixels, xargb hiColor, xargb lowColor )
{
    XErrorCode ret = CheckImages( image1, image2 );

    if ( ret == SuccessCode )
    {
        int width     = image1->width;
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int x, y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        uint32_t counter = 0;

        if ( image1->format == XPixelFormatGrayscale8 )
        {
            int     offset1  = stride1 - width;
            int     offset2  = stride2 - width;
            int16_t diff;
            uint8_t hiValue  = (uint8_t) ( RGB_TO_GRAY(  hiColor.components.r,  hiColor.components.g,  hiColor.components.b ) * 255 /  hiColor.components.a );
            uint8_t lowValue = (uint8_t) ( RGB_TO_GRAY( lowColor.components.r, lowColor.components.g, lowColor.components.b ) * 255 / lowColor.components.a );

            // grayscale version
            for ( y = 0; y < height; ++y )
            {
                for ( x = 0; x < width; ++x, ++ptr1, ++ptr2 )
                {
                    diff = (int16_t) *ptr1 - *ptr2;
                    if ( diff < 0 ) diff = -diff;

                    if ( diff >= threshold )
                    {
                        *ptr1 = hiValue;
                        ++counter;
                    }
                    else
                    {
                        *ptr1 = lowValue;
                    }
                }

                ptr1 += offset1;
                ptr2 += offset2;
            }
        }
        else if ( image1->format == XPixelFormatRGB24 )
        {
            // 24 bpp version
            int offset1  = stride1 - width * 3;
            int offset2  = stride2 - width * 3;
            uint8_t hiR  = (uint8_t) (  hiColor.components.r * 255 /  hiColor.components.a );
            uint8_t hiG  = (uint8_t) (  hiColor.components.g * 255 /  hiColor.components.a );
            uint8_t hiB  = (uint8_t) (  hiColor.components.b * 255 /  hiColor.components.a );
            uint8_t lowR = (uint8_t) ( lowColor.components.r * 255 / lowColor.components.a );
            uint8_t lowG = (uint8_t) ( lowColor.components.g * 255 / lowColor.components.a );
            uint8_t lowB = (uint8_t) ( lowColor.components.b * 255 / lowColor.components.a );

            int16_t  diffR, diffG, diffB;

            for ( y = 0; y < height; ++y )
            {
                for ( x = 0; x < width; ++x, ptr1 += 3, ptr2 += 3 )
                {
                    diffR = (int16_t) ptr1[RedIndex]   - ptr2[RedIndex];
                    diffG = (int16_t) ptr1[GreenIndex] - ptr2[GreenIndex];
                    diffB = (int16_t) ptr1[BlueIndex]  - ptr2[BlueIndex];

                    if ( diffR < 0 ) diffR = -diffR;
                    if ( diffG < 0 ) diffG = -diffG;
                    if ( diffB < 0 ) diffB = -diffB;

                    if ( diffR + diffG + diffB >= threshold )
                    {
                        ptr1[RedIndex]   = hiR;
                        ptr1[GreenIndex] = hiG;
                        ptr1[BlueIndex]  = hiB;
                        ++counter;
                    }
                    else
                    {
                        ptr1[RedIndex]   = lowR;
                        ptr1[GreenIndex] = lowG;
                        ptr1[BlueIndex]  = lowB;
                    }
                }

                ptr1 += offset1;
                ptr2 += offset2;
            }
        }
        else
        {
            // 32 bpp version (Alpha channel is ignored, only set to specified value)
            int offset1  = stride1 - width * 4;
            int offset2  = stride2 - width * 4;

            int16_t  diffR, diffG, diffB;

            for ( y = 0; y < height; ++y )
            {
                for ( x = 0; x < width; ++x, ptr1 += 4, ptr2 += 4 )
                {
                    diffR = (int16_t) ptr1[RedIndex]   - ptr2[RedIndex];
                    diffG = (int16_t) ptr1[GreenIndex] - ptr2[GreenIndex];
                    diffB = (int16_t) ptr1[BlueIndex]  - ptr2[BlueIndex];

                    if ( diffR < 0 ) diffR = -diffR;
                    if ( diffG < 0 ) diffG = -diffG;
                    if ( diffB < 0 ) diffB = -diffB;

                    if ( diffR + diffG + diffB >= threshold )
                    {
                        ptr1[RedIndex]   = hiColor.components.r;
                        ptr1[GreenIndex] = hiColor.components.g;
                        ptr1[BlueIndex]  = hiColor.components.b;
                        ptr1[AlphaIndex] = hiColor.components.a;
                        ++counter;
                    }
                    else
                    {
                        ptr1[RedIndex]   = lowColor.components.r;
                        ptr1[GreenIndex] = lowColor.components.g;
                        ptr1[BlueIndex]  = lowColor.components.b;
                        ptr1[AlphaIndex] = lowColor.components.a;
                    }
                }

                ptr1 += offset1;
                ptr2 += offset2;
            }
        }

        if ( diffPixels )
        {
            *diffPixels = counter;
        }
    }

    return ret;
}

// Blends two image using the specified blend mode. image1 is treated as top layer and image2 is base as top layer.
XErrorCode BlendImages( ximage* image1, const ximage* image2, XBlendMode blendMode )
{
    XErrorCode ret = CheckImages( image1, image2 );

    if ( ret == SuccessCode )
    {
        int height    = image1->height;
        int stride1   = image1->stride;
        int stride2   = image2->stride;
        int pixelSize = ( image1->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image1->format == XPixelFormatRGB24 ) ? 3 : 4;
        int lineSize  = pixelSize * image1->width;
        int y;

        uint8_t* ptr1 = image1->data;
        uint8_t* ptr2 = image2->data;

        /*
            a - top layer
            b - base layer
        */
        switch ( blendMode )
        {
        case BlendMode_Multiply:
            /*
                f(a, b) = a * b             , a,b in [0, 1]
                f(a, b) = a * b / 255       , a,b in [0, 255]
            */

            #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2 )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row1 = ptr1 + y * stride1;
                uint8_t* row2 = ptr2 + y * stride2;
                int x;

                // for 24/32 bpp image the loop can be unrolled a bit to increase performance
                for ( x = 0; x < lineSize; x++ )
                {
                    *row1 = (uint8_t) ( ( *row1 * *row2 ) / 255 );

                    row1++;
                    row2++;
                }
            }
            break;

        case BlendMode_Screen:
            /*
                f(a, b) = 1 - ( 1 - a ) * ( 1 - b )                 , a,b in [0, 1]
                f(a, b) = 255 - ( 255 - a ) * ( 255 - b ) / 255     , a,b in [0, 255]
            */

            #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2 )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row1 = ptr1 + y * stride1;
                uint8_t* row2 = ptr2 + y * stride2;
                int x;

                // for 24/32 bpp image the loop can be unrolled a bit to increase performance
                for ( x = 0; x < lineSize; x++ )
                {
                    *row1 = (uint8_t) ( 255 - ( 255 - *row1 ) * ( 255 - *row2 ) / 255 );

                    row1++;
                    row2++;
                }
            }

            break;

        case BlendMode_Overlay:
            /*
                f(a, b) = | 2 * a * b                      , if b < 0.5
                          | 1 - 2 * ( 1 - a ) * ( 1 - b )  , otherwise

                f(a, b) = | 2 * a * b / 255                            , if b < 128
                          | 255 - 2 * ( 255 - a ) * ( 255 - b ) / 255  , otherwise
            */

            #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2 )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row1 = ptr1 + y * stride1;
                uint8_t* row2 = ptr2 + y * stride2;
                int x;

                // for 24/32 bpp image the loop can be unrolled a bit to increase performance
                for ( x = 0; x < lineSize; x++ )
                {
                    if ( *row2 < 128 )
                    {
                        *row1 = (uint8_t) ( ( 2 * *row1 * *row2 ) / 255 );
                    }
                    else
                    {
                        *row1 = (uint8_t) ( 255 - ( 2 * ( 255 - *row1 ) * ( 255 - *row2 ) ) / 255 );
                    }

                    row1++;
                    row2++;
                }
            }
            break;

        case BlendMode_ColorDodge:
            /*
                f(a, b) = | b                                 , if b == 255
                          | min( 255, a * 255 / ( 255 - b ) ) , otherwise
            */

            #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2 )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row1 = ptr1 + y * stride1;
                uint8_t* row2 = ptr2 + y * stride2;
                int x, t;

                for ( x = 0; x < lineSize; x++ )
                {
                    if ( *row2 == 255 )
                    {
                        *row1 = 255;
                    }
                    else
                    {
                        t = ( 255 * *row1 ) / ( 255 - *row2 );
                        if ( t > 255 ) { t = 255; }

                        *row1 = (uint8_t) t;
                    }

                    row1++;
                    row2++;
                }
            }
            break;

        case BlendMode_ColorBurn:
            /*
                f(a, b) = | b                                     , if b == 0
                          | max( 0, 255 - ( 255 - a ) * 255 / b ) , otherwise
            */

            #pragma omp parallel for schedule(static) shared( ptr1, ptr2, lineSize, stride1, stride2 )
            for ( y = 0; y < height; y++ )
            {
                uint8_t* row1 = ptr1 + y * stride1;
                uint8_t* row2 = ptr2 + y * stride2;
                int x, t;

                for ( x = 0; x < lineSize; x++ )
                {
                    if ( *row2 == 0 )
                    {
                        *row1 = 0;
                    }
                    else
                    {
                        t = 255 - ( ( 255 - *row1 ) * 255 ) / *row2;
                        if ( t < 0 ) { t = 0; }

                        *row1 = (uint8_t) t;
                    }

                    row1++;
                    row2++;
                }
            }
            break;

        default:
            ret = ErrorInvalidArgument;
        }
    }

    return ret;
}
