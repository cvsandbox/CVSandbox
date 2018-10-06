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
#include "MaskImagePlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat MaskImagePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

MaskImagePlugin::MaskImagePlugin( ) :
    fillColor( { 0xFF000000 } ), maskMode( 0 )
{
}

void MaskImagePlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool MaskImagePlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode MaskImagePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats, XARRAY_SIZE( supportedFormats ) );
}

xstring MaskImagePlugin::GetSecondImageDescription( )
{
    return XStringAlloc( "A mask image" );
}

XSupportedImageSize MaskImagePlugin::GetSecondImageSupportedSize( )
{
    return XImageSizeEqual;
}

XPixelFormat MaskImagePlugin::GetSecondImageSupportedFormat( XPixelFormat inputPixelFormat )
{
    return ( IsPixelFormatSupportedImpl( supportedFormats, XARRAY_SIZE( supportedFormats ), inputPixelFormat ) ) ?
             XPixelFormatGrayscale8 : XPixelFormatUnknown;
}

// Process the specified source image and return new as a result
XErrorCode MaskImagePlugin::ProcessImage( const ximage* src, const ximage* src2, ximage** dst )
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
XErrorCode MaskImagePlugin::ProcessImageInPlace( ximage* src, const ximage* src2 )
{
    return MaskImage( src, src2, fillColor, ( maskMode == 0 ) );
}

// Get specified property value of the plug-in
XErrorCode MaskImagePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = maskMode;
        break;

    case 1:
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
XErrorCode MaskImagePlugin::SetProperty( int32_t id, const xvariant* value )
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
            maskMode = convertedValue.value.ubVal;
            break;

        case 1:
            fillColor = convertedValue.value.argbVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
