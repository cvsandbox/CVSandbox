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

#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "ximage.h"

// Pixel format names (human readable)
static char* pixelFormatNames[] =
{
    "Unknown",
    "8bpp Grayscale",
    "24bpp RGB",
    "32bpp RGBA",
    "16bpp Grayscale",
    "48bpp RGB",
    "64bpp RGBA",
    "1bpp Binary",
    "1bpp Indexed",
    "2bpp Indexed",
    "4bpp Indexed",
    "8bpp Indexed",
    "32bpp Grayscale",
    "64bpp Grayscale",
    "R4 Grayscale"
};

// Pixel format short names (suitable for serialization, scripting, etc.)
static char* pixelFormatShortNames[] =
{
    "Unknown",
    "Gray8",
    "RGB24",
    "RGBA32",
    "Gray16",
    "RGB48",
    "RGBA64",
    "Binary1",
    "Indexed1",
    "Indexed2",
    "Indexed4",
    "Indexed8",
    "Gray32",
    "Gray64",
    "GrayR4"
};

// Supported image size names
static char* supportedImageSizeNames[] =
{
    "Any",
    "Equal",
    "Equal or Bigger",
    "Equal or Smaller"
};

// Returns number of bits required for pixel in certain format
uint32_t XImageBitsPerPixel( XPixelFormat format )
{
    static int sizes[] = { 0, 8, 24, 32, 16, 48, 64, 1, 1, 2, 4, 8, 32, 64, sizeof( float ) * 8 };

    return ( format >= XARRAY_SIZE( sizes ) ) ? 0 : sizes[format];
}

// Returns number of bytes per stride when number of bits per line is known (stride is always 32 bit aligned)
uint32_t XImageBytesPerStride( uint32_t bitsPerLine )
{
    return ( ( bitsPerLine + 31 ) & ~31 ) >> 3;
}

// Returns number of bytes per line when number of bits per line is known (line is always 8 bit aligned)
uint32_t XImageBytesPerLine( uint32_t bitsPerLine )
{
    return ( bitsPerLine + 7 ) >> 3;
}

// Check if the specified pixel format is indexed (requires palette) or not
bool XImageIsPixelFormatIndexed( XPixelFormat format )
{
    return ( ( format == XPixelFormatIndexed1 ) ||
             ( format == XPixelFormatIndexed2 ) ||
             ( format == XPixelFormatIndexed4 ) ||
             ( format == XPixelFormatIndexed8 ) ) ? true : false;
}

// Get name of the pixel format
xstring XImageGetPixelFormatName( XPixelFormat format )
{
    if ( format >= XARRAY_SIZE( pixelFormatNames ) )
    {
        format = 0;
    }
    return XStringAlloc( pixelFormatNames[format] );
}

// Get short name of the pixel format
xstring XImageGetPixelFormatShortName( XPixelFormat format )
{
    if ( format >= XARRAY_SIZE( pixelFormatShortNames ) )
    {
        format = 0;
    }
    return XStringAlloc( pixelFormatShortNames[format] );
}

// Get pixel format from its short name
XPixelFormat XImageGetPixelFormatFromShortName( xstring name )
{
    XPixelFormat format = XPixelFormatUnknown;
    uint32_t     i;

    for ( i = 1; i < XARRAY_SIZE( pixelFormatShortNames ); i++ )
    {
        if ( strcmp( name, pixelFormatShortNames[i] ) == 0 )
        {
            format = i;
            break;
        }
    }

    return format;
}

// Check if the second specified image size matches to the first one.
bool XImageCheckIfImageSizeMatch( int32_t width, int32_t height, XSupportedImageSize supportedMatch,
                                  int32_t matchWidth, int32_t matchHeight )
{
    bool foundMatching = false;

    switch ( supportedMatch )
    {
        case XImageSizeAny:
            foundMatching = true;
            break;

        case XImageSizeEqual:
            if ( ( matchWidth == width ) && ( matchHeight == height ) )
            {
                foundMatching = true;
            }
            break;

        case XImageSizeEqualOrBigger:
            if ( ( matchWidth >= width ) && ( matchHeight >= height ) )
            {
                foundMatching = true;
            }
            break;

        case XImageSizeEqaulOrSmaller:
            if ( ( matchWidth <= width ) && ( matchHeight <= height ) )
            {
                foundMatching = true;
            }
            break;
    }

    return foundMatching;
}

// Get name of the supported image size
xstring XImageGetSupportedImageSizeName( XSupportedImageSize supportedSize )
{
    xstring ret = 0;

    if ( ( supportedSize >= XImageSizeAny ) && ( supportedSize <= XImageSizeEqaulOrSmaller ) )
    {
         ret = XStringAlloc( supportedImageSizeNames[supportedSize] );
    }

    return ret;
}

// Creates an image for existing image buffer - wraps into ximage structure
XErrorCode XImageCreate( uint8_t* data, int32_t width, int32_t height, int32_t stride, XPixelFormat format, ximage** image )
{
    XErrorCode ret = SuccessCode;

    if ( ( data == 0 ) || ( image == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( width <= 0 ) || ( height <= 0 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( format == XPixelFormatUnknown ) || ( format >= XPixelFormatLastValue ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        ximage* temp = (ximage*) XMAlloc( sizeof( ximage ) );

        if ( temp == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            temp->data      = data;
            temp->width     = width;
            temp->height    = height;
            temp->stride    = stride;
            temp->format    = format;
            temp->ownBuffer = 0;
            temp->palette   = 0;

            *image = temp;
        }
    }

    return ret;
}

// Allocates image structure and memory buffer for the image of specified size/format
static XErrorCode XImageAllocate_Internal( int32_t width, int32_t height, XPixelFormat format, ximage** image, bool initBuffer )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( width <= 0 ) || ( height <= 0 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( ( format == XPixelFormatUnknown ) || ( format >= XPixelFormatLastValue ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        ximage* temp = *image;

        if ( temp != 0 )
        {
            if ( ( temp->width == width ) && ( temp->height == height ) && ( temp->format == format ) )
            {
                // free palette if it was set - don't reuse it
                XPaletteFree( &(*image)->palette );

                if ( initBuffer == true )
                {
                    int32_t  stride   = temp->stride;
                    int32_t  lineSize = XImageBytesPerLine( XImageBitsPerPixel( format ) * width );
                    int32_t  y;
                    uint8_t* ptr      = temp->data;

                    for ( y = 0; y < height; y++ )
                    {
                        memset( ptr + y * stride, 0, lineSize );
                    }
                }
            }
            else
            {
                // free image since its size/format does not match
                XImageFree( image );
                temp = 0;
            }
        }

        if ( temp == 0 )
        {
            temp = (ximage*) XMAlloc( sizeof( ximage ) );

            if ( temp == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                temp->width     = width;
                temp->height    = height;
                temp->format    = format;
                temp->stride    = (int32_t) XImageBytesPerStride( XImageBitsPerPixel( format ) * width );
                temp->ownBuffer = 1;
                temp->palette   = 0;

                if ( initBuffer == true )
                {
                    temp->data = (uint8_t*) XCAlloc( 1, height * temp->stride );
                }
                else
                {
                    temp->data = (uint8_t*) XMAlloc( height * temp->stride );
                }

                if ( temp->data == 0 )
                {
                    // free structure if failed allocating image buffer
                    XFree( (void**) temp );
                    ret = ErrorOutOfMemory;
                }
                else
                {
                    *image = temp;
                }
            }
        }
    }

    return ret;
}

// Allocates image structure and memory buffer for the image of specified size/format (memory buffer is initialized with 0)
XErrorCode XImageAllocate( int32_t width, int32_t height, XPixelFormat format, ximage** image )
{
    return XImageAllocate_Internal( width, height, format, image, true );
}

// Allocates image structure and memory buffer for the image of specified size/format (memory buffer is not initialized)
XErrorCode XImageAllocateRaw( int32_t width, int32_t height, XPixelFormat format, ximage** image )
{
    return XImageAllocate_Internal( width, height, format, image, false );
}

// Frees image structure and image buffer if it was allocated
void XImageFree( ximage** image )
{
    if ( ( image != 0 ) && ( *image != 0 ) )
    {
        if ( ( (*image)->ownBuffer != 0 ) && ( (*image)->data != 0 ) )
        {
            XFree( (void**) &(*image)->data );
        }

        XPaletteFree( &(*image)->palette );
        XFree( (void**) image );
    }
}

// Copy content of an image
XErrorCode XImageCopyData( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( src->width != dst->width ) || ( src->height != dst->height ) || ( src->format != dst->format ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( ( src->format == XPixelFormatUnknown ) || ( src->format >= XPixelFormatLastValue ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        uint8_t* srcPtr = src->data;
        uint8_t* dstPtr = dst->data;

        // since some 3rd party libraries support negative stride,
        // we also try supporting it to increase potential interoperability
        int32_t  srcStride = src->stride;
        int32_t  dstStride = dst->stride;
        uint32_t toCopy    = XImageBytesPerLine( src->width * XImageBitsPerPixel( src->format ) );
        int32_t  height    = src->height;
        uint32_t bitsLeftToCopy    = 0;
        uint8_t  mask = 0, notMask = 0;

        // find number of bits which donot fit a whole byte
        if ( ( src->format == XPixelFormatBinary1 ) || ( src->format == XPixelFormatIndexed1 ) )
        {
            bitsLeftToCopy = dst->width & 7;
        }
        else if ( src->format == XPixelFormatIndexed2 )
        {
            bitsLeftToCopy = ( dst->width & 3 ) << 1;
        }
        else if ( src->format == XPixelFormatIndexed4 )
        {
            bitsLeftToCopy = ( dst->width & 1 ) << 2;
        }

        // calculate mask for pixels which donot fit a whole byte
        if ( bitsLeftToCopy != 0 )
        {
            // reduce number of bytes copy with memcpy()
            toCopy--;
            // mask for the last bits to copy
            mask    = (uint8_t) ( ( 0xFF00 >> bitsLeftToCopy ) & 0xFF );
            notMask = ~mask;
        }

        // copy all whole bytes
        while ( height != 0 )
        {
            memcpy( dstPtr, srcPtr, toCopy );

            srcPtr += srcStride;
            dstPtr += dstStride;
            height--;
        }

        // copy remaining bits
        if ( mask != 0 )
        {
            height = src->height;

            // move pointers to the last byte to handdle
            srcPtr = src->data + toCopy;
            dstPtr = dst->data + toCopy;

            while ( height != 0 )
            {
                *dstPtr &= notMask;
                *dstPtr |= ( *srcPtr & mask );

                srcPtr += srcStride;
                dstPtr += dstStride;
                height--;
            }
        }

        // copy palette for indexed images
        if ( ( src->format >= XPixelFormatIndexed1 ) && ( src->palette != 0 ) )
        {
            XPalleteClone( src->palette, &(dst->palette) );
        }
    }
    return ret;
}

// Allocate copy of an image
XErrorCode XImageClone( const ximage* src, ximage** dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        ret = XImageAllocateRaw( src->width, src->height, src->format, dst );

        if ( ret == SuccessCode )
        {
            ret = XImageCopyData( src, *dst );

            if ( ret != SuccessCode )
            {
                XImageFree( dst );
            }
        }
    }
    return ret;
}

// Get sub image of the specified source image (source image must stay alive because image data is not copied)
XErrorCode XImageGetSubImage( const ximage* src, ximage** dst, int32_t x, int32_t y, int32_t width, int32_t height )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format >= XPixelFormatBinary1 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        if ( ( x < 0 ) || ( y < 0 ) ||
             ( x + width > src->width ) || ( y + height > src->height ) )
        {
            ret = ErrorArgumentOutOfRange;
        }
        else
        {
            ret = XImageCreate( src->data + y * src->stride + XImageBytesPerLine( x * XImageBitsPerPixel( src->format ) ),
                                width, height, src->stride, src->format, dst );
        }
    }

    return ret;
}

// Put source image data into the target at the specified location (images must of same pixel format)
XErrorCode XImagePutImage( ximage* dst, const ximage* src, int32_t x, int32_t y )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != dst->format )
    {
        ret = ErrorImageParametersMismatch;
    }
    if ( ( src->format == XPixelFormatUnknown ) || ( src->format > XPixelFormatRGBA64 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int32_t targetX = x;
        int32_t targetY = y;
        int32_t srcSubX = 0;
        int32_t srcSubY = 0;
        int32_t srcSubW = src->width;
        int32_t srcSubH = src->height;

        if ( targetX < 0 )
        {
            srcSubX -= targetX;
            srcSubW += targetX;
            targetX = 0;
        }
        if ( targetY < 0 )
        {
            srcSubY -= targetY;
            srcSubH += targetY;
            targetY = 0;
        }

        // make sure destination rectangle still overlaps the destination image
        if ( ( srcSubW > 0 ) && ( srcSubH > 0 ) && ( targetX < dst->width ) && ( targetY < dst->height ) )
        {
            ximage* clippedSrc = (ximage*) src;

            // make further clipping if required
            if ( targetX + srcSubW > dst->width )
            {
                srcSubW = dst->width - targetX;
            }
            if ( targetY + srcSubH > dst->height )
            {
                srcSubH = dst->height - targetY;
            }

            if ( ( srcSubX != 0 ) || ( srcSubY != 0 ) || ( srcSubW != src->width ) || ( srcSubH != src->height ) )
            {
                ret = XImageGetSubImage( src, &clippedSrc, srcSubX, srcSubY, srcSubW, srcSubH );
            }

            if ( ret == SuccessCode )
            {
                ximage* clippedDst = dst;

                if ( ( targetX != 0 ) || ( targetY != 0 ) || ( srcSubW != dst->width ) || ( srcSubH != dst->height ) )
                {
                    ret = XImageGetSubImage( dst, &clippedDst, targetX, targetY, srcSubW, srcSubH );
                }

                if ( ret == SuccessCode )
                {
                    ret = XImageCopyData( clippedSrc, clippedDst );
                }

                // free clipped destination if required
                if ( clippedDst != dst )
                {
                    XImageFree( &clippedDst );
                }
            }

            // free clipped source if required
            if ( clippedSrc != src )
            {
                XImageFree( &clippedSrc );
            }
        }
    }

    return ret;
}

// Set specified line of the image to values from the given buffer
XErrorCode XImageSetLine( ximage* dst, int32_t lineIndex, uint8_t* line, uint32_t lineSize )
{
    XErrorCode ret = SuccessCode;

    if ( ( dst == 0 ) || ( line == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( lineIndex < 0 ) || ( lineIndex >= dst->height ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else if ( lineSize != XImageBytesPerLine( XImageBitsPerPixel( dst->format ) * dst->width ) )
    {
        ret = ErrorFailed;
    }
    else
    {
        memcpy( dst->data + lineIndex * dst->stride, line, lineSize );
    }

    return ret;
}

// Get specified pixel's color
XErrorCode XImageGetPixelColor( const ximage* image, int32_t x, int32_t y, xargb* color )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( color == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( x < 0 ) || ( y < 0 ) || ( x >= image->width ) || ( y >= image->height ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else if ( ( XImageIsPixelFormatIndexed( image->format ) == true ) &&
              ( ( image->palette == 0 ) || ( image->palette->colorsCount <= 0 ) || ( image->palette->values == 0 ) ) )
    {
        ret = ErrorImagePaletteIsMissing;
    }
    else
    {
        int32_t  stride = image->stride;
        uint8_t* ptr    = image->data;
        uint32_t shift;
        int32_t  colorIndex;

        switch ( image->format )
        {
        case XPixelFormatGrayscale8:
            ptr += stride * y + x;

            color->components.r = *ptr;
            color->components.g = *ptr;
            color->components.b = *ptr;
            color->components.a = 255;
            break;

        case XPixelFormatRGB24:
            ptr += stride * y + x * 3;

            color->components.r = ptr[RedIndex];
            color->components.g = ptr[GreenIndex];
            color->components.b = ptr[BlueIndex];
            color->components.a = 255;
            break;

        case XPixelFormatRGBA32:
            ptr += stride * y + x * 4;

            color->components.r = ptr[RedIndex];
            color->components.g = ptr[GreenIndex];
            color->components.b = ptr[BlueIndex];
            color->components.a = ptr[AlphaIndex];
            break;

        case XPixelFormatBinary1:
            {
                uint8_t mask = 1 << ( 7 - ( x & 7 ) );
                ptr += stride * y + ( (uint32_t) x >> 3 );

                color->components.r = ( ( *ptr & mask ) != 0 ) ? 255 : 0;
                color->components.g = color->components.r;
                color->components.b = color->components.r;
                color->components.a = 255;
            }
            break;

        case XPixelFormatIndexed1:
            {
                ptr += stride * y + ( (uint32_t) x >> 3 );
                shift = 7 - ( x & 7 );
                colorIndex = ( (*ptr) >> shift ) & 1;

                if ( colorIndex >= image->palette->colorsCount )
                {
                    colorIndex = 0;
                }

                *color = image->palette->values[colorIndex];
            }
            break;

        case XPixelFormatIndexed2:
            {
                ptr += stride * y + ( (uint32_t) x >> 2 );
                shift = ( 3 - ( x & 3 ) ) << 1;
                colorIndex = ( (*ptr) >> shift ) & 3;

                if ( colorIndex >= image->palette->colorsCount )
                {
                    colorIndex = 0;
                }

                *color = image->palette->values[colorIndex];
            }
            break;

        case XPixelFormatIndexed4:
            {
                ptr += stride * y + ( (uint32_t) x >> 1 );
                shift = ( 1 - ( x & 1 ) ) << 2;
                colorIndex = ( (*ptr) >> shift ) & 7;

                if ( colorIndex >= image->palette->colorsCount )
                {
                    colorIndex = 0;
                }

                *color = image->palette->values[colorIndex];
            }
            break;

        case XPixelFormatIndexed8:
            {
                ptr += stride * y + x;
                colorIndex = *ptr;

                if ( colorIndex >= image->palette->colorsCount )
                {
                    colorIndex = 0;
                }

                *color = image->palette->values[colorIndex];
            }
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }
    }

    return ret;
}

// Set specified pixel's color
XErrorCode XImageSetPixelColor( const ximage* image, int32_t x, int32_t y, xargb color )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( x < 0 ) || ( y < 0 ) || ( x >= image->width ) || ( y >= image->height ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else if ( XImageIsPixelFormatIndexed( image->format ) == true )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int32_t  stride    = image->stride;
        uint8_t* ptr       = image->data;
        uint8_t  grayValue = RGB_TO_GRAY( color.components.r, color.components.g, color.components.b );

        switch ( image->format )
        {
        case XPixelFormatGrayscale8:
            ptr += stride * y + x;

            *ptr = (uint8_t) ( grayValue * 255 / color.components.a );
            break;

        case XPixelFormatRGB24:
            ptr += stride * y + x * 3;

            ptr[RedIndex]   = (uint8_t) ( color.components.r * 255 / color.components.a );
            ptr[GreenIndex] = (uint8_t) ( color.components.g * 255 / color.components.a );
            ptr[BlueIndex]  = (uint8_t) ( color.components.b * 255 / color.components.a );
            break;

        case XPixelFormatRGBA32:
            ptr += stride * y + x * 4;

            ptr[RedIndex]   = color.components.r;
            ptr[GreenIndex] = color.components.g;
            ptr[BlueIndex]  = color.components.b;
            ptr[AlphaIndex] = color.components.a;
            break;

        case XPixelFormatBinary1:
            {
                uint8_t shift  = 7 - ( x & 7 );
                uint8_t setBit = ( ( ( grayValue * 255 / color.components.a ) >= 128 ) ? 1 : 0 ) << shift;
                uint8_t mask   = 1 << shift;

                ptr += stride * y + ( (uint32_t) x >> 3 );

                *ptr &= ~mask;
                *ptr |= setBit;
            }
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }
    }

    return ret;
}

// Get specified pixel's color index
XErrorCode XImageGetPixelColorIndex( const ximage* image, int32_t x, int32_t y, int32_t* colorIndex )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( colorIndex == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( x < 0 ) || ( y < 0 ) || ( x >= image->width ) || ( y >= image->height ) )
    {
        ret = ErrorArgumentOutOfRange;
    }
    else if ( XImageIsPixelFormatIndexed( image->format ) == false )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int32_t  stride = image->stride;
        uint8_t* ptr    = image->data;
        uint32_t shift;

        switch ( image->format )
        {
        case XPixelFormatIndexed1:
            {
                ptr += stride * y + ( (uint32_t) x >> 3 );
                shift = 7 - ( x & 7 );
                *colorIndex = ( (*ptr) >> shift ) & 1;
            }
            break;

        case XPixelFormatIndexed2:
            {
                ptr += stride * y + ( (uint32_t) x >> 2 );
                shift = ( 3 - ( x & 3 ) ) << 1;
                *colorIndex = ( (*ptr) >> shift ) & 3;
            }
            break;

        case XPixelFormatIndexed4:
            {
                ptr += stride * y + ( (uint32_t) x >> 1 );
                shift = ( 1 - ( x & 1 ) ) << 2;
                *colorIndex = ( (*ptr) >> shift ) & 7;
            }
            break;

        case XPixelFormatIndexed8:
            {
                ptr += stride * y + x;
                *colorIndex = *ptr;
            }
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }

        if ( ret == SuccessCode )
        {
            if ( *colorIndex >= image->palette->colorsCount )
            {
                *colorIndex = 0;
            }
        }
    }

    return ret;
}

// Fill the specified color plane of the image with the specified value
XErrorCode XImageFillPlane( ximage* image, XRGBComponent plane, uint16_t value )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( plane == AlphaIndex ) &&
              ( image->format != XPixelFormatRGBA32 ) &&
              ( image->format != XPixelFormatRGBA64 ) )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        int      width  = image->width;
        int      height = image->height;
        int      stride = image->stride;
        int      x, y;
        uint8_t* ptr    = image->data;

        if ( ( image->format == XPixelFormatRGB24 ) || ( image->format == XPixelFormatRGBA32 ) )
        {
            int pixelSize = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;
            uint8_t fillValue = (uint8_t) XMAX( 255, value );

            for ( y = 0; y < height; y++ )
            {
                uint8_t* row = ptr + y * stride + plane;

                for ( x = 0; x < width; x++ )
                {
                    *row = fillValue;
                    row += pixelSize;
                }
            }
        }
        else if ( ( image->format == XPixelFormatRGB48 ) || ( image->format == XPixelFormatRGBA64 ) )
        {
            int pixelSize = ( image->format == XPixelFormatRGB48 ) ? 3 : 4;

            for ( y = 0; y < height; y++ )
            {
                uint16_t* row = (uint16_t*)( ptr + y * stride );
                row += plane;

                for ( x = 0; x < width; x++ )
                {
                    *row = value;
                    row += pixelSize;
                }
            }
        }
        else
        {
            ret = ErrorUnsupportedPixelFormat;
        }
    }

    return ret;
}
