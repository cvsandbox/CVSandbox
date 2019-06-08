/*
    Motion detection plug-ins for Computer Vision Sandbox

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

#include "TwoFramesDifferenceDetectionPlugin.hpp"
#include <ximaging.h>

using namespace std;

// List of supported pixel formats
const XPixelFormat TwoFramesDifferenceDetectionPlugin::supportedPixelFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24
};

namespace Private
{
    // Internals of the plug-in
    class TwoFramesDifferenceDetectionPluginData
    {
    public:
        ximage*     PreviousFrame;
        ximage*     DiffImage;

        int16_t     PixelThreshold;
        float       MotionThreshold;
        float       MotionLevel;

        xargb       HiColor;
        xargb       LowColor;
        bool        HighlightMotion;
        float       HighlightAmount;

    public:
        TwoFramesDifferenceDetectionPluginData( ) :
            PreviousFrame( nullptr ), DiffImage( nullptr ),
            PixelThreshold( 60 ), MotionThreshold( 0.1f ), MotionLevel( 0.0f ),
            HiColor( { 0xFFFF0000 } ), LowColor( { 0xFF000000 } ),
            HighlightMotion( false ), HighlightAmount( 0.3f )
        { }
    };
}

TwoFramesDifferenceDetectionPlugin::TwoFramesDifferenceDetectionPlugin( ) :
    mData( new ::Private::TwoFramesDifferenceDetectionPluginData( ) )
{
}

TwoFramesDifferenceDetectionPlugin::~TwoFramesDifferenceDetectionPlugin( )
{
    delete mData;
}

void TwoFramesDifferenceDetectionPlugin::Dispose( )
{
    Reset( );
    delete this;
}

// Get specified property value of the plug-in
XErrorCode TwoFramesDifferenceDetectionPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type       = XVT_I2;
        value->value.sVal = mData->PixelThreshold;
        break;

    case 1:
        value->type       = XVT_R4;
        value->value.fVal = mData->MotionThreshold;
        break;

    case 2:
        value->type       = XVT_R4;
        value->value.fVal = mData->MotionLevel;
        break;

    case 3:
        value->type          = XVT_Bool;
        value->value.boolVal = mData->HighlightMotion;
        break;

    case 4:
        value->type          = XVT_ARGB;
        value->value.argbVal = mData->HiColor;
        break;

    case 5:
        value->type        = XVT_U2;
        value->value.usVal = static_cast<uint16_t>( mData->HighlightAmount * 100 );
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode TwoFramesDifferenceDetectionPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode  ret = ErrorFailed;
    xvariant    convertedValue;

    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 6, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            mData->PixelThreshold = convertedValue.value.sVal;
            break;

        case 1:
            mData->MotionThreshold = convertedValue.value.fVal;
            break;

        case 2:
            ret = ErrorReadOnlyProperty;
            break;

        case 3:
            mData->HighlightMotion = convertedValue.value.boolVal;
            break;

        case 4:
            mData->HiColor = convertedValue.value.argbVal;
            break;

        case 5:
            mData->HighlightAmount = static_cast<float>( XINRANGE( convertedValue.value.usVal, 10, 100 ) ) / 100.0f;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}

// Check if the plug-in does changes to input video frames or not
bool TwoFramesDifferenceDetectionPlugin::IsReadOnlyMode( )
{
    return ( !mData->HighlightMotion );
}

// Get pixel formats supported by the plug-in
XErrorCode TwoFramesDifferenceDetectionPlugin::GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedPixelFormats, XARRAY_SIZE( supportedPixelFormats ), pixelFormats, count );
}

// Process the specified video frame
XErrorCode TwoFramesDifferenceDetectionPlugin::ProcessImage( ximage* src )
{
    XErrorCode ret = ErrorFailed;

    // reset previous detected motion level
    mData->MotionLevel = 0.0f;

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( mData->PreviousFrame == nullptr )
        {
            ret = XImageClone( src, &mData->PreviousFrame );
        }
        else
        {
            if ( ( mData->PreviousFrame->width  != src->width  ) ||
                 ( mData->PreviousFrame->height != src->height ) ||
                 ( mData->PreviousFrame->format != src->format ) )
            {
                // should not happen, but better be ready
                Reset( );
                ret = XImageClone( src, &mData->PreviousFrame );
            }
            else
            {
                ret = XImageClone( src, &mData->DiffImage );

                if ( ret == SuccessCode )
                {
                    uint32_t diffPixels;

                    ret = DiffImagesThresholded( mData->DiffImage, mData->PreviousFrame, mData->PixelThreshold,
                                                 &diffPixels, mData->HiColor, mData->LowColor );

                    if ( ret == SuccessCode )
                    {
                        mData->MotionLevel = ( 100.0f * diffPixels ) / ( src->width * src->height );
                    }
                }

                if ( ret == SuccessCode )
                {
                    // update the previous frame for the next round
                    ret = XImageClone( src, &mData->PreviousFrame );
                }

                if ( ( ret == SuccessCode ) && ( mData->HighlightMotion ) )
                {
                    ret = AddImages( src, mData->DiffImage, mData->HighlightAmount );
                }
            }
        }
    }

    return ret;
}

// Check if the plug-in triggered detection on the last processed image
bool TwoFramesDifferenceDetectionPlugin::Detected( )
{
    return ( mData->MotionLevel >= mData->MotionThreshold );
}

// Reset run time state of the video processing plug-in
void TwoFramesDifferenceDetectionPlugin::Reset( )
{
    XImageFree( &mData->PreviousFrame );
    XImageFree( &mData->DiffImage );

    mData->MotionLevel = 0.0f;
}
