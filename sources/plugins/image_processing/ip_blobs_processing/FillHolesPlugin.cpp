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
#include "FillHolesPlugin.hpp"

namespace Private
{
    class FillHolesPluginData
    {
    public:
        uint8_t   FillCriteria;
        bool      CoupledFiltering;

        uint32_t  MinWidth;
        uint32_t  MinHeight;

        uint32_t  MinArea;
        xargb     FillColor;

        ximage*   MapImage;
        xrect*    Rectangles;
        uint32_t* Areas;
        uint8_t*  FillMap;
        uint32_t  ObjectsCountAllocated;

        uint32_t  HolesFilled;
        uint32_t  HolesLeft;

        uint32_t* TempLabelsMap;
        uint32_t  TempLabelsMapSize;

    public:
        FillHolesPluginData( ) :
            FillCriteria( 0 ), CoupledFiltering( false ),
            MinWidth( 5 ), MinHeight( 5 ), MinArea( 5 ),
            FillColor( { 0xFFFFFFFF } ),
            MapImage( nullptr ),
            Rectangles( nullptr ), Areas( nullptr ), FillMap( nullptr ), ObjectsCountAllocated( 0 ),
            HolesFilled( 0 ), HolesLeft( 0 ),
            TempLabelsMap( nullptr ), TempLabelsMapSize( 0 )
        {
        }

        ~FillHolesPluginData( )
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
    };
}

// Supported pixel formats of input/output images
const XPixelFormat FillHolesPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

FillHolesPlugin::FillHolesPlugin( ) :
    mData( new Private::FillHolesPluginData( ) )
{
}

void FillHolesPlugin::Dispose( )
{
    delete mData;
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool FillHolesPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode FillHolesPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode FillHolesPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode FillHolesPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

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
                ret = BcBuildBackgroundMap( src, mData->MapImage, &objectsCount, mData->TempLabelsMap, mData->TempLabelsMapSize );
            }

            if ( ret == SuccessCode )
            {
                if ( objectsCount == 0 )
                {
                    mData->HolesLeft   = 0;
                    mData->HolesFilled = 0;
                }
                else
                {
                    ret = mData->AllocateObjectsInfo( objectsCount );

                    if ( ret == SuccessCode )
                    {
                        ret = BcGetObjectsRectangles( mData->MapImage, objectsCount, mData->Rectangles );

                        if ( ( ret == SuccessCode ) && ( mData->FillCriteria ) )
                        {
                            ret = BcGetObjectsArea( mData->MapImage, objectsCount, mData->Areas, 0 );
                        }

                        if ( ret == SuccessCode )
                        {
                            switch ( mData->FillCriteria )
                            {
                            case 1:
                                ret = BcBuildFillMapNonEdgeBySizeObjects( src->width, src->height, mData->MinWidth, mData->MinHeight, mData->CoupledFiltering,
                                                                          objectsCount, mData->Rectangles, mData->FillMap, &mData->HolesFilled, &mData->HolesLeft );
                                break;

                            case 2:
                                ret = BcBuildFillMapNonEdgeByAreaObjects( src->width, src->height, mData->MinArea, objectsCount,
                                                                          mData->Rectangles, mData->Areas, mData->FillMap, &mData->HolesFilled, &mData->HolesLeft );
                                break;

                            default:
                                mData->HolesLeft = 0;
                                ret = BcBuildFillMapNonEdgeObjects( src->width, src->height, objectsCount, mData->Rectangles, mData->FillMap, &mData->HolesFilled );
                                break;
                            }

                            if ( ret == SuccessCode )
                            {
                                ret = BcFillObjects( src, mData->MapImage, mData->FillMap, mData->FillColor );
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
XErrorCode FillHolesPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = mData->FillCriteria;
        break;

    case 1:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinWidth;
        break;

    case 2:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinHeight;
        break;

    case 3:
        value->type = XVT_Bool;
        value->value.boolVal = mData->CoupledFiltering;
        break;

    case 4:
        value->type = XVT_U4;
        value->value.uiVal = mData->MinArea;
        break;

    case 5:
        value->type = XVT_ARGB;
        value->value.argbVal = mData->FillColor;
        break;

    case 6:
        value->type = XVT_U4;
        value->value.uiVal = mData->HolesFilled;
        break;

    case 7:
        value->type = XVT_U4;
        value->value.uiVal = mData->HolesLeft;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode FillHolesPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 8, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->FillCriteria = convertedValue.value.ubVal;
            break;

        case 1:
            mData->MinWidth = XINRANGE( convertedValue.value.uiVal, 1, 10000 );
            break;

        case 2:
            mData->MinHeight = XINRANGE( convertedValue.value.uiVal, 1, 10000 );
            break;

        case 3:
            mData->CoupledFiltering = convertedValue.value.boolVal;
            break;

        case 4:
            mData->MinArea = XINRANGE( convertedValue.value.uiVal, 1, 100000000 );
            break;

        case 5:
            mData->FillColor = convertedValue.value.argbVal;
            break;

        default:
            ret = ErrorReadOnlyProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
