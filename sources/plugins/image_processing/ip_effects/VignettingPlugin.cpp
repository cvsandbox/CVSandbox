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
#include "VignettingPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat VignettingPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

VignettingPlugin::VignettingPlugin( ) :
    startWidthFactor( 90.0f ), endWidthFactor( 150.0f ), decreaseBrightness( true ), decreaseSaturation( false )
{
}

void VignettingPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool VignettingPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode VignettingPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode VignettingPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = XImageClone( src, dst );

    if ( ret == SuccessCode )
    {
        ret = ProcessImageInPlace( *dst );

        if ( ret != SuccessCode )
        {
            XImageFree( dst );
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode VignettingPlugin::ProcessImageInPlace( ximage* src )
{
    return MakeVignetteImage( src, startWidthFactor / 100.0f, endWidthFactor / 100.0f, decreaseBrightness, decreaseSaturation );
}

// Get specified property value of the plug-in
XErrorCode VignettingPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = startWidthFactor;
        break;

    case 1:
        value->type = XVT_R4;
        value->value.fVal = endWidthFactor;
        break;

    case 2:
        value->type = XVT_Bool;
        value->value.boolVal = decreaseBrightness;
        break;

    case 3:
        value->type = XVT_Bool;
        value->value.boolVal = decreaseSaturation;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode VignettingPlugin::SetProperty( int32_t id, const xvariant* value )
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
            startWidthFactor = convertedValue.value.fVal;
            break;

        case 1:
            endWidthFactor = convertedValue.value.fVal;
            break;

        case 2:
            decreaseBrightness = convertedValue.value.boolVal;
            break;

        case 3:
            decreaseSaturation = convertedValue.value.boolVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
