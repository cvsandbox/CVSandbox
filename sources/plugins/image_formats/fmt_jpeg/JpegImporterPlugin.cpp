/*
    JPEG images handling plug-ins of Computer Vision Sandbox

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
#include "JpegImporterPlugin.hpp"

// List of supported file extensions
const char* JpegImporterPlugin::supportedFileExtensions[] =
{
    "jpg", "jpeg"
};

JpegImporterPlugin::JpegImporterPlugin( )
{
}

void JpegImporterPlugin::Dispose( )
{
    delete this;
}

// No properties to get/set
XErrorCode JpegImporterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}
XErrorCode JpegImporterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )

    return ErrorInvalidProperty;
}

// Get some short description of the file type
xstring JpegImporterPlugin::GetFileTypeDescription( )
{
    return XStringAlloc( "JPEG files" );
}

// Get file extensions supported by the importer
XErrorCode JpegImporterPlugin::GetSupportedExtensions( xstring* fileExtensions, int32_t* count )
{
    return GetSupportedExtensionsImpl( supportedFileExtensions, sizeof( supportedFileExtensions ) / sizeof( char* ),
                                       fileExtensions, count );
}

// Load image from the specified file
XErrorCode JpegImporterPlugin::ImportImage( xstring fileName, ximage** image )
{
    return XDecodeJpeg( fileName, image );
}
