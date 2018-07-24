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
#include <math.h>

// Calculate linear map for changing pixels' values
XErrorCode CalculateLinearMap( uint8_t* map, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax )
{
    XErrorCode ret = SuccessCode;

    if ( map == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        double k = 0, b = 0;
        int    i = 0;

        if ( inMax != inMin )
        {
            k = (double) ( outMax - outMin ) / (double) ( inMax - inMin );
            b = (double) outMin - k * inMin;
        }

        for ( ; i < 256; i++ )
        {
            uint8_t v = (uint8_t) i;

            if ( v >= inMax )
                v = outMax;
            else if ( v <= inMin )
                v = outMin;
            else
                v = (uint8_t) ( k * v + b );

            map[i] = v;
        }
    }

    return ret;
}

// Calculate linear map for changing pixels' values only within the specfied input range
XErrorCode CalculateLinearMapInInputRange( uint8_t* map, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax )
{
    XErrorCode ret = SuccessCode;

    if ( map == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        double k = 0, b = 0;
        int    i = inMin, end = inMax;

        if ( inMax != inMin )
        {
            k = (double) ( outMax - outMin ) / (double) ( inMax - inMin );
            b = (double) outMin - k * inMin;
        }
        else
        {
            k = 0;
            b = outMax;
        }

        for ( ; i <= end; i++ )
        {
            map[i] = (uint8_t) ( k * i + b );
        }
    }

    return ret;
}

// Calculate linear map which increases/decreases brightness
XErrorCode CalculateBrightnessChangeMap( uint8_t* map, int16_t increaseBrightnessBy )
{
    int16_t incBrightnessBy = XINRANGE( increaseBrightnessBy, -255, 255 );
    uint8_t inMin  = 0, inMax  = 255;
    uint8_t outMin = 0, outMax = 255;

    if ( incBrightnessBy > 0 )
    {
        outMin = (uint8_t) incBrightnessBy;
    }
    else
    {
        outMax = (uint8_t) ( (uint16_t) 255 + incBrightnessBy );
    }

    return CalculateLinearMap( map, inMin, inMax, outMin, outMax );
}

// Calculate linear map which increases/decreases contrast
XErrorCode CalculateContrastChangeMap( uint8_t* map, int16_t increaseContrastBy )
{
    int16_t incContrastBy = XINRANGE( increaseContrastBy, -127, 127 );
    uint8_t inMin  = 0, inMax  = 255;
    uint8_t outMin = 0, outMax = 255;

    if ( incContrastBy > 0 )
    {
        inMin = (uint8_t) increaseContrastBy;
        inMax = (uint8_t) ( (int16_t) 255 - increaseContrastBy );
    }
    else
    {
        outMin = (uint8_t) ( -increaseContrastBy );
        outMax = (uint8_t) ( (int16_t) 255 + increaseContrastBy );
    }

    return CalculateLinearMap( map, inMin, inMax, outMin, outMax );
}

// Calculate linear map which increases/decreases high/low levels
XErrorCode CalculateColorLevelChangeMap( uint8_t* map, int16_t increaseLowLevelBy, int16_t increaseHighLevelBy )
{
    int16_t incLowLevelBy  = XINRANGE( increaseLowLevelBy,  -255, 255 );
    int16_t incHighLevelBy = XINRANGE( increaseHighLevelBy, -255, 255 );

    uint8_t inMin  = 0, inMax  = 255;
    uint8_t outMin = 0, outMax = 255;

    if ( incHighLevelBy > 0 )
    {
        inMax = (uint8_t) ( (uint16_t) 255 - incHighLevelBy );
    }
    else
    {
        outMax = (uint8_t) ( (uint16_t) 255 + incHighLevelBy );
    }

    if ( incLowLevelBy > 0 )
    {
        outMin = (uint8_t) incLowLevelBy;
    }
    else
    {
        inMin = (uint8_t) (-incLowLevelBy);
    }

    return CalculateLinearMap( map, inMin, inMax, outMin, outMax );
}

// Levels linear filter for 8 bpp grayscale images
XErrorCode LevelsLinearGrayscale( ximage* src, uint8_t inMin, uint8_t inMax, uint8_t outMin, uint8_t outMax )
{
    uint8_t map[256];

    CalculateLinearMap( map, inMin, inMax, outMin, outMax );

    return GrayscaleRemapping( src, map );
}

// Levels linear filter for 24/32 bpp color images
XErrorCode LevelsLinear( ximage* src,
    uint8_t inRedMin,   uint8_t inRedMax,   uint8_t outRedMin,   uint8_t outRedMax,
    uint8_t inGreenMin, uint8_t inGreenMax, uint8_t outGreenMin, uint8_t outGreenMax,
    uint8_t inBlueMin,  uint8_t inBlueMax,  uint8_t outBlueMin,  uint8_t outBlueMax )
{
    uint8_t redMap[256];
    uint8_t greenMap[256];
    uint8_t blueMap[256];

    CalculateLinearMap( redMap,   inRedMin,   inRedMax,   outRedMin,   outRedMax );
    CalculateLinearMap( greenMap, inGreenMin, inGreenMax, outGreenMin, outGreenMax );
    CalculateLinearMap( blueMap,  inBlueMin,  inBlueMax,  outBlueMin,  outBlueMax );

    return ColorRemapping( src, redMap, greenMap, blueMap );
}

// Calculate gamma correction map
XErrorCode CalculateGammaCorrectionMap( uint8_t* map, float gamma, bool inverse )
{
    XErrorCode ret = SuccessCode;

    gamma = XINRANGE( gamma, 1.0f, 10.0f );

    if ( map == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        double correctedFactor = ( inverse == true ) ? 1.0 / gamma : gamma;
        int    i;

        for ( i = 0; i < 256; i++ )
        {
            map[i] = (uint8_t) ( pow( (double) i / 255.0, correctedFactor ) * 255 + 0.5 );
        }
    }

    return ret;
}

// Calculate "S-curve" map (contrast correction)
XErrorCode CalculateSCurveMap( uint8_t* map, float factor, bool inverse )
{
    XErrorCode ret = SuccessCode;

    factor = XINRANGE( factor, 1.0f, 10.0f );

    if ( map == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        double correctedFactor = ( inverse == true ) ? 1.0 / factor : factor;
        double value;
        int    i;

        for ( i = 0; i < 256; i++ )
        {
            value = (double) i / 255.0;

            if ( value < 0.5 )
            {
                map[i] = (uint8_t) ( ( pow( value * 2.0, correctedFactor ) / 2 ) * 255 + 0.5 );
            }
            else
            {
                map[i] = (uint8_t) ( ( 1.0 - pow( 2.0 - value * 2.0, correctedFactor ) / 2 ) * 255 + 0.5 );
            }
        }
    }

    return ret;
}

// Calculate filtering map - fill values outside/inside the range with the specified value
XErrorCode CalculateFilteringMap( uint8_t* map, uint8_t min, uint8_t max, uint8_t fillValue, bool fillOutsideRange )
{
    XErrorCode ret = SuccessCode;

    if ( map == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int i;

        if ( fillOutsideRange == true )
        {
            for ( i = 0; i < 256; i++ )
            {
                map[i] = ( ( i < min ) || ( i > max ) ) ? fillValue : (uint8_t) i;
            }
        }
        else
        {
            for ( i = 0; i < 256; i++ )
            {
                map[i] = ( ( i >= min ) && ( i <= max ) ) ? fillValue : (uint8_t) i;
            }
        }
    }

    return ret;
}
