/*
    Image folder plug-ins of Computer Vision Sandbox

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

#pragma once
#ifndef CVS_IMAGE_FOLDER_WRITER_PLUGIN_HPP
#define CVS_IMAGE_FOLDER_WRITER_PLUGIN_HPP

#include <memory>
#include <XImage.hpp>
#include <iplugintypescpp.hpp>

namespace Private
{
    class ImageFolderWriterPluginData;
}

class ImageFolderWriterPlugin : public IVideoProcessingPlugin
{
public:
    ImageFolderWriterPlugin( );
    ~ImageFolderWriterPlugin( );

    // IPluginBase interface
    virtual void Dispose( );

    virtual XErrorCode GetProperty( int32_t id, xvariant* value ) const;
    virtual XErrorCode SetProperty( int32_t id, const xvariant* value );

    // IVideoProcessingPlugin interface

    // Check if the plug-in does changes to input video frames or not
    virtual bool IsReadOnlyMode( );
    // Get pixel formats supported by the video processing plug-in
    virtual XErrorCode GetSupportedPixelFormats( XPixelFormat* pixelFormats, int32_t* count );
    // Process the specified image
    virtual XErrorCode ProcessImage( ximage* src );
    // Reset run time state of the video processing plug-in
    virtual void Reset( );

private:
    ::Private::ImageFolderWriterPluginData* mData;
    static const PropertyDescriptor**  propertiesDescription;
    static const XPixelFormat          supportedPixelFormats[];
};

#endif // CVS_IMAGE_FOLDER_WRITER_PLUGIN_HPP
