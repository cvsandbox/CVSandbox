/*
    MJPEG streams video source library of Computer Vision Sandbox

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
#ifndef CVS_XBASE_64_HPP
#define CVS_XBASE_64_HPP

namespace CVSandbox { namespace Video { namespace MJpeg { namespace Private
{

// Get length of encoded source for the given length of a plain source
size_t Base64EncodeLength( size_t sourceLength );

// Encodes the specifed source using Base64 encoding
size_t Base64Encode( char* encoded, const char* source, size_t sourceLength );

} } } } // namespace CVSandbox::Video::MJpeg::Private

#endif // CVS_XBASE_64_H
