/*
    Imaging formats library of Computer Vision Sandbox

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
#ifndef CVS_XIMAGING_FORMATS_H
#define CVS_XIMAGING_FORMATS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xtypes.h>
#include <ximage.h>

// Decode JPEG image from the specified file or memory buffer
// (Note: if user provides already allocated image, from previous decoding for example,
// then it will be reused in case its size/format matches)
XErrorCode XDecodeJpeg( const char* fileName, ximage** image );
XErrorCode XDecodeJpegFromMemory( const uint8_t* buffer, int bufferLength, ximage** image );
// Encode image into the specified JPEG file (quiality: [0, 100])
XErrorCode XEncodeJpeg( const char* fileName, const ximage* image, uint32_t quality );

// Decode PNG image from the specified file
// (Note: if user provides already allocated image, from previous decoding for example,
// then it will be reused in case its size/format matches)
XErrorCode XDecodePng( const char* fileName, ximage** image );
// Encode image into the specified PNG file
XErrorCode XEncodePng( const char* fileName, const ximage* image );

#ifdef __cplusplus
}
#endif

#endif // CVS_XIMAGING_FORMATS_H
