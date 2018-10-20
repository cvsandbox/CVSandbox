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

#pragma once
#ifndef CVS_FIND_BIGGEST_BLOB_PLUGIN_HPP
#define CVS_FIND_BIGGEST_BLOB_PLUGIN_HPP

#include <iplugintypescpp.hpp>

namespace Private
{
    class FindBiggestBlobPluginData;
}

class FindBiggestBlobPlugin : public IImageProcessingPlugin
{
public:
    FindBiggestBlobPlugin( );

    // IPluginBase interface
    void Dispose( );

    XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    XErrorCode SetProperty( int32_t id, const xvariant* value );

    // IImageProcessingPlugin interface
    XErrorCode GetSupportedPixelFormats( XPixelFormat* formats, int32_t* count );
    XErrorCode ProcessImage( const ximage* image );

private:
    static const PropertyDescriptor**   propertiesDescription;
    static const XPixelFormat           supportedFormats[];
    Private::FindBiggestBlobPluginData* mData;
};

#endif // CVS_FIND_BIGGEST_BLOB_PLUGIN_HPP
