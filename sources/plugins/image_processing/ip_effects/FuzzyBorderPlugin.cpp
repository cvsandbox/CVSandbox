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

#include <ximaging.h>
#include <xtextures.h>
#include "FuzzyBorderPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat FuzzyBorderPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

FuzzyBorderPlugin::FuzzyBorderPlugin( ) :
    borderWidth( 16 ), gradientWidth( 8 ), waviness( 5 ), randValue( (uint16_t) ( rand( ) % 10000 ) ), textureImage( nullptr )
{
    borderColor.argb  = 0xFFFFFFFF;
}

FuzzyBorderPlugin::~FuzzyBorderPlugin( )
{
    XImageFree( &textureImage );
}

void FuzzyBorderPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool FuzzyBorderPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode FuzzyBorderPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode FuzzyBorderPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode FuzzyBorderPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // free old texture if its size does not match
        if ( ( textureImage != nullptr ) && (
             ( textureImage->width != src->width ) || ( textureImage->height != src->height ) ) )
        {
            XImageFree( &textureImage );
        }

        // generate new texture
        if ( textureImage == nullptr )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscale8, &textureImage );

            if ( ret == SuccessCode )
            {
                ret = GenerateFuzzyBorderTexture( textureImage, randValue, borderWidth, gradientWidth, waviness );

                if ( ret != SuccessCode )
                {
                    XImageFree( &textureImage );
                }
            }
        }

        if ( ret == SuccessCode )
        {
            ret = XDrawingMaskedFill( src, textureImage, 0, 0, borderColor );
        }
    }

    return ret;
}

// Get specified property value of the plug-in
XErrorCode FuzzyBorderPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_ARGB;
        value->value.argbVal = borderColor;
        break;

    case 1:
        value->type = XVT_U2;
        value->value.usVal = borderWidth;
        break;

    case 2:
        value->type = XVT_U2;
        value->value.usVal = gradientWidth;
        break;

    case 3:
        value->type = XVT_U2;
        value->value.usVal = waviness;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode FuzzyBorderPlugin::SetProperty( int32_t id, const xvariant* value )
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
            borderColor = convertedValue.value.argbVal;
            break;

        case 1:
            borderWidth = convertedValue.value.usVal;
            break;

        case 2:
            gradientWidth = convertedValue.value.usVal;
            break;

        case 3:
            waviness = convertedValue.value.usVal;
            break;
        }

        if ( ( id >= 1 ) && ( id <= 3 ) )
        {
            XImageFree( &textureImage );
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
