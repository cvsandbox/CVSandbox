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
#ifndef CVS_XIMAGE_IMPORTING_PLUGIN_HPP
#define CVS_XIMAGE_IMPORTING_PLUGIN_HPP

#include <vector>
#include <string>
#include <XImage.hpp>
#include "XPlugin.hpp"

class XImageImportingPlugin : public XPlugin
{
private:
    XImageImportingPlugin( void* plugin, bool ownIt );

public:
    virtual ~XImageImportingPlugin( );

    // Create plug-in wrapper
    static const std::shared_ptr<XImageImportingPlugin> Create( void* plugin, bool ownIt = true );

    // Get some short description of the file type
    const std::string GetFileTypeDescription( ) const;
    // Get list of supported file extensions
    const std::vector<std::string> GetSupportedExtensions( ) const;
    // Load image from the specified file
    XErrorCode ImportImage( const std::string& fileName, std::shared_ptr<CVSandbox::XImage>& dst ) const;

private:
    std::vector<std::string> mSupportedExtensions;
};

#endif // CVS_XIMAGE_IMPORTING_PLUGIN_HPP
