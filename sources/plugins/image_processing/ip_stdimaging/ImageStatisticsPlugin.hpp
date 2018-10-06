/*
    Standard image processing plug-ins of Computer Vision Sandbox

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
#ifndef CVS_IMAGE_STATISTICS_PLUGIN_HPP
#define CVS_IMAGE_STATISTICS_PLUGIN_HPP

#include <iplugintypescpp.hpp>

namespace Private
{
    class ImageStatisticsPluginData;
}

class ImageStatisticsPlugin : public IImageProcessingPlugin
{
public:
    ImageStatisticsPlugin( );

    // IPluginBase interface
    void Dispose( );

    XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    XErrorCode SetProperty( int32_t id, const xvariant* value );

    XErrorCode GetIndexedProperty( int32_t id, uint32_t index, xvariant* value ) const;

    // IImageProcessingPlugin interface
    XErrorCode GetSupportedPixelFormats( XPixelFormat* formats, int32_t* count );
    XErrorCode ProcessImage( const ximage* image );

private:
    static const XPixelFormat supportedFormats[];
    static const PropertyDescriptor** propertiesDescription;
    Private::ImageStatisticsPluginData* mData;
};

#endif // CVS_IMAGE_STATISTICS_PLUGIN_HPP
