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

#include <chrono>
#include <ximaging_effects.h>
#include <xtextures.h>
#include "GrainPlugin.hpp"

using namespace std::chrono;

// Supported pixel formats of input/output images
const XPixelFormat GrainPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

GrainPlugin::GrainPlugin( ) :
    spacing( 50 ), density( 0.5 ), isVertical( true ), randSeed( 0 ), staticSeed( false ), textureImage( nullptr ),
    oldGrainSeed( 0 ), regenerateTexture( true )
{
}

GrainPlugin::~GrainPlugin( )
{
    XImageFree( &textureImage );
}

void GrainPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool GrainPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode GrainPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode GrainPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode GrainPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->format != XPixelFormatGrayscale8 ) &&
              ( src->format != XPixelFormatRGB24 ) &&
              ( src->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        // free old texture if its size does not match
        if ( ( textureImage != nullptr ) && (
             ( textureImage->width != src->width ) || ( textureImage->height != src->height ) ) )
        {
            XImageFree( &textureImage );
        }

        // allocate new texture
        if ( textureImage == nullptr )
        {
            XImageAllocateTextureRaw( src->width, src->height, &textureImage );
            regenerateTexture = true;
        }

        if ( textureImage == nullptr )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            uint16_t newGrainSeed = ( staticSeed ) ? randSeed :
                static_cast<uint16_t>( duration_cast<std::chrono::microseconds>( steady_clock::now( ).time_since_epoch( ) ).count( ) );

            if ( ( regenerateTexture ) || ( newGrainSeed != oldGrainSeed ) )
            // generate grain texture
            ret = GenerateGrainTexture( textureImage, newGrainSeed, spacing, density, isVertical );

            if ( ret == SuccessCode )
            {
                regenerateTexture = false;
                oldGrainSeed = newGrainSeed;
            }
        }

        if ( ret == SuccessCode )
        {
            if ( src->format == XPixelFormatGrayscale8 )
            {
                BlendImages( src, textureImage, BlendMode_Screen );
            }
            else
            {
                ximage* textureImageColor = 0;

                // allocate image for color texture
                ret = XImageAllocateRaw( src->width, src->height, src->format, &textureImageColor );

                if ( ret == SuccessCode )
                {
                    ret = GrayscaleToColor( textureImage, textureImageColor );

                    if ( ret == SuccessCode )
                    {
                        BlendImages( src, textureImageColor, BlendMode_Screen );
                    }

                    XImageFree( &textureImageColor );
                }

                XImageFree( &textureImage );
            }
        }
    }

    return ret;
}

// Get specified property value of the plug-in
XErrorCode GrainPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U2;
        value->value.usVal = spacing;
        break;

    case 1:
        value->type = XVT_R4;
        value->value.fVal = density;
        break;

    case 2:
        value->type = XVT_Bool;
        value->value.boolVal = isVertical;
        break;

    case 3:
        value->type = XVT_Bool;
        value->value.boolVal = staticSeed;
        break;

    case 4:
        value->type = XVT_U2;
        value->value.usVal = randSeed;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode GrainPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 5, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            spacing = convertedValue.value.usVal;
            break;

        case 1:
            density = convertedValue.value.fVal;
            break;

        case 2:
            isVertical = convertedValue.value.boolVal;
            break;

        case 3:
            staticSeed = convertedValue.value.boolVal;
            break;

        case 4:
            randSeed = convertedValue.value.usVal;
            break;

        default:
            ret = ErrorInvalidProperty;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
