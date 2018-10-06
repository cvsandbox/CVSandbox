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
#include "MorphologyOperatorPlugin.hpp"
#include <new>

// Supported pixel formats of input/output images
const XPixelFormat MorphologyOperatorPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

MorphologyOperatorPlugin::MorphologyOperatorPlugin( ) :
    structuringElementSize( 3 ), structuringElement( nullptr ),
    operatorType( OperatorType::Erosion ), tempImage( nullptr ), tempImage2( nullptr )
{
    structuringElement = new int8_t[structuringElementSize * structuringElementSize];

    for ( uint32_t i = 0, n = structuringElementSize * structuringElementSize; i < n; i++ )
    {
        structuringElement[i] = 1;
    }
}

MorphologyOperatorPlugin::~MorphologyOperatorPlugin( )
{
    delete [] structuringElement;
    XImageFree( &tempImage );
    XImageFree( &tempImage2 );
}

void MorphologyOperatorPlugin::Dispose( )
{
    delete this;
}

// The plug-in can not process image in-place
bool MorphologyOperatorPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode MorphologyOperatorPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode MorphologyOperatorPlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;
        
    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
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

        // check if we need a temporary image to keep intermediate image
        if ( ( ret == SuccessCode ) && ( operatorType >= OperatorType::Opening ) )
        {
            ret = XImageAllocateRaw( src->width, src->height, src->format, &tempImage );
        }

        // check if we need second  temporary image to keep intermediate image
        if ( ( ret == SuccessCode ) && ( operatorType == OperatorType::TopHat ) )
        {
            ret = XImageAllocateRaw( src->width, src->height, src->format, &tempImage2 );
        }

        if ( ret == SuccessCode )
        {
            switch ( operatorType )
            {
            case OperatorType::Erosion:
                ret = Erosion( src, *dst, structuringElement, structuringElementSize );
                break;

            case OperatorType::Dilatation:
                ret = Dilatation( src, *dst, structuringElement, structuringElementSize );
                break;

            case OperatorType::Opening:
                ret = Erosion( src, tempImage, structuringElement, structuringElementSize );
                if ( ret == SuccessCode )
                {
                    ret = Dilatation( tempImage, *dst, structuringElement, structuringElementSize );
                }
                break;

            case OperatorType::Closing:
                ret = Dilatation( src, tempImage, structuringElement, structuringElementSize );
                if ( ret == SuccessCode )
                {
                    ret = Erosion( tempImage, *dst, structuringElement, structuringElementSize );
                }
                break;

            case OperatorType::TopHat:
                ret = Erosion( src, tempImage, structuringElement, structuringElementSize );
                if ( ret == SuccessCode )
                {
                    ret = Dilatation( tempImage, tempImage2, structuringElement, structuringElementSize );

                    if ( ret == SuccessCode )
                    {
                        XImageCopyData( src, *dst );
                        ret = SubtractImages( *dst, tempImage2, 1.0f );
                    }
                }
                break;

            case OperatorType::BottomHat:
                ret = Dilatation( src, tempImage, structuringElement, structuringElementSize );
                if ( ret == SuccessCode )
                {
                    ret = Erosion( tempImage, *dst, structuringElement, structuringElementSize );

                    if ( ret == SuccessCode )
                    {
                        ret = SubtractImages( *dst, src, 1.0f );
                    }
                }
                break;

            default:
                ret = ErrorFailed;
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
XErrorCode MorphologyOperatorPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode MorphologyOperatorPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type        = XVT_U1;
        value->value.ubVal = static_cast<uint8_t>( operatorType );
        break;

    case 1:
        {
            xarray2d* array = nullptr;

            ret = XArrayAllocate2d( &array, XVT_I1, structuringElementSize, structuringElementSize );

            if ( ret == SuccessCode )
            {
                xvariant v;

                v.type = XVT_I1;

                for ( uint32_t i = 0; i < structuringElementSize; i++ )
                {
                    for ( uint32_t j = 0; j < structuringElementSize; j++ )
                    {
                        v.value.bVal = structuringElement[i * structuringElementSize + j];
                        XArraySet2d( array, i, j, &v );
                    }
                }

                value->type = XVT_I1 | XVT_Array2d;
                value->value.array2Val = array;
            }
        }
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode MorphologyOperatorPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 2, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            operatorType = static_cast<OperatorType>( XMIN( convertedValue.value.ubVal, static_cast<int>( OperatorType::BottomHat ) ) );
            break;

        case 1:
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
                    if ( rows != structuringElementSize )
                    {
                        delete [] structuringElement;
                        structuringElement     = nullptr;
                        structuringElementSize = 0;
                    }

                    if ( structuringElement == nullptr )
                    {
                        structuringElement = new (std::nothrow) int8_t[rows * cols];
                    }

                    if ( structuringElement == nullptr )
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

                                structuringElement[i * cols + j] = var.value.bVal;
                            }
                        }

                        structuringElementSize = rows;
                    }
                }
            }
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
