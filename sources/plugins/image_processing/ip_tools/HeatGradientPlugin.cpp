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

#include <ximaging.h>
#include "HeatGradientPlugin.hpp"

// Supported pixel formats of input images
const XPixelFormat HeatGradientPlugin::supportedInputFormats[] =
{
    XPixelFormatGrayscale8
};
// Result pixel formats of output images
const XPixelFormat HeatGradientPlugin::supportedOutputFormats[] =
{
    XPixelFormatRGB24,
};

HeatGradientPlugin::HeatGradientPlugin( )
{
}

void HeatGradientPlugin::Dispose( )
{
    delete this;
}

// The plug-in cannot process image in-place since it changes its pixel format
bool HeatGradientPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode HeatGradientPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedInputFormats, supportedOutputFormats,
        sizeof( supportedInputFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode HeatGradientPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // create output image of required format
        ret = XImageAllocateRaw( src->width, src->height, XPixelFormatRGB24, dst );

        if ( ret == SuccessCode )
        {
            ret = GrayscaleToHeatGradient( src, *dst );

            if ( ret != SuccessCode )
            {
                // free result image on failure
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Cannot process the image itself
XErrorCode HeatGradientPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// No properties to get/set
XErrorCode HeatGradientPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}
XErrorCode HeatGradientPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}
