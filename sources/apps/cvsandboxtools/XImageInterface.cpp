/*
    Common tools for Computer Vision Sandbox

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

#include "XImageInterface.hpp"
#include <ximaging.h>

using namespace std;
using namespace CVSandbox;

// Convert Qt image to XImage
const shared_ptr<XImage> XImageInterface::QtoXimage( const QImage& image, bool convert32to24 )
{
    const QImage*       imageToConvert = &image;
    QImage              srcCopy;
    shared_ptr<XImage>  xImage;

    if ( !image.isNull( ) )
    {
        // check if source image format is supported by the routine
        // and create copy of the image as 32 bpp if not
        QImage::Format srcFormat = image.format( );

        if ( ( srcFormat != QImage::Format_ARGB32 ) &&
           ( ( srcFormat != QImage::Format_Indexed8 ) || ( !image.isGrayscale( ) ) ) )
        {
            srcCopy = image.convertToFormat( QImage::Format_ARGB32 );
            imageToConvert = &srcCopy;
        }
    }

    if ( !imageToConvert->isNull( ) )
    {
        QImage::Format  srcFormat = imageToConvert->format( );
        XPixelFormat    dstFormat = ( srcFormat == QImage::Format_Indexed8 ) ?
                                    XPixelFormatGrayscale8 : ( ( convert32to24 ) ? XPixelFormatRGB24 : XPixelFormatRGBA32 );

        int width  = imageToConvert->width( );
        int height = imageToConvert->height( );

        // create result image
        xImage = XImage::Allocate( width, height, dstFormat );
        // wrap buffer of source image as XImage
        shared_ptr<XImage> srcImage = XImage::Create( (uint8_t*) imageToConvert->bits( ), width, height,
            imageToConvert->bytesPerLine( ),
            ( srcFormat == QImage::Format_Indexed8 ) ? XPixelFormatGrayscale8 : XPixelFormatRGBA32 );

        if ( ( xImage ) && ( srcImage ) )
        {
            XErrorCode errorCode = SuccessCode;

            if ( dstFormat == XPixelFormatGrayscale8 )
            {
                // just copy data as are
                errorCode = XImageCopyData( srcImage->ImageData( ), xImage->ImageData( ) );
            }
            else
            {
                if ( convert32to24 )
                {
                    // convert 32 bpp image to 24 by removing alpha channel
                    errorCode = RemoveAlphaChannel( srcImage->ImageData( ), xImage->ImageData( ) );
                    if ( errorCode == SuccessCode )
                    {
                        errorCode = SwapRedBlue( xImage->ImageData( ) );
                    }

                }
                else
                {
                    // copy image swapping Red and Blue components
                    errorCode = SwapRedBlueCopy( srcImage->ImageData( ), xImage->ImageData( ) );
                }
            }

            Q_ASSERT( errorCode == SuccessCode );
        }
    }

    return xImage;
}

// Convert XImage to Qt image
const shared_ptr<QImage> XImageInterface::XtoQimage( const shared_ptr<const XImage>& image )
{
    shared_ptr<QImage> qImage;

    XtoQimage( image, qImage );

    return qImage;
}

// Convert XImage to Qt image and try to reuse target image if it has correct size/format
void XImageInterface::XtoQimage( const shared_ptr<const XImage>& image, shared_ptr<QImage>& qImage )
{
    if ( image )
    {
        XPixelFormat   srcFormat = image->Format( );
        QImage::Format qFormat = ( ( srcFormat == XPixelFormatGrayscale8 ) || ( srcFormat == XPixelFormatBinary1 ) ) ?
                                    QImage::Format_Indexed8 : QImage::Format_ARGB32;
        bool           dstJustCreated = false;

        int32_t width  = image->Width( );
        int32_t height = image->Height( );

        if ( ( qImage ) &&
             ( ( qImage->width( ) != width ) || ( qImage->height( ) != height ) || ( qImage->format( ) != qFormat ) ) )
        {
            // release old image if its size/format does not match
            qImage.reset( );
        }

        if ( !qImage )
        {
            qImage.reset( new QImage( width, height, qFormat ) );
            dstJustCreated = true;
        }

        if ( qImage )
        {
            // wrap destination Qt image's buffer as XImage
            shared_ptr<XImage> dstImage = XImage::Create( (uint8_t*) qImage->bits( ), width, height,
                qImage->bytesPerLine( ),
                ( qFormat == QImage::Format_Indexed8 ) ? XPixelFormatGrayscale8 : XPixelFormatRGBA32 );

            XErrorCode errorCode = SuccessCode;

            if ( ( srcFormat == XPixelFormatRGB24 ) || ( srcFormat == XPixelFormatRGBA32 ) )
            {
                if ( dstJustCreated )
                {
                    XDrawingFillImage( dstImage->ImageData( ), XColor( 0xFF000000 ) );
                }
                // copy image swapping Red and Blue components
                errorCode = SwapRedBlueCopy( image->ImageData( ), dstImage->ImageData( ) );
            }
            else if ( srcFormat == XPixelFormatGrayscale8 )
            {
                // copy image data as are
                errorCode = XImageCopyData( image->ImageData( ), dstImage->ImageData( ) );
            }
            else if ( srcFormat == XPixelFormatBinary1 )
            {
                // convert 1 bpp binary image to grayscale
                errorCode = BinaryToGrayscale( image->ImageData( ), dstImage->ImageData( ) );
            }
            else if ( ( srcFormat == XPixelFormatIndexed1 ) ||
                      ( srcFormat == XPixelFormatIndexed2 ) ||
                      ( srcFormat == XPixelFormatIndexed4 ) ||
                      ( srcFormat == XPixelFormatIndexed8 ) )
            {
                // convert 1,2,4,8 bpp indexed image to 32 bpp color and swap red and blue channels
                errorCode = IndexedToColor( image->ImageData( ), dstImage->ImageData( ) );
                if ( errorCode == SuccessCode )
                {
                    errorCode = SwapRedBlue( dstImage->ImageData( ) );
                }
            }

            Q_ASSERT( errorCode == SuccessCode );

            // set palette for grayscale image
            if ( qFormat == QImage::Format_Indexed8 )
            {
                qImage->setColorCount( 256 );
                for ( int i = 0; i < 256; i++ )
                {
                    qImage->setColor( i, 0xFF000000 + ( i << 16 ) + ( i << 8 ) + i );
                }
            }
        }
    }
}
