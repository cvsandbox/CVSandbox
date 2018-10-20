/*
    Image processing tools plug-ins of Computer Vision Sandbox

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
#include "BlendImagesPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat BlendImagesPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

BlendImagesPlugin::BlendImagesPlugin( ) :
    blendMode( BlendMode_Multiply )
{
}

void BlendImagesPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool BlendImagesPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode BlendImagesPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

xstring BlendImagesPlugin::GetSecondImageDescription( )
{
    return XStringAlloc( "An image to blend with (base layer)" );
}

XSupportedImageSize BlendImagesPlugin::GetSecondImageSupportedSize( )
{
    return XImageSizeEqual;
}

XPixelFormat BlendImagesPlugin::GetSecondImageSupportedFormat( XPixelFormat inputPixelFormat )
{
    return ( IsPixelFormatSupportedImpl( supportedFormats, sizeof( supportedFormats ) / sizeof( XPixelFormat ), inputPixelFormat ) ) ?
        inputPixelFormat : XPixelFormatUnknown;
}

// Process the specified source image and return new as a result
XErrorCode BlendImagesPlugin::ProcessImage( const ximage* src, const ximage* src2, ximage** dst )
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
XErrorCode BlendImagesPlugin::ProcessImageInPlace( ximage* src, const ximage* src2 )
{
    return BlendImages( src, src2, blendMode );
}

// Get specified property value of the plug-in
XErrorCode BlendImagesPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = blendMode;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode BlendImagesPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 1, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            blendMode = convertedValue.value.ubVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
