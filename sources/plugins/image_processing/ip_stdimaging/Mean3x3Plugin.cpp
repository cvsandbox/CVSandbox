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
#include "Mean3x3Plugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat Mean3x3Plugin::supportedFormats[] =
{
	XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

Mean3x3Plugin::Mean3x3Plugin( )
{
}

void Mean3x3Plugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool Mean3x3Plugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode Mean3x3Plugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
	return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
		sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode Mean3x3Plugin::ProcessImage( const ximage* src, ximage** dst )
{
	XErrorCode ret = SuccessCode;
		
	if ( ( src == 0 ) || ( dst == 0 ) )
	{
		ret = ErrorNullParameter;
	}
	else
	{
		// create output image of required format
		if ( ( src->format == XPixelFormatGrayscale8 ) ||
             ( src->format == XPixelFormatRGB24 ) ||
             ( src->format == XPixelFormatRGBA32 ) )
		{
            ret = XImageAllocateRaw( src->width, src->height, src->format, dst );
		}
		else
		{
			ret = ErrorUnsupportedPixelFormat;
		}

		if ( ret == SuccessCode )
		{
			ret = Mean3x3( src, *dst );

			if ( ret != SuccessCode )
			{
				XImageFree( dst );
			}
		}
	}

    return ret;
}

// Process the specified source image by changing it
XErrorCode Mean3x3Plugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// No properties to get/set
XErrorCode Mean3x3Plugin::GetProperty( int32_t id, xvariant* value ) const
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}
XErrorCode Mean3x3Plugin::SetProperty( int32_t id, const xvariant* value )
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}
