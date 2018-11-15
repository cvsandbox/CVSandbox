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

#include "ColorSnakeVirtualVideoSourcePlugin.hpp"
#include <memory.h>
#include <time.h>
#include <ximaging.h>

using namespace std;

// ==========================================================================

ColorSnakeVirtualVideoSourcePlugin::ColorSnakeVirtualVideoSourcePlugin( ) :
    mFrameWidth( 320 ), mFrameHeight( 240 ), mFrameRate( 15 )
{
}

// Get specified property value of the plug-in
XErrorCode ColorSnakeVirtualVideoSourcePlugin::GetProperty( int32_t id, xvariant* value ) const
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
XErrorCode ColorSnakeVirtualVideoSourcePlugin::SetProperty( int32_t id, const xvariant* value )
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
void ColorSnakeVirtualVideoSourcePlugin::VideoSourceWorker( )
{
    ximage* imageToProvide = 0;
    uint8_t redMap  [256];
    uint8_t greenMap[256];
    uint8_t blueMap [256];

    srand( (uint32_t) time( 0 ) );

    if ( ( SuccessCode == XImageAllocate( mFrameWidth, mFrameHeight, XPixelFormatRGB24, &imageToProvide ) ) &&
         ( SuccessCode == CalculateHeatGradientColorMap( redMap, greenMap, blueMap ) ) )
    {
        int32_t  widthM2        = mFrameWidth  - 2;
        int32_t  heightM2       = mFrameHeight - 2;
        int32_t  stride         = imageToProvide->stride;
        uint8_t* basePtr        = imageToProvide->data;
        int32_t  colorIndex     = rand( ) % 255;
        int32_t  colorIndexStep = 1;

        int32_t  x  = rand( ) % widthM2;
        int32_t  y  = 0;
        int32_t  dx = 1;
        int32_t  dy = 1;

        uint8_t* ptr;
        uint32_t timeBetweenFramesMs = 1000 / mFrameRate;

        uint8_t  red, green, blue;

        // TODO: need to take into account time spent to produce a frame
        while ( !WaitTillNeedToStop( timeBetweenFramesMs ) )
        {
            red   = redMap  [colorIndex];
            green = greenMap[colorIndex];
            blue  = blueMap [colorIndex];

            ptr = basePtr + y * stride + x * 3;

            // pixel pointed by the ptr
            ptr[RedIndex  ] = red;
            ptr[GreenIndex] = green;
            ptr[BlueIndex ] = blue;
            // pixel at the right
            ptr[3 + RedIndex  ] = red;
            ptr[3 + GreenIndex] = green;
            ptr[3 + BlueIndex ] = blue;
            // pixel at the bottom
            ptr[stride + RedIndex  ] = red;
            ptr[stride + GreenIndex] = green;
            ptr[stride + BlueIndex ] = blue;
            // pixel at the bottom-right
            ptr[stride + 3 + RedIndex  ] = red;
            ptr[stride + 3 + GreenIndex] = green;
            ptr[stride + 3 + BlueIndex ] = blue;

            // move to the next position
            x += dx;
            y += dy;

            // reverse motion when edge was hit
            if ( x == widthM2 )
            {
                dx = -1;
            }
            if ( x == 0 )
            {
                dx = 1;
            }
            if ( y == heightM2 )
            {
                dy = -1;
            }
            if ( y == 0 )
            {
                dy = 1;
            }

            // iterate color
            colorIndex += colorIndexStep;

            if ( colorIndex == 255 )
            {
                colorIndexStep = -1;
            }
            if ( colorIndex == 0 )
            {
                colorIndexStep = 1;
            }

            NewFrameNotify( imageToProvide );
        }
    }

    XImageFree( &imageToProvide );
}
