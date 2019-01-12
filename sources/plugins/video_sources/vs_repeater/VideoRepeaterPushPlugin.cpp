/*
    Video repeater plug-ins of Computer Vision Sandbox

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

#include "VideoRepeaterPushPlugin.hpp"
#include "VideoRepeaterRegistry.hpp"
#include "VideoRepeaterPlugin.hpp"
#include <XVariant.hpp>

using namespace std;
using namespace CVSandbox;

// List of supported pixel formats
const XPixelFormat VideoRepeaterPushPlugin::supportedPixelFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

namespace Private
{
    // Internals of video file writer class
    class VideoRepeaterPushPluginData : private Uncopyable
    {
    public:
        VideoRepeaterPushPluginData( ) :
            RepeaterId( )
        {
        }

    public:
        string RepeaterId;

    };
}

VideoRepeaterPushPlugin::VideoRepeaterPushPlugin( ) :
    mData( new ::Private::VideoRepeaterPushPluginData( ) )
{
}

VideoRepeaterPushPlugin::~VideoRepeaterPushPlugin( )
{
    delete mData;
}

void VideoRepeaterPushPlugin::Dispose( )
{
    Reset( );
    delete this;
}

// Get specified property value of the plug-in
XErrorCode VideoRepeaterPushPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XErrorCode ret = SuccessCode;

    switch ( id )
    {
    case 0:
        value->type         = XVT_String;
        value->value.strVal = XStringAlloc( mData->RepeaterId.c_str( ) );
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Set specified property value of the plug-in
XErrorCode VideoRepeaterPushPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XErrorCode ret = SuccessCode;
    XVariant   xvar( *value );

    switch ( id )
    {
    case 0:
        if ( xvar.Type( ) == XVT_String )
        {
            mData->RepeaterId = xvar.ToString( &ret );
        }
        else
        {
            ret = ErrorIncompatibleTypes;
        }
        break;

    default:
        ret = ErrorInvalidProperty;
        break;
    }

    return ret;
}

// Check if the plug-in does changes to input video frames or not
bool VideoRepeaterPushPlugin::IsReadOnlyMode( )
{
    return true;
}

// Get pixel formats supported by the video processing plug-in
XErrorCode VideoRepeaterPushPlugin::GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedPixelFormats, XARRAY_SIZE( supportedPixelFormats ), pixelFormats, count );
}

// Process the specified video frame
XErrorCode VideoRepeaterPushPlugin::ProcessImage( ximage* src )
{
    XErrorCode ret = ErrorInvalidConfiguration;

    if ( src == nullptr )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        VideoRepeaterPlugin* repeater = VideoRepeaterRegistry::Instance( )->GetRepeater( mData->RepeaterId );

        if ( repeater != nullptr )
        {
            repeater->PushImage( src );
            ret = SuccessCode;
        }
    }

    return ret;
}

// Reset run time state of the video processing plug-in
void VideoRepeaterPushPlugin::Reset( )
{
    // nothing to be done
}
