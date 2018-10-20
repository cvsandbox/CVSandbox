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
#ifndef CVS_FILTER_BLOBS_BY_SIZE_PLUGIN_HPP
#define CVS_FILTER_BLOBS_BY_SIZE_PLUGIN_HPP

#include <iplugintypescpp.hpp>

namespace Private
{
    class FilterBlobsBySizePluginData;
}

class FilterBlobsBySizePlugin : public IImageProcessingFilterPlugin
{
public:
    FilterBlobsBySizePlugin( );
    ~FilterBlobsBySizePlugin( );

    // IPluginBase interface
    void Dispose( );

    XErrorCode SetProperty( int32_t id, const xvariant* value );
    XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    XErrorCode GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const;

    // IImageProcessingFilterPlugin interface
    bool CanProcessInPlace( );
    XErrorCode GetPixelFormatTranslations( XPixelFormat* inputFormats, XPixelFormat* outputFormats, int32_t* count );
    XErrorCode ProcessImage( const ximage* src, ximage** dst );
    XErrorCode ProcessImageInPlace( ximage* src );

private:
    static const PropertyDescriptor**     propertiesDescription;
    static const XPixelFormat             supportedFormats[];
    Private::FilterBlobsBySizePluginData* mData;
};

#endif // CVS_FILTER_BLOBS_BY_SIZE_PLUGIN_HPP
