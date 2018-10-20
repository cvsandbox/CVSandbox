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
#include "TextileTexturePlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat TextileTexturePlugin::supportedFormats[] =
{
	XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

TextileTexturePlugin::TextileTexturePlugin( ) :
    stitchSize( 7 ), stitchOffset( 0 ), amountToKeep( 0.5f ), randValue( (uint16_t) ( rand( ) % 10000 ) )
{
}

void TextileTexturePlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool TextileTexturePlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode TextileTexturePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
	return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
		sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode TextileTexturePlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode TextileTexturePlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = ErrorFailed;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ximage* texture = 0;
        
        ret = XImageAllocateTexture( src->width, src->height, &texture );

        if ( ret == SuccessCode )
        {
            if ( GenerateTextileTexture( texture, randValue, stitchSize, stitchOffset ) == SuccessCode )
            {
                ret = XImageApplyTexture( texture, src, amountToKeep, 255 );
            }

            XImageFree( &texture );
        }
    }

    return ret;
}

// Get specified property value of the plug-in
XErrorCode TextileTexturePlugin::GetProperty( int32_t id, xvariant* value ) const
{
	XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = stitchSize;
        break;

    case 1:
        value->type = XVT_U1;
        value->value.ubVal = stitchOffset;
        break;

    case 2:
        value->type = XVT_R4;
        value->value.fVal = amountToKeep;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode TextileTexturePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

	// make sure property value has expected type
	ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 3, &convertedValue );

	if ( ret == SuccessCode )
	{
		switch ( id )
		{
		case 0:
			stitchSize = convertedValue.value.ubVal;
			break;

		case 1:
			stitchOffset = convertedValue.value.ubVal;
			break;

		case 2:
            amountToKeep = convertedValue.value.fVal;
			break;
		}
	}

    XVariantClear( &convertedValue );

    return ret;
}
