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
#include "PngImporterPlugin.hpp"

// List of supported file extensions
const char* PngImporterPlugin::supportedFileExtensions[] =
{
	"png"
};

PngImporterPlugin::PngImporterPlugin( )
{
}

void PngImporterPlugin::Dispose( )
{
	delete this;
}

// No properties to get/set
XErrorCode PngImporterPlugin::GetProperty( int32_t id, xvariant* value ) const
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )
    return ErrorInvalidProperty;
}
XErrorCode PngImporterPlugin::SetProperty( int32_t id, const xvariant* value )
{
    XUNREFERENCED_PARAMETER( id )
    XUNREFERENCED_PARAMETER( value )
    return ErrorInvalidProperty;
}

// Get some short description of the file type
xstring PngImporterPlugin::GetFileTypeDescription( )
{
	return XStringAlloc( "PNG files" );
}

// Get file extensions supported by the importer
XErrorCode PngImporterPlugin::GetSupportedExtensions( xstring* fileExtensions, int32_t* count )
{
	return GetSupportedExtensionsImpl( supportedFileExtensions, sizeof( supportedFileExtensions ) / sizeof( char* ),
									   fileExtensions, count );
}

// Load image from the specified file
XErrorCode PngImporterPlugin::ImportImage( xstring fileName, ximage** image )
{
	return XDecodePng( fileName, image );
}
