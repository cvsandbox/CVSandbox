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
#include "ConvolutionPlugin.hpp"
#include <new>

// Supported pixel formats of input/output images
const XPixelFormat ConvolutionPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

ConvolutionPlugin::ConvolutionPlugin( ) :
    kernelSize( 3 ), kernel( nullptr ), autoDivisor( true ), divisor( 9 ), offset( 0 ), borderMode( BHMode_Extend )
{
    kernel = new float[kernelSize * kernelSize];

    for ( uint32_t i = 0, n = kernelSize * kernelSize; i < n; i++ )
    {
        kernel[i] = 1;
    }
}

ConvolutionPlugin::~ConvolutionPlugin( )
{
    delete[] kernel;
}

void ConvolutionPlugin::Dispose( )
{
    delete this;
}

// The plug-in can not process image in-place
bool ConvolutionPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode ConvolutionPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode ConvolutionPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int32_t dstWidth  = src->width;
        int32_t dstHeight = src->height;

        if ( borderMode == BHMode_Crop )
        {
            dstWidth  -= ( kernelSize - 1 );
            dstHeight -= ( kernelSize - 1 );

            if ( ( dstWidth < 1 ) || ( dstHeight < 1 ) )
            {
                ret = ErrorImageIsTooSmall;
            }
        }

        if ( ret == SuccessCode )
        {
            // create output image of required format
            if ( ( src->format == XPixelFormatGrayscale8 ) ||
                 ( src->format == XPixelFormatRGB24 ) ||
                 ( src->format == XPixelFormatRGBA32 ) )
            {
                ret = XImageAllocateRaw( dstWidth, dstHeight, src->format, dst );
            }
            else
            {
                ret = ErrorUnsupportedPixelFormat;
            }

            if ( ret == SuccessCode )
            {
                ret = ConvolutionEx( src, *dst, kernel, kernelSize, divisor, offset, borderMode );

                if ( ret != SuccessCode )
                {
                    XImageFree( dst );
                }
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode ConvolutionPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode ConvolutionPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        {
            xarray2d* array = nullptr;

            ret = XArrayAllocate2d( &array, XVT_R4, kernelSize, kernelSize );

            if ( ret == SuccessCode )
            {
                xvariant v;

                v.type = XVT_R4;

                for ( uint32_t i = 0; i < kernelSize; i++ )
                {
                    for ( uint32_t j = 0; j < kernelSize; j++ )
                    {
                        v.value.fVal = kernel[i * kernelSize + j];
                        XArraySet2d( array, i, j, &v );
                    }
                }

                value->type = XVT_R4 | XVT_Array2d;
                value->value.array2Val = array;
            }
        }
        break;

    case 1:
        value->type = XVT_Bool;
        value->value.boolVal = autoDivisor;
        break;

    case 2:
        value->type = XVT_R4;
        value->value.fVal = divisor;
        break;

    case 3:
        value->type = XVT_R4;
        value->value.fVal = offset;
        break;

    case 4:
        value->type = XVT_U1;
        value->value.ubVal = borderMode;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode ConvolutionPlugin::SetProperty( int32_t id, const xvariant* value )
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
            {
                xarray2d* array = convertedValue.value.array2Val;
                uint32_t  rows, cols;

                XArraySize( array, &rows, &cols );

                if ( ( rows != cols ) || ( ( rows % 2 ) != 1 ) || ( rows < 3 ) || ( rows > 51 ) )
                {
                    ret = ErrorInvalidArraySize;
                }
                else
                {
                    if ( rows != kernelSize )
                    {
                        delete [] kernel;
                        kernel     = nullptr;
                        kernelSize = 0;
                    }

                    if ( kernel == nullptr )
                    {
                        kernel = new (std::nothrow) float[rows * cols];
                    }

                    if ( kernel == nullptr )
                    {
                        ret = ErrorOutOfMemory;
                    }
                    else
                    {
                        xvariant var;
                        XVariantInit( &var );

                        for ( uint32_t i = 0; i < rows; i++ )
                        {
                            for ( uint32_t j = 0; j < cols; j++ )
                            {
                                XArrayGet2d( array, i, j, &var );

                                kernel[i * cols + j] = var.value.fVal;
                            }
                        }

                        kernelSize = rows;
                    }
                }
            }
            break;

        case 1:
            autoDivisor = convertedValue.value.boolVal;
            break;

        case 2:
            if ( !autoDivisor )
            {
                divisor = convertedValue.value.fVal;
            }
            break;

        case 3:
            offset = convertedValue.value.fVal;
            break;

        case 4:
            borderMode = XMIN( convertedValue.value.ubVal, 3 );
            break;
        }
    }

    XVariantClear( &convertedValue );

    // recalculate divisor if needed
    if ( ( ret == SuccessCode ) && ( ( id == 0 ) || ( id == 1 ) ) && ( autoDivisor ) )
    {
        divisor = 0;

        for ( uint32_t i = 0, n = kernelSize * kernelSize; i < n; i++ )
        {
            divisor += kernel[i];
        }

        if ( divisor == 0 )
        {
            divisor = 1;
        }
    }

    return ret;
}
