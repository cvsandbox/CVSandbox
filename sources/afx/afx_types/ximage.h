/*
    Core types library of Computer Vision Sandbox

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
#ifndef CVS_XIMAGE_H
#define CVS_XIMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "xtypes.h"
#include "xerrors.h"
#include "xpalette.h"

// A very close approximation of the famous BT709 Grayscale coefficients: (0.2125, 0.7154, 0.0721)
// Pre-multiplied by 0x10000, so integer grayscaling could be done
#define GRAY_COEF_RED   (0x3666)
#define GRAY_COEF_GREEN (0xB724)
#define GRAY_COEF_BLUE  (0x1276)

// A macro to get gray value (intensity) out of RGB values
#define RGB_TO_GRAY(r, g, b) ((uint32_t) ( GRAY_COEF_RED * (r) + GRAY_COEF_GREEN * (g) + GRAY_COEF_BLUE * (b) ) >> 16 )

// Same as above, but grayscale coefficient are pre-multiplied by 0x100, so we have 8-bit value
#define GRAY_COEF_RED8   (54)
#define GRAY_COEF_GREEN8 (183)
#define GRAY_COEF_BLUE8  (19)

#define RGB_TO_GRAY8(r, g, b) ((uint32_t) ( GRAY_COEF_RED * (r) + GRAY_COEF_GREEN * (g) + GRAY_COEF_BLUE * (b) ) >> 8 )

// ===== Supported pixel formats enumeration  =====
enum
{
    XPixelFormatUnknown = 0,    // Reserved value for cases when image format was not set, unknown, etc.

    XPixelFormatGrayscale8,     // 8 bpp grayscale image
    XPixelFormatRGB24,          // 24 bpp RGB color image
    XPixelFormatRGBA32,         // 32 bpp RGBA color image

    XPixelFormatGrayscale16,    // 16 bpp grayscale image
    XPixelFormatRGB48,          // 48 bpp RGB color image
    XPixelFormatRGBA64,         // 64 bpp RGBA color image

    XPixelFormatBinary1,        // 1 bpp binary image

    XPixelFormatIndexed1,       // 1 bpp indexed image
    XPixelFormatIndexed2,       // 2 bpp indexed image
    XPixelFormatIndexed4,       // 4 bpp indexed image
    XPixelFormatIndexed8,       // 8 bpp indexed image

    XPixelFormatGrayscale32,    // 32 bpp grayscale (used internally to represent integral images, object maps, whatever)
    XPixelFormatGrayscale64,    // 64 bpp grayscale (again used internally - squared integral images, etc)
    XPixelFormatGrayscaleR4,    // grayscale image, which has values in [0.0, 255.0] range represented as real numbers with 32 bit precision (float)

    XPixelFormatLastValue       // Last value in the enum
};
typedef uint32_t XPixelFormat;

// ===== Supported image sizes enumeration =====
enum
{
    XImageSizeAny = 0,
    XImageSizeEqual,
    XImageSizeEqualOrBigger,
    XImageSizeEqaulOrSmaller
};
typedef uint32_t XSupportedImageSize;

// ===== Order of RGB components =====
enum
{
#ifdef PIXEL_FORMAT_BGRA
    BlueIndex  = 0,
    GreenIndex = 1,
    RedIndex   = 2,
#else
    RedIndex   = 0,
    GreenIndex = 1,
    BlueIndex  = 2,
#endif
    AlphaIndex = 3
};
typedef uint32_t XRGBComponent;

// ===== Some common imaging constants =====
enum
{
    MinPixelValue      = 0,
    Max8bppPixelValue  = 255,
    Max16bppPixelValue = 65535,

    Transparent         = 0,
    NotTransparent8bpp  = 255,
    NotTransparent16bpp = 65535
};

// ===== Image structure =====
typedef struct _ximage
{
    uint8_t*     data;
    int32_t      width;
    int32_t      height;
    int32_t      stride;
    XPixelFormat format;
    uint8_t      ownBuffer;
    xpalette*    palette;
}
ximage;

// ===== Tool functions =====

// Returns number of bits required for pixel in certain format
uint32_t XImageBitsPerPixel( XPixelFormat format );
// Returns number of bytes per stride when number of bits per line is known (stride is always 32 bit aligned)
uint32_t XImageBytesPerStride( uint32_t bitsPerLine );
// Returns number of bytes per line when number of bits per line is known (line is always 8 bit aligned)
uint32_t XImageBytesPerLine( uint32_t bitsPerLine );
// Check if the specified pixel format is indexed (requires palette) or not
bool XImageIsPixelFormatIndexed( XPixelFormat format );
// Get name of the pixel format
xstring XImageGetPixelFormatName( XPixelFormat format );
// Get short name of the pixel format
xstring XImageGetPixelFormatShortName( XPixelFormat format );
// Get pixel format from its short name
XPixelFormat XImageGetPixelFormatFromShortName( xstring name );
// Check if the second specified image size matches to the first one.
//     The function is a helper for image processing routines which accept two images,
//     where first image is the source image for an image processing routine, but the
//     second image is more of a parameter, but with the size acceptable by the image
//     processing routine.
bool XImageCheckIfImageSizeMatch( int32_t width, int32_t height, XSupportedImageSize supportedMatch,
                                  int32_t matchWidth, int32_t matchHeight );
// Get name of the supported image size
xstring XImageGetSupportedImageSizeName( XSupportedImageSize supportedSize );

// ===== Image related functions =====

// Creates an image for existing image buffer - wraps into ximage structure
XErrorCode XImageCreate( uint8_t* data, int32_t width, int32_t height, int32_t stride, XPixelFormat format, ximage** image );
// Allocates image structure and memory buffer for the image of specified size/format (memory buffer is initialized with 0)
// Note: the *image must bet either NULL or a valid previously allocated image (if so it will be re-used if its size/format matches)
XErrorCode XImageAllocate( int32_t width, int32_t height, XPixelFormat format, ximage** image );
// Allocates image structure and memory buffer for the image of specified size/format (memory buffer is not initialized)
// Note: the *image must bet either NULL or a valid previously allocated image (if so it will be re-used if its size/format matches)
XErrorCode XImageAllocateRaw( int32_t width, int32_t height, XPixelFormat format, ximage** image );
// Frees image structure and image buffer if it was allocated
void XImageFree( ximage** image );
// Copy content of an image
XErrorCode XImageCopyData( const ximage* src, ximage* dst );
// Allocate copy of an image
// Note: the *dst must bet either NULL or a valid previously allocated image (if so it will be re-used if its size/format matches)
XErrorCode XImageClone( const ximage* src, ximage** dst );
// Get sub image of the specified source image (source image must stay alive because image data is not copied)
XErrorCode XImageGetSubImage( const ximage* src, ximage** dst, int32_t x, int32_t y, int32_t width, int32_t height );
// Put source image data into the target at the specified location (images must of same pixel format)
XErrorCode XImagePutImage( ximage* dst, const ximage* src, int32_t x, int32_t y );

// Set specified line of the image to values from the given buffer
XErrorCode XImageSetLine( ximage* src, int32_t lineIndex, uint8_t* line, uint32_t lineSize );

// Get specified pixel's color
XErrorCode XImageGetPixelColor( const ximage* image, int32_t x, int32_t y, xargb* color );
// Get specified pixel's color index
XErrorCode XImageGetPixelColorIndex( const ximage* image, int32_t x, int32_t y, int32_t* colorIndex );

// Fill the specified color plane of the image with the specified value
XErrorCode XImageFillPlane( ximage* image, XRGBComponent plane, uint16_t value );

#ifdef __cplusplus
}
#endif

#endif // CVS_XIMAGE_H
