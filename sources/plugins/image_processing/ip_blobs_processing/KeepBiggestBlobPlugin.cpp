/*
    Blobs' processing plug-ins of Computer Vision Sandbox

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
#include "KeepBiggestBlobPlugin.hpp"

#define SIZE_CRITERIA_RECTANGLE (0)
#define SIZE_CRITERIA_AREA      (1)

namespace Private
{
    class KeepBiggestBlobPluginData
    {
    public:
        uint8_t   SizeCriteria;

        ximage*   MapImage;
        xrect*    Rectangles;
        uint32_t* Areas;
        uint32_t  ObjectsCountAllocated;

        xrect     BlobRectangle;
        uint32_t  BlobArea;

        uint32_t* TempLabelsMap;
        uint32_t  TempLabelsMapSize;

    public:
        KeepBiggestBlobPluginData( ) :
            SizeCriteria( SIZE_CRITERIA_RECTANGLE ), MapImage( nullptr ),
            Rectangles( nullptr ), Areas( nullptr ), ObjectsCountAllocated( 0 ),
            BlobRectangle( { 0, 0, 0, 0 } ), BlobArea( 0 ),
            TempLabelsMap( nullptr ), TempLabelsMapSize( 0 )
        {
        }

        ~KeepBiggestBlobPluginData( )
        {
            XImageFree( &MapImage );
            FreeLabelsMap( );
            FreeObjectsInfo( );
        }

        XErrorCode AllocateLabelsMap( uint32_t maxObjects )
        {
            XErrorCode ret = SuccessCode;

            if ( TempLabelsMapSize < maxObjects )
            {
                FreeLabelsMap( );
            }

            if ( TempLabelsMap == nullptr )
            {
                TempLabelsMap = (uint32_t*) malloc( maxObjects * sizeof( uint32_t ) );

                if ( TempLabelsMap == nullptr )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    TempLabelsMapSize = maxObjects;
                }
            }

            return ret;
        }

        void FreeLabelsMap( )
        {
            if ( TempLabelsMap != nullptr )
            {
                free( TempLabelsMap );
                TempLabelsMap = nullptr;
            }
            TempLabelsMapSize = 0;
        }

        XErrorCode AllocateObjectsInfo( uint32_t objectsCount )
        {
            XErrorCode ret = SuccessCode;

            // check if allocated memory can be reused
            if ( ObjectsCountAllocated < objectsCount )
            {
                FreeObjectsInfo( );
            }

            if ( ObjectsCountAllocated == 0 )
            {
                Rectangles = (xrect*)    malloc( objectsCount * sizeof( xrect ) );
                Areas      = (uint32_t*) malloc( objectsCount * sizeof( uint32_t ) );

                if ( ( Rectangles == nullptr ) || ( Areas == nullptr ) )
                {
                    ret = ErrorOutOfMemory;
                    FreeObjectsInfo( );
                }
                else
                {
                    ObjectsCountAllocated = objectsCount;
                }
            }

            return ret;
        }

        void FreeObjectsInfo( )
        {
            if ( Rectangles != nullptr )
            {
                free( Rectangles );
                Rectangles = nullptr;
            }
            if ( Areas != nullptr )
            {
                free( Areas );
                Areas = nullptr;
            }

            ObjectsCountAllocated = 0;
        }
    };
}

// Supported pixel formats of input/output images
const XPixelFormat KeepBiggestBlobPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

KeepBiggestBlobPlugin::KeepBiggestBlobPlugin( ) :
    mData( new Private::KeepBiggestBlobPluginData( ) )
{
}

void KeepBiggestBlobPlugin::Dispose( )
{
    delete mData;
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool KeepBiggestBlobPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode KeepBiggestBlobPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode KeepBiggestBlobPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode KeepBiggestBlobPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XImageAllocate( src->width, src->height, XPixelFormatGrayscale32, &mData->MapImage );

        if ( ret == SuccessCode )
        {
            uint32_t objectsCount = 0;

            ret = mData->AllocateLabelsMap( ( ( src->width / 2 ) + 1 ) * ( ( src->height / 2 ) + 1 ) + 1 );

            if ( ret == SuccessCode )
            {
                ret = BcBuildObjectsMap( src, mData->MapImage, &objectsCount, mData->TempLabelsMap, mData->TempLabelsMapSize );
            }

            if ( ret == SuccessCode )
            {
                if ( objectsCount == 0 )
                {
                    mData->BlobRectangle = { 0, 0, 0, 0 };
                    mData->BlobArea      = 0;
                }
                else
                {
                    ret = mData->AllocateObjectsInfo( objectsCount );

                    if ( ret == SuccessCode )
                    {
                        ret = BcGetObjectsRectanglesAndArea( mData->MapImage, objectsCount, mData->Rectangles, mData->Areas );

                        if ( ret == SuccessCode )
                        {
                            uint32_t id = 0;

                            if ( mData->SizeCriteria == SIZE_CRITERIA_RECTANGLE )
                            {
                                uint32_t maxSize    = 0;
                                uint32_t size;
                                xrect*   rectangles = mData->Rectangles;

                                // find ID of the biggest blob
                                for ( uint32_t i = 0; i < objectsCount; i++ )
                                {
                                    size = (uint32_t) ( ( rectangles[i].x2 - rectangles[i].x1 + 1 ) * ( rectangles[i].y2 - rectangles[i].y1 + 1 ) );

                                    if ( size > maxSize )
                                    {
                                        maxSize = size;
                                        id      = i + 1;
                                    }
                                }
                            }
                            else
                            {
                                uint32_t  maxArea = 0;
                                uint32_t* areas   = mData->Areas;

                                for ( uint32_t i = 0; i < objectsCount; i++ )
                                {
                                    if ( areas[i] > maxArea )
                                    {
                                        maxArea = areas[i];
                                        id      = i + 1;
                                    }
                                }
                            }

                            mData->BlobRectangle = mData->Rectangles[id - 1];
                            mData->BlobArea      = mData->Areas[id - 1];

                            ret = BcKeepObjectIDOnly( src, mData->MapImage, id, { 0xFF000000 } );
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// Get the specified property value of the plug-in
XErrorCode KeepBiggestBlobPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = mData->SizeCriteria;
        break;

    case 1:
        value->type = XVT_Point;
        value->value.pointVal.x = mData->BlobRectangle.x1;
        value->value.pointVal.y = mData->BlobRectangle.y1;
        break;

    case 2:
        value->type = XVT_Point;
        value->value.pointVal.x = mData->BlobRectangle.x2;
        value->value.pointVal.y = mData->BlobRectangle.y2;
        break;

    case 3:
        value->type = XVT_U4;
        value->value.uiVal = mData->BlobArea;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode KeepBiggestBlobPlugin::SetProperty( int32_t id, const xvariant* value )
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
            mData->SizeCriteria = convertedValue.value.ubVal;
            break;

        default:
            ret = ErrorReadOnlyProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
