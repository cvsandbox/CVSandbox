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
#include "HsvColorFilterPlugin.hpp"

namespace Private
{
    class HsvColorFilterPluginData
    {
    public:
        xhsv  MinValues;
        xhsv  MaxValues;
        bool  FillOutside;
        xargb FillColor;

    public:
        HsvColorFilterPluginData( ) :
            MinValues( { 0, 0.0f, 0.0f } ), MaxValues( { 359, 1.0f, 1.0f } ), 
            FillOutside( true ), FillColor( { 0xFF000000 }  )
        {
        }
    };

}

// Supported pixel formats of input/output images
const XPixelFormat HsvColorFilterPlugin::supportedFormats[] =
{
    XPixelFormatRGB24, XPixelFormatRGBA32
};

HsvColorFilterPlugin::HsvColorFilterPlugin( ) :
    mData( new Private::HsvColorFilterPluginData )
{
}

HsvColorFilterPlugin::~HsvColorFilterPlugin( )
{
    delete mData;
}

void HsvColorFilterPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool HsvColorFilterPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode HsvColorFilterPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode HsvColorFilterPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode HsvColorFilterPlugin::ProcessImageInPlace( ximage* src )
{
    return HsvColorFiltering( src, mData->MinValues, mData->MaxValues, mData->FillOutside, mData->FillColor );
}

// Get specified property value of the plug-in
XErrorCode HsvColorFilterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
        case 0:
            value->type = XVT_Range;
            value->value.rangeVal.min = mData->MinValues.Hue;
            value->value.rangeVal.max = mData->MaxValues.Hue;
            break;

        case 1:
            value->type = XVT_RangeF;
            value->value.frangeVal.min = mData->MinValues.Saturation;
            value->value.frangeVal.max = mData->MaxValues.Saturation;
            break;

        case 2:
            value->type = XVT_RangeF;
            value->value.frangeVal.min = mData->MinValues.Value;
            value->value.frangeVal.max = mData->MaxValues.Value;
            break;

        case 3:
            value->type = XVT_Bool;
            value->value.boolVal = mData->FillOutside;
            break;

        case 4:
            value->type = XVT_ARGB;
            value->value.argbVal = mData->FillColor;
            break;

        default:
            ret = ErrorInvalidProperty;
            break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode HsvColorFilterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    static const xrange  validHueRange = { 0, 359 };
    static const xrangef validSLRange  = { 0.0f, 1.0f };
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 5, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
            case 0:
                XRangeIntersect( &convertedValue.value.rangeVal, &validHueRange );
                mData->MinValues.Hue = static_cast<uint16_t>( convertedValue.value.rangeVal.min );
                mData->MaxValues.Hue = static_cast<uint16_t>( convertedValue.value.rangeVal.max );
                break;

            case 1: 
                XRangeIntersectF( &convertedValue.value.frangeVal, &validSLRange );
                mData->MinValues.Saturation = convertedValue.value.frangeVal.min;
                mData->MaxValues.Saturation = convertedValue.value.frangeVal.max;
                break;

            case 2: 
                XRangeIntersectF( &convertedValue.value.frangeVal, &validSLRange );
                mData->MinValues.Value = convertedValue.value.frangeVal.min;
                mData->MaxValues.Value = convertedValue.value.frangeVal.max;
                break;

            case 3:
                mData->FillOutside = convertedValue.value.boolVal;
                break;

            case 4:
                mData->FillColor = convertedValue.value.argbVal;
                break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
