/*
    Virtual video source plug-ins of Computer Vision Sandbox

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

#include "PlasmaEffectVirtualVideoSourcePlugin.hpp"
#include <memory.h>
#include <math.h>
#include <ximaging.h>

using namespace std;

// ==========================================================================

PlasmaEffectVirtualVideoSourcePlugin::PlasmaEffectVirtualVideoSourcePlugin( ) :
    mFrameWidth( 320 ), mFrameHeight( 240 ), mFrameRate( 15 )
{
}

// Get specified property value of the plug-in
XErrorCode PlasmaEffectVirtualVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U2;
        value->value.usVal = mFrameWidth;
        break;

    case 1:
        value->type = XVT_U2;
        value->value.usVal = mFrameHeight;
        break;

    case 2:
        value->type = XVT_U2;
        value->value.usVal = mFrameRate;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode PlasmaEffectVirtualVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = 0;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 3, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
            case 0:
                mFrameWidth = convertedValue.value.usVal;
                break;

            case 1:
                mFrameHeight = convertedValue.value.usVal;
                break;

            case 2:
                mFrameRate = convertedValue.value.usVal;
                break;
        }
    }

    return ret;
}

// Run video loop in a background thread
void PlasmaEffectVirtualVideoSourcePlugin::VideoSourceWorker( )
{
    ximage* imageGray = 0;
    ximage* imageToProvide = 0;

    if ( ( SuccessCode == XImageAllocate( mFrameWidth, mFrameHeight, XPixelFormatGrayscale8, &imageGray ) ) &&
         ( SuccessCode == XImageAllocateRaw( mFrameWidth, mFrameHeight, XPixelFormatRGB24, &imageToProvide ) ) )
    {
        uint8_t  redMap  [256] = { 0 };
        uint8_t  greenMap[256] = { 0 };
        uint8_t  blueMap [256] = { 0 };
        int32_t  sin_value[512];
        uint8_t* ptr;
        uint32_t timeBetweenFramesMs = 1000 / mFrameRate;
        int32_t  imageStride = imageGray->stride;

        uint16_t pos1 = 0, pos3 = 0;
        uint16_t tpos1, tpos2, tpos3, tpos4;
        int32_t  t;

        /* create sine values lookup table */
        double    rad;

        for ( int i = 0; i < 512; i++ )
        {
            /* 360 / 512 * degree to rad, 360 degrees spread over 512 values to be able to use AND 512-1 instead of using modulo 360*/
            rad =  ( (double) i * ( 360.0 / 512.0 ) ) * ( XPI / 180.0 );
            /* using fixed point math with 1024 as base */
            sin_value[i] = (int32_t) ( sin( rad ) * 1024 );
        }

        /* initialize color map */
        for ( uint8_t i = 0; i < 64; i++ )
        {
            redMap[i]       = i << 2;
            redMap[i + 64]  = 255;
            redMap[i + 128] = 255 - ( ( i << 2 ) + 1 );

            greenMap[i]       = 255 - ( ( i << 2 ) + 1 );
            greenMap[i + 64]  = ( i << 2 ) + 1;
            greenMap[i + 128] = 255 - ( ( i << 2 ) + 1 );
            greenMap[i + 192] = ( i << 2 ) + 1;
        }

        // TODO: need to take into account time spent to produce a frame
        while ( !WaitTillNeedToStop( timeBetweenFramesMs ) )
        {
            tpos3 = pos3;
            tpos4 = 0;

            // fill new grayscale frame
            for ( int y = 0; y < mFrameHeight; y++ )
            {
                ptr = imageGray->data + y * imageStride;

                tpos1  = pos1 + 5;
                tpos2  = 3;
                tpos3 &= 511;
                tpos4 &= 511;

                for ( int x = 0; x < mFrameWidth; x++ )
                {
                    tpos1 &= 511;
                    tpos2 &= 511;

                    t    = sin_value[tpos1] + sin_value[tpos2] + sin_value[tpos3] + sin_value[tpos4];
                    *ptr = (uint8_t) ( 128 + ( t >> 4 ) );

                    ptr++;

                    tpos1 += 5;
                    tpos2 += 3;
                }

                tpos3 += 1;
                tpos4 += 3;
            }

            pos1 += 9;
            pos3 += 8;

            // turn grayscale image to color
            GrayscaleRemappingToRGB( imageGray, imageToProvide, redMap, greenMap, blueMap );

            NewFrameNotify( imageToProvide );
        }
    }

    XImageFree( &imageGray );
    XImageFree( &imageToProvide );
}
