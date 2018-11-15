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

#include "FireEffectVirtualVideoSourcePlugin.hpp"
#include <memory.h>
#include <time.h>
#include <ximaging.h>

using namespace std;

#define FLAME_WIDTH     (320)
#define FLAME_HEIGHT    (240)
#define FLAME_HOT_SPOTS (5)

// ==========================================================================

FireEffectVirtualVideoSourcePlugin::FireEffectVirtualVideoSourcePlugin( ) :
    mFrameWidth( 320 ), mFrameHeight( 240 ), mFrameRate( 15 )
{
}

// Get specified property value of the plug-in
XErrorCode FireEffectVirtualVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
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
XErrorCode FireEffectVirtualVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
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
void FireEffectVirtualVideoSourcePlugin::VideoSourceWorker( )
{
    ximage* image1 = 0;
    ximage* image2 = 0;
    ximage* imageResized = 0;
    ximage* imageToProvide = 0;

    if ( ( SuccessCode == XImageAllocate( FLAME_WIDTH, FLAME_HEIGHT, XPixelFormatGrayscale8, &image1 ) ) &&
         ( SuccessCode == XImageAllocate( FLAME_WIDTH, FLAME_HEIGHT, XPixelFormatGrayscale8, &image2 ) ) &&
         (
            ( ( FLAME_WIDTH == mFrameWidth ) && ( FLAME_HEIGHT == mFrameHeight ) ) ||
            ( SuccessCode == XImageAllocate( mFrameWidth, mFrameHeight, XPixelFormatGrayscale8, &imageResized ) )
         ) &&
         ( SuccessCode == XImageAllocateRaw( mFrameWidth, mFrameHeight, XPixelFormatRGB24, &imageToProvide ) ) )
    {
        uint8_t  redMap  [256] = { 0 };
        uint8_t  greenMap[256] = { 0 };
        uint8_t  blueMap [256] = { 0 };
        uint8_t* ptr;
        uint8_t* ptrPrev;
        uint32_t timeBetweenFramesMs = 1000 / mFrameRate;
        int32_t  imageStride = image1->stride;

        for ( uint8_t i = 0; i < 32; i++ )
        {
            /* black to red, 64 values */
            redMap[i]      = i << 2;
            redMap[i + 32] = 128 + (i << 2);

            /* red to yellow, 32 values */
            redMap  [i + 64] = 255;
            greenMap[i + 64] = i << 3;

            /* yellow to white, 162 */
            redMap  [i + 96] = 255;
            greenMap[i + 96] = 255;
            blueMap [i + 96] = i << 2;

            redMap  [i + 128] = 255;
            greenMap[i + 128] = 255;
            blueMap [i + 128] = 64 + ( i << 2 );

            redMap  [i + 160] = 255;
            greenMap[i + 160] = 255;
            blueMap [i + 160] = 128 + ( i << 2 );

            redMap  [i + 192] = 255;
            greenMap[i + 192] = 255;
            blueMap [i + 192] = 192 + i;

            redMap  [i + 224] = 255;
            greenMap[i + 224] = 255;
            blueMap [i + 224] = 224 + i;
        }

        srand( (uint32_t) time( 0 ) );

        // TODO: need to take into account time spent to produce a frame
        while ( !WaitTillNeedToStop( timeBetweenFramesMs ) )
        {
            // shift the flame up and fade it
            for ( int y = 2; y < FLAME_HEIGHT; y++ )
            {
                ptr     = image1->data + y * imageStride;
                ptrPrev = ptr - imageStride;

                for ( int x = 0; x < FLAME_WIDTH; x++ )
                {
                    *ptrPrev = ( *ptr > 0 ) ? *ptr - 1 : 0;

                    ptr++;
                    ptrPrev++;
                }
            }

            // fade the last row
            ptr = image1->data + ( FLAME_HEIGHT - 1 ) * imageStride;
            for ( int x = 0; x < FLAME_WIDTH; x++ )
            {
                if ( *ptr > 0 )
                {
                    (*ptr)--;
                }
                ptr++;
            }

            // put hot spots
            for ( int i = 0; i < FLAME_HOT_SPOTS; i++ )
            {
                ptr = image1->data + ( FLAME_HEIGHT - 1 ) * image1->stride + ( rand( ) % FLAME_WIDTH );
                *ptr = 255;
            }

            // blur pixels
            Mean3x3( image1, image2 );
            // turn grayscale image to color
            if  ( ( FLAME_WIDTH == mFrameWidth ) && ( FLAME_HEIGHT == mFrameHeight ) )
            {
                GrayscaleRemappingToRGB( image2, imageToProvide, redMap, greenMap, blueMap );
            }
            else
            {
                ResizeImageBilinear( image2, imageResized );
                GrayscaleRemappingToRGB( imageResized, imageToProvide, redMap, greenMap, blueMap );
            }

            NewFrameNotify( imageToProvide );

            // swap
            ximage* temp = image1;
            image1 = image2;
            image2 = temp;
        }
    }

    XImageFree( &image1 );
    XImageFree( &image2 );
    XImageFree( &imageResized );
    XImageFree( &imageToProvide );
}
