/*
    Image processing effects plug-ins of Computer Vision Sandbox

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

#include <stdlib.h>
#include <xtextures.h>
#include "GenerateGrainTexturePlugin.hpp"

GenerateGrainTexturePlugin::GenerateGrainTexturePlugin( ) :
    width( 640 ), height( 480 ), spacing( 50 ), density( 0.5 ), isVertical( true ), randValue( (uint16_t) ( rand( ) % 10000 ) )
{
}

void GenerateGrainTexturePlugin::Dispose( )
{
    delete this;
}

// Generate the texture's image
XErrorCode GenerateGrainTexturePlugin::GenerateImage( ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( dst == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XImageAllocateTexture( width, height, dst );

        if ( ret == SuccessCode )
        {
            ret = GenerateGrainTexture( *dst, randValue, spacing, density, isVertical );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }

    return ret;
}

// Get specified property value of the plug-in
XErrorCode GenerateGrainTexturePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type = XVT_U2;
        value->value.usVal = width;
        break;

    case 1:
        value->type = XVT_U2;
        value->value.usVal = height;
        break;

    case 2:
        value->type = XVT_U2;
        value->value.usVal = spacing;
        break;

    case 3:
        value->type = XVT_R4;
        value->value.fVal = density;
        break;

    case 4:
        value->type = XVT_Bool;
        value->value.boolVal = isVertical;
        break;

    default:
        ret = ErrorInvalidProperty;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode GenerateGrainTexturePlugin::SetProperty( int32_t id, const xvariant* value )
{
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
            width = convertedValue.value.usVal;
            break;

        case 1:
            height = convertedValue.value.usVal;
            break;

        case 2:
            spacing = convertedValue.value.usVal;
            break;

        case 3:
            density = convertedValue.value.fVal;
            break;

        case 4:
            isVertical = convertedValue.value.boolVal;
            break;
        }
    }

    XVariantClear( &convertedValue );

    return ret;
}
