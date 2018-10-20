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
#include "GradientReColoring4Plugin.hpp"

// Supported pixel formats of input images
const XPixelFormat GradientReColoring4Plugin::supportedInputFormats[] =
{
    XPixelFormatGrayscale8
};
// Result pixel formats of output images
const XPixelFormat GradientReColoring4Plugin::supportedOutputFormats[] =
{
    XPixelFormatRGB24
};

GradientReColoring4Plugin::GradientReColoring4Plugin( )
{
    startColor.argb   = 0xFF004000;
    middleColor1.argb = 0xFFA0FF80;
    middleColor2.argb = 0xFF005000;
    middleColor3.argb = 0xFF50A080;
    endColor.argb     = 0xFFC0E0FF;
    threshold1        = 63;
    threshold2        = 127;
    threshold3        = 191;
}

void GradientReColoring4Plugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool GradientReColoring4Plugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode GradientReColoring4Plugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedInputFormats, supportedOutputFormats,
        sizeof( supportedInputFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode GradientReColoring4Plugin::ProcessImage( const ximage* src, ximage** dst )
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
            ret = GradientGrayscaleReColoring4( src, *dst, startColor, middleColor1, middleColor2, middleColor3, endColor,
                                                threshold1, threshold2, threshold3 );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode GradientReColoring4Plugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// No properties to get/set
XErrorCode GradientReColoring4Plugin::GetProperty( int32_t id, xvariant* value ) const
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
            value->value.argbVal = middleColor1;
            break;

        case 2:
            value->type = XVT_ARGB;
            value->value.argbVal = middleColor2;
            break;

        case 3:
            value->type = XVT_ARGB;
            value->value.argbVal = middleColor3;
            break;

        case 4:
            value->type = XVT_ARGB;
            value->value.argbVal = endColor;
            break;

        case 5:
            value->type = XVT_U1;
            value->value.ubVal = threshold1;
            break;

        case 6:
            value->type = XVT_U1;
            value->value.ubVal = threshold2;
            break;

        case 7:
            value->type = XVT_U1;
            value->value.ubVal = threshold3;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
    }

    return ret;
}

XErrorCode GradientReColoring4Plugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 8, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            startColor = convertedValue.value.argbVal;
            break;

        case 1:
            middleColor1 = convertedValue.value.argbVal;
            break;

        case 2:
            middleColor2 = convertedValue.value.argbVal;
            break;

        case 3:
            middleColor3 = convertedValue.value.argbVal;
            break;

        case 4:
            endColor = convertedValue.value.argbVal;
            break;

        case 5:
            threshold1 = convertedValue.value.ubVal;
            break;

        case 6:
            threshold2 = convertedValue.value.ubVal;
            break;

        case 7:
            threshold3 = convertedValue.value.ubVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
