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

#ifdef WIN32
    #include <windows.h>
#endif

#include <png.h>
#include "ximaging_formats.h"

// Get AForgeX pixel format from PNG's color type and number of bits per pixel
static XPixelFormat GetPixelFormatFromColorAndBpp( int colorType, int bpp )
{
    XPixelFormat outputPixelFormat = XPixelFormatUnknown;

    if ( colorType == PNG_COLOR_TYPE_GRAY )
    {
        if ( bpp == 8 )
        {
            outputPixelFormat = XPixelFormatGrayscale8;
        }
        else if ( bpp == 1 )
        {
            outputPixelFormat = XPixelFormatBinary1;
        }
    }
    else if ( ( colorType == PNG_COLOR_TYPE_RGB ) && ( bpp == 24 ) )
    {
        outputPixelFormat = XPixelFormatRGB24;
    }
    else if ( ( colorType == PNG_COLOR_TYPE_RGBA ) && ( bpp == 32 ) )
    {
        outputPixelFormat = XPixelFormatRGBA32;
    }
    else if ( colorType == PNG_COLOR_TYPE_PALETTE )
    {
        if ( bpp == 1 )
        {
            outputPixelFormat = XPixelFormatIndexed1;
        }
        else if ( bpp == 2 )
        {
            outputPixelFormat = XPixelFormatIndexed2;
        }
        else if ( bpp == 4 )
        {
            outputPixelFormat = XPixelFormatIndexed4;
        }
        else if ( bpp == 8 )
        {
            outputPixelFormat = XPixelFormatIndexed8;
        }
    }

    return outputPixelFormat;
}

// Retrieve color palette from PNG file information
static XErrorCode GetPngColorPalette( png_structp ptrPng, png_infop ptrInfo, xpalette** pPalette )
{
    XErrorCode      ret = SuccessCode;
    png_colorp      palette;
    png_bytep       trans;
    png_color_16p   trans_values;
    int             num_palette;
    int             num_trans;
    int             i;

    if ( png_get_PLTE( ptrPng, ptrInfo, &palette, &num_palette ) == 0 )
    {
        ret = ErrorFailedImageDecoding;
    }
    else
    {
        ret = XPalleteAllocate( (int32_t) num_palette, pPalette );

        if ( ret == SuccessCode )
        {
            xargb* colors = (*pPalette)->values;

            for ( i = 0; i < num_palette; i++ )
            {
                colors[i].components.r = palette[i].red;
                colors[i].components.g = palette[i].green;
                colors[i].components.b = palette[i].blue;
                colors[i].components.a = (uint8_t) NotTransparent8bpp;
            }

            if ( png_get_tRNS( ptrPng, ptrInfo, &trans, &num_trans, &trans_values ) != 0 )
            {
                for ( i = 0; i < num_trans; i++ )
                {
                    colors[i].components.a = trans[i];
                }
            }
        }
    }

    return ret;
}

// Decode PNG image from the specified file
XErrorCode XDecodePng( const char* fileName, ximage** image )
{
    png_structp  ptrPng;
    png_infop    ptrInfo;

    FILE*        file = NULL;
    XErrorCode   ret  = SuccessCode;

    png_uint_32  width, height;
    size_t       rowSize;
    int          bitDepth, channelsCount, colorType, interlaceType, bpp;

    XPixelFormat outputPixelFormat = XPixelFormatUnknown;
    xpalette*    palette = 0;

    if ( ( fileName == 0 ) || ( image == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // get palette of the image, if it was already allocated
        if ( *image != 0 )
        {
            palette = (*image)->palette;
            // set it to NULL for now, so we don't have dangling pointer in the case if palette can not be reused and must be reallocated
            // (it will be restored on success)
            (*image)->palette = 0;
        }

        // open image file
        #ifdef WIN32
            {
                int charsRequired = MultiByteToWideChar( CP_UTF8, 0, fileName, -1, NULL, 0 );

                if ( charsRequired > 0 )
                {
                    WCHAR* filenameUtf16 = (WCHAR*) malloc( sizeof( WCHAR ) * charsRequired );

                    if ( MultiByteToWideChar( CP_UTF8, 0, fileName, -1, filenameUtf16, charsRequired ) > 0 )
                    {
                        file = _wfopen( filenameUtf16, L"rb" );
                    }

                    free( filenameUtf16 );
                }
            }
        #else
            file = fopen( fileName, "rb" );
        #endif

        if ( file == NULL )
        {
            XPaletteFree( &palette );
            XImageFree( image );

            ret = ErrorIOFailure;
        }
        else
        {
            // 1 - Initialize structures and error handling
            ptrPng  = png_create_read_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
            ptrInfo = png_create_info_struct( ptrPng );

            if ( ( ptrPng == 0 ) || ( ptrInfo == 0 ) )
            {
                if ( ptrPng != 0 )
                {
                    png_destroy_read_struct( &ptrPng, 0, 0 );
                }

                fclose( file );

                XPaletteFree( &palette );
                XImageFree( image );

                return ErrorFailedImageDecoding;
            }

            // set error handling
            if ( setjmp( png_jmpbuf( ptrPng ) ) )
            {
                png_destroy_read_struct( &ptrPng, &ptrInfo, 0 );
                fclose( file );

                XPaletteFree( &palette );
                XImageFree( image );

                return ErrorFailedImageDecoding;
            }

            // 2 - tell libpng which file to read
            png_init_io( ptrPng, file );

            // 3 - read info of the image
            png_read_info( ptrPng, ptrInfo );
            png_get_IHDR( ptrPng, ptrInfo, &width, &height, &bitDepth, &colorType, &interlaceType, NULL, NULL );
            channelsCount = png_get_channels( ptrPng, ptrInfo );

            // 4 - configure options

            /*
            printf( "size: %dx%d \n", width, height );
            printf( "bits: %d \n", bitDepth );
            printf( "channels: %d \n", channelsCount );
            printf( "color type: %d \n", colorType );
            printf( "\n" );
            */

            // tell libpng to strip 16 bit/color files down to 8 bits/color
            png_set_scale_16( ptrPng );

            if ( ( bitDepth <= 8 ) && ( colorType == PNG_COLOR_TYPE_PALETTE ) )
            {
                ret = GetPngColorPalette( ptrPng, ptrInfo, &palette );
            }
            else if ( ( bitDepth != 1 ) || ( colorType != PNG_COLOR_TYPE_GRAY ) )
            {
                // extract multiple pixels with bit depths of 1, 2, and 4 from a single
                // byte into separate bytes (useful for paletted and grayscale images)
                png_set_packing( ptrPng );

                // expand paletted colors into true RGB triplets
                if ( colorType == PNG_COLOR_TYPE_PALETTE )
                {
                    png_set_palette_to_rgb( ptrPng );
                }

                // expand 2 and 4 bpp grayscale images to the full 8 bits
                if ( ( colorType == PNG_COLOR_TYPE_GRAY ) && ( bitDepth != 1 ) )
                {
                    png_set_expand_gray_1_2_4_to_8( ptrPng );
                }

                // expand RGB images with transparency to full alpha channels
                // so the data will be available as RGBA quartets
                if ( png_get_valid( ptrPng, ptrInfo, PNG_INFO_tRNS ) )
                {
                    png_set_tRNS_to_alpha( ptrPng );
                }
            }

            if ( ret == SuccessCode )
            {
                // don't care of transparency for grayscale images
                if ( colorType == PNG_COLOR_TYPE_GRAY_ALPHA )
                {
                    png_set_strip_alpha( ptrPng );
                }

                /*
                // flip the RGB pixels to BGR (or RGBA to BGRA)
                if ( colorType & PNG_COLOR_MASK_COLOR )
                {
                    png_set_bgr( ptrPng );
                }
                */

                // 5 - update information so configuration changes are reflected
                png_read_update_info( ptrPng, ptrInfo );
                png_get_IHDR( ptrPng, ptrInfo, &width, &height, &bitDepth, &colorType, &interlaceType, NULL, NULL );
                channelsCount = png_get_channels( ptrPng, ptrInfo );

                /*
                printf( "size: %dx%d \n", width, height );
                printf( "bits: %d \n", bitDepth );
                printf( "channels: %d \n", channelsCount );
                printf( "color type: %d \n", colorType );
                printf( "\n" );
                */

                // calculate bits per pixel
                bpp = bitDepth * channelsCount;
                // get row size in bytes
                rowSize = png_get_rowbytes( ptrPng, ptrInfo );
                // get final output pixel format
                outputPixelFormat = GetPixelFormatFromColorAndBpp( colorType, bpp );

                // check if output image format is resolved
                if ( outputPixelFormat == XPixelFormatUnknown )
                {
                    // should not happen with all the above settings, but in case it does
                    ret = ErrorUnsupportedPixelFormat;
                }
                else
                {
                    // 6 - allocate image and decompression buffer
                    png_bytep rowPtr = (png_bytep) png_malloc( ptrPng, rowSize );

                    if ( rowPtr == 0 )
                    {
                        ret = ErrorOutOfMemory;
                    }
                    else
                    {
                        // if image's size/format matches it will not be reallocated, but reused
                        ret = XImageAllocateRaw( width, height, outputPixelFormat, image );

                        if ( ret == SuccessCode )
                        {
                            // 7 - decode the image
                            png_uint_32 y;

                            for ( y = 0; y < height; y++ )
                            {
                                png_read_rows( ptrPng, &rowPtr, NULL, 1 );

                                if ( XImageSetLine( *image, y, rowPtr, (uint32_t) rowSize ) != SuccessCode )
                                {
                                    ret = ErrorFailedImageDecoding;
                                    break;
                                }
                            }
                        }
                    }

                    if ( rowPtr != 0 )
                    {
                        png_free( ptrPng, rowPtr );
                    }
                }

                // 8 - clean up
                png_read_end( ptrPng, ptrInfo );
            }

            png_destroy_read_struct( &ptrPng, &ptrInfo, 0 );
            fclose( file );

            // check for any errors
            if ( ret != SuccessCode )
            {
                XPaletteFree( &palette );
                XImageFree( image );
            }
            else
            {
                // update palette pointer
                (*image)->palette = palette;
            }
        }
    }

    return ret;
}

// Encode image into the specified PNG file
XErrorCode XEncodePng( const char* fileName, const ximage* image )
{
    png_structp ptrPng;
    png_infop   ptrInfo;
    png_color_8 sig_bit;

    FILE*       file = NULL;
    XErrorCode  ret  = SuccessCode;

    int         bitDepth, colorType;
    int32_t     y;
    png_bytep   rowPtr;

    if ( ( fileName == 0 ) || ( image == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) &&
              ( image->format != XPixelFormatRGBA32 ) &&
              ( image->format != XPixelFormatBinary1 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        // open image file
        #ifdef WIN32
            {
                int charsRequired = MultiByteToWideChar( CP_UTF8, 0, fileName, -1, NULL, 0 );

                if ( charsRequired > 0 )
                {
                    WCHAR* filenameUtf16 = (WCHAR*) malloc( sizeof( WCHAR ) * charsRequired );

                    if ( MultiByteToWideChar( CP_UTF8, 0, fileName, -1, filenameUtf16, charsRequired ) > 0 )
                    {
                        file = _wfopen( filenameUtf16, L"wb" );
                    }

                    free( filenameUtf16 );
                }
            }
        #else
            file = fopen( fileName, "wb" );
        #endif

        if ( file == NULL )
        {
            ret = ErrorIOFailure;
        }
        else
        {
            // 1 - Initialize structures and error handling
            ptrPng  = png_create_write_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
            ptrInfo = png_create_info_struct( ptrPng );

            if ( ( ptrPng == 0 ) || ( ptrInfo == 0 ) )
            {
                if ( ptrPng != 0 )
                {
                    png_destroy_read_struct( &ptrPng, 0, 0 );
                }
                ret = ErrorFailedImageEncoding;
            }
            else
            {
                // set error handling
                if ( setjmp( png_jmpbuf( ptrPng ) ) )
                {
                    png_destroy_write_struct( &ptrPng, &ptrInfo );
                    fclose( file );

                    return ErrorFailedImageDecoding;
                }

                // 2 - tell libpng which file to write to
                png_init_io( ptrPng, file );

                // 3 - specify some image information
                bitDepth = 8;

                if ( image->format == XPixelFormatGrayscale8 )
                {
                    colorType = PNG_COLOR_TYPE_GRAY;
                    sig_bit.gray = 8;
                }
                else if ( image->format == XPixelFormatRGB24 )
                {
                    colorType = PNG_COLOR_TYPE_RGB;
                    sig_bit.red   = 8;
                    sig_bit.green = 8;
                    sig_bit.blue  = 8;
                }
                else if ( image->format == XPixelFormatRGBA32 )
                {
                    colorType = PNG_COLOR_TYPE_RGB_ALPHA;
                    sig_bit.red   = 8;
                    sig_bit.green = 8;
                    sig_bit.blue  = 8;
                    sig_bit.alpha = 8;
                }
                else if ( image->format == XPixelFormatBinary1 )
                {
                    colorType = PNG_COLOR_TYPE_GRAY;
                    sig_bit.gray = 1;
                    bitDepth = 1;
                }
                else
                {
                    ret = ErrorUnsupportedPixelFormat;
                }

                if ( ret == SuccessCode )
                {
                    png_set_IHDR( ptrPng, ptrInfo, image->width, image->height, bitDepth, colorType,
                        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE );
                    png_set_sBIT( ptrPng, ptrInfo, &sig_bit );

                    // 4 - write the file header information
                    png_write_info( ptrPng, ptrInfo );

                    // 5 - some extra configuration

                    // shift the pixels up to a legal bit depth and fill in as appropriate
                    // to correctly scale the image
                    png_set_shift( ptrPng, &sig_bit );

                    if ( image->format != XPixelFormatBinary1 )
                    {
                        // pack pixels into bytes
                        png_set_packing( ptrPng );
                    }

                    // 6 - write image
                    for ( y = 0; y < image->height; y++ )
                    {
                        rowPtr = image->data + image->stride * y;
                        png_write_rows( ptrPng, &rowPtr, 1 );
                    }

                    // 7 - write footer
                    png_write_end( ptrPng, ptrInfo );
                }

                // 8 clean up
                png_destroy_write_struct( &ptrPng, &ptrInfo );
            }

            fclose( file );
        }
    }

    return ret;
}
