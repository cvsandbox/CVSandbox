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
#include "ColorFilterPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat ColorFilterPlugin::supportedFormats[] =
{
    XPixelFormatRGB24, XPixelFormatRGBA32
};

ColorFilterPlugin::ColorFilterPlugin( ) :
    minRed( 0 ), maxRed( 255 ), minGreen( 0 ), maxGreen( 255 ), minBlue( 0 ), maxBlue( 255 ),
    fillOutside( true ), fillColor( )
{
    fillColor.argb = 0xFF000000;
}

void ColorFilterPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool ColorFilterPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode ColorFilterPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode ColorFilterPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = XImageClone( src, dst );

    if ( ret == SuccessCode )
    {
        ret = ColorFiltering( *dst, minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue, fillOutside, fillColor );

        if ( ret != SuccessCode )
        {
            XImageFree( dst );
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode ColorFilterPlugin::ProcessImageInPlace( ximage* src )
{
    return ColorFiltering( src, minRed, maxRed, minGreen, maxGreen, minBlue, maxBlue, fillOutside, fillColor );
}

// Get specified property value of the plug-in
XErrorCode ColorFilterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    value->type = XVT_Range;

    switch ( id )
    {
        case 0:
            value->value.rangeVal.min = minRed;
            value->value.rangeVal.max = maxRed;
            break;

        case 1:
            value->value.rangeVal.min = minGreen;
            value->value.rangeVal.max = maxGreen;
            break;

        case 2:
            value->value.rangeVal.min = minBlue;
            value->value.rangeVal.max = maxBlue;
            break;

        case 3:
            value->type = XVT_Bool;
            value->value.boolVal = fillOutside;
            break;

        case 4:
            value->type = XVT_ARGB;
            value->value.argbVal = fillColor;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ColorFilterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    static const xrange validRange = { 0, 255 };
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 5, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
            case 0:
                XRangeIntersect( &convertedValue.value.rangeVal, &validRange );
                minRed = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
                maxRed = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
                break;

            case 1:
                XRangeIntersect( &convertedValue.value.rangeVal, &validRange );
                minGreen = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
                maxGreen = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
                break;

            case 2:
                XRangeIntersect( &convertedValue.value.rangeVal, &validRange );
                minBlue = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
                maxBlue = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
                break;

            case 3:
                fillOutside = convertedValue.value.boolVal;
                break;

            case 4:
                fillColor = convertedValue.value.argbVal;
                break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
