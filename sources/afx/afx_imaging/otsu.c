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
#include <string.h>

// N. Otsu, "A threshold selection method from gray-level histograms",
// IEEE Trans. Systems, Man and Cybernetics 9(1), pp. 62–66, 1979.

static uint16_t CalculateOtsuThresholdFromHistogram( double* histogram, double meanValue );

// Calculate optimal threshold for grayscale image using Otsu algorithm
XErrorCode CalculateOtsuThreshold( const ximage* src, uint16_t* threshold )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( threshold == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int integerHistogram[256];
        double histogram[256];
        double imageMean = 0;

        int width  = src->width;
        int height = src->height;
        int offset = src->stride - width;
        int y, x, i;

        int pixelCount = width * height;

        uint8_t* ptr = src->data;

        // initialize histogram to zeros
        memset( (void*) integerHistogram, 0, sizeof( integerHistogram ) );

        // calculate histogram first
        for ( y = 0; y < height; y++, ptr += offset )
        {
            for ( x = 0; x < width; x++, ptr++ )
            {
                integerHistogram[*ptr]++;
            }
        }

        // convert histogram to doubles and calculate intensity’s mean value
        for ( i = 0; i < 256; i++ )
        {
            histogram[i] = (double) integerHistogram[i] / pixelCount;
            imageMean += histogram[i] * i;
        }

        *threshold = CalculateOtsuThresholdFromHistogram( histogram, imageMean );
    }

    return ret;
}

// Apply Otsu thresholding to an image
XErrorCode OtsuThresholding( ximage* image )
{
    uint16_t   threshold = 0;
    XErrorCode ret = CalculateOtsuThreshold( image, &threshold );

    if ( ret == SuccessCode )
    {
        ret = ThresholdImage( image, threshold );
    }

    return ret;
}

// Calculate value of the Otsu threshold based on image's histogram
static uint16_t CalculateOtsuThresholdFromHistogram( double* histogram, double meanValue )
{
    uint16_t calculatedThreshold = 0;
    double max = 0;

    // initial class probabilities
    double class1Probability = 0;
    double class2Probability = 1;

    // initial class 1 mean value
    double class1MeanInit = 0;

    // some temp variables
    int threshold;
    double class1Mean, class2Mean, meanDiff, betweenClassVariance;

    // check all thresholds
    for ( threshold = 0; ( threshold < 256 ) && ( class2Probability > 0 ); threshold++ )
    {
        // calculate classes' means for the given threshold
        class1Mean = class1MeanInit;
        class2Mean = ( meanValue - ( class1Mean * class1Probability ) ) / class2Probability;
        meanDiff = class1Mean - class2Mean;

        // calculate between class variance
        betweenClassVariance = ( class1Probability ) * ( 1.0 - class1Probability ) * meanDiff * meanDiff;

        // check if we found new threshold candidate
        if ( betweenClassVariance > max )
        {
            max = betweenClassVariance;
            calculatedThreshold = (uint16_t) threshold;
        }

        // update initial probabilities and mean value
        class1MeanInit *= class1Probability;

        class1Probability += histogram[threshold];
        class2Probability -= histogram[threshold];

        class1MeanInit += histogram[threshold] * threshold;

        if ( class1Probability != 0 )
        {
            class1MeanInit /= class1Probability;
        }
    }

    return calculatedThreshold;
}
