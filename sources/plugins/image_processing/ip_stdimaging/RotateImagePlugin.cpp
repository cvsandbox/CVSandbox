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
#include "RotateImagePlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat RotateImagePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

RotateImagePlugin::RotateImagePlugin( ) :
    angle( 0.0f ), fillColor( ), resizeToFit( false )
{
}

void RotateImagePlugin::Dispose( )
{
    delete this;
}

// The plug-in cannot process image in-place since it changes theis size
bool RotateImagePlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode RotateImagePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode RotateImagePlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( !resizeToFit )
        {
            ret = XImageAllocateRaw( src->width, src->height, src->format, dst );
        }
        else
        {
            int32_t newWidth, newHeight;

            ret = CalculateRotatedImageSize( src->width, src->height, angle, &newWidth, &newHeight );

            if ( ret == SuccessCode )
            {
                ret = XImageAllocateRaw( newWidth, newHeight, src->format, dst );
            }
        }

        if ( ret == SuccessCode )
        {
            ret = RotateImageBilinear( src, *dst, angle, fillColor );
        }

        if ( ret != SuccessCode )
        {
            // free result image on failure
            XImageFree( dst );
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode RotateImagePlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode RotateImagePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = angle;
        break;

    case 1:
        value->type = XVT_ARGB;
        value->value.argbVal = fillColor;
        break;

    case 2:
        value->type = XVT_Bool;
        value->value.boolVal = resizeToFit;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode RotateImagePlugin::SetProperty( int32_t id, const xvariant* value )
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
            angle = convertedValue.value.fVal;
            break;

        case 1:
            fillColor = convertedValue.value.argbVal;
            break;

        case 2:
            resizeToFit = convertedValue.value.boolVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
