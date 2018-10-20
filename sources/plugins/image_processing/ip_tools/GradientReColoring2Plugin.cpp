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
#include "GradientReColoring2Plugin.hpp"

// Supported pixel formats of input images
const XPixelFormat GradientReColoring2Plugin::supportedInputFormats[] =
{
    XPixelFormatGrayscale8
};
// Result pixel formats of output images
const XPixelFormat GradientReColoring2Plugin::supportedOutputFormats[] =
{
    XPixelFormatRGB24
};

GradientReColoring2Plugin::GradientReColoring2Plugin( )
{
    startColor.argb  = 0xFF004000;
    middleColor.argb = 0xFF50A080;
    endColor.argb    = 0xFFC0E0FF;
    threshold        = 127;
}

void GradientReColoring2Plugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool GradientReColoring2Plugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode GradientReColoring2Plugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedInputFormats, supportedOutputFormats,
        sizeof( supportedInputFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode GradientReColoring2Plugin::ProcessImage( const ximage* src, ximage** dst )
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
            ret = GradientGrayscaleReColoring2( src, *dst, startColor, middleColor, endColor, threshold );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode GradientReColoring2Plugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// No properties to get/set
XErrorCode GradientReColoring2Plugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
        case 0:
            value->type = XVT_ARGB;
            value->value.argbVal = startColor;
            break;

        case 1:
            value->type = XVT_ARGB;
            value->value.argbVal = middleColor;
            break;

        case 2:
            value->type = XVT_ARGB;
            value->value.argbVal = endColor;
            break;

        case 3:
            value->type = XVT_U1;
            value->value.ubVal = threshold;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
    }

    return ret;
}

XErrorCode GradientReColoring2Plugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 4, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            startColor = convertedValue.value.argbVal;
            break;

        case 1:
            middleColor = convertedValue.value.argbVal;
            break;

        case 2:
            endColor = convertedValue.value.argbVal;
            break;

        case 3:
            threshold = convertedValue.value.ubVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
