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
#include "GradientReColoringPlugin.hpp"

// Supported pixel formats of input images
const XPixelFormat GradientReColoringPlugin::supportedInputFormats[] =
{
    XPixelFormatGrayscale8
};
// Result pixel formats of output images
const XPixelFormat GradientReColoringPlugin::supportedOutputFormats[] =
{
    XPixelFormatRGB24
};

GradientReColoringPlugin::GradientReColoringPlugin( )
{
    startColor.argb = 0xFF004000;
    endColor.argb   = 0xFFC0FFC0;
}

void GradientReColoringPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool GradientReColoringPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode GradientReColoringPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedInputFormats, supportedOutputFormats,
        sizeof( supportedInputFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode GradientReColoringPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // create output image of required format
        if ( src->format == XPixelFormatGrayscale8 )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatRGB24, dst );
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }

        if ( ret == SuccessCode )
        {
            ret = GradientGrayscaleReColoring( src, *dst, startColor, endColor );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode GradientReColoringPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// No properties to get/set
XErrorCode GradientReColoringPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    value->type = XVT_ARGB;

    switch ( id )
    {
        case 0:
            value->value.argbVal = startColor;
            break;

        case 1:
            value->value.argbVal = endColor;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
    }

    return ret;
}

XErrorCode GradientReColoringPlugin::SetProperty( int32_t id, const xvariant* value )
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
            startColor = convertedValue.value.argbVal;
            break;

        case 1:
            endColor = convertedValue.value.argbVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
