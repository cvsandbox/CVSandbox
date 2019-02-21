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

#include <new>
#include "XImage.hpp"

using namespace std;

namespace CVSandbox
{

// Create empty image
XImage::XImage( ) : mImage( 0 )
{
}

// Destroy image
XImage::~XImage( )
{
    XImageFree( &mImage );
}

// Allocate image of the specified size and format (image buffer is zero initialized)
const shared_ptr<XImage> XImage::Allocate( int32_t width, int32_t height, XPixelFormat format )
{
    XImage* image = new (nothrow) XImage( );

    if ( image != nullptr )
    {
        if ( XImageAllocate( width, height, format, &image->mImage ) != SuccessCode )
        {
            delete image;
            image = nullptr;
        }
    }

    return shared_ptr<XImage>( image );
}

// Allocate image of the specified size and format (image buffer is not initialized)
const shared_ptr<XImage> XImage::AllocateRaw( int32_t width, int32_t height, XPixelFormat format )
{
    XImage* image = new (nothrow) XImage( );

    if ( image != nullptr )
    {
        if ( XImageAllocateRaw( width, height, format, &image->mImage ) != SuccessCode )
        {
            delete image;
            image = nullptr;
        }
    }

    return shared_ptr<XImage>( image );
}

// Create image by wrapping existing memory buffer
const shared_ptr<XImage> XImage::Create( uint8_t* data, int32_t width, int32_t height, int32_t stride, XPixelFormat format )
{
    XImage* image = new (nothrow) XImage( );

    if ( image != nullptr )
    {
        if ( XImageCreate( data, width, height, stride, format, &image->mImage ) != SuccessCode )
        {
            delete image;
            image = nullptr;
        }
    }

    return shared_ptr<XImage>( image );
}

// Create image by wrapping existing C image structure
const shared_ptr<XImage> XImage::Create( ximage** image, bool ownIt )
{
    shared_ptr<XImage> guardedImage;

    if ( ( image != nullptr ) && ( *image != nullptr ) )
    {
        if ( !ownIt )
        {
            guardedImage = Create( (*image)->data, (*image)->width, (*image)->height, (*image)->stride, (*image)->format );
        }
        else
        {
            XImage* wrapperImage = new (nothrow) XImage( );

            if ( wrapperImage != nullptr )
            {
                wrapperImage->mImage = *image;
                guardedImage.reset( wrapperImage );
            }
            else
            {
                // free the source image if we don't have memory to create wrapper around it
                XImageFree( image );
            }

            // now this object owns it, not the caller
            *image = nullptr;
        }
    }

    return guardedImage;
}

// Create const image by wrapping existing C const image structure (without taking ownership/responsibility of it)
const shared_ptr<const XImage> XImage::Create( const ximage* image )
{
    return Create( const_cast<ximage**>( &image ), false );
}

// Clone image - make a deep copy of it
const shared_ptr<XImage> XImage::Clone( ) const
{
    XImage* image = new (nothrow) XImage( );

    if ( image != nullptr )
    {
        if ( XImageClone( mImage, &image->mImage ) != SuccessCode )
        {
            delete image;
            image = nullptr;
        }
    }

    return shared_ptr<XImage>( image );
}

// Copy content of the image
bool XImage::CopyData( const shared_ptr<XImage>& copyTo ) const
{
    return ( XImageCopyData( mImage, copyTo->ImageData( ) ) == SuccessCode ) ? true : false;
}

// Copy content of the image into the specified one if its size/format is same or make a clone
bool XImage::CopyDataOrClone( std::shared_ptr<XImage>& copyTo ) const
{
    bool ret;

    if ( ( !copyTo ) ||
         ( copyTo->Width( )  != Width( ) ) ||
         ( copyTo->Height( ) != Height( ) ) ||
         ( copyTo->Format( ) != Format( ) ) )
    {
        copyTo = Clone( );
        ret = static_cast<bool>( copyTo );
    }
    else
    {
        ret = CopyData( copyTo );
    }

    return ret;
}

// Get sub image of the specified source image (source image must stay alive - it does not make deep copy)
const shared_ptr<XImage> XImage::GetSubImage( int32_t x, int32_t y, int32_t width, int32_t height ) const
{
    shared_ptr<XImage> guardedImage;
    ximage*            ximage = nullptr;

    if ( XImageGetSubImage( mImage, &ximage, x, y, width, height ) == SuccessCode )
    {
        guardedImage = Create( &ximage, true );
    }

    return guardedImage;
}

// Put the specified image data into this image at the specified location (images must of same pixel format)
XErrorCode XImage::PutImage( const shared_ptr<const XImage>& image, int32_t x, int32_t y )
{
    return ( image ) ? XImagePutImage( mImage, image->mImage, x, y ) : ErrorNullParameter;
}

// Get color of the specified pixel
const XColor XImage::GetPixelColor( int32_t x, int32_t y ) const
{
    XColor color;
    xargb  argb;

    if ( XImageGetPixelColor( mImage, x, y, &argb ) == SuccessCode )
    {
        color = argb;
    }

    return color;
}

// Get color index of the specified pixel in an indexed image
int32_t XImage::GetPixelColorIndex( int32_t x, int32_t y ) const
{
    int32_t colorIndex = -1;

    XImageGetPixelColorIndex( mImage, x, y, &colorIndex );

    return colorIndex;
}

// === Static members ===

// Get name of the specified pixel format
const string XImage::PixelFormatName( XPixelFormat format )
{
    string  name;
    xstring xname = XImageGetPixelFormatName( format );

    if ( xname != nullptr )
    {
        name = string( xname );
        XStringFree( &xname );
    }

    return name;
}

// Get name of the supported image size
const string XImage::SupportedImageSizeName( XSupportedImageSize supportedSize )
{
    string  name;
    xstring xname = XImageGetSupportedImageSizeName( supportedSize );

    if ( xname != nullptr )
    {
        name = string( xname );
        XStringFree( &xname );
    }

    return name;
}

} // namespace CVSandbox
