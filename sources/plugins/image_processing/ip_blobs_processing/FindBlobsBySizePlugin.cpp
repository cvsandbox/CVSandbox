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

#include "FindBlobsBySizePlugin.hpp"

// Supported pixel formats of input/output images
const XPixelFormat FindBlobsBySizePlugin::supportedFormats[] =
{
    XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

FindBlobsBySizePlugin::FindBlobsBySizePlugin( )
{
    xvariant var;

    var.type = XVT_Bool;
    var.value.boolVal = false;

    // using blobs filtering plug-in, but not updating the image
    filterPlugin.SetProperty( 0, &var );
}

void FindBlobsBySizePlugin::Dispose( )
{
    delete this;
}

// Provide supported pixel formats
XErrorCode FindBlobsBySizePlugin::GetSupportedPixelFormats( XPixelFormat* formats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedFormats, XARRAY_SIZE( supportedFormats ), formats, count );
}

// Process the specified source image by changing it
XErrorCode FindBlobsBySizePlugin::ProcessImage( const ximage* image )
{
    return filterPlugin.ProcessImageInPlace( const_cast<ximage*>( image ) );
}

// Get the specified property value of the plug-in
XErrorCode FindBlobsBySizePlugin::GetProperty( int32_t id, xvariant* value ) const
{
    return filterPlugin.GetProperty( id + 1, value );
}

// Set the specified property value of the plug-in
XErrorCode FindBlobsBySizePlugin::SetProperty( int32_t id, const xvariant* value )
{
    return filterPlugin.SetProperty( id + 1, value );
}

// Get individual values of blobs' position/size/area
XErrorCode FindBlobsBySizePlugin::GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const
{
    return filterPlugin.GetIndexedProperty( id + 1, index, value );
}
