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
#include "ExtractRGBChannelPlugin.hpp"

// Supported pixel formats of input images
const XPixelFormat ExtractRGBChannelPlugin::supportedInputFormats[] =
{
    XPixelFormatRGB24, XPixelFormatRGBA32
};
// Result pixel formats of output images
const XPixelFormat ExtractRGBChannelPlugin::supportedOutputFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatGrayscale8
};

ExtractRGBChannelPlugin::ExtractRGBChannelPlugin( ) :
    channelIndex( 0 )
{
}

void ExtractRGBChannelPlugin::Dispose( )
{
    delete this;
}

// The plug-in cannot process image in-place since it changes its pixel format
bool ExtractRGBChannelPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode ExtractRGBChannelPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedInputFormats, supportedOutputFormats,
        sizeof( supportedInputFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode ExtractRGBChannelPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    const static uint8_t rgbChannels[] = { RedIndex, GreenIndex, BlueIndex, AlphaIndex };

    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // create output image of required format
        if ( ( src->format == XPixelFormatRGB24 ) || ( src->format == XPixelFormatRGBA32 ) )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscale8, dst );
        }
        else if ( ( src->format == XPixelFormatRGB48 ) || ( src->format == XPixelFormatRGBA64 ) )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscale16, dst );
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }

        if ( ret == SuccessCode )
        {
            ret = ExtractRGBChannel( src, *dst, rgbChannels[channelIndex] );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Cannot process the image itself
XErrorCode ExtractRGBChannelPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode ExtractRGBChannelPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = channelIndex;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ExtractRGBChannelPlugin::SetProperty( int32_t id, const xvariant* value )
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
            if ( convertedValue.value.ubVal < 4 )
            {
                channelIndex = convertedValue.value.ubVal;
            }
            else
            {
                ret = ErrorArgumentOutOfRange;
            }
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
