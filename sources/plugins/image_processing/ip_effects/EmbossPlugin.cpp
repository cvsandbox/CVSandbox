/*
    Image processing effects plug-ins of Computer Vision Sandbox

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

#include <ximaging_effects.h>
#include "EmbossPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat EmbossPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

EmbossPlugin::EmbossPlugin( ) :
    azimuth( 0 ), elevation( 45 ), depth( 10 )
{
}

void EmbossPlugin::Dispose( )
{
    delete this;
}

// The plug-in can not process image in-place without creating new image as a result
bool EmbossPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode EmbossPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode EmbossPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;
    
    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XImageAllocateRaw( src->width, src->height, src->format, dst );
    
        if ( ret == SuccessCode )
        {
            ret = EmbossImage( src, *dst, azimuth, elevation, depth );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode EmbossPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode EmbossPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = azimuth;
        break;

    case 1:
        value->type = XVT_R4;
        value->value.fVal = elevation;
        break;

    case 2:
        value->type = XVT_R4;
        value->value.fVal = depth;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode EmbossPlugin::SetProperty( int32_t id, const xvariant* value )
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
            azimuth = convertedValue.value.fVal;
            break;

        case 1:
            elevation = convertedValue.value.fVal;
            break;

        case 2:
            depth = convertedValue.value.fVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
