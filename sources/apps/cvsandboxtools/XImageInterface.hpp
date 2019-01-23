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
#ifndef CVS_XMAGE_INTERFACE_HPP
#define CVS_XMAGE_INTERFACE_HPP

#include <QImage>
#include <XImage.hpp>

#include "cvsandboxtools_global.h"

// Class providing set of static methods to interface with XImage
class CVS_SHARED_EXPORT XImageInterface
{
private:
    XImageInterface( );

public:
    // Convert Qt image to XImage
    static const std::shared_ptr<CVSandbox::XImage> QtoXimage( const QImage& image, bool convert32to24 );
    // Convert XImage to Qt image
    static const std::shared_ptr<QImage> XtoQimage( const std::shared_ptr<const CVSandbox::XImage>& image );
    // Convert XImage to Qt image and try to reuse target image if it has correct size/format
    static void XtoQimage( const std::shared_ptr<const CVSandbox::XImage>& image, std::shared_ptr<QImage>& qImage );
};

#endif // CVS_XMAGE_INTERFACE_HPP
