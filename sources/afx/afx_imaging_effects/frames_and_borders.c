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

#include <memory.h>
#include <math.h>
#include "xtextures.h"
#include "ximaging.h"

// Generate rounded border texture
XErrorCode GenerateRoundedBorderTexture( ximage* texture, uint16_t borderWidth, uint16_t xRoundness, uint16_t yRoundness, uint16_t xRoundnessShift, uint16_t yRoundnessShift, bool addBloor )
{
    XErrorCode ret = SuccessCode;

    if ( texture == 0 )
    {
        ret = ErrorNullParameter;
    }
    else if ( texture->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width             = texture->width;
        int      height            = texture->height;
        int      widthM1           = width - 1;
        int      heightM1          = height - 1;
        int      stride            = texture->stride;
        int      borderSize        = (int) XMIN3( borderWidth, width, height );
        int      widthMborder      = width  - borderSize;
        int      heightMborder     = height - borderSize;
        int      strideM2border    = XMAX( 0, stride - borderSize * 2 );
        int      xRoundnessCorrect = XMIN( (int) xRoundness, width  / 2 - borderSize + xRoundnessShift );
        int      yRoundnessCorrect = XMIN( (int) yRoundness, height / 2 - borderSize + yRoundnessShift );
        int      xRoundnessMxShift = xRoundnessCorrect - xRoundnessShift;
        int      yRoundnessMyShift = yRoundnessCorrect - yRoundnessShift;
        int      borderPxRoundes   = borderSize + xRoundnessMxShift;
        int      borderPyRoundes   = borderSize + yRoundnessMyShift;
        int      x, y;

        uint8_t* ptr = texture->data;
        uint8_t* row1;
        uint8_t* row2;

        // make sure the picture is clean
        for ( y = borderSize; y < heightMborder; y++ )
        {
            row1 = ptr + y * stride + borderSize;
            memset( row1, 0, strideM2border );
        }

        // vertical border
        for ( y = 0; y < height; y++ )
        {
            // left border
            row1 = ptr + y * stride;
            // right border
            row2 = row1 + widthM1;

            for ( x = 0; x < borderSize; x++ )
            {
                *row1 = 255;
                *row2 = 255;
                row1++;
                row2--;
            }
        }

        // horizontal border
        for ( x = borderSize; x < widthMborder; x++ )
        {
            // top border
            row1 = ptr + x;
            // bottom border
            row2 = ptr + heightM1 * stride + x;

            for ( y = 0; y < borderSize; y++ )
            {
                *row1 = 255;
                *row2 = 255;
                row1 += stride;
                row2 -= stride;
            }
        }

        if ( ( xRoundnessShift < xRoundnessCorrect ) &&
             ( yRoundnessShift < yRoundnessCorrect ) )
        {
            double   xRoundnessSq = ( xRoundnessCorrect + 1 ) * ( xRoundnessCorrect + 1 );
            double   yRoundnessSq = ( yRoundnessCorrect + 1 ) * ( yRoundnessCorrect + 1 );
            double   temp;
            int      lastY = -1, lastX = -1;
            int      tempY, tempX, lastProcessedX;
            uint8_t  value, lastValue = 0;
            uint8_t* row3;
            uint8_t* row4;

            // follow X direction first
            for ( x = 0; x <= xRoundnessMxShift; x++ )
            {
                temp  = yRoundnessSq - ( yRoundnessSq * x * x ) / xRoundnessSq;
                temp  = ( temp < 0 ) ? 0.0 : sqrt( temp );
                y     = (int) ( temp + 0.5 );
                value = (uint8_t) ( ( 1.0 - fabs( temp - y ) * 1 )  * 255 );

                // if we had greater value on this row already - use it then
                if ( ( y == lastY ) && ( value < lastValue ) )
                {
                    value = lastValue;
                }
                else
                {
                    lastY = y;
                    lastValue = value;
                }

                if ( y <= yRoundnessMyShift )
                {
                    row1  = ptr + ( borderPyRoundes - y ) * stride + borderPxRoundes - x;
                    row2  = ptr + ( borderPyRoundes - y ) * stride + widthM1 - ( borderPxRoundes - x );
                    row3  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + borderPxRoundes - x;
                    row4  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + widthM1 - ( borderPxRoundes - x );
                    *row1 = value;
                    *row2 = value;
                    *row3 = value;
                    *row4 = value;
                }

                // anti-aliasing "below" (as if doing top-left corner)
                if ( ( y > temp ) && ( y - 1 <= yRoundnessMyShift ) )
                {
                    value = (uint8_t) ( fabs( temp - y ) * 255 );
                    row1  = ptr + ( borderPyRoundes - ( y - 1 ) ) * stride + borderPxRoundes - x;
                    row2  = ptr + ( borderPyRoundes - ( y - 1 ) ) * stride + widthM1 - ( borderPxRoundes - x );
                    row3  = ptr + ( heightM1 - ( borderPyRoundes - ( y - 1 ) ) ) * stride + borderPxRoundes - x;
                    row4  = ptr + ( heightM1 - ( borderPyRoundes - ( y - 1 ) ) ) * stride + widthM1 - ( borderPxRoundes - x );
                    *row1 = value;
                    *row2 = value;
                    *row3 = value;
                    *row4 = value;
                }

                // fill "above" (as if doing top-left corner)
                tempY = y;
                while ( ++tempY <= yRoundnessMyShift )
                {
                    row1  = ptr + ( borderPyRoundes - tempY ) * stride + borderPxRoundes - x;
                    row2  = ptr + ( borderPyRoundes - tempY ) * stride + widthM1 - ( borderPxRoundes - x );
                    row3  = ptr + ( heightM1 - ( borderPyRoundes - tempY ) ) * stride + borderPxRoundes - x;
                    row4  = ptr + ( heightM1 - ( borderPyRoundes - tempY ) ) * stride + widthM1 - ( borderPxRoundes - x );
                    *row1 = 255;
                    *row2 = 255;
                    *row3 = 255;
                    *row4 = 255;
                }

                // stop if X directon is no longer major
                if ( (double) y / yRoundnessSq <= (double) x / xRoundnessSq )
                {
                    break;
                }
            }

            lastProcessedX = x;

            // follow Y direction
            for ( y = 0; y <= yRoundnessMyShift; y++ )
            {
                temp = xRoundnessSq - ( xRoundnessSq * y * y ) / yRoundnessSq;
                temp = ( temp < 0 ) ? 0.0 : sqrt( temp );
                x    = (int) ( temp + 0.5 );
                value = (uint8_t) ( ( 1.0 - fabs( temp - x ) * 1 )  * 255 );

                // if we had greater value on this column already - use it then
                if ( ( x == lastX ) && ( value < lastValue ) )
                {
                    value = lastValue;
                }
                else
                {
                    lastX = x;
                    lastValue = value;
                }

                if ( x <= xRoundnessMxShift )
                {
                    row1  = ptr + ( borderPyRoundes - y ) * stride + borderPxRoundes - x;
                    row2  = ptr + ( borderPyRoundes - y ) * stride + widthM1 - ( borderPxRoundes - x );
                    row3  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + borderPxRoundes - x;
                    row4  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + widthM1 - ( borderPxRoundes - x );
                    *row1 = value;
                    *row2 = value;
                    *row3 = value;
                    *row4 = value;
                }

                // anti-aliasing "on the right" (as if doing top-left corner)
                if ( ( x > temp ) && ( x - 1 <= xRoundnessMxShift ) )
                {
                    row1  = ptr + ( borderPyRoundes - y ) * stride + borderPxRoundes - ( x - 1 );

                    // check only first pointer since others will have same value because of symmetry
                    if ( *row1 == 0 )
                    {
                        value = (uint8_t) ( (  fabs( temp - x ) * 1 )  * 255 );
                        row2  = ptr + ( borderPyRoundes - y  ) * stride + widthM1 - ( borderPxRoundes - ( x - 1 ) );
                        row3  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + borderPxRoundes - ( x - 1 );
                        row4  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + widthM1 - ( borderPxRoundes - ( x - 1 ) );
                        *row1 = value;
                        *row2 = value;
                        *row3 = value;
                        *row4 = value;
                    }
                }

                // fill "on the left" (as if doing top-left corner)
                tempX = x;
                while ( ++tempX <= xRoundnessMxShift )
                {
                    row1  = ptr + ( borderPyRoundes - y ) * stride + borderPxRoundes - tempX;
                    row2  = ptr + ( borderPyRoundes - y ) * stride + widthM1 - ( borderPxRoundes - tempX );
                    row3  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + borderPxRoundes - tempX;
                    row4  = ptr + ( heightM1 - ( borderPyRoundes - y ) ) * stride + widthM1 - ( borderPxRoundes - tempX );
                    *row1 = 255;
                    *row2 = 255;
                    *row3 = 255;
                    *row4 = 255;
                }

                // stop if Y directon is no longer major
                if ( (double) x / xRoundnessSq < (double) y / yRoundnessSq )
                    break;
            }

            for ( x = lastProcessedX + 1; x <= xRoundnessMxShift; x++ )
            {
                row1 = ptr + borderSize * stride + borderPxRoundes - x;
                row2 = ptr + borderSize * stride + widthM1 - ( borderPxRoundes - x );
                row3 = ptr + ( heightM1 - borderSize ) * stride + borderPxRoundes - x;
                row4 = ptr + ( heightM1 - borderSize ) * stride + widthM1 - ( borderPxRoundes - x );

                while ( *row1 == 0 )
                {
                    *row1 = 255;
                    *row2 = 255;
                    *row3 = 255;
                    *row4 = 255;
                    row1 += stride;
                    row2 += stride;
                    row3 -= stride;
                    row4 -= stride;
                }
            }
        }

        // add bloor if required
        if ( addBloor )
        {
            ximage* bluredImage = 0;

            if ( XImageAllocate( width, height, XPixelFormatGrayscale8, &bluredImage ) == SuccessCode )
            {
                BlurImage( texture, bluredImage );
                BlurImage( bluredImage, texture );

                XImageFree( &bluredImage );
            }
        }
    }

    return ret;
}
