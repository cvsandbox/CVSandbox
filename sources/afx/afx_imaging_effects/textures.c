/*
    Imaging effects library of Computer Vision Sandbox

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

#include "xtextures.h"

// Allocates texture image - 8bpp grayscale image of the specified size
XErrorCode XImageAllocateTexture( int32_t width, int32_t height, ximage** texture )
{
    return XImageAllocate( width, height, XPixelFormatGrayscale8, texture );
}

/* Allocates texture image - 8bpp grayscale image of the specified size (image memory is not initialized with zeros) */
XErrorCode XImageAllocateTextureRaw( int32_t width, int32_t height, ximage** texture )
{
    return XImageAllocateRaw( width, height, XPixelFormatGrayscale8, texture );
}

// Apply the specified texture to the 8 bpp grayscale image
static void XImageApplyTexture8( const ximage* texture, ximage* image, float amountToKeep, uint8_t textureBaseLevel )
{
    int         width       = image->width;
    int         height      = image->height;
    int         imgStride   = image->stride;
    int         txtStride   = texture->stride;
    uint8_t*    imgPtr      = image->data;
    uint8_t*    txtPtr      = texture->data;
    int         y;
    float       anoutToUse  = 1.0f - amountToKeep;
    float       textureBase = (float) textureBaseLevel;

    #pragma omp parallel for schedule(static) shared( imgPtr, txtPtr, width, imgStride, txtStride, amountToKeep, anoutToUse, textureBase )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* imgRow = imgPtr + y * imgStride;
        uint8_t* txtRow = txtPtr + y * txtStride;
        float    value;
        int      x;

        for ( x = 0; x < width; x++ )
        {
            value = ( amountToKeep * *imgRow ) + ( anoutToUse * *imgRow ) * ( (float) *txtRow / textureBase );
            if ( value > 255 )
            {
                value = 255;
            }
            *imgRow = (uint8_t) value;

            imgRow++;
            txtRow++;
        }
    }
}

// Apply the specified texture to the 24/32 bpp color image
static void XImageApplyTexture24( const ximage* texture, ximage* image, float amountToKeep, uint8_t textureBaseLevel )
{
    int         width       = image->width;
    int         height      = image->height;
    int         imgStride   = image->stride;
    int         txtStride   = texture->stride;
    int         pixelSize   = ( image->format == XPixelFormatRGB24 ) ? 3 : 4;
    uint8_t*    imgPtr      = image->data;
    uint8_t*    txtPtr      = texture->data;
    int         y;
    float       anoutToUse  = 1.0f - amountToKeep;
    float       textureBase = (float) textureBaseLevel;

    #pragma omp parallel for schedule(static) shared( imgPtr, txtPtr, width, imgStride, txtStride, pixelSize, amountToKeep, anoutToUse, textureBase )
    for ( y = 0; y < height; y++ )
    {
        uint8_t* imgRow = imgPtr + y * imgStride;
        uint8_t* txtRow = txtPtr + y * txtStride;
        int      x;
        float    txtCoef;
        float    value;

        for ( x = 0; x < width; x++ )
        {
            txtCoef = (float) *txtRow / textureBase;

            // red
            value = ( amountToKeep * imgRow[RedIndex] ) +
                    ( anoutToUse * imgRow[RedIndex] * txtCoef );
            if ( value > 255 )
            {
                value = 255;
            }
            imgRow[RedIndex] = (uint8_t) value;

            // green
            value = ( amountToKeep * imgRow[GreenIndex] ) +
                    ( anoutToUse * imgRow[GreenIndex] * txtCoef );
            if ( value > 255 )
            {
                value = 255;
            }
            imgRow[GreenIndex] = (uint8_t) value;

            // blue
            value = ( amountToKeep * imgRow[BlueIndex] ) +
                    ( anoutToUse * imgRow[BlueIndex] * txtCoef );
            if ( value > 255 )
            {
                value = 255;
            }
            imgRow[BlueIndex] = (uint8_t) value;

            imgRow += pixelSize;
            txtRow++;
        }
    }
}

// Apply the specified texture to the image
XErrorCode XImageApplyTexture( const ximage* texture, ximage* image, float amountToKeep, uint8_t textureBaseLevel )
{
    XErrorCode ret = SuccessCode;

    amountToKeep = XINRANGE( amountToKeep, 0.0f, 1.0f );
    textureBaseLevel = XMAX( textureBaseLevel, 1 );

    if ( ( texture == 0 ) || ( image == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( texture->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->width > texture->width ) ||
              ( image->height > texture->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        switch ( image->format )
        {
        case XPixelFormatGrayscale8:
            XImageApplyTexture8( texture, image, amountToKeep, textureBaseLevel );
            break;

        case XPixelFormatRGB24:
        case XPixelFormatRGBA32:
            XImageApplyTexture24( texture, image, amountToKeep, textureBaseLevel );
            break;

        default:
            ret = ErrorUnsupportedPixelFormat;
            break;
        }
    }

    return ret;
}
