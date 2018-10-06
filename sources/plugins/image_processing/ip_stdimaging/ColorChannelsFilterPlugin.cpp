/*
    Standard image processing plug-ins of Computer Vision Sandbox

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

#include <ximaging.h>
#include "ColorChannelsFilterPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat ColorChannelsFilterPlugin::supportedFormats[] =
{
    XPixelFormatRGB24, XPixelFormatRGBA32
};

ColorChannelsFilterPlugin::ColorChannelsFilterPlugin( ) :
    redMin  ( 0 ), redMax  ( 255 ), redFill  ( 0 ), redFillOutside  ( true ),
    greenMin( 0 ), greenMax( 255 ), greenFill( 0 ), greenFillOutside( true ),
    blueMin ( 0 ), blueMax ( 255 ), blueFill ( 0 ), blueFillOutside ( true )
{
    CalculateFilteringMap( redMap,   redMin,   redMax,   redFill,   redFillOutside );
    CalculateFilteringMap( greenMap, greenMin, greenMax, greenFill, greenFillOutside );
    CalculateFilteringMap( blueMap,  blueMin,  blueMax,  blueFill,  blueFillOutside );
}

void ColorChannelsFilterPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool ColorChannelsFilterPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode ColorChannelsFilterPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode ColorChannelsFilterPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = XImageClone( src, dst );

    if ( ret == SuccessCode )
    {
        ret = ProcessImageInPlace( *dst );

        if ( ret != SuccessCode )
        {
            XImageFree( dst );
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode ColorChannelsFilterPlugin::ProcessImageInPlace( ximage* src )
{
    return ColorRemapping( src, redMap, greenMap, blueMap );
}

// Get specified property value of the plug-in
XErrorCode ColorChannelsFilterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    value->type = XVT_Range;

    switch ( id )
    {
        case 0:
            value->value.rangeVal.min = redMin;
            value->value.rangeVal.max = redMax;
            break;

        case 1:
            value->type = XVT_U1;
            value->value.ubVal = redFill;
            break;

        case 2:
            value->type = XVT_Bool;
            value->value.boolVal = redFillOutside;
            break;

        case 3:
            value->value.rangeVal.min = greenMin;
            value->value.rangeVal.max = greenMax;
            break;

        case 4:
            value->type = XVT_U1;
            value->value.ubVal = greenFill;
            break;

        case 5:
            value->type = XVT_Bool;
            value->value.boolVal = greenFillOutside;
            break;

        case 6:
            value->value.rangeVal.min = blueMin;
            value->value.rangeVal.max = blueMax;
            break;

        case 7:
            value->type = XVT_U1;
            value->value.ubVal = blueFill;
            break;

        case 8:
            value->type = XVT_Bool;
            value->value.boolVal = blueFillOutside;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ColorChannelsFilterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    static const xrange validRange = { 0, 255 };
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 9, &convertedValue );

if ( ret == SuccessCode )
    {
        switch ( id )
        {
            case 0:
                XRangeIntersect( &convertedValue.value.rangeVal, &validRange );
                redMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
                redMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
                break;

            case 1:
                redFill = convertedValue.value.ubVal;
                break;

            case 2:
                redFillOutside = convertedValue.value.boolVal;
                break;

            case 3:
                XRangeIntersect( &convertedValue.value.rangeVal, &validRange );
                greenMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
                greenMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
                break;

            case 4:
                greenFill = convertedValue.value.ubVal;
                break;

            case 5:
                greenFillOutside = convertedValue.value.boolVal;
                break;

            case 6:
                XRangeIntersect( &convertedValue.value.rangeVal, &validRange );
                blueMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
                blueMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
                break;

            case 7:
                blueFill = convertedValue.value.ubVal;
                break;

            case 8:
                blueFillOutside = convertedValue.value.boolVal;
                break;
        }

        if ( id < 3 )
        {
            CalculateFilteringMap( redMap, redMin, redMax, redFill, redFillOutside );
        }
        else if ( id < 6 )
        {
            CalculateFilteringMap( greenMap, greenMin, greenMax, greenFill, greenFillOutside );
        }
        else
        {
            CalculateFilteringMap( blueMap, blueMin, blueMax, blueFill, blueFillOutside );
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
