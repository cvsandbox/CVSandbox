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
#include "FilterBlobsBySizePlugin.hpp"

#define SIZE_CRITERIA_RECTANGLE (0)
#define SIZE_CRITERIA_AREA      (1)

namespace Private
{
    class FilterBlobsBySizePluginData
    {
    public:
        bool      PerformImageFiltering;
        uint8_t   SizeCriteria;
        bool      CoupledFiltering;

        uint32_t  MinWidth;
        uint32_t  MinHeight;
        uint32_t  MaxWidth;
        uint32_t  MaxHeight;

        uint32_t  MinArea;
        uint32_t  MaxArea;

        ximage*   MapImage;
        xrect*    Rectangles;
        uint32_t* Areas;
        uint8_t*  FillMap;
        uint32_t  ObjectsCountAllocated;

        uint32_t  BlobsLeft;
        uint32_t  BlobsRemoved;
        xpoint*   BlobsPositions;
        xsize*    BlobsSizes;
        uint32_t* BlobsAreas;
        uint32_t  KeptObjectsAllocated;
        bool      KeptObjectsInfoReady;

        uint32_t* TempLabelsMap;
        uint32_t  TempLabelsMapSize;

    public:
        FilterBlobsBySizePluginData( ) :
            PerformImageFiltering( true ), SizeCriteria( SIZE_CRITERIA_RECTANGLE ), CoupledFiltering( false ),
            MinWidth( 5 ), MinHeight( 5 ), MaxWidth( 10000 ), MaxHeight( 10000 ),
            MinArea( 5 ), MaxArea( 100000000 ),
            MapImage( nullptr ),
            Rectangles( nullptr ), Areas( nullptr ), FillMap( nullptr ), ObjectsCountAllocated( 0 ),
            BlobsLeft( 0 ), BlobsRemoved( 0 ),
            BlobsPositions( nullptr ), BlobsSizes( nullptr ), BlobsAreas( nullptr ),
            KeptObjectsAllocated( 0 ), KeptObjectsInfoReady( false ),
            TempLabelsMap( nullptr ), TempLabelsMapSize( 0 )
        {

        }

        ~FilterBlobsBySizePluginData( )
        {
            XImageFree( &MapImage );
            FreeLabelsMap( );
            FreeObjectsInfo( );
            FreeKeptObjectsInfo( );
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
                FillMap    = (uint8_t*)  malloc( ( objectsCount + 1 ) * sizeof( uint8_t ) );

                if ( ( Rectangles == nullptr ) || ( Areas == nullptr ) || ( FillMap == nullptr ) )
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
            if ( FillMap != nullptr )
            {
                free( FillMap );
                FillMap = nullptr;
            }

            ObjectsCountAllocated = 0;
        }

        XErrorCode PrepareKeptObjectsInfo( )
        {
            XErrorCode ret = SuccessCode;

            if ( !KeptObjectsInfoReady )
            {
                if ( BlobsLeft == 0 )
                {
                    KeptObjectsInfoReady = true;
                }
                else
                {
                    ret = AllocateKeptObjectsInfo( );

                    if ( ret == SuccessCode )
                    {
                        uint32_t objectsCount = BlobsRemoved + BlobsLeft;
                        uint32_t i, j, blobId;

                        for ( i = 0, j = 0, blobId = 1; i < objectsCount; i++, blobId++ )
                        {
                            if ( FillMap[blobId] == 0 )
                            {
                                // the object was kept
                                BlobsPositions[j].x  = Rectangles[i].x1;
                                BlobsPositions[j].y  = Rectangles[i].y1;
                                BlobsSizes[j].width  = Rectangles[i].x2 - Rectangles[i].x1 + 1;
                                BlobsSizes[j].height = Rectangles[i].y2 - Rectangles[i].y1 + 1;
                                BlobsAreas[j]        = Areas[i];
                                j++;
                            }
                        }

                        KeptObjectsInfoReady = true;
                    }
                }
            }

            return ret;
        }

    private:
        XErrorCode AllocateKeptObjectsInfo( )
        {
            XErrorCode ret = SuccessCode;

            if ( KeptObjectsAllocated < BlobsLeft )
            {
                FreeKeptObjectsInfo( );
            }

            if ( KeptObjectsAllocated == 0 )
            {
                BlobsPositions = (xpoint*)   malloc( BlobsLeft * sizeof( xpoint ) );
                BlobsSizes     = (xsize*)    malloc( BlobsLeft * sizeof( xsize ) );
                BlobsAreas     = (uint32_t*) malloc( BlobsLeft * sizeof( uint32_t ) );

                if ( ( BlobsPositions == nullptr ) || ( BlobsSizes == nullptr ) || ( BlobsAreas == nullptr ) )
                {
                    ret = ErrorOutOfMemory;
                    FreeKeptObjectsInfo( );
                }
                else
                {
                    KeptObjectsAllocated = BlobsLeft;
                }
            }

            return ret;
        }

        void FreeKeptObjectsInfo( )
        {
            if ( BlobsPositions != nullptr )
            {
                free( BlobsPositions );
                BlobsPositions = nullptr;
            }
            if ( BlobsSizes != nullptr )
            {
                free( BlobsAreas );
                BlobsAreas = nullptr;
            }
            if ( BlobsAreas != nullptr )
            {
                free( BlobsAreas );
                BlobsAreas = nullptr;
            }

            KeptObjectsAllocated = 0;
            KeptObjectsInfoReady = false;
        }

    };
}

// Supported pixel formats of input/output images
const XPixelFormat FilterBlobsBySizePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

FilterBlobsBySizePlugin::FilterBlobsBySizePlugin( ) :
    mData( new Private::FilterBlobsBySizePluginData )
{
}

FilterBlobsBySizePlugin::~FilterBlobsBySizePlugin( )
{
    delete mData;
}

void FilterBlobsBySizePlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool FilterBlobsBySizePlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode FilterBlobsBySizePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode FilterBlobsBySizePlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode FilterBlobsBySizePlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    mData->KeptObjectsInfoReady = false;
    mData->BlobsRemoved         = 0;
    mData->BlobsLeft            = 0;

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscale32, &mData->MapImage );

        if ( ret == SuccessCode )
        {
            uint32_t objectsCount = 0;

            ret = mData->AllocateLabelsMap( ( ( src->width / 2 ) + 1 ) * ( ( src->height / 2 ) + 1 ) + 1 );

            if ( ret == SuccessCode )
            {
                ret = BcBuildObjectsMap( src, mData->MapImage, &objectsCount, mData->TempLabelsMap, mData->TempLabelsMapSize );
            }

            if ( ( ret == SuccessCode ) && ( objectsCount != 0 ) )
            {
                ret = mData->AllocateObjectsInfo( objectsCount );

                if ( ret == SuccessCode )
                {
                    ret = BcGetObjectsRectanglesAndArea( mData->MapImage, objectsCount, mData->Rectangles, mData->Areas );

                    if ( ret == SuccessCode )
                    {
                        ret = ( mData->SizeCriteria == SIZE_CRITERIA_RECTANGLE ) ?
                                BcBuildFillMapOutOfSizeObjects( mData->MinWidth, mData->MinHeight, mData->MaxWidth, mData->MaxHeight,
                                                                mData->CoupledFiltering, objectsCount, mData->Rectangles, mData->FillMap, &mData->BlobsLeft ) :
                                BcBuildFillMapOutOfAreaObjects( mData->MinArea, mData->MaxArea, objectsCount, mData->Areas, mData->FillMap, &mData->BlobsLeft );

                        if ( ret == SuccessCode )
                        {
                            mData->BlobsRemoved = objectsCount - mData->BlobsLeft;

                            if ( mData->PerformImageFiltering )
                            {
                                ret = BcFillObjects( src, mData->MapImage, mData->FillMap, { 0xFF000000 } );
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
XErrorCode FilterBlobsBySizePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_Bool;
        value->value.boolVal = mData->PerformImageFiltering;
        break;

    case 1:
        value->type = XVT_U1;
        value->value.ubVal = mData->SizeCriteria;
        break;

    case 2:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinWidth;
        break;

    case 3:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinHeight;
        break;

    case 4:
        value->type = XVT_U4;
        value->value.uiVal = mData->MaxWidth;
        break;

    case 5:
        value->type = XVT_U4;
        value->value.uiVal = mData->MaxHeight;
        break;

    case 6:
        value->type = XVT_Bool;
        value->value.boolVal = mData->CoupledFiltering;
        break;

    case 7:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinArea;
        break;

    case 8:
        value->type = XVT_U4;
        value->value.uiVal = mData->MaxArea;
        break;

    case 9:
        value->type = XVT_U4;
        value->value.uiVal = mData->BlobsLeft;
        break;

    case 10:
        value->type = XVT_U4;
        value->value.uiVal = mData->BlobsRemoved;
        break;

    case 11:
    case 12:
    case 13:
        ret = mData->PrepareKeptObjectsInfo( );

        if ( ret == SuccessCode )
        {
            xarray*  array = nullptr;
            xvariant v;
            uint32_t i;

            switch ( id )
            {
            case 11:
                ret = XArrayAllocate( &array, XVT_Point, mData->BlobsLeft );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_Point;

                    for ( i = 0; i < mData->BlobsLeft; i++ )
                    {
                        v.value.pointVal = mData->BlobsPositions[i];
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_Point | XVT_Array;
                    value->value.arrayVal = array;
                }
                break;

            case 12:
                ret = XArrayAllocate( &array, XVT_Size, mData->BlobsLeft );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_Size;

                    for ( i = 0; i < mData->BlobsLeft; i++ )
                    {
                        v.value.sizeVal = mData->BlobsSizes[i];
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_Size | XVT_Array;
                    value->value.arrayVal = array;
                }
                break;

            case 13:
                ret = XArrayAllocate( &array, XVT_U4, mData->BlobsLeft );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_U4;

                    for ( i = 0; i < mData->BlobsLeft; i++ )
                    {
                        v.value.uiVal = mData->BlobsAreas[i];
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_U4 | XVT_Array;
                    value->value.arrayVal = array;
                }
                break;
            }
        }
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode FilterBlobsBySizePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 14, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->PerformImageFiltering = convertedValue.value.boolVal;
            break;

        case 1:
            mData->SizeCriteria = convertedValue.value.ubVal;
            break;

        case 2:
            mData->MinWidth = XINRANGE( convertedValue.value.uiVal, 1, 10000 );
            break;

        case 3:
            mData->MinHeight = XINRANGE( convertedValue.value.uiVal, 1, 10000 );
            break;

        case 4:
            mData->MaxWidth = XINRANGE( convertedValue.value.uiVal, 1, 10000 );
            break;

        case 5:
            mData->MaxHeight = XINRANGE( convertedValue.value.uiVal, 1, 10000 );
            break;

        case 6:
            mData->CoupledFiltering = convertedValue.value.boolVal;
            break;

        case 7:
            mData->MinArea = XINRANGE( convertedValue.value.uiVal, 1, 100000000 );
            break;

        case 8:
            mData->MaxArea = XINRANGE( convertedValue.value.uiVal, 1, 100000000 );
            break;

        default:
            ret = ErrorReadOnlyProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}

// Get individual values of blobs' position/size/area
XErrorCode FilterBlobsBySizePlugin::GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 11:
    case 12:
    case 13:
        ret = mData->PrepareKeptObjectsInfo( );

        if ( ret == SuccessCode )
        {
            if ( ( index >= mData->BlobsLeft ) || ( index >= mData->KeptObjectsAllocated ) )
            {
                ret = ErrorIndexOutOfBounds;
            }
            else
            {
                switch ( id )
                {
                case 11:
                    value->type = XVT_Point;
                    value->value.pointVal = mData->BlobsPositions[index];
                    break;

                case 12:
                    value->type = XVT_Size;
                    value->value.sizeVal = mData->BlobsSizes[index];
                    break;

                case 13:
                    value->type = XVT_Size;
                    value->value.uiVal = mData->BlobsAreas[index];
                    break;
                }
            }
        }

        break;
    default:

        ret = ( ( id >= 0 ) && ( id < 11 ) ) ? ErrorNotIndexedProperty : ErrorInvalidProperty;
        break;
    }

    return ret;
}
