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

// Calculates Gassian blur 1D kernel of the specified size using the specified sigma value.
// Kernel must be an allocated array of size: radius * 2 + 1
XErrorCode CreateGaussianBlurKernel1D( float sigma, int radius, float* kernel )
{
    XErrorCode ret = SuccessCode;

    if ( kernel == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( radius < 1 )    || ( radius > 10 ) ||
              ( sigma  < 0.1f ) || ( sigma  > 10 ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        double sqrSigma  = sigma * sigma;
        float* kernelPtr = kernel;
        int    x;

        for ( x = -radius; x <= radius; x++ )
        {
            *kernelPtr = (float) exp( (double) x * x / ( -2.0 * sqrSigma ) );
            kernelPtr++;
        }
    }

    return ret;
}

// Calculates Gassian blur 2D kernel of the specified size using the specified sigma value.
// Kernel must be an allocated array of size: (radius * 2 + 1) * (radius * 2 + 1)
XErrorCode CreateGaussianBlurKernel2D( float sigma, int radius, float* kernel )
{
    XErrorCode ret = SuccessCode;

    if ( kernel == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( radius < 1 )    || ( radius > 10 ) ||
              ( sigma  < 0.1f ) || ( sigma  > 10 ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else
    {
        double sqrSigma  = sigma * sigma;
        float* kernelPtr = kernel;
        int    x, y;

        for ( y = -radius; y <= radius; y++ )
        {
            for ( x = -radius; x <= radius; x++ )
            {
                *kernelPtr = (float) exp( (double) ( x * x + y * y ) / ( -2.0 * sqrSigma ) );
                kernelPtr++;
            }
        }
    }

    return ret;
}

// Calculates Gassian sharpen kernel of the specified size using the specified sigma value.
// Kernel must be an allocated array of size: (radius * 2 + 1) * (radius * 2 + 1)
XErrorCode CreateGaussianSharpenKernel2D( float sigma, int radius, float* kernel )
{
    XErrorCode ret = CreateGaussianBlurKernel2D( sigma, radius, kernel );

    if ( ret == SuccessCode )
    {
        // invert the kernel so it does sharpening
        int    i, j;
        int    kernelSize   = radius * 2 + 1;
        int    kernelSizeSq = kernelSize * kernelSize;
        float  kernelSum    = 0;
        float* kernelPtr    = kernel;

        for ( i = 0; i < kernelSizeSq; i++ )
        {
            kernelSum += kernel[i];
        }

        for ( i = 0; i < kernelSize; i++ )
        {
            for ( j = 0; j < kernelSize; j++ )
            {
                if ( ( i == radius ) && ( j == radius ) )
                {
                    // calculate central value
                    *kernelPtr = 2 * kernelSum - *kernelPtr;
                }
                else
                {
                    // invert value
                    *kernelPtr = -( *kernelPtr );
                }

                kernelPtr++;
            }
        }
    }

    return ret;
}

// Perform Gaussian blur with the specified sigma value and blurring radius
XErrorCode GaussianBlur( const ximage* src, ximage* dst, float sigma, uint8_t radius )
{
    XErrorCode ret        = SuccessCode;
    int        kernelSize = radius * 2 + 1;
    float*     kernel     = 0;

    kernel = (float*) malloc( sizeof( float ) * kernelSize * kernelSize );

    if ( kernel == 0 )
    {
        ret = ErrorOutOfMemory;
    }
    else
    {
        ret = CreateGaussianBlurKernel2D( sigma, radius, kernel );

        if ( ret == SuccessCode )
        {
            ret = Convolution( src, dst, kernel, kernelSize, true );
        }

        free( kernel );
    }

    return ret;
}

// Perform Gaussian sharpening with the specified sigma value and sharpening radius
XErrorCode GaussianSharpen( const ximage* src, ximage* dst, float sigma, uint8_t radius )
{
    XErrorCode ret        = SuccessCode;
    int        kernelSize = radius * 2 + 1;
    float*     kernel     = 0;

    kernel = (float*) malloc( sizeof( float ) * kernelSize * kernelSize );

    if ( kernel == 0 )
    {
        ret = ErrorOutOfMemory;
    }
    else
    {
        ret = CreateGaussianSharpenKernel2D( sigma, radius, kernel );

        if ( ret == SuccessCode )
        {
            ret = Convolution( src, dst, kernel, kernelSize, true );
        }

        free( kernel );
    }

    return ret;
}
