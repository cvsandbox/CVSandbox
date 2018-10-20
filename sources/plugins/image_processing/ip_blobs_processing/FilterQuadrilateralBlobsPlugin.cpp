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
#include "FilterQuadrilateralBlobsPlugin.hpp"

#include <math.h>

// Support types of quadrilaterals
enum class QuadType
{
    All        = 0,
    Rectangles = 1,
    Squares    = 2
};

namespace Private
{
    class FilterQuadrilateralBlobsPluginData
    {
    public:
        bool      PerformImageFiltering;
        QuadType  QuadsToKeep;
        uint32_t  MinSize;
        uint32_t  MaxSize;
        float     RelDistortionLimit;
        float     MinAcceptableDistortion;
        float     MaxAngleError;
        float     MaxLengthError;

        uint32_t  ObjectsFound;
        ximage*   MapImage;
        xrect*    Rectangles;
        xpoint*   Quads;
        uint8_t*  FillMap;
        uint32_t  ObjectsCountAllocated;

        uint32_t* TempLabelsMap;
        uint32_t  TempLabelsMapSize;

        xpoint*   BlobEdgePoints;
        uint32_t  AllocatedEdgePointsCount;

        //
        uint32_t  QuadrilateralsFound;
        uint32_t  KeptObjectsAllocated;
        xpoint*   KeptQuads;
        bool      KeptObjectsInfoReady;

    public:
        FilterQuadrilateralBlobsPluginData( ) :
            PerformImageFiltering( true ), QuadsToKeep( QuadType::All ), MinSize( 10 ), MaxSize( 10000 ),
            RelDistortionLimit( 5.0f ), MinAcceptableDistortion( 0.5f ),
            MaxAngleError( 7 ), MaxLengthError( 15 ),
            ObjectsFound( 0 ), MapImage( nullptr ), Rectangles( nullptr ), Quads( nullptr ),
            FillMap( nullptr ), ObjectsCountAllocated( 0 ),
            TempLabelsMap( nullptr ), TempLabelsMapSize( 0 ), BlobEdgePoints( nullptr ),
            AllocatedEdgePointsCount( 0 ), QuadrilateralsFound( 0 ), KeptObjectsAllocated( 0 ), KeptQuads( nullptr ),
            KeptObjectsInfoReady( false )
        {

        }

        ~FilterQuadrilateralBlobsPluginData( )
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
                Rectangles = (xrect*)   malloc( objectsCount * sizeof( xrect ) );
                Quads      = (xpoint*)  malloc( objectsCount * 4 * sizeof( xpoint ) );
                FillMap    = (uint8_t*) malloc( ( objectsCount + 1 ) * sizeof( uint8_t ) );

                if ( ( Rectangles == nullptr ) || ( Quads == nullptr ) || ( FillMap == nullptr ) )
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
            if ( Quads != nullptr )
            {
                free( Quads );
                Quads = nullptr;
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
                if ( QuadrilateralsFound == 0 )
                {
                    KeptObjectsInfoReady = true;
                }
                else
                {
                    ret = AllocateKeptObjectsInfo( );

                    if ( ret == SuccessCode )
                    {
                        uint32_t i, blobId, si, di;

                        for ( i = 0, si = 0, di = 0, blobId = 1; i < ObjectsFound; i++, blobId++, si += 4 )
                        {
                            if ( FillMap[blobId] == 0 )
                            {
                                KeptQuads[di]     = Quads[si];
                                KeptQuads[di + 1] = Quads[si + 1];
                                KeptQuads[di + 2] = Quads[si + 2];
                                KeptQuads[di + 3] = Quads[si + 3];
                                di += 4;
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

            if ( KeptObjectsAllocated < QuadrilateralsFound )
            {
                FreeKeptObjectsInfo( );
            }

            if ( KeptObjectsAllocated == 0 )
            {
                KeptQuads = (xpoint*) malloc( 4 * QuadrilateralsFound * sizeof( xpoint ) );

                if ( KeptQuads == nullptr )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    KeptObjectsAllocated = QuadrilateralsFound;
                }
            }

            return ret;
        }

        void FreeKeptObjectsInfo( )
        {
            if ( KeptQuads != nullptr )
            {
                free( KeptQuads );
                KeptQuads = nullptr;
            }

            KeptObjectsAllocated = 0;
            KeptObjectsInfoReady = false;
        }
    };
}

// Supported pixel formats of input/output images
const XPixelFormat FilterQuadrilateralBlobsPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

FilterQuadrilateralBlobsPlugin::FilterQuadrilateralBlobsPlugin( ) :
    mData( new Private::FilterQuadrilateralBlobsPluginData )
{
}

FilterQuadrilateralBlobsPlugin::~FilterQuadrilateralBlobsPlugin( )
{
    delete mData;
}

void FilterQuadrilateralBlobsPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool FilterQuadrilateralBlobsPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode FilterQuadrilateralBlobsPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode FilterQuadrilateralBlobsPlugin::ProcessImage( const ximage* src, ximage** dst )
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

// Calculate angle between the two specified lines
static float AngleBetweenLines( xpoint line1Start, xpoint line1End, xpoint line2Start, xpoint line2End )
{
    bool  line1IsVertical = true;
    bool  line2IsVertical = true;
    float k1 = 0, k2 = 0;
    float angle = 0.0f;

    if ( line1Start.x != line1End.x )
    {
        line1IsVertical = false;
        k1 = static_cast<float>( line1End.y - line1Start.y ) / ( line1End.x - line1Start.x );
    }

    if ( line2Start.x != line2End.x )
    {
        line2IsVertical = false;
        k2 = static_cast<float>( line2End.y - line2Start.y ) / ( line2End.x - line2Start.x );
    }

    if ( ( ( !line1IsVertical ) || ( !line2IsVertical ) ) && ( k1 != k2 ) )
    {
        if ( ( !line1IsVertical ) && ( !line2IsVertical ) )
        {
            angle = static_cast<float>( atan( ( ( k2 > k1 ) ? ( k2 - k1 ) : ( k1 - k2 ) ) / ( 1.0f + k1 * k2 ) ) );
        }
        else
        {
            if ( line1IsVertical )
            {
                angle = static_cast<float>( XPIHalf - atan( ( k2 > 0 ) ? k2 : -k2 ) );
            }
            else
            {
                angle = static_cast<float>( XPIHalf - atan( ( k1 > 0 ) ? k1 : -k1 ) );
            }
        }

        angle *= static_cast<float>( 180.0f / XPI );

        if ( angle < 0 )
        {
            angle = -angle;
        }
    }

    return angle;
}

// Process the specified source image by changing it
XErrorCode FilterQuadrilateralBlobsPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    mData->ObjectsFound         = 0;
    mData->QuadrilateralsFound  = 0;
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
                    uint32_t minSize2 = mData->MinSize * mData->MinSize;

                    mData->FillMap[0] = 0;
                    for ( i = 0, blobId = 1; i < mData->ObjectsFound; i++, blobId++ )
                    {
                        xrect    blobRect   = mData->Rectangles[i];
                        uint32_t blobWidth  = blobRect.x2 - blobRect.x1 + 1;
                        uint32_t blobHeight = blobRect.y2 - blobRect.y1 + 1;
                        uint8_t  removeIt   = 1;

                        if ( ( blobWidth >= mData->MinSize ) && ( blobHeight >= mData->MinSize ) &&
                             ( blobWidth <= mData->MaxSize ) && ( blobHeight <= mData->MaxSize ) )
                        {
                            uint32_t edgePointsCount;
                            xpoint*  quad = &( mData->Quads[i * 4] );

                            // get edge points of the object
                            BcGetObjectEdgePoints( mData->MapImage, blobId, blobRect, mData->AllocatedEdgePointsCount,
                                                   mData->BlobEdgePoints, &edgePointsCount, nullptr );

                            if ( CheckPointsFitQuadrilateral( mData->BlobEdgePoints, edgePointsCount, blobRect,
                                                              mData->RelDistortionLimit / 100.0f, mData->MinAcceptableDistortion,
                                                              quad ) == SuccessCode )
                            {
                                uint32_t qi = 0;
                                float    sideLength[4];

                                for ( ; qi < 4; qi++ )
                                {
                                    uint32_t next  = ( qi < 3 ) ? qi + 1 : 0;
                                    uint32_t dx    = quad[qi].x - quad[next].x;
                                    uint32_t dy    = quad[qi].y - quad[next].y;
                                    uint32_t dist2 = dx * dx + dy * dy;

                                    sideLength[qi] = static_cast<float>( dist2 );

                                    if ( dist2 < minSize2 )
                                    {
                                        break;
                                    }
                                }

                                if ( qi == 4 )
                                {
                                    // all sides' length are fine - keeping it for now

                                    if ( mData->QuadsToKeep == QuadType::All )
                                    {
                                        removeIt = 0;
                                    }
                                    else
                                    {
                                        // make sure we have at least rectangle

                                        // get angles between 2 pairs of opposite sides
                                        float angleBetween1stPair = AngleBetweenLines( quad[0], quad[1], quad[2], quad[3] );
                                        float angleBetween2ndPair = AngleBetweenLines( quad[1], quad[2], quad[3], quad[0] );

                                        if ( ( angleBetween1stPair <= mData->MaxAngleError ) && ( angleBetween2ndPair <= mData->MaxAngleError ) )
                                        {
                                            // we have parallelogram now, but need rectangle
                                            float angleDiff = AngleBetweenLines( quad[1], quad[0], quad[1], quad[2] ) - 90.f;

                                            if ( angleDiff < 0.0f ) { angleDiff = -angleDiff; }

                                            if ( angleDiff <= mData->MaxAngleError )
                                            {
                                                // rectangle at least
                                                if ( mData->QuadsToKeep == QuadType::Rectangles )
                                                {
                                                    removeIt = 0;
                                                }
                                                else
                                                {
                                                    // further check to see if it is a square
                                                    float avgSideLength = 0.0f;

                                                    // get sides' length, not the squared value
                                                    for ( qi = 0; qi < 4; qi++ )
                                                    {
                                                        sideLength[qi] = static_cast<float>( sqrt( sideLength[qi] ) );
                                                        avgSideLength += sideLength[qi];
                                                    }

                                                    avgSideLength /= 4;

                                                    float maxLengthDiff = ( mData->MaxLengthError / 100.0f ) * avgSideLength;
                                                    float lengthDiff1   = sideLength[0] - sideLength[1];
                                                    float lengthDiff2   = sideLength[2] - sideLength[3];

                                                    if ( lengthDiff1 < 0 ) { lengthDiff1 = -lengthDiff1; }
                                                    if ( lengthDiff2 < 0 ) { lengthDiff2 = -lengthDiff2; }

                                                    if ( ( lengthDiff1 <= maxLengthDiff ) || ( lengthDiff2 <= maxLengthDiff ) )
                                                    {
                                                        removeIt = 0;
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        mData->QuadrilateralsFound += ( 1 - removeIt );
                        mData->FillMap[blobId] = removeIt;
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
XErrorCode FilterQuadrilateralBlobsPlugin::GetProperty( int32_t id, xvariant* value ) const
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
        value->value.ubVal = static_cast<uint8_t>( mData->QuadsToKeep );
        break;

    case 2:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinSize;
        break;

    case 3:
        value->type = XVT_U4;
        value->value.uiVal = mData->MaxSize;
        break;

    case 4:
        value->type = XVT_R4;
        value->value.fVal = mData->RelDistortionLimit;
        break;

    case 5:
        value->type = XVT_R4;
        value->value.fVal = mData->MinAcceptableDistortion;
        break;

    case 6:
        value->type = XVT_R4;
        value->value.fVal = mData->MaxAngleError;
        break;

    case 7:
        value->type = XVT_R4;
        value->value.fVal = mData->MaxLengthError;
        break;

    case 8:
        value->type = XVT_U4;
        value->value.uiVal = mData->QuadrilateralsFound;
        break;

    case 9:
        ret = mData->PrepareKeptObjectsInfo( );

        if ( ret == SuccessCode )
        {
            xarrayJagged* array = nullptr;
            uint32_t      i;

            ret = XArrayAllocateJagged( &array, XVT_Point, mData->QuadrilateralsFound );
            if ( ret == SuccessCode )
            {
                for ( i = 0; i < mData->QuadrilateralsFound; i++ )
                {
                    xpoint*  quad = &( mData->KeptQuads[i * 4] );
                    xvariant pointVar;
                    int      j;

                    ret = XArrayAllocateJaggedSub( array, i, 4 );

                    if ( ret != SuccessCode )
                    {
                        break;
                    }
                    else
                    {
                        for ( j = 0; j < 4; j++ )
                        {
                            pointVar.type           = XVT_Point;
                            pointVar.value.pointVal = quad[j];

                            XArraySetJagged( array, i, j, &pointVar );
                        }
                    }
                }

                if ( ret != SuccessCode )
                {
                    // free the main array if any of the inner array failed to create
                    XArrayFreeJagged( &array );
                }
                else
                {
                    value->type = XVT_Point | XVT_ArrayJagged;
                    value->value.arrayJaggedVal = array;
                }
            }
        }
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode FilterQuadrilateralBlobsPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 10, &convertedValue );

    if ( ( ret == SuccessCode ) && ( id >= 8 ) )
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
            mData->QuadsToKeep = static_cast<QuadType>( convertedValue.value.ubVal );
            break;

        case 2:
            mData->MinSize = XINRANGE( convertedValue.value.uiVal, 5, 10000 );
            break;

        case 3:
            mData->MaxSize = XINRANGE( convertedValue.value.uiVal, 5, 10000 );
            break;

        case 4:
            mData->RelDistortionLimit = XINRANGE( convertedValue.value.fVal, 1.0f, 50.0f );
            break;

        case 5:
            mData->MinAcceptableDistortion = XINRANGE( convertedValue.value.fVal, 0.05f, 10.0f );
            break;

        case 6:
            mData->MaxAngleError = XINRANGE( convertedValue.value.fVal, 0.0f, 20.0f );
            break;

        case 7:
            mData->MaxLengthError = XINRANGE( convertedValue.value.fVal, 0.0f, 50.0f );
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
XErrorCode FilterQuadrilateralBlobsPlugin::GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 9:
        ret = mData->PrepareKeptObjectsInfo( );

        if ( ret == SuccessCode )
        {
            if ( index >= mData->QuadrilateralsFound )
            {
                ret = ErrorIndexOutOfBounds;
            }
            else
            {
                xpoint*  quad  = &( mData->KeptQuads[index * 4] );
                xarray*  array = nullptr;
                xvariant v;
                uint32_t i;

                ret = XArrayAllocate( &array, XVT_Point, 4 );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_Point;

                    for ( i = 0; i < 4; i++ )
                    {
                        v.value.pointVal = quad[i];
                        XArrayMove( array, i, &v );
                    }

                    value->type = XVT_Point | XVT_Array;
                    value->value.arrayVal = array;
                }
            }
        }
        break;

    default:
        ret = ( ( id >= 0 ) && ( id < 9 ) ) ? ErrorNotIndexedProperty : ErrorInvalidProperty;
        break;
    }

    return ret;
}
