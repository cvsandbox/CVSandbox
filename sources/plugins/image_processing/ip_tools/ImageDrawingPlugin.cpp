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
#include "ImageDrawingPlugin.hpp"

using namespace std;

ImageDrawingPlugin::ImageDrawingPlugin( ) :
    buffer( nullptr ), allocatedBufferSize( 0 ),
    allocatedPoints( nullptr ), allocatedPointsCount( 0 )
{
}

void ImageDrawingPlugin::Dispose( )
{
    if ( buffer )
    {
        free( buffer );
    }
    if ( allocatedPoints )
    {
        free( allocatedPoints );
    }
    delete this;
}

// No properties to get/set
XErrorCode ImageDrawingPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}
XErrorCode ImageDrawingPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}

XErrorCode ImageDrawingPlugin::CallFunction( int32_t id, xvariant* returnValue, const xarray* arguments )
{
    XErrorCode ret = ValidateFunctionArguments( id, arguments, functionsDescription, functionsCount );

    if ( ret == SuccessCode )
    {
        switch ( id )
        {
        case 0:
            ret = XDrawingLine( arguments->elements[0].value.imageVal,
                                arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                arguments->elements[2].value.pointVal.x, arguments->elements[2].value.pointVal.y,
                                arguments->elements[3].value.argbVal );
            break;

        case 1:
            ret = XDrawingRectanle( arguments->elements[0].value.imageVal,
                                    arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                    arguments->elements[2].value.pointVal.x, arguments->elements[2].value.pointVal.y,
                                    arguments->elements[3].value.argbVal );
            break;

        case 2:
            ret = XDrawingCircle( arguments->elements[0].value.imageVal,
                                  arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                  arguments->elements[2].value.usVal,
                                  arguments->elements[3].value.argbVal );
            break;

        case 3:
            ret = XDrawingEllipse( arguments->elements[0].value.imageVal,
                                   arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                   arguments->elements[2].value.usVal,
                                   arguments->elements[3].value.usVal,
                                   arguments->elements[4].value.argbVal );
            break;

        case 4:
            ret = XDrawingText( arguments->elements[0].value.imageVal,
                                arguments->elements[1].value.strVal,
                                arguments->elements[2].value.pointVal.x, arguments->elements[2].value.pointVal.y,
                                arguments->elements[3].value.argbVal,
                                arguments->elements[4].value.argbVal,
                                true );
            break;

        case 5:
            ret = XDrawingImage( arguments->elements[0].value.imageVal,
                                 arguments->elements[1].value.imageVal,
                                 arguments->elements[2].value.pointVal.x, arguments->elements[2].value.pointVal.y );
            break;

        case 6:
        case 7:
            if ( arguments->elements[1].type == ( XVT_Array | XVT_Point ) )
            {
                xarray* points = arguments->elements[1].value.arrayVal;

                if ( points->length > 1 )
                {
                    for ( uint32_t i = 1; ( i < points->length ) && ( ret == SuccessCode ); i++ )
                    {
                        ret = XDrawingLine( arguments->elements[0].value.imageVal,
                                points->elements[i - 1].value.pointVal.x, points->elements[i - 1].value.pointVal.y,
                                points->elements[i].value.pointVal.x, points->elements[i].value.pointVal.y,
                                arguments->elements[2].value.argbVal );
                    }

                    if ( ( id == 7 ) && ( ret == SuccessCode ) )
                    {
                        ret = XDrawingLine( arguments->elements[0].value.imageVal,
                                points->elements[points->length - 1].value.pointVal.x, points->elements[points->length - 1].value.pointVal.y,
                                points->elements[0].value.pointVal.x, points->elements[0].value.pointVal.y,
                                arguments->elements[2].value.argbVal );
                    }
                }
            }
            else
            {
                ret = ErrorInvalidArgument;
            }
            break;

        case 8:
            ret = XDrawingBlendRectanle( arguments->elements[0].value.imageVal,
                                         arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                         arguments->elements[2].value.pointVal.x, arguments->elements[2].value.pointVal.y,
                                         arguments->elements[3].value.argbVal );
            break;

        case 9:
            ret = XDrawingBlendFrame( arguments->elements[0].value.imageVal,
                                      arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                      arguments->elements[2].value.pointVal.x, arguments->elements[2].value.pointVal.y,
                                      arguments->elements[3].value.usVal,
                                      arguments->elements[4].value.usVal,
                                      arguments->elements[5].value.argbVal );
            break;

        case 10:
            ret = XDrawingBlendCircle( arguments->elements[0].value.imageVal,
                                       arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                       arguments->elements[2].value.usVal,
                                       arguments->elements[3].value.argbVal );
            break;

        case 11:
            ret = XDrawingBlendEllipse( arguments->elements[0].value.imageVal,
                                        arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                        arguments->elements[2].value.usVal,
                                        arguments->elements[3].value.usVal,
                                        arguments->elements[4].value.argbVal );
            break;

        case 12:
            ret = XDrawingBlendRing( arguments->elements[0].value.imageVal,
                                     arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                     arguments->elements[2].value.usVal,
                                     arguments->elements[3].value.usVal,
                                     arguments->elements[4].value.argbVal );
            break;

        case 13:
        {
            uint16_t bigRadius  = XMAX( arguments->elements[2].value.usVal, arguments->elements[3].value.usVal );
            uint32_t bufferSize = ( bigRadius * 2 + 1 ) * 10;

            if ( ( allocatedBufferSize < bufferSize ) && ( buffer != nullptr ) )
            {
                free( buffer );
                buffer = nullptr;
                allocatedBufferSize = 0;
            }

            if ( buffer == nullptr )
            {
                buffer = static_cast<int32_t*>( malloc( bufferSize * sizeof( int32_t ) ) );

                if ( buffer == nullptr )
                {
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    allocatedBufferSize = bufferSize;
                }
            }

            if ( ret == SuccessCode )
            {
                ret = XDrawingBlendPie( arguments->elements[0].value.imageVal,
                                        arguments->elements[1].value.pointVal.x, arguments->elements[1].value.pointVal.y,
                                        arguments->elements[2].value.usVal,
                                        arguments->elements[3].value.usVal,
                                        arguments->elements[4].value.fVal,
                                        arguments->elements[5].value.fVal,
                                        arguments->elements[6].value.argbVal,
                                        buffer, &bufferSize );
            }
            break;
        }

        case 14:
            if ( arguments->elements[1].type != ( XVT_Array | XVT_Point ) )
            {
                ret = ErrorInvalidArgument;
            }
            else
            {
                xarray* points = arguments->elements[1].value.arrayVal;

                if ( points->length > 2 )
                {
                    if ( ( allocatedPointsCount < points->length ) && ( allocatedPoints != nullptr ) )
                    {
                        free( allocatedPoints );
                        allocatedPoints = nullptr;
                        allocatedPointsCount = 0;
                    }

                    if ( allocatedPoints == nullptr )
                    {
                        allocatedPoints = static_cast<xpoint*>( malloc( points->length * sizeof( xpoint ) ) );

                        if ( allocatedPoints == nullptr )
                        {
                            ret = ErrorOutOfMemory;
                        }
                        else
                        {
                            allocatedPointsCount = points->length;
                        }
                    }

                    if ( ret == SuccessCode )
                    {
                        for ( uint32_t i = 0; i < points->length; i++ )
                        {
                            allocatedPoints[i] = points->elements[i].value.pointVal;
                        }

                        ret = XDrawingBlendConvexPolygon( arguments->elements[0].value.imageVal, allocatedPoints, points->length,
                                                          arguments->elements[2].value.argbVal );
                    }
                }
            }
            break;

        default:
            ret = ErrorInvalidFunction;
            break;
        }

        // nothing to return
        XVariantClear( returnValue );
    }

    return ret;
}
