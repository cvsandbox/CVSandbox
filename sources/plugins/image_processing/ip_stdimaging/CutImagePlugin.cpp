/*
    Standard image processing plug-ins of Computer Vision Sandbox

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

#include <ximage.h>
#include "CutImagePlugin.hpp"

enum CutTpes
{
    RelativeEdgesGap,
    AbsoluteEdgesGap,
    AbsoluteCoordinates
};

// Supported pixel formats of input/output images
const XPixelFormat CutImagePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32,
    XPixelFormatGrayscale16, XPixelFormatRGB48, XPixelFormatRGBA64
};

CutImagePlugin::CutImagePlugin( ) :
    cutType( RelativeEdgesGap ),
    relLeftGap( 0.1f ), relTopGap( 0.1f ), relRightGap( 0.1f ), relBottomGap( 0.1f ),
    leftGap( 10 ), topGap( 10 ), rightGap( 10 ), bottomGap( 10 ),
    cutX( 0 ), cutY( 0 ), cutWidth( 200 ), cutHeight( 200 )
{
}

void CutImagePlugin::Dispose( )
{
    delete this;
}

// The plug-in cannot process image in-place since it changes theis size
bool CutImagePlugin::CanProcessInPlace( )
{
    return false;
}

// Provide supported pixel formats
XErrorCode CutImagePlugin::GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count )
{
    return GetPixelFormatTranslationsImpl( inputFormats, outputFormats, count, supportedFormats, supportedFormats,
        sizeof( supportedFormats ) / sizeof( XPixelFormat ) );
}

// Process the specified source image and return new as a result
XErrorCode CutImagePlugin::ProcessImage( const ximage* src, ximage** dst )
{
    XErrorCode ret      = SuccessCode;
    ximage*    subImage = nullptr;

    if ( ( src == nullptr ) || ( dst == nullptr ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        switch ( cutType )
        {
        case AbsoluteCoordinates:
            ret = XImageGetSubImage( src, &subImage, cutX, cutY, cutWidth, cutHeight );
            break;

        default:
            {
                int32_t left   = ( cutType == AbsoluteEdgesGap ) ? leftGap   : static_cast<int32_t>( relLeftGap   * src->width );
                int32_t right  = ( cutType == AbsoluteEdgesGap ) ? rightGap  : static_cast<int32_t>( relRightGap  * src->width );
                int32_t top    = ( cutType == AbsoluteEdgesGap ) ? topGap    : static_cast<int32_t>( relTopGap    * src->height );
                int32_t bottom = ( cutType == AbsoluteEdgesGap ) ? bottomGap : static_cast<int32_t>( relBottomGap * src->height );

                if ( ( left + right >= src->width ) ||
                     ( top + bottom >= src->height ) )
                {
                    ret = ErrorArgumentOutOfRange;
                }
                else
                {
                    ret = XImageGetSubImage( src, &subImage, left, top, src->width - left - right, src->height - top - bottom );
                }
            }
            break;
        }
    }

    if ( ( ret == SuccessCode ) && ( subImage != nullptr ) )
    {
        ret = XImageClone( subImage, dst );
    }

    if ( subImage != nullptr )
    {
        XImageFree( &subImage );
    }

    return ret;
}

// Process the specified source image by changing it
XErrorCode CutImagePlugin::ProcessImageInPlace( ximage* src )
{
    XUNREFERENCED_PARAMETER( src )

    return ErrorNotImplemented;
}

// Get specified property value of the plug-in
XErrorCode CutImagePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U1;
        value->value.ubVal = cutType;
        break;

    case 1:
        value->type = XVT_R4;
        value->value.fVal = relLeftGap;
        break;

    case 2:
        value->type = XVT_R4;
        value->value.fVal = relTopGap;
        break;

    case 3:
        value->type = XVT_R4;
        value->value.fVal = relRightGap;
        break;

    case 4:
        value->type = XVT_R4;
        value->value.fVal = relBottomGap;
        break;

    case 5:
        value->type = XVT_U4;
        value->value.uiVal = leftGap;
        break;

    case 6:
        value->type = XVT_U4;
        value->value.uiVal = topGap;
        break;

    case 7:
        value->type = XVT_U4;
        value->value.uiVal = rightGap;
        break;

    case 8:
        value->type = XVT_U4;
        value->value.uiVal = bottomGap;
        break;

    case 9:
        value->type = XVT_U4;
        value->value.uiVal = cutX;
        break;

    case 10:
        value->type = XVT_U4;
        value->value.uiVal = cutY;
        break;

    case 11:
        value->type = XVT_U4;
        value->value.uiVal = cutWidth;
        break;

    case 12:
        value->type = XVT_U4;
        value->value.uiVal = cutHeight;
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode CutImagePlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;

    xvariant convertedValue;
    XVariantInit( &convertedValue );

    // make sure property value has expected type
    ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 13, &convertedValue );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            cutType = convertedValue.value.ubVal;
            break;

        case 1:
            relLeftGap = convertedValue.value.fVal;
            break;

        case 2:
            relTopGap = convertedValue.value.fVal;
            break;

        case 3:
            relRightGap = convertedValue.value.fVal;
            break;

        case 4:
            relBottomGap = convertedValue.value.fVal;
            break;

        case 5:
            leftGap = convertedValue.value.uiVal;
            break;

        case 6:
            topGap = convertedValue.value.uiVal;
            break;

        case 7:
            rightGap = convertedValue.value.uiVal;
            break;

        case 8:
            bottomGap = convertedValue.value.uiVal;
            break;

        case 9:
            cutX = convertedValue.value.uiVal;
            break;

        case 10:
            cutY = convertedValue.value.uiVal;
            break;

        case 11:
            cutWidth = convertedValue.value.uiVal;
            break;

        case 12:
            cutHeight = convertedValue.value.uiVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
