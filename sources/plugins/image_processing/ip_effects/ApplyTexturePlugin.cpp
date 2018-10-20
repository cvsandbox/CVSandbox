/*
    Image processing effects plug-ins of Computer Vision Sandbox

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
#include <xtextures.h>
#include "ApplyTexturePlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat ApplyTexturePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

ApplyTexturePlugin::ApplyTexturePlugin( ) :
    amountToKeep( 0.5f ), textureBaseLevel( 255 )
{
}

void ApplyTexturePlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool ApplyTexturePlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode ApplyTexturePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

xstring ApplyTexturePlugin::GetSecondImageDescription( )
{
    return XStringAlloc( "Grayscale (8bpp) texture image" );;
}

XSupportedImageSize ApplyTexturePlugin::GetSecondImageSupportedSize( )
{
    return XImageSizeEqualOrBigger;
}

XPixelFormat ApplyTexturePlugin::GetSecondImageSupportedFormat( XPixelFormat inputPixelFormat )
{
    return ( IsPixelFormatSupportedImpl( supportedFormats, sizeof( supportedFormats ) / sizeof( XPixelFormat ), inputPixelFormat ) ) ?
        XPixelFormatGrayscale8 : XPixelFormatUnknown;
}

// Process the specified source image and return new as a result
XErrorCode ApplyTexturePlugin::ProcessImage( const ximage* src, const ximage* src2, ximage** dst )
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
XErrorCode ApplyTexturePlugin::ProcessImageInPlace( ximage* src, const ximage* src2 )
{
    return XImageApplyTexture( src2, src, amountToKeep, textureBaseLevel );
}

// Get specified property value of the plug-in
XErrorCode ApplyTexturePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = amountToKeep;
        break;

    case 1:
        value->type = XVT_U1;
        value->value.ubVal = textureBaseLevel;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ApplyTexturePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 2, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            amountToKeep = convertedValue.value.fVal;
            break;

        case 1:
            textureBaseLevel = convertedValue.value.ubVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
