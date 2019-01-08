/*
    Computer vision library of Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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

#include "xvision.h"

// Build integral image for the specified image (RGB channel must be specified for color images)
XErrorCode BuildIntegralImage( const ximage* image, ximage* integralImage, XRGBComponent rgbChannel )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( integralImage == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( ( image->format != XPixelFormatGrayscale8 ) &&
                ( image->format != XPixelFormatRGB24 ) &&
                ( image->format != XPixelFormatRGBA32 ) ) ||
                ( integralImage->format != XPixelFormatGrayscale32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) && ( rgbChannel >= AlphaIndex ) )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( (uint64_t) image->width * image->height > 0x00FFFFFF )
    {
        ret = ErrorImageIsTooBig;
    }
    else if ( ( image->width  + 1 != integralImage->width  ) ||
              ( image->height + 1 != integralImage->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int pixelSize = ( image->format == XPixelFormatGrayscale8 ) ? 1 :
                        ( image->format == XPixelFormatRGB24 ) ? 3 : 4;
        int width     = image->width;
        int height    = image->height;
        int stride    = image->stride;
        int offset    = stride - ( width * pixelSize );
        int intStride = integralImage->stride;
        int x, y;

        uint8_t*  src = image->data;

        uint32_t  rowSum;
        uint8_t*  intRowPtr = integralImage->data;
        uint32_t* intRowPtr1;
        uint32_t* intRowPtr2;

        // align to required RGB channel for color images
        if ( image->format != XPixelFormatGrayscale8 )
        {
            src += rgbChannel;
        }

        // fill the first row of integral image with 0
        intRowPtr1 = (uint32_t*) intRowPtr;
        for ( x = 0; x <= width; x++, intRowPtr1++ )
        {
            *intRowPtr1 = 0;
        }

        for ( y = 0; y < height; y++ )
        {
            rowSum = 0;

            intRowPtr2 = (uint32_t*) intRowPtr;
            intRowPtr += intStride;
            intRowPtr1 = (uint32_t*) intRowPtr;

            // set first value in the row to 0
            *intRowPtr1 = 0;

            for ( x = 0; x < width; x++, src += pixelSize )
            {
                ++intRowPtr1;
                ++intRowPtr2;

                rowSum     += *src;
                *intRowPtr1 = *intRowPtr2 + rowSum;
            }

            src += offset;
        }
    }

    return ret;
}

// Build integral and squared integral images for the specified image (RGB channel must be specified for color images)
XErrorCode BuildIntegralImage2( const ximage* image, ximage* integralImage, ximage* sqIntegralImage, XRGBComponent rgbChannel )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( integralImage == 0 ) || ( sqIntegralImage  == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( ( image->format != XPixelFormatGrayscale8 ) &&
                ( image->format != XPixelFormatRGB24 ) &&
                ( image->format != XPixelFormatRGBA32 ) ) ||
                ( integralImage->format != XPixelFormatGrayscale32 ) || 
                ( sqIntegralImage->format != XPixelFormatGrayscale64 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) && ( rgbChannel >= AlphaIndex ) )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( (uint64_t) image->width * image->height > 0x00FFFFFF )
    {
        ret = ErrorImageIsTooBig;
    }
    else if ( ( image->width  + 1 != integralImage->width  ) ||
              ( image->height + 1 != integralImage->height ) ||
              ( integralImage->width  != sqIntegralImage->width  ) ||
              ( integralImage->height != sqIntegralImage->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int pixelSize   = ( image->format == XPixelFormatGrayscale8 ) ? 1 :
                          ( image->format == XPixelFormatRGB24 ) ? 3 : 4;
        int width       = image->width;
        int height      = image->height;
        int stride      = image->stride;
        int offset      = stride - ( width * pixelSize );
        int intStride   = integralImage->stride;
        int sqIntStride = sqIntegralImage->stride;
        int x, y;

        uint8_t*  src = image->data;

        uint32_t  rowSum;
        uint8_t*  intRowPtr = integralImage->data;
        uint32_t* intRowPtr1;
        uint32_t* intRowPtr2;

        uint64_t  sqRowSum;
        uint8_t*  sqIntRowPtr = sqIntegralImage->data;
        uint64_t* sqIntRowPtr1;
        uint64_t* sqIntRowPtr2;

        // align to required RGB channel for color images
        if ( image->format != XPixelFormatGrayscale8 )
        {
            src += rgbChannel;
        }

        // fill the first row of integral images with 0
        intRowPtr1   = (uint32_t*) intRowPtr;
        sqIntRowPtr1 = (uint64_t*) sqIntRowPtr; 
        for ( x = 0; x <= width; x++, intRowPtr1++, sqIntRowPtr1++ )
        {
            *intRowPtr1   = 0;
            *sqIntRowPtr1 = 0;
        }

        for ( y = 0; y < height; y++ )
        {
            rowSum   = 0;
            sqRowSum = 0;

            intRowPtr2 = (uint32_t*) intRowPtr;
            intRowPtr += intStride;
            intRowPtr1 = (uint32_t*) intRowPtr;

            sqIntRowPtr2 = (uint64_t*) sqIntRowPtr;
            sqIntRowPtr += sqIntStride;
            sqIntRowPtr1 = (uint64_t*) sqIntRowPtr;

            // set first value in the row to 0
            *intRowPtr1   = 0;
            *sqIntRowPtr1 = 0;

            for ( x = 0; x < width; x++, src += pixelSize )
            {
                ++intRowPtr1;
                ++intRowPtr2;
                ++sqIntRowPtr1;
                ++sqIntRowPtr2;

                rowSum       += *src;
                sqRowSum     += *src * *src;
                *intRowPtr1   = *intRowPtr2 + rowSum;
                *sqIntRowPtr1 = *sqIntRowPtr2 + sqRowSum;
            }

            src += offset;
        }
    }

    return ret;
}
