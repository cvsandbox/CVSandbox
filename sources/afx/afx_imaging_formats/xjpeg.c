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

#include <stdio.h>
#include <setjmp.h>
#include <jpeglib.h>
#include "ximaging_formats.h"

#ifdef AFX_CORRECT_ORIENTATION
#include <libexif/exif-data.h>
#include <ximaging.h>

static XErrorCode CorrectJpegOrientation( const char* fileName, ximage** image );
#endif

static XErrorCode PerformJpegDecoding( struct jpeg_decompress_struct* cinfo, ximage** image );

// Structure which is used for custom error handling from libjpeg
struct CustomeErrorManager
{
    struct  jpeg_error_mgr pub;
    jmp_buf setjmpBuffer;
};

static void my_error_exit( j_common_ptr cinfo )
{
    struct CustomeErrorManager* myerr = (struct CustomeErrorManager*) cinfo->err;
    // return control to the setjmp point
    longjmp( myerr->setjmpBuffer, 1 );
}

static void my_output_message( j_common_ptr cinfo )
{
    // do nothing - kill the message
    XUNREFERENCED_PARAMETER( cinfo )
}

// Decode JPEG image from the specified file
XErrorCode XDecodeJpeg( const char* fileName, ximage** image )
{
    struct jpeg_decompress_struct   cinfo;
    struct CustomeErrorManager      jerr;
    FILE*                           file = NULL;
    XErrorCode                      ret  = SuccessCode;

    if ( ( fileName == 0 ) || ( image == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
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
            ret = ErrorIOFailure;
        }
        else
        {
            // 1 - allocate and initialize JPEG decompression object
            cinfo.err               = jpeg_std_error( &jerr.pub );
            jerr.pub.error_exit     = my_error_exit;
            jerr.pub.output_message = my_output_message;

            // establish the setjmp return context
            if ( setjmp( jerr.setjmpBuffer ) )
            {
                // if we get here, the JPEG code has signalled an error
                jpeg_destroy_decompress( &cinfo );
                fclose( file );

                // free image, if it was already allocated
                XImageFree( image );

                return ErrorFailedImageDecoding;
            }

            jpeg_create_decompress( &cinfo );

            // 2 - specify data source
            jpeg_stdio_src( &cinfo, file );

            // 3-7 - perform actual decoding
            ret = PerformJpegDecoding( &cinfo, image );

            // 8 - clean up
            jpeg_destroy_decompress( &cinfo );
            fclose( file );

            #ifdef AFX_CORRECT_ORIENTATION
            if ( ret == SuccessCode )
            {
                ret = CorrectJpegOrientation( fileName, image );
            }
            #endif
        }
    }

    return ret;
}

// Decode JPEG image from the specified memory buffer
XErrorCode XDecodeJpegFromMemory( const uint8_t* buffer, int bufferLength, ximage** image )
{
    struct jpeg_decompress_struct   cinfo;
    struct CustomeErrorManager      jerr;
    XErrorCode                      ret = SuccessCode;

    if ( ( buffer == 0 ) || ( image == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        // 1 - allocate and initialize JPEG decompression object
        cinfo.err               = jpeg_std_error( &jerr.pub );
        jerr.pub.error_exit     = my_error_exit;
        jerr.pub.output_message = my_output_message;

        // establish the setjmp return context
        if ( setjmp( jerr.setjmpBuffer ) )
        {
            // if we get here, the JPEG code has signalled an error
            jpeg_destroy_decompress( &cinfo );

            // free image, if it was already allocated
            XImageFree( image );

            return ErrorFailedImageDecoding;
        }

        jpeg_create_decompress( &cinfo );

        // 2 - specify data source
        jpeg_mem_src( &cinfo, (unsigned char*) buffer, bufferLength );

        // 3-7 - perform actual decoding
        ret = PerformJpegDecoding( &cinfo, image );

        // 8 - clean up
        jpeg_destroy_decompress( &cinfo );
    }

    return ret;
}

// Perform actual decoding of JPEG image
XErrorCode PerformJpegDecoding( struct jpeg_decompress_struct* cinfo, ximage** image )
{
    XErrorCode ret = SuccessCode;

    // 3 - read file parameters
    jpeg_read_header( cinfo, TRUE );

    // 4 - set parameters for decompression

    // although libjpeg set RGB format by default for colour images, we
    // better force it just to avoid surprise
    if ( cinfo->jpeg_color_space != JCS_GRAYSCALE )
    {
        cinfo->out_color_space = JCS_RGB;
    }

    // 5 - start de-compressor
    jpeg_start_decompress( cinfo );

    if ( ( cinfo->out_color_space != JCS_GRAYSCALE ) && ( cinfo->out_color_space != JCS_RGB ) )
    {
        // should not happen, but in case it does
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        XPixelFormat destinationPixelFormat = ( cinfo->out_color_space == JCS_GRAYSCALE ) ? XPixelFormatGrayscale8 : XPixelFormatRGB24;

        // if image's size/format matches it will not be reallocated, but reused
        ret = XImageAllocateRaw( cinfo->output_width, cinfo->output_height, destinationPixelFormat, image );

        if ( ret == SuccessCode )
        {
            int lineSize        = cinfo->output_width * cinfo->output_components;
            JSAMPARRAY buffer   = (*cinfo->mem->alloc_sarray)( (j_common_ptr) cinfo, JPOOL_IMAGE, lineSize, 1 );
            uint32_t lineIndex  = 0;

            // 6 - decode the image
            while ( cinfo->output_scanline < cinfo->output_height )
            {
                jpeg_read_scanlines( cinfo, buffer, 1 );
                XImageSetLine( *image, lineIndex++, buffer[0], lineSize );
            }

            // 7 - finish decompression
            jpeg_finish_decompress( cinfo );
        }
    }

    return ret;
}

// Encode image into the specified JPEG file (quality: [0, 100])
XErrorCode XEncodeJpeg( const char* fileName, const ximage* image, uint32_t quality )
{
    struct jpeg_compress_struct cinfo;
    struct CustomeErrorManager  jerr;
    FILE*                       file = NULL;
    JSAMPROW                    row_pointer[1];
    XErrorCode                  ret = SuccessCode;

    if ( ( fileName == 0 ) || ( image == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatGrayscale8 ) &&
              ( image->format != XPixelFormatRGB24 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
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
            // 1 - allocate and initialize JPEG decompression object
            cinfo.err               = jpeg_std_error( &jerr.pub );
            jerr.pub.error_exit     = my_error_exit;
            jerr.pub.output_message = my_output_message;

            // establish the setjmp return context
            if ( setjmp( jerr.setjmpBuffer ) )
            {
                // if we get here, the JPEG code has signaled an error
                jpeg_destroy_compress( &cinfo );
                fclose( file );

                return ErrorFailedImageEncoding;
            }

            jpeg_create_compress( &cinfo );

            // 2 - specify data destination
            jpeg_stdio_dest( &cinfo, file );

            // 3 - set parameters for compression
            cinfo.image_width  = image->width;
            cinfo.image_height = image->height;

            if ( image->format == XPixelFormatRGB24 )
            {
                cinfo.input_components = 3;
                cinfo.in_color_space   = JCS_RGB;
            }
            else
            {
                cinfo.input_components = 1;
                cinfo.in_color_space   = JCS_GRAYSCALE;
            }

            // set default compression parameters
            jpeg_set_defaults( &cinfo ) ;
            // set quality
            quality = XMIN( quality, 100 );
            quality = XMAX( quality, 0 );
            jpeg_set_quality( &cinfo, (int) quality, TRUE /* limit to baseline-JPEG values */ );

            // 4 - start compressor
            jpeg_start_compress( &cinfo, TRUE );

            // 5 - do compression
            while ( cinfo.next_scanline < cinfo.image_height )
            {
                row_pointer[0] = image->data + image->stride * cinfo.next_scanline;

                jpeg_write_scanlines( &cinfo, row_pointer, 1 );
            }

            // 6 - finish compression
            jpeg_finish_compress( &cinfo );

            // 7 - clean up
            fclose( file );
            jpeg_destroy_compress( &cinfo );
        }
    }

    return ret;
}

#ifdef AFX_CORRECT_ORIENTATION
// Correct image's orientation based on EXIF information
static XErrorCode CorrectJpegOrientation( const char* fileName, ximage** image )
{
    XErrorCode ret  = SuccessCode;
    ExifData*  data = exif_data_new_from_file( fileName );

    if ( data )
    {
        ExifEntry* entry = exif_content_get_entry( data->ifd[EXIF_IFD_0], EXIF_TAG_ORIENTATION );

        if ( entry )
        {
            ExifByteOrder byteOrder   = exif_data_get_byte_order( entry->parent->parent );
            ExifShort     orientation = exif_get_short( entry->data, byteOrder );
            ximage*       newImage    = 0;

            if ( orientation >= 5 )
            {
                // allocate new image with new orientation
                ret = XImageAllocateRaw( (*image)->height, (*image)->width, (*image)->format, &newImage );
            }

            if ( ret == SuccessCode )
            {
                switch ( orientation )
                {
                    case 1: // top-left
                        // do nothing, since image is oriented as it should be
                        break;

                    case 2: // top-right
                        ret = MirrorImage( newImage, false, true );
                        break;

                    case 3: // bottom-right
                        ret = MirrorImage( newImage, true, true );
                        break;

                    case 4: // bottom-left
                        ret = MirrorImage( newImage, true, false );
                        break;

                    case 5: // left-top
                        ret = RotateImage90( *image, newImage );
                        if ( ret == SuccessCode )
                        {
                            ret = MirrorImage( newImage, true, false );
                        }
                        break;

                    case 6: // right-top
                        ret = RotateImage270( *image, newImage );
                        break;

                    case 7: // right-bottom
                        ret = RotateImage270( *image, newImage );
                        if ( ret == SuccessCode )
                        {
                            ret = MirrorImage( newImage, true, false );
                        }
                        break;

                    case 8: // left-bottom
                        ret = RotateImage90( *image, newImage );
                        break;
                }

                if ( newImage != 0 )
                {
                    XImageFree( image );
                    *image = newImage;
                }
            }
        }

        exif_data_unref( data );
    }

    return ret;
}
#endif
