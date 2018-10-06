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
#include "LevelsLinearPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat LevelsLinearPlugin::supportedFormats[] =
{
	XPixelFormatRGB24, XPixelFormatRGBA32
};

LevelsLinearPlugin::LevelsLinearPlugin( ) :
    inRedMin  ( 0 ), inRedMax  ( 255 ), outRedMin  ( 0 ), outRedMax  ( 255 ),
	inGreenMin( 0 ), inGreenMax( 255 ), outGreenMin( 0 ), outGreenMax( 255 ),
	inBlueMin ( 0 ), inBlueMax ( 255 ), outBlueMin ( 0 ), outBlueMax ( 255 )
{
	CalculateLinearMap( redMap,   inRedMin,   inRedMax,   outRedMin,   outRedMax );
	CalculateLinearMap( greenMap, inGreenMin, inGreenMax, outGreenMin, outGreenMax );
	CalculateLinearMap( blueMap,  inBlueMin,  inBlueMax,  outBlueMin,  outBlueMax );
}

void LevelsLinearPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool LevelsLinearPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode LevelsLinearPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
	return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
		sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode LevelsLinearPlugin::ProcessImage( const ximage* src, ximage** dst )
{
	XErrorCode ret = XImageClone( src, dst );

	if ( ret == SuccessCode )
	{
		ret = ColorRemapping( *dst, redMap, greenMap, blueMap );

		if ( ret != SuccessCode )
		{
			XImageFree( dst );
		}
	}

    return ret;
}

// Process the specified source image by changing it
XErrorCode LevelsLinearPlugin::ProcessImageInPlace( ximage* src )
{
	XErrorCode ret = SuccessCode;

	if ( src == 0 )
	{
		ret = ErrorNullParameter;
	}
	else
	{
		ret = ColorRemapping( src, redMap, greenMap, blueMap );
	}

    return ret;
}

// Get specified property value of the plug-in
XErrorCode LevelsLinearPlugin::GetProperty( int32_t id, xvariant* value ) const
{
	XErrorCode ret = SuccessCode;

	value->type = XVT_Range;

    switch ( id )
    {
		// red channel properties
		case 0:
			value->value.rangeVal.min = inRedMin;
			value->value.rangeVal.max = inRedMax;
			break;
		case 1:
			value->value.rangeVal.min = outRedMin;
			value->value.rangeVal.max = outRedMax;
			break;

		// green channel properties
		case 2:
			value->value.rangeVal.min = inGreenMin;
			value->value.rangeVal.max = inGreenMax;
			break;
		case 3:
			value->value.rangeVal.min = outGreenMin;
			value->value.rangeVal.max = outGreenMax;
			break;

		// blue channel properties
		case 4:
			value->value.rangeVal.min = inBlueMin;
			value->value.rangeVal.max = inBlueMax;
			break;
		case 5:
			value->value.rangeVal.min = outBlueMin;
			value->value.rangeVal.max = outBlueMax;
			break;

        default:
	        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode LevelsLinearPlugin::SetProperty( int32_t id, const xvariant* value )
{
	static const xrange validRange = { 0, 255 };
    XErrorCode ret = 0;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

	// make sure property value has expected type
	ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 6, &convertedValue );

	if ( ret == SuccessCode )
	{
		XRangeIntersect( &convertedValue.value.rangeVal, &validRange );

		switch ( id )
		{
			case 0: 
			case 1:
				if ( id == 0 )
				{
					inRedMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					inRedMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				else
				{
					outRedMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					outRedMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				CalculateLinearMap( redMap, inRedMin, inRedMax, outRedMin, outRedMax );
				break;
				
			case 2:
			case 3:
				if ( id == 2 )
				{
					inGreenMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					inGreenMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				else
				{
					outGreenMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					outGreenMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				CalculateLinearMap( greenMap, inGreenMin, inGreenMax, outGreenMin, outGreenMax );
				break;
				
			case 4:
			case 5:
				if ( id == 4 )
				{
					inBlueMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					inBlueMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				else
				{
					outBlueMin = static_cast<uint8_t>( convertedValue.value.rangeVal.min );
					outBlueMax = static_cast<uint8_t>( convertedValue.value.rangeVal.max );
				}
				CalculateLinearMap( blueMap, inBlueMin, inBlueMax, outBlueMin, outBlueMax );
				break;
		}
    }

    XVariantClear( &convertedValue );

    return ret;
}
