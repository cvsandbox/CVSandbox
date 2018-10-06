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
#include "GaussianBlurPlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat GaussianBlurPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

GaussianBlurPlugin::GaussianBlurPlugin( ) :
    sigma( 1.4f ), radius( 2 ), kernel2D( nullptr ), kernel1D( nullptr ), tempImage( nullptr )
{
    kernel2D = new (std::nothrow) float[( radius * 2 + 1 ) * ( radius * 2 + 1 )];
    kernel1D = new (std::nothrow) float[  radius * 2 + 1 ];

    if ( kernel2D != nullptr )
    {
        CreateGaussianBlurKernel2D( sigma, radius, kernel2D );
    }

    if ( kernel1D != nullptr )
    {
        CreateGaussianBlurKernel1D( sigma, radius, kernel1D );
    }
}

void GaussianBlurPlugin::Dispose( )
{
    delete [] kernel2D;
    delete [] kernel1D;
    XImageFree( &tempImage );
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool GaussianBlurPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode GaussianBlurPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode GaussianBlurPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( kernel2D == nullptr ) || ( kernel1D == nullptr ) )
    {
        ret = ErrorOutOfMemory;
    }
    else
    {
        // create output image of required format
        if ( ( src->format == XPixelFormatGrayscale8 ) ||
             ( src->format == XPixelFormatRGB24 ) ||
             ( src->format == XPixelFormatRGBA32 ) )
        {
            ret = XImageAllocateRaw( src->width, src->height, src->format, dst );
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }

        if ( ( ret == SuccessCode ) && ( src->format != XPixelFormatRGBA32 ) )
        {
            // allocate temporary image to be used when doing separable convolution
            ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscaleR4, &tempImage );
        }

        if ( ret == SuccessCode )
        {
            if ( src->format != XPixelFormatRGBA32 )
            {
                ret = SeparableConvolution( src, *dst, tempImage, kernel1D, kernel1D, radius * 2 + 1 );
            }
            else
            {
                ret = Convolution( src, *dst, kernel2D, radius * 2 + 1, true );
            }

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode GaussianBlurPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get the specified property value of the plug-in
XErrorCode GaussianBlurPlugin::GetProperty( int32_t id, xvariant* value ) const
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

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode GaussianBlurPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret       = SuccessCode;
    uint8_t    oldRadius = radius;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 2, &convertedValue );

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
        }
    }

    XVariantClear( &convertedValue );

    if ( ( oldRadius != radius ) || ( kernel2D == nullptr ) )
    {
        delete [] kernel2D;
        kernel2D = new (std::nothrow) float[( radius * 2 + 1 ) * ( radius * 2 + 1 )];
    }

    if ( ( oldRadius != radius ) || ( kernel1D == nullptr ) )
    {
        delete [] kernel1D;
        kernel1D = new (std::nothrow) float[  radius * 2 + 1 ];
    }

    if ( kernel2D != nullptr )
    {
        CreateGaussianBlurKernel2D( sigma, radius, kernel2D );
    }

    if ( kernel1D != nullptr )
    {
        CreateGaussianBlurKernel1D( sigma, radius, kernel1D );
    }

    return ret;
}
