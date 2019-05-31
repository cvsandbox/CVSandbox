/*
    Image processing tools plug-ins of Computer Vision Sandbox

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
#include "PutTextPlugin.hpp"

using namespace std;

// Supported pixel formats of input/output images
const XPixelFormat PutTextPlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

PutTextPlugin::PutTextPlugin( ) :
    line1( ), line2( ), line3( ), line4( ), line5( ),
    alignment( 0 ), alignToCorners( false )
{
    coordinates.x = 0;
    coordinates.y = 0;

    textColor.argb = 0xFFFFFFFF;
    bgColor.argb   = 0xFF000000;
}

void PutTextPlugin::Dispose( )
{
    delete this;
}

// The plug-in can process image in-place without creating new image as a result
bool PutTextPlugin::CanProcessInPlace( )
{
    return true;
}

// Provide supported pixel formats
XErrorCode PutTextPlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode PutTextPlugin::ProcessImage( const ximage* src, ximage** dst )
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
XErrorCode PutTextPlugin::ProcessImageInPlace( ximage* src )
{
    XErrorCode ret = SuccessCode;

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int32_t x1 = coordinates.x, x2 = coordinates.x, x3 = coordinates.x, x4 = coordinates.x, x5 = coordinates.x;
        int32_t y1 = coordinates.y, y2 = coordinates.y + 10, y3 = coordinates.y + 20, y4 = coordinates.y + 30, y5 = coordinates.y + 40;

        if ( alignToCorners )
        {
            if ( ( alignment == 0 ) || ( alignment == 3 ) )
            {
                x1 = x2 = x3 = x4 = x5 = 0;
            }
            else
            {
                x1 = x2 = x3 = x4 = x5 = src->width;
            }

            if ( ( alignment == 0 ) || ( alignment == 1 ) )
            {
                y1 = 0; y2 = 10; y3 = 20; y4 = 30; y5 = 40;
            }
            else
            {
                // will be corrected later
                y1 = src->height;
                y2 = y1 + 10;
                y3 = y1 + 20;
                y4 = y1 + 30;
                y5=  y1 + 40;
            }
        }

        // 0 - top-left (default), 1 - top-right, 2 - bottom-right, 3 - bottom-left

        if ( ( alignment == 2 ) || ( alignment == 3 ) )
        {
            y1 -= 10;
            y2 -= 30;
            y3 -= 50;
            y4 -= 70;
            y5 -= 90;
        }

        if ( ( alignment == 1 ) || ( alignment == 2 ) )
        {
            x1 -= ( static_cast<int32_t>( line1.length( ) ) * 8 + 2 );
            x2 -= ( static_cast<int32_t>( line2.length( ) ) * 8 + 2 );
            x3 -= ( static_cast<int32_t>( line3.length( ) ) * 8 + 2 );
            x4 -= ( static_cast<int32_t>( line4.length( ) ) * 8 + 2 );
            x5 -= ( static_cast<int32_t>( line5.length( ) ) * 8 + 2 );
        }

        ret =
        XDrawingText( src, line1.c_str( ), x1, y1, textColor, bgColor, true );
        XDrawingText( src, line2.c_str( ), x2, y2, textColor, bgColor, true );
        XDrawingText( src, line3.c_str( ), x3, y3, textColor, bgColor, true );
        XDrawingText( src, line4.c_str( ), x4, y4, textColor, bgColor, true );
        XDrawingText( src, line5.c_str( ), x5, y5, textColor, bgColor, true );
    }

    return ret;
}

// Get specified property value of the plug-in
XErrorCode PutTextPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( line1.c_str( ) );
        break;

    case 1:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( line2.c_str( ) );
        break;

    case 2:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( line3.c_str( ) );
        break;

    case 3:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( line4.c_str( ) );
        break;

    case 4:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( line5.c_str( ) );
        break;

    case 5:
        value->type          = XVT_ARGB;
        value->value.argbVal = textColor;
        break;

    case 6:
        value->type          = XVT_ARGB;
        value->value.argbVal = bgColor;
        break;

    case 7:
        value->type        = XVT_U1;
        value->value.ubVal = alignment;
        break;

    case 8:
        value->type          = XVT_Bool;
        value->value.boolVal = alignToCorners;
        break;

    case 9:
        value->type           = XVT_Point;
        value->value.pointVal = coordinates;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode PutTextPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 10, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            line1 = string( convertedValue.value.strVal );
            break;

        case 1:
            line2 = string( convertedValue.value.strVal );
            break;

        case 2:
            line3 = string( convertedValue.value.strVal );
            break;

        case 3:
            line4 = string( convertedValue.value.strVal );
            break;

        case 4:
            line5 = string( convertedValue.value.strVal );
            break;

        case 5:
            textColor = convertedValue.value.argbVal;
            break;

        case 6:
            bgColor = convertedValue.value.argbVal;
            break;

        case 7:
            printf( "alignment = %d (%d) \n", convertedValue.value.ubVal, value->value.ubVal );
            alignment = convertedValue.value.ubVal;
            break;

        case 8:
            alignToCorners = convertedValue.value.boolVal;
            break;

        case 9:
            coordinates = convertedValue.value.pointVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
