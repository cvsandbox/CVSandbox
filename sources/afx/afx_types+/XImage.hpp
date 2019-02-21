/*
    Core C++ types library of Computer Vision Sandbox

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
#ifndef CVS_XIMAGE_HPP
#define CVS_XIMAGE_HPP

#include <string>
#include <memory>
#include <ximage.h>
#include "XInterfaces.hpp"
#include "XColor.hpp"

namespace CVSandbox
{

// Class encapsulating image data
class XImage : private Uncopyable
{
private:
    XImage( );

public:
    ~XImage( );

    // Allocate image of the specified size and format (image buffer is zero initialized)
    static const std::shared_ptr<XImage> Allocate( int32_t width, int32_t height, XPixelFormat format );
    // Allocate image of the specified size and format (image buffer is not initialized)
    static const std::shared_ptr<XImage> AllocateRaw( int32_t width, int32_t height, XPixelFormat format );
    // Create image by wrapping existing memory buffer
    static const std::shared_ptr<XImage> Create( uint8_t* data, int32_t width, int32_t height, int32_t stride, XPixelFormat format );
    // Create image object for the specified image structure. If the "ownIt" is set to "false", the image structure
    // must stay alive while the C++ object is alive and then be released using XImageFree() (the created object is
    // not responsible for the source image structure). If the "ownIt" is set to "true", the source image structure
    // will be owned by the created image object and will be released when the object is released.
    static const std::shared_ptr<XImage> Create( ximage** image, bool ownIt = false );
    static const std::shared_ptr<const XImage> Create( const ximage* image );

    // Clone image - make a deep copy of it
    const std::shared_ptr<XImage> Clone( ) const;
    // Copy content of the image
    bool CopyData( const std::shared_ptr<XImage>& copyTo ) const;
    // Copy content of the image into the specified one if its size/format is same or make a clone
    bool CopyDataOrClone( std::shared_ptr<XImage>& copyTo ) const;

    // Image properties
    int32_t Width( )  const { return mImage->width; }
    int32_t Height( ) const { return mImage->height; }
    int32_t Stride( ) const { return mImage->stride; }
    XPixelFormat Format( ) const { return mImage->format; }
    // Raw data of the image
    uint8_t* Data( ) const { return mImage->data; }

    // Get pointer to the underlying C image structure
    ximage* ImageData( ) const { return mImage; }
    // Replace C image pointer with a new one
    void Reset( ximage* newImage ) { mImage = newImage; }

    // Get sub image of the specified source image (source image must stay alive - it does not make a deep copy)
    const std::shared_ptr<XImage> GetSubImage( int32_t x, int32_t y, int32_t width, int32_t height ) const;
    // Put the specified image data into this image at the specified location (images must of same pixel format)
    XErrorCode PutImage( const std::shared_ptr<const XImage>& image, int32_t x, int32_t y );

    // Get color of the specified pixel
    const XColor GetPixelColor( int32_t x, int32_t y ) const;
    // Get color index of the specified pixel in an indexed image
    int32_t GetPixelColorIndex( int32_t x, int32_t y ) const;

public:
    // Get name of the specified pixel format
    static const std::string PixelFormatName( XPixelFormat format );

    // Get name of the supported image size
    static const std::string SupportedImageSizeName( XSupportedImageSize supportedSize );

private:
    ximage* mImage;
};

} // namespace CVSandbox

#endif // CVS_XIMAGE_HPP
