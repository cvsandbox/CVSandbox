/*
    Glyphs recognition and tracking plug-ins for Computer Vision Sandbox

    Copyright (C) 2011-2019, cvsandbox
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

#include "GlyphDetectorPlugin.hpp"
#include <xvision.h>

namespace Private
{
    class GlyphDetectorPluginData
    {
    public:
        GlyphDetectionContext* Context;
        bool                   LastProcessingSucceeded;
        uint32_t               GlyphSize;
        uint32_t               MaxGlyphs;

    public:
        GlyphDetectorPluginData( ) :
            Context( nullptr ), LastProcessingSucceeded( false ),
            GlyphSize( 3 ), MaxGlyphs( 3 )
        {
        }

        ~GlyphDetectorPluginData( )
        {
            FreeGlyphDetectionContext( &Context );
        }
    };
};

// Supported pixel formats of input/output images
const XPixelFormat GlyphDetectorPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

GlyphDetectorPlugin::GlyphDetectorPlugin( ) :
    mData( new Private::GlyphDetectorPluginData( ) )
{
}

GlyphDetectorPlugin::~GlyphDetectorPlugin( )
{
    delete mData;
}

void GlyphDetectorPlugin::Dispose( )
{
    delete this;
}

// Provide supported pixel formats
XErrorCode GlyphDetectorPlugin::GetSupportedPixelFormats( XPixelFormat* formats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedFormats, XARRAY_SIZE( supportedFormats ), formats, count );
}

// Process the specified source image by changing it
XErrorCode GlyphDetectorPlugin::ProcessImage( const ximage* image )
{
    XErrorCode ret = FindGlyphs( image, mData->GlyphSize, mData->MaxGlyphs, &mData->Context );

    mData->LastProcessingSucceeded = ( ret == SuccessCode );

    return ret;
}

// Get the specified property value of the plug-in
XErrorCode GlyphDetectorPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    if ( ( id >= 2 ) && ( !mData->LastProcessingSucceeded ) )
    {
        ret = ErrorInitializationFailed;
    }
    else
    {
        switch ( id )
        {
        case 0:
            value->type = XVT_U4;
            value->value.uiVal = mData->GlyphSize;
            break;

        case 1:
            value->type = XVT_U4;
            value->value.uiVal = mData->MaxGlyphs;
            break;

        case 2:
            value->type = XVT_U4;
            value->value.uiVal = mData->Context->DetectedGlyphsCount;
            break;

        case 3:
            {
                xarray*  array = nullptr;
                xvariant v;
                uint32_t i;

                ret = XArrayAllocate( &array, XVT_String, mData->Context->DetectedGlyphsCount );
                if ( ret == SuccessCode )
                {
                    for ( i = 0; i < mData->Context->DetectedGlyphsCount; i++ )
                    {
                        v.type         = XVT_String;
                        v.value.strVal = XStringAlloc( mData->Context->DetectedGlyphs[i].Code );

                        XArrayMove( array, i, &v );
                    }

                    value->type = XVT_String | XVT_Array;
                    value->value.arrayVal = array;
                }
            }
            break;

        case 4:
            {
                xarrayJagged* array = nullptr;
                uint32_t      i;

                ret = XArrayAllocateJagged( &array, XVT_Point, mData->Context->DetectedGlyphsCount );
                if ( ret == SuccessCode )
                {
                    for ( i = 0; i < mData->Context->DetectedGlyphsCount; i++ )
                    {
                        xpoint*  quad = mData->Context->DetectedGlyphs[i].Quadrilateral;
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
                        value->type = XVT_Point |  XVT_ArrayJagged;
                        value->value.arrayJaggedVal = array;
                    }
                }
            }
            break;

        case 5:
            {
                xarray*  array = nullptr;
                xvariant v;
                uint32_t i;
                xrect*   rect;

                ret = XArrayAllocate( &array, XVT_Point, mData->Context->DetectedGlyphsCount );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_Point;

                    for ( i = 0; i < mData->Context->DetectedGlyphsCount; i++ )
                    {
                        rect = &( mData->Context->DetectedGlyphs[i].BoundingRect );

                        v.value.pointVal.x = rect->x1;
                        v.value.pointVal.y = rect->y1;
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_Point | XVT_Array;
                    value->value.arrayVal = array;
                }
            }
            break;

        case 6:
            {
                xarray*  array = nullptr;
                xvariant v;
                uint32_t i;
                xrect*   rect;

                ret = XArrayAllocate( &array, XVT_Size, mData->Context->DetectedGlyphsCount );
                if ( ret == SuccessCode )
                {
                    v.type = XVT_Size;

                    for ( i = 0; i < mData->Context->DetectedGlyphsCount; i++ )
                    {
                        rect = &( mData->Context->DetectedGlyphs[i].BoundingRect );

                        v.value.sizeVal.width  = rect->x2 - rect->x1 + 1;
                        v.value.sizeVal.height = rect->y2 - rect->y1 + 1;
                        XArraySet( array, i, &v );
                    }

                    value->type = XVT_Size | XVT_Array;
                    value->value.arrayVal = array;
                }
            }
            break;

        default:
            ret = ErrorInvalidProperty;
        }
    }

    return ret;
}

// Set the specified property value of the plug-in
XErrorCode GlyphDetectorPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 7, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->GlyphSize = XINRANGE( convertedValue.value.uiVal, 2, 10 );
            break;

        case 1:
            mData->MaxGlyphs = XINRANGE( convertedValue.value.uiVal, 1, 10 );
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
XErrorCode GlyphDetectorPlugin::GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    if ( id < 3 )
    {
        ret = ErrorNotIndexedProperty;
    }
    else if ( id > 6 )
    {
        ret = ErrorInvalidProperty;
    }
    else if ( !mData->LastProcessingSucceeded )
    {
        ret = ErrorInitializationFailed;
    }
    else if ( index >= mData->Context->DetectedGlyphsCount )
    {
        ret = ErrorIndexOutOfBounds;
    }
    else
    {
        switch ( id )
        {
        case 3:
            value->type         = XVT_String;
            value->value.strVal = XStringAlloc( mData->Context->DetectedGlyphs[index].Code );
            break;

        case 4:
            {
                xpoint*  quad  = mData->Context->DetectedGlyphs[index].Quadrilateral;
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
            break;

        case 5:
            value->type = XVT_Point;
            value->value.pointVal.x = mData->Context->DetectedGlyphs[index].BoundingRect.x1;
            value->value.pointVal.y = mData->Context->DetectedGlyphs[index].BoundingRect.y1;
            break;

        case 6:
            {
                xrect* rect = &( mData->Context->DetectedGlyphs[index].BoundingRect );

                value->type = XVT_Size;
                value->value.sizeVal.width  = rect->x2 - rect->x1 + 1;
                value->value.sizeVal.height = rect->y2 - rect->y1 + 1;
            }
            break;
        }
    }

    return ret;
}
