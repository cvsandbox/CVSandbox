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
#include "FindBiggestBlobPlugin.hpp"

#define SIZE_CRITERIA_RECTANGLE (0)
#define SIZE_CRITERIA_AREA      (1)

#define SEARCH_FOR_BLOB (0)
#define SEARCH_FOR_HOLE (1)

namespace Private
{
    class FindBiggestBlobPluginData
    {
    public:
        uint8_t   SizeCriteria;
        uint8_t   SearchFor;
        bool      AllowEdgeObjects;

        ximage*   MapImage;
        xrect*    Rectangles;
        uint32_t* Areas;
        uint32_t  ObjectsCountAllocated;

        xrect     BlobRectangle;
        uint32_t  BlobArea;

        uint32_t* TempLabelsMap;
        uint32_t  TempLabelsMapSize;

    public:
        FindBiggestBlobPluginData( ) :
            SizeCriteria( 0 ), SearchFor( SEARCH_FOR_BLOB ), AllowEdgeObjects( true ), MapImage( nullptr ),
            Rectangles( nullptr ), Areas( nullptr ), ObjectsCountAllocated( 0 ),
            BlobRectangle( { 0, 0, 0, 0 } ), BlobArea( 0 ),
            TempLabelsMap( nullptr ), TempLabelsMapSize( 0 )
        {
        }

        ~FindBiggestBlobPluginData( )
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
                Rectangles = (xrect*) malloc( objectsCount * sizeof( xrect ) );
                Areas = (uint32_t*) malloc( objectsCount * sizeof( uint32_t ) );

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
const XPixelFormat FindBiggestBlobPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

FindBiggestBlobPlugin::FindBiggestBlobPlugin( ) :
    mData( new Private::FindBiggestBlobPluginData( ) )
{
}

void FindBiggestBlobPlugin::Dispose( )
{
    delete mData;
    delete this;
}

// Provide supported pixel formats
XErrorCode FindBiggestBlobPlugin::GetSupportedPixelFormats( XPixelFormat* formats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedFormats, XARRAY_SIZE( supportedFormats ), formats, count );
}

// Process the specified source image by changing it
XErrorCode FindBiggestBlobPlugin::ProcessImage( const ximage* image )
{
    XErrorCode ret = SuccessCode;

    if ( image == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XImageAllocateRaw( image->width, image->height, XPixelFormatGrayscale32, &mData->MapImage );

        if ( ret == SuccessCode )
        {
            uint32_t objectsCount = 0;

            ret = mData->AllocateLabelsMap( ( ( image->width / 2 ) + 1 ) * ( ( image->height / 2 ) + 1 ) + 1 );

            if ( ret == SuccessCode )
            {
                if ( mData->SearchFor == SEARCH_FOR_BLOB )
                {
                    ret = BcBuildObjectsMap( image, mData->MapImage, &objectsCount, mData->TempLabelsMap, mData->TempLabelsMapSize );
                }
                else
                {
                    ret = BcBuildBackgroundMap( image, mData->MapImage, &objectsCount, mData->TempLabelsMap, mData->TempLabelsMapSize );
                }
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
                        int32_t widthM1  = image->width  - 1;
                        int32_t heightM1 = image->height - 1;

                        ret = BcGetObjectsRectanglesAndArea( mData->MapImage, objectsCount, mData->Rectangles, mData->Areas );

                        if ( ret == SuccessCode )
                        {
                            xrect*   rectangles = mData->Rectangles;
                            uint32_t id         = 0;

                            if ( mData->SizeCriteria == 0 )
                            {
                                uint32_t maxSize = 0;
                                uint32_t size;

                                // find ID of the biggest blob
                                for ( uint32_t i = 0; i < objectsCount; i++ )
                                {
                                    size = (uint32_t) ( ( rectangles[i].x2 - rectangles[i].x1 + 1 ) * ( rectangles[i].y2 - rectangles[i].y1 + 1 ) );

                                    if ( size > maxSize )
                                    {
                                        if ( ( mData->AllowEdgeObjects ) ||
                                           ( ( rectangles[i].x1 != 0 ) && ( rectangles[i].y1 != 0 ) &&
                                             ( rectangles[i].x2 != widthM1 ) && ( rectangles[i].y2 != heightM1 ) ) )
                                        {
                                            maxSize = size;
                                            id      = i + 1;
                                        }
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
                                        if ( ( mData->AllowEdgeObjects ) ||
                                           ( ( rectangles[i].x1 != 0 ) && ( rectangles[i].y1 != 0 ) &&
                                             ( rectangles[i].x2 != widthM1 ) && ( rectangles[i].y2 != heightM1 ) ) )
                                        {
                                            maxArea = areas[i];
                                            id      = i + 1;
                                        }
                                    }
                                }
                            }

                            if ( id != 0 )
                            {
                                mData->BlobRectangle = mData->Rectangles[id - 1];
                                mData->BlobArea      = mData->Areas[id - 1];
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}

// Get the specified property value of the plug-in
XErrorCode FindBiggestBlobPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = mData->SizeCriteria;
        break;

    case 1:
        value->type = XVT_U1;
        value->value.ubVal = mData->SearchFor;
        break;

    case 2:
        value->type = XVT_Bool;
        value->value.boolVal = mData->AllowEdgeObjects;
        break;

    case 3:
        value->type = XVT_Point;
        value->value.pointVal.x = mData->BlobRectangle.x1;
        value->value.pointVal.y = mData->BlobRectangle.y1;
        break;

    case 4:
        value->type = XVT_Point;
        value->value.pointVal.x = mData->BlobRectangle.x2;
        value->value.pointVal.y = mData->BlobRectangle.y2;
        break;

    case 5:
        value->type = XVT_U4;
        value->value.uiVal = mData->BlobArea;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode FindBiggestBlobPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 6, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->SizeCriteria = convertedValue.value.ubVal;
            break;

        case 1:
            mData->SearchFor = convertedValue.value.ubVal;
            break;

        case 2:
            mData->AllowEdgeObjects = convertedValue.value.boolVal;
            break;

        default:
            ret = ErrorReadOnlyProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
