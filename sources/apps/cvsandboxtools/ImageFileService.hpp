/*
    Common tools for Computer Vision Sandbox

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
#ifndef CVS_IMAGE_FILE_SERVICE_HPP
#define CVS_IMAGE_FILE_SERVICE_HPP

#include "cvsandboxtools_global.h"
#include "IImageFileService.hpp"
#include <XPluginsEngine.hpp>

namespace Private
{
    class ImageFileServiceData;
}

// Implementation of the service which opens/saves images using plug-ins
class CVS_SHARED_EXPORT ImageFileService : public IImageFileService
{
public:
    ImageFileService( const std::shared_ptr<const XPluginsEngine>& pluginsEngine );
    virtual ~ImageFileService( );

    // Prompt user to select image file by showing file open dialog
    virtual const QString PromptFileOpen( const QString& title, const QString& folder = "", const QWidget* parent = 0 );

    // Prompt user to specify file name to save image to
    virtual const QString PromptFileSave( const QString& title, const QString& folder = "", const QWidget* parent = 0 );

    // Open specified image file
    virtual XErrorCode Open( const QString& fileName, std::shared_ptr<CVSandbox::XImage>& image );

    // Save image to the specified file
    virtual XErrorCode Save( const QString& fileName, const std::shared_ptr<const CVSandbox::XImage>& image );

private:
    Private::ImageFileServiceData*        mData;
    std::shared_ptr<const XPluginsEngine> mPluginsEngine;
};

#endif // CVS_IMAGE_FILE_SERVICE_HPP
