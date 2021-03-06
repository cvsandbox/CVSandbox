/*
    MJPEG streams source library of Computer Vision Sandbox

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
#ifndef CVS_VIDEO_INTERNAL_TOOLS_HPP
#define CVS_VIDEO_INTERNAL_TOOLS_HPP

#include <stdint.h>
#include <string>

namespace CVSandbox { namespace Video { namespace MJpeg { namespace Private
{

// Search buffer for specified sequence
int MemFind( const uint8_t* buffer, int bufferSize, const uint8_t* searchFor, int searchSize );

// Extract title of HTTP document
std::string ExtractTitle( const char* szHtml, bool* pIsHtml );

} } } } // namespace CVSandbox::Video::MJpeg::Private

#endif // CVS_VIDEO_INTERNAL_TOOLS_HPP
