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

#define THRESHOLD (128)

// Perform error diffusion dithering of the specified 8 bpp grayscale image putting result into the specified 1 bpp binary image
// Note: destination image must be "clean" - black pixels only
XErrorCode BinaryErrorDiffusionDithering( const ximage* src, ximage* dst, ximage* tmp,
                                          int32_t diffusionLinesCount, const int32_t* diffusionLinesLength,
                                          const uint8_t** diffusionLinesCoefficients )
{
    XErrorCode ret = SuccessCode;
    int32_t    coefficientsSum = 0;
    ximage*    tempImage = 0;

    if ( ( src == 0 ) || ( dst == 0 ) || ( diffusionLinesLength == 0 ) || ( diffusionLinesCoefficients == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) || ( dst->format != XPixelFormatBinary1 ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( diffusionLinesCount <= 0 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        int32_t i, j;

        for ( i = 0; i < diffusionLinesCount; i++ )
        {
            if ( diffusionLinesLength[i] <= 0 )
            {
                ret = ErrorInvalidArgument;
                break;
            }
            else if ( diffusionLinesCoefficients[i] == 0 )
            {
                ret = ErrorNullParameter;
                break;
            }
            else
            {
                for ( j = 0; j < diffusionLinesLength[i]; j++ )
                {
                    if ( diffusionLinesCoefficients[i][j] < 0 )
                    {
                        ret = ErrorInvalidArgument;
                    }
                    else
                    {
                        coefficientsSum += diffusionLinesCoefficients[i][j];
                    }
                }
            }
        }
    }

    if ( ret == SuccessCode )
    {
        // check specified temp image to see if it can be reused
        if ( tmp != 0 )
        {
            if ( ( tmp->format != XPixelFormatGrayscale8 ) ||
                 ( tmp->width < src->width ) ||
                 ( tmp->height < src->height ) )
            {
                // although we could create our own temp image, we
                // better report an error here, so user know he gave junk
                ret = ErrorImageParametersMismatch;
            }
            else
            {
                // since we allow bigger temp images, we need to get its sub-image
                ret = XImageGetSubImage( tmp, &tempImage, 0, 0, src->width, src->height );
            }
        }
        else
        {
            // allocate our own temp image
            ret = XImageAllocate( src->width, src->height, XPixelFormatGrayscale8, &tempImage );
        }
    }

    if ( ret == SuccessCode )
    {
        int      width     = src->width;
        int      height    = src->height;
        int      dstStride = dst->stride;
        int      tmpStride = tempImage->stride;
        int      x, y;
        uint32_t shift;
        int32_t  error;
        int32_t  diffusion;
        int      i;

        uint8_t* tmpPtr = tempImage->data;
        uint8_t* dstPtr = dst->data;
        uint8_t* tmpRow2;
        int32_t  value;

        int      imageIndex;
        int      coefficientIndex;
        int      coefficientsCount;

        const uint8_t* coefficients;

        // copy source image to temp, since we need to modify it
        XImageCopyData( src, tempImage );

        for ( y = 0; y < height; y++ )
        {
            uint8_t* tmpRow = tmpPtr + y * tmpStride;
            uint8_t* dstRow = dstPtr + y * dstStride;

            for ( x = 0; x < width; x++ )
            {
                value = (int32_t) *tmpRow;
                shift = 7 - ( x & 7 );

                // thresholding and error calculation
                if ( value >= THRESHOLD )
                {
                    *dstRow |= ( 1 << shift );
                    error = value - 255;
                }
                else
                {
                    error = value;
                }

                // error diffusion to the pixels on the right of the just processed one
                coefficients = diffusionLinesCoefficients[0];
                coefficientsCount = diffusionLinesLength[0];

                for ( coefficientIndex = 0, imageIndex = 1; coefficientIndex < coefficientsCount; coefficientIndex++, imageIndex++ )
                {
                    if ( x + imageIndex >= width )
                    {
                        break;
                    }

                    diffusion = ( error * coefficients[coefficientIndex] ) / coefficientsSum + tmpRow[imageIndex];
                    diffusion = XINRANGE( diffusion, 0, 255 );
                    tmpRow[imageIndex] = (uint8_t) diffusion;
                }

                // error diffusion to the pixels under the one just processed
                for ( i = 1; i < diffusionLinesCount; i++ )
                {
                    if ( y + i >= height )
                    {
                        break;
                    }

                    // get the line to diffuse error to
                    tmpRow2 = tmpRow + i * tmpStride;

                    coefficients = diffusionLinesCoefficients[i];
                    coefficientsCount = diffusionLinesLength[i];

                    for  ( coefficientIndex = 0, imageIndex = -( coefficientsCount >> 1 ); coefficientIndex < coefficientsCount; coefficientIndex++, imageIndex++ )
                    {
                        if ( x + imageIndex >= width )
                        {
                            break;
                        }
                        if ( x + imageIndex < 0 )
                        {
                            continue;
                        }

                        diffusion = ( error * coefficients[coefficientIndex] ) / coefficientsSum + tmpRow2[imageIndex];
                        diffusion = XINRANGE( diffusion, 0, 255 );
                        tmpRow2[imageIndex] = (uint8_t) diffusion;
                    }
                }

                // increment pointers
                if ( shift == 0 )
                {
                    dstRow++;
                }
                tmpRow++;
            }
        }

        // free temp image
        if ( tempImage != 0 )
        {
            XImageFree( &tempImage );
        }
    }

    return ret;
}

// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Floyd-Steinberg algorithm
XErrorCode FloydSteinbergBinaryDithering( const ximage* src, ximage* dst, ximage* tmp )
{
    static const uint8_t coefficientsLine1[] = { 7 };
    static const uint8_t coefficientsLine2[] = { 3, 5, 1 };

    static const int32_t diffusionLinesLength[] =
    {
        1, 3
    };
    static const uint8_t* diffusionLinesCoefficients[] =
    {
        coefficientsLine1,
        coefficientsLine2
    };

    return BinaryErrorDiffusionDithering( src, dst, tmp, 2, diffusionLinesLength, diffusionLinesCoefficients );
}

// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Burkes algorithm
XErrorCode BurkesBinaryDithering( const ximage* src, ximage* dst, ximage* tmp )
{
    static const uint8_t coefficientsLine1[] = { 8, 4 };
    static const uint8_t coefficientsLine2[] = { 2, 4, 8, 4, 2 };

    static const int32_t diffusionLinesLength[] =
    {
        2, 5
    };
    static const uint8_t* diffusionLinesCoefficients[] =
    {
        coefficientsLine1,
        coefficientsLine2
    };

    return BinaryErrorDiffusionDithering( src, dst, tmp, 2, diffusionLinesLength, diffusionLinesCoefficients );
}

// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Jarvis-Judice-Ninke algorithm
XErrorCode JarvisJudiceNinkeBinaryDithering( const ximage* src, ximage* dst, ximage* tmp )
{
    static const uint8_t coefficientsLine1[] = { 7, 5 };
    static const uint8_t coefficientsLine2[] = { 3, 5, 7, 5, 3 };
    static const uint8_t coefficientsLine3[] = { 1, 3, 5, 3, 1 };

    static const int32_t diffusionLinesLength[] =
    {
        2, 5, 5
    };
    static const uint8_t* diffusionLinesCoefficients[] =
    {
        coefficientsLine1,
        coefficientsLine2,
        coefficientsLine3
    };

    return BinaryErrorDiffusionDithering( src, dst, tmp, 3, diffusionLinesLength, diffusionLinesCoefficients );
}

// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Sierra algorithm
XErrorCode SierraBinaryDithering( const ximage* src, ximage* dst, ximage* tmp )
{
    static const uint8_t coefficientsLine1[] = { 5, 3 };
    static const uint8_t coefficientsLine2[] = { 2, 4, 5, 4, 2 };
    static const uint8_t coefficientsLine3[] = { 2, 3, 2 };

    static const int32_t diffusionLinesLength[] =
    {
        2, 5, 3
    };
    static const uint8_t* diffusionLinesCoefficients[] =
    {
        coefficientsLine1,
        coefficientsLine2,
        coefficientsLine3
    };

    return BinaryErrorDiffusionDithering( src, dst, tmp, 3, diffusionLinesLength, diffusionLinesCoefficients );
}

// Perform error diffusion dithering of the specified 8 bpp grayscale image using the Stucki algorithm
XErrorCode StuckiBinaryDithering( const ximage* src, ximage* dst, ximage* tmp )
{
    static const uint8_t coefficientsLine1[] = { 8, 4 };
    static const uint8_t coefficientsLine2[] = { 2, 4, 8, 4, 2 };
    static const uint8_t coefficientsLine3[] = { 1, 2, 4, 2, 1 };

    static const int32_t diffusionLinesLength[] =
    {
        2, 5, 5
    };
    static const uint8_t* diffusionLinesCoefficients[] =
    {
        coefficientsLine1,
        coefficientsLine2,
        coefficientsLine3
    };

    return BinaryErrorDiffusionDithering( src, dst, tmp, 3, diffusionLinesLength, diffusionLinesCoefficients );
}
