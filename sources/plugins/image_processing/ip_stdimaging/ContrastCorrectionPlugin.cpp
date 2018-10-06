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
#include "ContrastCorrectionPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat ContrastCorrectionPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

ContrastCorrectionPlugin::ContrastCorrectionPlugin( ) :
    factor( 0.2f ), processRed( true ), processGreen( true ), processBlue( true ), needUpdate( true )
{
    CalculateLinearMap( identityMap, 0, 255, 0, 255 );
}

void ContrastCorrectionPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool ContrastCorrectionPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode ContrastCorrectionPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode ContrastCorrectionPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode ContrastCorrectionPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( needUpdate )
    {
        bool inverseGammaFilter = ( factor < 0.0f );
        float sCurveFactor      = ( inverseGammaFilter ) ? -factor + 1 : factor + 1;

        needUpdate = false;

        ret = CalculateSCurveMap( gammaMap, sCurveFactor, inverseGammaFilter );
    }

    if ( ret == SuccessCode )
    {
        if ( src->format == XPixelFormatGrayscale8 )
        {
            ret = GrayscaleRemapping( src, gammaMap );
        }
        else if ( ( src->format == XPixelFormatRGB24 ) || ( src->format == XPixelFormatRGBA32 ) )
        {
            uint8_t* redMap   = ( processRed )   ? gammaMap : identityMap;
            uint8_t* greenMap = ( processGreen ) ? gammaMap : identityMap;
            uint8_t* blueMap  = ( processBlue )  ? gammaMap : identityMap;

            ret = ColorRemapping( src, redMap, greenMap, blueMap );
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }
    }

    return ret;
}

// Get the specified property value of the plug-in
XErrorCode ContrastCorrectionPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = factor;
        break;

    case 1:
        value->type = XVT_Bool;
        value->value.boolVal = processRed;
        break;

    case 2:
        value->type = XVT_Bool;
        value->value.boolVal = processGreen;
        break;

    case 3:
        value->type = XVT_Bool;
        value->value.boolVal = processBlue;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode ContrastCorrectionPlugin::SetProperty( int32_t id, const xvariant* value )
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
            factor = convertedValue.value.fVal;
            needUpdate = true;
            break;

        case 1:
            processRed = convertedValue.value.boolVal;
            break;

        case 2:
            processGreen = convertedValue.value.boolVal;
            break;

        case 3:
            processBlue = convertedValue.value.boolVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
