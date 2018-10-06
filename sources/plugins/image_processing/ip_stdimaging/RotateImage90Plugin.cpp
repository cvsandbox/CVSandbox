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
#include "RotateImage90Plugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat RotateImage90Plugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

RotateImage90Plugin::RotateImage90Plugin( ) :
    rotationType( 90 )
{
}

void RotateImage90Plugin::Dispose( )
{
    delete this;
}

// The plug-in cannot process image in-place since it changes theis size
bool RotateImage90Plugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode RotateImage90Plugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode RotateImage90Plugin::ProcessImage( const ximage* src, ximage** dst )
{
    int32_t    newWidth  = ( rotationType == 180 ) ? src->width  : src->height;
    int32_t    newHeight = ( rotationType == 180 ) ? src->height : src->width;
    XErrorCode ret       = XImageAllocateRaw( newWidth, newHeight, src->format, dst );

    if ( ret == SuccessCode )
    {
        switch ( rotationType )
        {
        case 90:
            ret = RotateImage90( src, *dst );
            break;

        case 180:
            ret = XImageCopyData( src, *dst );
            if ( ret == SuccessCode )
            {
                ret = MirrorImage( *dst, true, true );
            }
            break;

        case 270:
            ret = RotateImage270( src, *dst );
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
XErrorCode RotateImage90Plugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode RotateImage90Plugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U2;
        value->value.usVal = rotationType;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode RotateImage90Plugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 1, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            rotationType = convertedValue.value.usVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
