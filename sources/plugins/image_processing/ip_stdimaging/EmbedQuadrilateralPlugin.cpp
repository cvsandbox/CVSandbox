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

#include <stdlib.h>
#include <ximaging.h>
#include "EmbedQuadrilateralPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat EmbedQuadrilateralPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

EmbedQuadrilateralPlugin::EmbedQuadrilateralPlugin( ) :
    useInterpolation( true )
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

void EmbedQuadrilateralPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool EmbedQuadrilateralPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode EmbedQuadrilateralPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats, XARRAY_SIZE( supportedFormats ) );
}

xstring EmbedQuadrilateralPlugin::GetSecondImageDescription( )
{
    return XStringAlloc( "Quadrilateral image to embed" );
}

XSupportedImageSize EmbedQuadrilateralPlugin::GetSecondImageSupportedSize( )
{
    return XImageSizeAny;
}

XPixelFormat EmbedQuadrilateralPlugin::GetSecondImageSupportedFormat( XPixelFormat inputPixelFormat )
{
    return ( IsPixelFormatSupportedImpl( supportedFormats, XARRAY_SIZE( supportedFormats ), inputPixelFormat ) ) ?
                inputPixelFormat : XPixelFormatUnknown;
}

// Process the specified source image and return new as a result
XErrorCode EmbedQuadrilateralPlugin::ProcessImage( const ximage* src, const ximage* src2, ximage** dst )
{
    XErrorCode ret = XImageClone( src, dst );

    if ( ret == SuccessCode )
    {
        ret = ProcessImageInPlace( *dst, src2 );

        if ( ret != SuccessCode )
        {
            XImageFree( dst );
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode EmbedQuadrilateralPlugin::ProcessImageInPlace( ximage* src, const ximage* src2 )
{
    return EmbedQuadrilateral( src, src2, points, useInterpolation );
}

// Get specified property value of the plug-in
XErrorCode EmbedQuadrilateralPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
    case 1:
    case 2:
    case 3:
        value->type = XVT_Point;
        value->value.pointVal = points[id];
        break;

    case 4:
        value->type = XVT_Bool;
        value->value.boolVal = useInterpolation;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode EmbedQuadrilateralPlugin::SetProperty( int32_t id, const xvariant* value )
{
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
        case 1:
        case 2:
        case 3:
            points[id] = convertedValue.value.pointVal;
            break;

        case 4:
            useInterpolation = convertedValue.value.boolVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
