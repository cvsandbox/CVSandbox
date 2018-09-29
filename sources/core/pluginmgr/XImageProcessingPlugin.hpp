/*
    Plug-ins' management library of Computer Vision Sandbox

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
#ifndef CVS_XIMAGE_PROCESSING_PLUGIN_HPP
#define CVS_XIMAGE_PROCESSING_PLUGIN_HPP

#include <vector>
#include <XImage.hpp>
#include "XPlugin.hpp"

class XImageProcessingPlugin : public XPlugin
{
private:
    XImageProcessingPlugin( void* plugin, bool ownIt );

public:
    virtual ~XImageProcessingPlugin( );

    // Create plug-in wrapper
    static const std::shared_ptr<XImageProcessingPlugin> Create( void* plugin, bool ownIt = true );

    // Check if certain pixel format is supported by the filter
    bool IsPixelFormatSupported( XPixelFormat pixelFormat ) const;
    // Get pixel formats supported by the plug-in
    const std::vector<XPixelFormat> GetSupportedPixelFormats( ) const;

    // Process the specified image
    XErrorCode ProcessImage( const std::shared_ptr<CVSandbox::XImage>& image ) const;

private:
    std::vector<XPixelFormat> mSupportedFormats;
};

#endif // CVS_XIMAGE_PROCESSING_PLUGIN_HPP
