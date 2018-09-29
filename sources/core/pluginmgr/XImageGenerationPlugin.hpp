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
#ifndef CVS_XIMAGE_GENERATION_PLUGIN_HPP
#define CVS_XIMAGE_GENERATION_PLUGIN_HPP

#include <XImage.hpp>
#include "XPlugin.hpp"

class XImageGenerationPlugin : public XPlugin
{
private:
    XImageGenerationPlugin( void* plugin, bool ownIt );

public:
    virtual ~XImageGenerationPlugin( );

    // Create plug-in wrapper
    static const std::shared_ptr<XImageGenerationPlugin> Create( void* plugin, bool ownIt = true );

    // Generates an image
    XErrorCode GenerateImage( std::shared_ptr<CVSandbox::XImage>& dst );
};

#endif // CVS_XIMAGE_GENERATION_PLUGIN_HPP
