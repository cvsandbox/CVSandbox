/*
    PNG images handling plug-ins of Computer Vision Sandbox

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

#include <ximaging_formats.h>
#include "PngExporterPlugin.hpp"

// List of supported file extensions
const char* PngExporterPlugin::supportedFileExtensions[] =
{
    "png"
};

// List of supported pixel formats
const XPixelFormat PngExporterPlugin::supportedPixelFormats[] =
{
    XPixelFormatBinary1, XPixelFormatGrayscale8, XPixelFormatRGB24, XPixelFormatRGBA32
};

PngExporterPlugin::PngExporterPlugin( )
{
}

void PngExporterPlugin::Dispose( )
{
    delete this;
}

// Get property of the plug-in
XErrorCode PngExporterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}

// Set property of the plug-in
XErrorCode PngExporterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}

// Get some short description of the file type
xstring PngExporterPlugin::GetFileTypeDescription( )
{
    return XStringAlloc( "PNG files" );
}

// Get file extensions supported by the exporter
XErrorCode PngExporterPlugin::GetSupportedExtensions( xstring* fileExtensions, int32_t* count )
{
    return GetSupportedExtensionsImpl( supportedFileExtensions, XARRAY_SIZE( supportedFileExtensions ),
        fileExtensions, count );
}

// Get pixel formats supported by the exporter
XErrorCode PngExporterPlugin::GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count )
{
    return GetSupportedPixelFormatsImpl( supportedPixelFormats, XARRAY_SIZE( supportedPixelFormats ),
        pixelFormats, count );
}

// Save image to the specified file
XErrorCode PngExporterPlugin::ExportImage( xstring fileName, const ximage* image )
{
    return XEncodePng( fileName, image );
}
