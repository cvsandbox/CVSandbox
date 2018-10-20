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
#include "FilterCircleBlobsPlugin.hpp"

namespace Private
{
    class FilterCircleBlobsPluginData
    {
    public:
        bool      PerformImageFiltering;
        uint32_t  MinRadius;
        uint32_t  MaxRadius;
        float     RelDistortionLimit;
        float     MinAcceptableDistortion;

        uint32_t  ObjectsFound;
        ximage*   MapImage;
        xrect*    Rectangles;
        xpointf*  Centers;
        float*    Radiuses;
        float*    MeanDeviations;
        uint8_t*  FillMap;
        uint32_t  ObjectsCountAllocated;

        uint32_t* TempLabelsMap;
        uint32_t  TempLabelsMapSize;

        xpoint*   BlobEdgePoints;
        uint32_t  AllocatedEdgePointsCount;

        //
        uint32_t  CirclesFound;
        uint32_t  KeptObjectsAllocated;
        xpointf*  KeptObjectsCenters;
        float*    KeptObjectsRadiuses;
        float*    KeptMeanDeviations;
        bool      KeptObjectsInfoReady;

    public:
        FilterCircleBlobsPluginData( ) :
            PerformImageFiltering( true ), MinRadius( 2 ), MaxRadius( 5000 ),
            RelDistortionLimit( 3.0f ), MinAcceptableDistortion( 0.5f ),
            ObjectsFound( 0 ), MapImage( nullptr ), Rectangles( nullptr ), Centers( nullptr ), Radiuses( nullptr ),
            MeanDeviations( nullptr ), FillMap( nullptr ), ObjectsCountAllocated( 0 ),
            TempLabelsMap( nullptr ), TempLabelsMapSize( 0 ),
            BlobEdgePoints( nullptr ), AllocatedEdgePointsCount( 0 ),
            CirclesFound( 0 ), KeptObjectsAllocated( 0 ),
            KeptObjectsCenters( nullptr ), KeptObjectsRadiuses( nullptr ), KeptMeanDeviations( nullptr ),
            KeptObjectsInfoReady( false )
        {

        }

        ~FilterCircleBlobsPluginData( )
        {
            XImageFree( &MapImage );
            FreeObjectsInfo( );
            FreeLabelsMap( );
            FreeEdgePoints( );
            FreeKeptObjectsInfo( );
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
                Rectangles     = (xrect*)   malloc( objectsCount * sizeof( xrect ) );
                Centers        = (xpointf*) malloc( objectsCount * sizeof( xpointf ) );
                Radiuses       = (float*)   malloc( objectsCount * sizeof( float ) );
                MeanDeviations = (float*)   malloc( objectsCount * sizeof( float ) );
                FillMap        = (uint8_t*) malloc( ( objectsCount + 1 ) * sizeof( uint8_t ) );

                if ( ( Rectangles == nullptr ) || ( Centers == nullptr ) || ( Radiuses == nullptr ) || 
                     ( MeanDeviations == nullptr ) || ( FillMap == nullptr ) )
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
            if ( Centers != nullptr )
            {
                free( Centers );
                Centers = nullptr;
            }
            if ( Radiuses != nullptr )
            {
                free( Radiuses );
                Radiuses = nullptr;
            }
            if ( MeanDeviations != nullptr )
            {
                free( MeanDeviations );
                MeanDeviations = nullptr;
            }
            if ( FillMap != nullptr )
            {
                free( FillMap );
                FillMap = nullptr;
            }

            ObjectsCountAllocated = 0;
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

        XErrorCode AllocateEdgePoints( uint32_t maxEdgePointCount )
        {
            XErrorCode ret = SuccessCode;

            if ( AllocatedEdgePointsCount < maxEdgePointCount )
            {
                FreeEdgePoints( );
            }

            if ( BlobEdgePoints == nullptr )
            {
                BlobEdgePoints = (xpoint*) malloc( maxEdgePointCount * sizeof( xpoint ) );
                if ( BlobEdgePoints == nullptr )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    AllocatedEdgePointsCount = maxEdgePointCount;
                }
            }

            return ret;
        }

        void FreeEdgePoints( )
        {
            if ( BlobEdgePoints != nullptr )
            {
                free( BlobEdgePoints );
                BlobEdgePoints = nullptr;
            }
            AllocatedEdgePointsCount = 0;
        }

        XErrorCode PrepareKeptObjectsInfo( )
        {
            XErrorCode ret = SuccessCode;

            if ( !KeptObjectsInfoReady )
            {
                if ( CirclesFound == 0 )
                {
                    KeptObjectsInfoReady = true;
                }
                else
                {
                    ret = AllocateKeptObjectsInfo( );

                    if ( ret == SuccessCode )
                    {
                        uint32_t i, j, blobId;

                        for ( i = 0, j = 0, blobId = 1; i < ObjectsFound; i++, blobId++ )
                        {
                            if ( FillMap[blobId] == 0 )
                            {
                                KeptObjectsCenters[j]  = Centers[i];
                                KeptObjectsRadiuses[j] = Radiuses[i];
                                KeptMeanDeviations[j]  = MeanDeviations[i];
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

            if ( KeptObjectsAllocated < CirclesFound )
            {
                FreeKeptObjectsInfo( );
            }

            if ( KeptObjectsAllocated == 0 )
            {
                KeptObjectsCenters  = (xpointf*) malloc( CirclesFound * sizeof( xpointf ) );
                KeptObjectsRadiuses = (float*)   malloc( CirclesFound * sizeof( float ) );
                KeptMeanDeviations  = (float*)   malloc( CirclesFound * sizeof( float ) );

                if ( ( KeptObjectsCenters == nullptr ) || ( KeptObjectsRadiuses == nullptr ) || ( KeptMeanDeviations == nullptr ) )
                {
                    ret = ErrorOutOfMemory;
                    FreeKeptObjectsInfo( );
                }
                else
                {
                    KeptObjectsAllocated = CirclesFound;
                }
            }

            return ret;
        }

        void FreeKeptObjectsInfo( )
        {
            if ( KeptObjectsCenters != nullptr )
            {
                free( KeptObjectsCenters );
                KeptObjectsCenters = nullptr;
            }
            if ( KeptObjectsRadiuses != nullptr )
            {
                free( KeptObjectsRadiuses );
                KeptObjectsRadiuses = nullptr;
            }
            if ( KeptMeanDeviations != nullptr )
            {
                free( KeptMeanDeviations );
                KeptMeanDeviations = nullptr;
            }

            KeptObjectsAllocated = 0;
            KeptObjectsInfoReady = false;
        }
    };
}

// Supported pixel formats of input/output images
const XPixelFormat FilterCircleBlobsPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

FilterCircleBlobsPlugin::FilterCircleBlobsPlugin( ) :
    mData( new Private::FilterCircleBlobsPluginData )
{
}

FilterCircleBlobsPlugin::~FilterCircleBlobsPlugin( )
{
    delete mData;
}

void FilterCircleBlobsPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool FilterCircleBlobsPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode FilterCircleBlobsPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode FilterCircleBlobsPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode FilterCircleBlobsPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    mData->ObjectsFound         = 0;
    mData->CirclesFound         = 0;
    mData->KeptObjectsInfoReady = false;

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XImageAllocateRaw( src->width, src->height, XPixelFormatGrayscale32, &mData->MapImage );

        // make sure we have memory for extracting objects' edge points
        if ( ret == SuccessCode )
        {
            ret = mData->AllocateEdgePoints( ( src->width + src->height ) * 2 );
        }

        if ( ret == SuccessCode )
        {
            uint32_t maxObjects = ( ( src->width / 2 ) + 1 ) * ( ( src->height / 2 ) + 1 ) + 1;

            ret = mData->AllocateLabelsMap( maxObjects );

            if ( ret == SuccessCode )
            {
                ret = BcBuildObjectsMap( src, mData->MapImage, &mData->ObjectsFound, mData->TempLabelsMap, mData->TempLabelsMapSize );
            }

            if ( ( ret == SuccessCode ) && ( mData->ObjectsFound != 0 ) )
            {
                ret = mData->AllocateObjectsInfo( mData->ObjectsFound );

                if ( ret == SuccessCode )
                {
                    ret = BcGetObjectsRectangles( mData->MapImage, mData->ObjectsFound, mData->Rectangles );
                }

                if ( ret == SuccessCode )
                {
                    uint32_t i, blobId;

                    mData->FillMap[0] = 0;
                    for ( i = 0, blobId = 1; i < mData->ObjectsFound; i++, blobId++ )
                    {
                        xrect    blobRect = mData->Rectangles[i];
                        uint32_t edgePointsCount;

                        // get edge points of the object
                        BcGetObjectEdgePoints( mData->MapImage, blobId, blobRect, mData->AllocatedEdgePointsCount,
                                               mData->BlobEdgePoints, &edgePointsCount, nullptr );

                        if ( ( CheckPointsFitCircle( mData->BlobEdgePoints, edgePointsCount, blobRect,
                                                     mData->RelDistortionLimit / 100.0f, mData->MinAcceptableDistortion,
                                                     &( mData->Centers[i] ), &( mData->Radiuses[i] ), &( mData->MeanDeviations[i] ) ) == SuccessCode ) &&
                             ( mData->Radiuses[i] >= mData->MinRadius ) && ( mData->Radiuses[i] <= mData->MaxRadius ) )
                        {
                            mData->FillMap[blobId] = 0;
                            mData->CirclesFound++;
                        }
                        else
                        {
                            mData->FillMap[blobId] = 1;
                        }
                    }

                    if ( mData->PerformImageFiltering )
                    {
                        ret = BcFillObjects( src, mData->MapImage, mData->FillMap, { 0xFF000000 } );
                    }
                }
            }
        }
    }

    return ret;
}

// Get the specified property value of the plug-in
XErrorCode FilterCircleBlobsPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_Bool;
        value->value.boolVal = mData->PerformImageFiltering;
        break;

    case 1:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinRadius;
        break;

    case 2:
        value->type = XVT_U4;
        value->value.uiVal = mData->MaxRadius;
        break;

    case 3:
        value->type = XVT_R4;
        value->value.fVal = mData->RelDistortionLimit;
        break;

    case 4:
        value->type = XVT_R4;
        value->value.fVal = mData->MinAcceptableDistortion;
        break;

    case 5:
        value->type = XVT_U4;
        value->value.uiVal = mData->CirclesFound;
        break;

    case 6:
    case 7:
    case 8:
        ret = mData->PrepareKeptObjectsInfo( );

        if ( ret == SuccessCode )
        {
            xarray*  array = nullptr;
            xvariant v;
            uint32_t i;

            switch ( id )
            {
            case 6:
                ret = XArrayAllocate( &array, XVT_PointF, mData->CirclesFound );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_PointF;

                    for ( i = 0; i < mData->CirclesFound; i++ )
                    {
                        v.value.fpointVal = mData->KeptObjectsCenters[i];
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_PointF | XVT_Array;
                    value->value.arrayVal = array;
                }
                break;

            case 7:
                ret = XArrayAllocate( &array, XVT_R4, mData->CirclesFound );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_R4;

                    for ( i = 0; i < mData->CirclesFound; i++ )
                    {
                        v.value.fVal = mData->KeptObjectsRadiuses[i];
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_R4 | XVT_Array;
                    value->value.arrayVal = array;
                }
                break;

            case 8:
                ret = XArrayAllocate( &array, XVT_R4, mData->CirclesFound );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_R4;

                    for ( i = 0; i < mData->CirclesFound; i++ )
                    {
                        v.value.fVal = mData->KeptMeanDeviations[i];
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_R4 | XVT_Array;
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
XErrorCode FilterCircleBlobsPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 9, &convertedValue );

    if ( ( ret == SuccessCode ) && ( id >= 5 ) )
    {
        ret = ErrorReadOnlyProperty;
    }

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->PerformImageFiltering = convertedValue.value.boolVal;
            break;

        case 1:
            mData->MinRadius = XINRANGE( convertedValue.value.uiVal, 2, 5000 );
            break;

        case 2:
            mData->MaxRadius = XINRANGE( convertedValue.value.uiVal, 2, 5000 );
            break;

        case 3:
            mData->RelDistortionLimit = XINRANGE( convertedValue.value.fVal, 1.0f, 50.0f );
            break;

        case 4:
            mData->MinAcceptableDistortion = XINRANGE( convertedValue.value.fVal, 0.05f, 10.0f );
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
XErrorCode FilterCircleBlobsPlugin::GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 6:
    case 7:
    case 8:
        ret = mData->PrepareKeptObjectsInfo( );

        if ( ret == SuccessCode )
        {
            if ( index >= mData->CirclesFound )
            {
                ret = ErrorIndexOutOfBounds;
            }
            else
            {
                switch ( id )
                {
                case 6:
                    value->type = XVT_PointF;
                    value->value.fpointVal = mData->KeptObjectsCenters[index];
                    break;

                case 7:
                    value->type = XVT_R4;
                    value->value.fVal = mData->KeptObjectsRadiuses[index];
                    break;

                case 8:
                    value->type = XVT_R4;
                    value->value.fVal = mData->KeptMeanDeviations[index];
                    break;
                }
            }
        }

        break;
    default:

        ret = ( ( id >= 0 ) && ( id < 6 ) ) ? ErrorNotIndexedProperty : ErrorInvalidProperty;
        break;
    }

    return ret;
}
