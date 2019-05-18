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

    public:
        TwoFramesDifferenceDetectionPluginData( ) { }
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
    default:
        ret = ErrorInvalidProperty;
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
    // ret = PropertyChangeTypeHelper( id, value, propertiesDescription, 11, &convertedValue );

    if ( ret == SuccessCode )
    {
    }

    XVariantClear( &convertedValue );

    return ret;
}

// Check if the plug-in does changes to input video frames or not
bool TwoFramesDifferenceDetectionPlugin::IsReadOnlyMode( )
{
    return true;
}

// Get pixel formats supported by the plug-in
XErrorCode TwoFramesDifferenceDetectionPlugin::GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedPixelFormats, XARRAY_SIZE( supportedPixelFormats ), pixelFormats, count );
}

// Process the specified video frame
XErrorCode TwoFramesDifferenceDetectionPlugin::ProcessImage( ximage* src )
{
    XErrorCode ret = SuccessCode;

    return ret;
}

// Check if the plug-in triggered detection on the last processed image
bool TwoFramesDifferenceDetectionPlugin::Detected( )
{
    return false;
}

// Reset run time state of the video processing plug-in
void TwoFramesDifferenceDetectionPlugin::Reset( )
{
}
