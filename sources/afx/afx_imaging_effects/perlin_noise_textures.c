/*
    Imaging effects library of Computer Vision Sandbox

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

#include <stdlib.h>
#include <math.h>
#include <memory.h>
#include <time.h>
#include "xtextures.h"
#include "ximaging.h"

// ======= Local types =======
// ===========================

// Perlin noise generation description comes from the page below:
// http://freespace.virgin.net/hugo.elias/models/m_perlin.htm

// Settings for perlin noise generator
typedef struct perlinNoiseSettingsTag
{
    // The property sets initial frequency of the first octave. Frequencies for
    // next octaves are calculated using the next equation:
    // frequency[i] = initFrequency * 2^i,  where i = [0, octaves).
    double initFrequency;

    // The property sets initial amplitude of the first octave. Amplitudes for
    // next octaves are calculated using the next equation:
    // amplitude[i] = initAmplitude * persistence^i,  where i = [0, octaves).
    double initAmplitude;

    // The property sets so called persistence value, which controls the way
    // amplitude is calculated for each octave comprising the Perlin noise function.
    double persistence;

    // The property sets the number of noise functions, which sum up the resulting
    // Perlin noise function.
    int    octaves;
}
PerlinNoiseSettings;

// ======= Local functions =======
// ===============================

// Fills pointed structure with default Perlin noise settings
/* not in use for now
static void GetDefaultPerlinNoiseSettings( PerlinNoiseSettings* settings )
{
    if ( settings != 0 )
    {
        settings->initFrequency = 1.0;
        settings->initAmplitude = 1.0;
        settings->persistence   = 0.65;
        settings->octaves       = 4;
    }
}
*/

// Cosine interpolation function
static double CosineInterpolate( double x1, double x2, double a )
{
    double f = ( 1 - cos( a * XPI ) ) * 0.5;
    return x1 * ( 1 - f ) + x2 * f;
}

// Base noise generation functions
static double Noise1D( int x )
{
    int n = ( x << 13 ) ^ x;

    return ( 1.0 - ( ( n * ( n * n * 15731 + 789221 ) + 1376312589 ) & 0x7fffffff ) / 1073741824.0 );
}
static double Noise2D( int x, int y )
{
    int n = x + y * 57;
    n = ( n << 13 ) ^ n;

    return ( 1.0 - ( ( n * ( n * n * 15731 + 789221 ) + 1376312589 ) & 0x7fffffff ) / 1073741824.0 );
}

// Smoothed noise generation functions
static double SmoothedNoise1D( double x )
{
    int     xInt = (int) x;
    double  xFrac = x - xInt;

    return CosineInterpolate( Noise1D( xInt ), Noise1D( xInt + 1 ), xFrac );
}
static double SmoothedNoise2D( double x, double y )
{
    int     xInt = (int) x;
    int     yInt = (int) y;
    double  xFrac = x - xInt;
    double  yFrac = y - yInt;

    // get four noise values
    double  x0y0 = Noise2D( xInt,     yInt );
    double  x1y0 = Noise2D( xInt + 1, yInt );
    double  x0y1 = Noise2D( xInt,     yInt + 1 );
    double  x1y1 = Noise2D( xInt + 1, yInt + 1 );

    // x interpolation
    double  v1 = CosineInterpolate( x0y0, x1y0, xFrac );
    double  v2 = CosineInterpolate( x0y1, x1y1, xFrac );
    // y interpolation
    return CosineInterpolate( v1, v2, yFrac );
}

// 1D Perlin noise function
static double PerlinNoise1D( const PerlinNoiseSettings* settings, double x )
{
    double  sum = 0;

    if ( settings != 0 )
    {
        double  frequency = settings->initFrequency;
        double  amplitude = settings->initAmplitude;
        int     i;

        // octaves
        for ( i = 0; i < settings->octaves; i++ )
        {
            sum += SmoothedNoise1D( x * frequency ) * amplitude;

            frequency *= 2;
            amplitude *= settings->persistence;
        }
    }
    return sum;
}

// 2D Perlin noise function
static double PerlinNoise2D( const PerlinNoiseSettings* settings, double x, double y )
{
    double  sum = 0;

    if ( settings != 0 )
    {
        double  frequency = settings->initFrequency;
        double  amplitude = settings->initAmplitude;
        int     i;

        // octaves
        for ( i = 0; i < settings->octaves; i++ )
        {
            sum += SmoothedNoise2D( x * frequency, y * frequency ) * amplitude;

            frequency *= 2;
            amplitude *= settings->persistence;
        }
    }
    return sum;
}

// ======= Public API =======
// ==========================

// Generate textile looking texture
XErrorCode GenerateTextileTexture( ximage* texture, uint16_t randNumber, uint8_t stitchSize, uint8_t stitchOffset )
{
    XErrorCode ret = SuccessCode;

    if ( texture == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( texture->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width  = texture->width;
        int height = texture->height;
        int stride = texture->stride;
        int randN  = (int) randNumber;
        int y;

        uint8_t* ptr = texture->data;

        // scaling factor
        double factor = 2.0 * XPI / stitchSize;

        PerlinNoiseSettings perlinSettngs;

        perlinSettngs.octaves       = 3;
        perlinSettngs.persistence   = 0.65;
        perlinSettngs.initFrequency = 1.0 / 8;
        perlinSettngs.initAmplitude = 1.0;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            double   px, py, value;
            int      x;

            for ( x = 0; x < width; x++ )
            {
                px = factor * ( x + randN );
                py = factor * ( y + randN );

                value = ( (
                          sin( factor * ( x + stitchOffset ) + PerlinNoise2D( &perlinSettngs, px, py ) ) +
                          sin( factor * ( y + stitchOffset ) + PerlinNoise2D( &perlinSettngs, px, py ) )
                        ) * 0.25 + 0.5 ) * 255;

                if ( value < 0.0 )
                {
                    value = 0.0;
                }
                if ( value > 255.0 )
                {
                    value = 255.0;
                }

                *row = (uint8_t) value;
                row++;
            }
        }
    }

    return ret;
}

// Generate marble looking texture
XErrorCode GenerateMarbleTexture( ximage* texture, uint16_t randNumber, float xPeriod, float yPeriod )
{
    XErrorCode ret = SuccessCode;

    if ( texture == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( texture->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width  = texture->width;
        int      height = texture->height;
        int      stride = texture->stride;
        int      randN  = (int) randNumber;
        double   xFact  = xPeriod / width;
        double   yFact  = yPeriod / height;
        int      y;
        uint8_t* ptr = texture->data;

        PerlinNoiseSettings perlinSettngs;

        perlinSettngs.octaves       = 2;
        perlinSettngs.persistence   = 0.65;
        perlinSettngs.initFrequency = 1.0 / 32;
        perlinSettngs.initAmplitude = 1.0;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride, xFact, yFact )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            double   value;
            int      x;

            for ( x = 0; x < width; x++ )
            {
                value = sin( ( x * xFact + y * yFact + PerlinNoise2D( &perlinSettngs, x + randN, y + randN ) ) * XPI ) * 255;

                if ( value < 0 )
                {
                    value = -value;
                }

                *row = (uint8_t) value;
                row++;
            }
        }
    }

    return ret;
}

// Generate clouds looking texture
XErrorCode GenerateCloudsTexture( ximage* texture, uint16_t randNumber )
{
    XErrorCode ret = SuccessCode;

    if ( texture == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( texture->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width  = texture->width;
        int      height = texture->height;
        int      stride = texture->stride;
        int      randN  = (int) randNumber;
        int      y;
        uint8_t* ptr = texture->data;

        PerlinNoiseSettings perlinSettngs;

        perlinSettngs.octaves       = 8;
        perlinSettngs.persistence   = 0.5;
        perlinSettngs.initFrequency = 1.0 / 32;
        perlinSettngs.initAmplitude = 1.0;

        #pragma omp parallel for schedule(static) shared( ptr, width, stride )
        for ( y = 0; y < height; y++ )
        {
            uint8_t* row = ptr + y * stride;
            double   value;
            int      x;

            for ( x = 0; x < width; x++ )
            {
                value = ( PerlinNoise2D( &perlinSettngs, x + randN, y + randN ) * 0.5 + 0.5 ) * 255;
                value = XINRANGE(value, 0, 255);
                *row  = (uint8_t) value;
                row++;
            }
        }
    }

    return ret;
}

// Generate fuzzy border texture
XErrorCode GenerateFuzzyBorderTexture( ximage* texture, uint16_t randNumber, uint16_t borderWidth, uint16_t gradientWidth, uint16_t waviness )
{
    XErrorCode ret = SuccessCode;

    if ( texture == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( texture->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width    = texture->width;
        int      widthM1  = width - 1;
        int      height   = texture->height;
        int      heightM1 = height - 1;
        int      stride   = texture->stride;
        int      randN    = (int) randNumber;
        int      x, y;
        int      waveSize;
        int      gradientStart;
        int      gradientEnd;
        int      borderSize   = (int) borderWidth;
        int      gradientSize = (int) gradientWidth;
        uint8_t* ptr = texture->data;
        uint8_t* row;
        uint8_t  newValue;

        PerlinNoiseSettings perlinSettngs;

        perlinSettngs.octaves       = 8;
        perlinSettngs.persistence   = 0.25;
        perlinSettngs.initFrequency = 1.0 / 16;
        perlinSettngs.initAmplitude = 1.0;

        // make sure border size is not bigger than width or height
        borderSize = XMIN3( borderSize, width / 2, height / 2 );

        if ( gradientSize > borderSize )
        {
            gradientSize = borderSize;
        }

        // make sure the picture is clean
        for ( y = 0; y < height; y++ )
        {
            row = ptr + y * stride;
            memset( row, 0, stride );
        }

        // horizontal borders
        for ( x = 0; x < width; x++ )
        {
            // top
            waveSize = (int) ( ( PerlinNoise1D( &perlinSettngs, x + randN ) * 0.5 + 0.5 ) * waviness );
            if ( waveSize < 0 )
            {
                waveSize = 0;
            }

            gradientEnd   = borderSize - waveSize;
            gradientStart = gradientEnd - gradientSize;

            for ( y = 0; y < gradientEnd; y++ )
            {
                row = ptr + y * stride;

                if ( y < gradientStart )
                {
                    row[x] = 255;
                }
                else
                {
                    row[x] = (uint8_t) ( 255 - ( y - gradientStart ) * 255  / gradientSize );
                }
            }

            // bottom
            waveSize = (int) ( ( PerlinNoise1D( &perlinSettngs, x + randN + width ) * 0.5 + 0.5 ) * waviness );
            if ( waveSize < 0 )
            {
                waveSize = 0;
            }

            gradientEnd   = heightM1 - borderSize + waveSize;
            gradientStart = gradientEnd + gradientSize;

            for ( y = heightM1; y > gradientEnd; y-- )
            {
                row = ptr + y * stride;

                if ( y > gradientStart )
                {
                    row[x] = 255;
                }
                else
                {
                    row[x] = (uint8_t) ( 255 - ( gradientStart - y ) * 255  / gradientSize );
                }
            }
        }

        // vertical border
        for ( y = 0; y < height ; y++ )
        {
            row = ptr + y * stride;

            // left
            waveSize = (int) ( ( PerlinNoise1D( &perlinSettngs, y + randN ) * 0.5 + 0.5 ) * waviness );
            if ( waveSize < 0 )
            {
                waveSize = 0;
            }

            gradientEnd   = borderSize - waveSize;
            gradientStart = gradientEnd - gradientSize;

            for ( x = 0; x < gradientEnd; x++ )
            {
                if ( x < gradientStart )
                {
                    newValue = 255;
                }
                else
                {
                    newValue = (uint8_t) ( 255 - ( x - gradientStart ) * 255  / gradientSize );
                }

                if ( row[x] < newValue )
                {
                    row[x] = newValue;
                }
            }

            // right
            waveSize = (int) ( ( PerlinNoise1D( &perlinSettngs, y + randN + height ) * 0.5 + 0.5 ) * waviness );
            if ( waveSize < 0 )
            {
                waveSize = 0;
            }

            gradientEnd   = widthM1 - borderSize + waveSize;
            gradientStart = gradientEnd + gradientSize;

            for ( x = widthM1; x > gradientEnd; x-- )
            {
                if ( x > gradientStart )
                {
                    newValue = 255;
                }
                else
                {
                    newValue = (uint8_t) ( 255 - ( gradientStart - x ) * 255  / gradientSize );
                }

                if ( row[x] < newValue )
                {
                    row[x] = newValue;
                }
            }
        }

        // blur the texture
        {
            ximage* bluredImage = 0;

            if ( XImageAllocate( width, height, XPixelFormatGrayscale8, &bluredImage ) == SuccessCode )
            {
                BlurImage( texture, bluredImage );
                BlurImage( bluredImage, texture );

                XImageFree( &bluredImage );
            }
        }
    }

    return ret;
}

// Generate grain texture
XErrorCode GenerateGrainTexture( ximage* texture, uint16_t randNumber, uint16_t spacing, float density, bool isVertical )
{
    XErrorCode ret = SuccessCode;

    if ( texture == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( texture->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width    = texture->width;
        int      height   = texture->height;
        int      stride   = texture->stride;
        int      randN    = (int) randNumber;
        int      x, y, value;
        uint8_t* ptr = texture->data;
        uint8_t* row;
        double   dec = 0.25 * ( 1.0 - XINRANGE( density, 0.0, 1.0 ) );

        PerlinNoiseSettings perlinSettngs;

        perlinSettngs.octaves       = 8;
        perlinSettngs.persistence   = 0.75;
        perlinSettngs.initFrequency = 1.0 / 2;
        perlinSettngs.initAmplitude = 1.0;

        // initialize random number generator
        srand( randNumber );

        // min spacing is 1
        spacing = XMAX( spacing, 1 );

        // make sure the picture is clean
        for ( y = 0; y < height; y++ )
        {
            row = ptr + y * stride;
            memset( row, 0, stride );
        }

        if ( isVertical )
        {
            // create vertical grain
            x = rand( ) % spacing + 1;

            for ( ; x < width; )
            {
                row = ptr + x;

                for ( y = 0; y < height; y++ )
                {
                    value = (int) ( ( PerlinNoise2D( &perlinSettngs, x + randN, y + randN ) * 0.25 - dec ) * 255 );
                    *row = (uint8_t) ( ( value > 255 ) ? 255 : ( ( value < 0 ) ? 0 : value ) );
                    row += stride;
                }

                x += rand( ) % spacing + 1;
            }
        }
        else
        {
            // create horizontal grain
            y = rand( ) % spacing + 1;

            for ( ; y < height; )
            {
                row = ptr + stride * y;

                for ( x = 0; x < width; x++ )
                {
                    value = (int) ( ( PerlinNoise2D( &perlinSettngs, x + randN, y + randN ) * 0.25 - dec ) * 255 );
                    *row = (uint8_t) ( ( value > 255 ) ? 255 : ( ( value < 0 ) ? 0 : value ) );
                    row++;
                }

                y += rand( ) % spacing + 1;
            }
        }
    }

    return ret;
}
