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

#include <chrono>
#include <ximaging.h>
#include "SaltAndPepperNoisePlugin.hpp"

using namespace std::chrono;

// Supported pixel formats of input/output images
const XPixelFormat SaltAndPepperNoisePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

SaltAndPepperNoisePlugin::SaltAndPepperNoisePlugin( ) :
    randSeed( 0 ),
    noiseAmount( 10.0f ), pepperValue( 0 ), saltValue( 255 ), staticSeed( false )
{
}

void SaltAndPepperNoisePlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool SaltAndPepperNoisePlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode SaltAndPepperNoisePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode SaltAndPepperNoisePlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode SaltAndPepperNoisePlugin::ProcessImageInPlace( ximage* src )
{
    return SaltAndPepperNoise( src, ( staticSeed ) ? randSeed :
        static_cast<uint32_t>( duration_cast<std::chrono::microseconds>( steady_clock::now( ).time_since_epoch( ) ).count( ) ),
        noiseAmount, pepperValue, saltValue );
}

// Get specified property value of the plug-in
XErrorCode SaltAndPepperNoisePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = noiseAmount;
        break;

    case 1:
        value->type = XVT_U1;
        value->value.ubVal = pepperValue;
        break;

    case 2:
        value->type = XVT_U1;
        value->value.ubVal = saltValue;
        break;

    case 3:
        value->type = XVT_Bool;
        value->value.boolVal = staticSeed;
        break;

    case 4:
        value->type = XVT_U4;
        value->value.uiVal = randSeed;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode SaltAndPepperNoisePlugin::SetProperty( int32_t id, const xvariant* value )
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
            noiseAmount = convertedValue.value.fVal;
            break;

        case 1:
            pepperValue = convertedValue.value.ubVal;
            break;

        case 2:
            saltValue = convertedValue.value.ubVal;
            break;

        case 3:
            staticSeed = convertedValue.value.boolVal;
            break;

        case 4:
            randSeed = convertedValue.value.uiVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
