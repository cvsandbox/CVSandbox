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
#include "ResizeImagePlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat ResizeImagePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

ResizeImagePlugin::ResizeImagePlugin( ) :
    newWidth( 640 ), newHeight( 480 ), interpolation( 0 )
{
}

void ResizeImagePlugin::Dispose( )
{
    delete this;
}

// The plug-in cannot process image in-place since it changes theis size
bool ResizeImagePlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode ResizeImagePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode ResizeImagePlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = XImageAllocateRaw( newWidth, newHeight, src->format, dst );

    if ( ret == SuccessCode )
    {
        switch ( interpolation )
        {
        case 0:
            ret = ResizeImageNearestNeighbor( src, *dst );
            break;

        case 1:
            ret = ResizeImageBilinear( src, *dst );
            break;

        default:
            ret = ErrorInvalidArgument;
            break;
        }

        if ( ret != SuccessCode )
        {
            XImageFree( dst );
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode ResizeImagePlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode ResizeImagePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_I4;
        value->value.iVal = newWidth;
        break;

    case 1:
        value->type = XVT_I4;
        value->value.iVal = newHeight;
        break;

    case 2:
        value->type = XVT_U1;
        value->value.ubVal = interpolation;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ResizeImagePlugin::SetProperty( int32_t id, const xvariant* value )
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
            newWidth = convertedValue.value.iVal;
            break;

        case 1:
            newHeight = convertedValue.value.iVal;
            break;

        case 2:
            interpolation = convertedValue.value.ubVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
