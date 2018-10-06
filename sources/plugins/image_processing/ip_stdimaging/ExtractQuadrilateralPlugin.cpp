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
#include "ExtractQuadrilateralPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat ExtractQuadrilateralPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

ExtractQuadrilateralPlugin::ExtractQuadrilateralPlugin( ) :
    quadWidth( 200 ), quadHeight( 200 ), useInterpolation( true )
{
    points[0].x = 0;
    points[0].y = 120;

    points[1].x = 160;
    points[1].y = 0;

    points[2].x = 319;
    points[2].y = 120;

    points[3].x = 160;
    points[3].y = 239;
}

void ExtractQuadrilateralPlugin::Dispose( )
{
    delete this;
}

// The plug-in cannot process image in-place since it changes its size
bool ExtractQuadrilateralPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode ExtractQuadrilateralPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode ExtractQuadrilateralPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = XImageAllocateRaw( quadWidth, quadHeight, src->format, dst );

    if ( ret == SuccessCode )
    {
        ret = ExtractQuadrilateral( src, *dst, points, useInterpolation );

        if ( ret != SuccessCode )
        {
            XImageFree( dst );
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode ExtractQuadrilateralPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode ExtractQuadrilateralPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
    case 1:
    case 2:
    case 3:
        value->type           = XVT_Point;
        value->value.pointVal = points[id];
        break;

    case 4:
        value->type       = XVT_I4;
        value->value.iVal = quadWidth;
        break;

    case 5:
        value->type       = XVT_I4;
        value->value.iVal = quadHeight;
        break;

    case 6:
        value->type          = XVT_Bool;
        value->value.boolVal = useInterpolation;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ExtractQuadrilateralPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 7, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
        case 1:
        case 2:
        case 3:
            points[id] = convertedValue.value.pointVal;
            break;

        case 4:
            quadWidth = convertedValue.value.iVal;
            break;

        case 5:
            quadHeight = convertedValue.value.iVal;
            break;

        case 6:
            useInterpolation = convertedValue.value.boolVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
