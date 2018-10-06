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
#include "LevelsLinearGrayscalePlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat LevelsLinearGrayscalePlugin::supportedFormats[] =
{
	XPixelFormatGrayscale8
};

LevelsLinearGrayscalePlugin::LevelsLinearGrayscalePlugin( ) :
	inGrayMin( 0 ),  inGrayMax( 255 ),  outGrayMin( 0 ),  outGrayMax( 255 )
{
	CalculateLinearMap( grayMap,  inGrayMin,  inGrayMax,  outGrayMin,  outGrayMax );
}

void LevelsLinearGrayscalePlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool LevelsLinearGrayscalePlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode LevelsLinearGrayscalePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
	return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
		sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode LevelsLinearGrayscalePlugin::ProcessImage( const ximage* src, ximage** dst )
{
	XErrorCode ret = XImageClone( src, dst );

	if ( ret == SuccessCode )
	{
		ret = GrayscaleRemapping( *dst, grayMap );

		if ( ret != SuccessCode )
		{
			XImageFree( dst );
		}
	}

    return ret;
}

// Process the specified source image by changing it
XErrorCode LevelsLinearGrayscalePlugin::ProcessImageInPlace( ximage* src )
{
	XErrorCode ret = SuccessCode;

	if ( src == 0 )
	{
		ret = ErrorNullParameter;
	}
	else
	{
		ret = GrayscaleRemapping( src, grayMap );
	}

    return ret;
}

// Get specified property value of the plug-in
XErrorCode LevelsLinearGrayscalePlugin::GetProperty( int32_t id, xvariant* value ) const
{
	XErrorCode ret = SuccessCode;

	value->type = XVT_Range;

    switch ( id )
    {
		case 0:
			value->value.rangeVal.min = inGrayMin;
			value->value.rangeVal.max = inGrayMax;
			break;
		case 1:
			value->value.rangeVal.min = outGrayMin;
			value->value.rangeVal.max = outGrayMax;
			break;

	    default:
	        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode LevelsLinearGrayscalePlugin::SetProperty( int32_t id, const xvariant* value )
{
	static const xrange validRange = { 0, 255 };
    XErrorCode ret = 0;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

	// make sure property value has expected type
	ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 2, &convertedValue );

	if ( ret == SuccessCode )
	{
		XRangeIntersect( &convertedValue.value.rangeVal, &validRange );

		switch ( id )
		{
			case 0:
			case 1:
				if ( id == 0 )
				{
					inGrayMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					inGrayMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				else
				{
					outGrayMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					outGrayMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				CalculateLinearMap( grayMap, inGrayMin, inGrayMax, outGrayMin, outGrayMax );
				break;
		}
    }

    XVariantClear( &convertedValue );

    return ret;
}
