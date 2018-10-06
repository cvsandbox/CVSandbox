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
#include <new>
#include "CannyEdgeDetectorPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat CannyEdgeDetectorPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8
};

CannyEdgeDetectorPlugin::CannyEdgeDetectorPlugin( ) :
    sigma( 1.4f ), radius( 2 ), lowThreshold( 20 ), highThreshold( 100 ), blurKernel1D( nullptr ),
    tempBlurImage1( nullptr ), tempBlurImage2( nullptr ), tempEdgesImage( nullptr ), gradients( nullptr ), orientations( nullptr )
{
    blurKernel1D = new (std::nothrow) float[radius * 2 + 1];

    if ( blurKernel1D != nullptr )
    {
        CreateGaussianBlurKernel1D( sigma, radius, blurKernel1D );
    }
}

void CannyEdgeDetectorPlugin::Dispose( )
{
    XImageFree( &tempBlurImage1 );
    XImageFree( &tempBlurImage2 );
    XImageFree( &tempEdgesImage );
    XImageFree( &gradients );
    XImageFree( &orientations );
    delete [] blurKernel1D;
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool CannyEdgeDetectorPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode CannyEdgeDetectorPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode CannyEdgeDetectorPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // create output image of required format
        if ( src->format == XPixelFormatGrayscale8 )
        {
            ret = XImageAllocateRaw( src->width, src->height, src->format, dst );
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }

        // allocate temporary blur image 1
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscaleR4, &tempBlurImage1 );
        }

        // allocate temporary blur image 2
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscaleR4, &tempBlurImage2 );
        }

        // allocate temporary edges image
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscale8, &tempEdgesImage );
        }

        // allocate temporary image for gradients
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscaleR4, &gradients );
        }

        // allocate temporary image for orientations
        if ( ret == SuccessCode )
        {
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscale8, &orientations );
        }

        if ( ret == SuccessCode )
        {
            ret = CannyEdgeDetector( src, *dst, tempBlurImage1, tempBlurImage2, tempEdgesImage, gradients, orientations,
                                     blurKernel1D, radius * 2 + 1, lowThreshold, highThreshold );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode CannyEdgeDetectorPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode CannyEdgeDetectorPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_R4;
        value->value.fVal = sigma;
        break;

    case 1:
        value->type = XVT_U1;
        value->value.ubVal = radius;
        break;

    case 2:
        value->type = XVT_U2;
        value->value.usVal = lowThreshold;
        break;

    case 3:
        value->type = XVT_U2;
        value->value.usVal = highThreshold;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode CannyEdgeDetectorPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret       = SuccessCode;
    uint8_t    oldRadius = radius;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 4, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            sigma = XINRANGE( convertedValue.value.fVal, 0.1f, 10.0f );
            break;

        case 1:
            radius = XMIN( convertedValue.value.ubVal, 10 );
            break;

        case 2:
            lowThreshold = convertedValue.value.usVal;
            break;

        case 3:
            highThreshold = convertedValue.value.usVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    if ( ( oldRadius != radius ) || ( blurKernel1D == nullptr ) )
    {
        delete [] blurKernel1D;
        blurKernel1D = new ( std::nothrow ) float[radius * 2 + 1];
    }

    if ( blurKernel1D != nullptr )
    {
        CreateGaussianBlurKernel1D( sigma, radius, blurKernel1D );
    }

    return ret;
}
