/*
    Imaging library of Computer Vision Sandbox

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

#include "ximaging.h"

// forward declaration ----
static void RemoveAlphaChannel32( const ximage* src, ximage* dst );
static void RemoveAlphaChannel64( const ximage* src, ximage* dst );
static void AddAlphaChannel24( const ximage* src, ximage* dst );
static void AddAlphaChannel48( const ximage* src, ximage* dst );
// ------------------------

// Removes alpha channel from 32/64 bpp images - copies only color data
XErrorCode RemoveAlphaChannel( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( src->format == XPixelFormatRGBA32 )
    {
        if ( dst->format != XPixelFormatRGB24 )
        {
            ret = ErrorImageParametersMismatch;
        }
        else
        {
            RemoveAlphaChannel32( src, dst );
        }
    }
    else if ( src->format == XPixelFormatRGBA64 )
    {
        if ( dst->format != XPixelFormatRGB48 )
        {
            ret = ErrorImageParametersMismatch;
        }
        else
        {
            RemoveAlphaChannel64( src, dst );
            ret = ErrorNotImplemented;
        }
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    return ret;
}

// Remove alpha channel from 32 bit image
static void RemoveAlphaChannel32( const ximage* src, ximage* dst )
{
    int width  = src->width;
    int height = src->height;
    int x, y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;

    int srcOffset = src->stride - width * 4;
    int dstOffset = dst->stride - width * 3;

    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++, srcPtr += 4, dstPtr += 3 )
        {
            dstPtr[RedIndex]   = srcPtr[RedIndex];
            dstPtr[GreenIndex] = srcPtr[GreenIndex];
            dstPtr[BlueIndex]  = srcPtr[BlueIndex];
        }
        srcPtr += srcOffset;
        dstPtr += dstOffset;
    }
}

// Remove alpha channel from 64 bit image
static void RemoveAlphaChannel64( const ximage* src, ximage* dst )
{
    // TODO: implement

    XUNREFERENCED_PARAMETER( src );
    XUNREFERENCED_PARAMETER( dst );
}

// Adds alpha channel to 24/48 bpp images - copies color data and sets alpha values to none transparent
XErrorCode AddAlphaChannel( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( src->format == XPixelFormatRGB24 )
    {
        if ( dst->format != XPixelFormatRGBA32 )
        {
            ret = ErrorImageParametersMismatch;
        }
        else
        {
            AddAlphaChannel24( src, dst );
        }
    }
    else if ( src->format == XPixelFormatRGB48 )
    {
        if ( dst->format != XPixelFormatRGBA64 )
        {
            ret = ErrorImageParametersMismatch;
        }
        else
        {
            AddAlphaChannel48( src, dst );
            ret = ErrorNotImplemented;
        }
    }
    else
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    return ret;
}

// Add alpha channel to a 24 bpp color image
void AddAlphaChannel24( const ximage* src, ximage* dst )
{
    int width  = src->width;
    int height = src->height;
    int x, y;

    uint8_t* srcPtr = src->data;
    uint8_t* dstPtr = dst->data;

    int srcOffset = src->stride - width * 3;
    int dstOffset = dst->stride - width * 4;

    for ( y = 0; y < height; y++ )
    {
        for ( x = 0; x < width; x++, srcPtr += 3, dstPtr += 4 )
        {
            dstPtr[RedIndex]   = srcPtr[RedIndex];
            dstPtr[GreenIndex] = srcPtr[GreenIndex];
            dstPtr[BlueIndex]  = srcPtr[BlueIndex];
            dstPtr[AlphaIndex] = NotTransparent8bpp;
        }
        srcPtr += srcOffset;
        dstPtr += dstOffset;
    }
}

// Add alpha channel to a 48 bpp color image
void AddAlphaChannel48( const ximage* src, ximage* dst )
{
    // TODO: implement

    XUNREFERENCED_PARAMETER( src );
    XUNREFERENCED_PARAMETER( dst );
}

// Premultiply RGB values of 32 bpp image with alpha channel
XErrorCode PremultiplyAlphaChannel( ximage* image )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( image->format != XPixelFormatRGBA32 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width  = image->width;
        int height = image->height;
        int offset = image->stride - width * 4;
        int x, y;

        uint8_t* ptr = image->data;
        uint8_t  alpha;

        for ( y = 0; y < height; y++ )
        {
            for ( x = 0; x < width; x++, ptr += 4 )
            {
                alpha = ptr[AlphaIndex];

                ptr[RedIndex]   = (uint8_t)( (uint32_t) ptr[RedIndex]   * alpha / 255 );
                ptr[GreenIndex] = (uint8_t)( (uint32_t) ptr[GreenIndex] * alpha / 255 );
                ptr[BlueIndex]  = (uint8_t)( (uint32_t) ptr[BlueIndex]  * alpha / 255 );
            }
            ptr += offset;
        }
    }

    return ret;
}

// Undo premultiplication of RGB values in 32 bpp image with alpha channel
XErrorCode UndoPremultiplyAlphaChannel( ximage* image )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( image->format != XPixelFormatRGBA32 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int width  = image->width;
        int height = image->height;
        int offset = image->stride - width * 4;
        int x, y;

        uint8_t* ptr = image->data;
        uint8_t  alpha;

        for ( y = 0; y < height; y++ )
        {
            for ( x = 0; x < width; x++, ptr += 4 )
            {
                alpha = ptr[AlphaIndex];

                if ( alpha != 0 )
                {
                    ptr[RedIndex]   = (uint8_t)( (uint32_t) ptr[RedIndex]   * 255 / alpha );
                    ptr[GreenIndex] = (uint8_t)( (uint32_t) ptr[GreenIndex] * 255 / alpha );
                    ptr[BlueIndex]  = (uint8_t)( (uint32_t) ptr[BlueIndex]  * 255 / alpha );
                }
            }
            ptr += offset;
        }
    }

    return ret;
}
