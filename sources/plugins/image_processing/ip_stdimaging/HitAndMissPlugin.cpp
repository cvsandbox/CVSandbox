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
#include "HitAndMissPlugin.hpp"
#include <new>

// Supported pixel formats of input/output images
const XPixelFormat HitAndMissPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8
};

HitAndMissPlugin::HitAndMissPlugin( ) :
    structuringElementSize( 3 ), structuringElement( nullptr ), operatorType( OperatorType::HitAndMiss )
{
    structuringElement = new int8_t[structuringElementSize * structuringElementSize];

    for ( uint32_t i = 0, n = structuringElementSize * structuringElementSize; i < n; i++ )
    {
        structuringElement[i] = 1;
    }
}

HitAndMissPlugin::~HitAndMissPlugin( )
{
    delete [] structuringElement;
}

void HitAndMissPlugin::Dispose( )
{
    delete this;
}

// The plug-in can not process image in-place
bool HitAndMissPlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode HitAndMissPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode HitAndMissPlugin::ProcessImage( const ximage* src, ximage** dst )
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

        if ( ret == SuccessCode )
        {
            ret = HitAndMiss( src, *dst, structuringElement, structuringElementSize, static_cast<XHitAndMissMode>( operatorType ) );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode HitAndMissPlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode HitAndMissPlugin::GetProperty( int32_t id, xvariant* value ) const
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
XErrorCode HitAndMissPlugin::SetProperty( int32_t id, const xvariant* value )
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
            operatorType = static_cast<OperatorType>( XMIN( convertedValue.value.ubVal, static_cast<int>( OperatorType::Thickening ) ) );
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
