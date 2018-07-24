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
#include <xmath.h>

const float TOLERANCE = 1e-13f;

#define Det2( a, b, c, d ) ( (a) * (d) - (b) * (c) )

// Map square to quadrilateral
static int MapSquareToQuad( const xpoint* quad, xmatrix3* matrix )
{
    int   ret = 0;
    float px = (float) ( quad[0].x - quad[1].x + quad[2].x - quad[3].x );
    float py = (float) ( quad[0].y - quad[1].y + quad[2].y - quad[3].y );

    if ( ( px < TOLERANCE ) && ( px > -TOLERANCE ) &&
         ( py < TOLERANCE ) && ( py > -TOLERANCE ) )
    {
        matrix->m11 = (float) ( quad[1].x - quad[0].x );
        matrix->m12 = (float) ( quad[2].x - quad[1].x );
        matrix->m13 = (float) quad[0].x;

        matrix->m21 = (float) ( quad[1].y - quad[0].y );
        matrix->m22 = (float) ( quad[2].y - quad[1].y );
        matrix->m23 = (float) quad[0].y;

        matrix->m31 = 0.0f;
        matrix->m32 = 0.0f;
        matrix->m33 = 1.0f;
    }
    else
    {
        float dx1, dx2, dy1, dy2, det;

        dx1 = (float) ( quad[1].x - quad[2].x );
        dx2 = (float) ( quad[3].x - quad[2].x );
        dy1 = (float) ( quad[1].y - quad[2].y );
        dy2 = (float) ( quad[3].y - quad[2].y );

        det = Det2( dx1, dx2, dy1, dy2 );

        if ( det == 0.0 )
        {
            ret = 1;
        }
        else
        {
            matrix->m31 = Det2( px, dx2, py, dy2 ) / det;
            matrix->m32 = Det2( dx1, px, dy1, py ) / det;
            matrix->m33 = 1.0f;

            matrix->m11 = quad[1].x - quad[0].x + matrix->m31 * quad[1].x;
            matrix->m12 = quad[3].x - quad[0].x + matrix->m32 * quad[3].x;
            matrix->m13 = (float) quad[0].x;

            matrix->m21 = quad[1].y - quad[0].y + matrix->m31 * quad[1].y;
            matrix->m22 = quad[3].y - quad[0].y + matrix->m32 * quad[3].y;
            matrix->m23 = (float) quad[0].y;
        }
    }

    return ret;
}

// Map one quadrilateral to another
static int MapQuadToQuad( const xpoint* targetQuadrilateral, const xpoint* sourceQuadrilateral, xmatrix3* matrix )
{
    xmatrix3 squareToTarget  = { 0 };
    xmatrix3 squareToSource  = { 0 };
    int      ret             = 0;

    if ( ( MapSquareToQuad( targetQuadrilateral, &squareToTarget ) != 0 ) ||
         ( MapSquareToQuad( sourceQuadrilateral, &squareToSource ) != 0 ) )
    {
        ret = 1;
    }
    else
    {
        xmatrix3 adj;

        MatrixAdjugate( &squareToTarget, &adj );
        MatrixMultiply( &squareToSource, &adj, matrix );
    }

    return ret;
}

// Perform quadrilateral transformation using the specified matrix
static void TransformQuadrilateral( const ximage* source, ximage* target, xmatrix3* transformMatrix, int tx1, int ty1, int tx2, int ty2, bool interpolate, bool fillSpaces )
{
    xmatrix3 transMatrix = *transformMatrix;

    int dstStride = target->stride;
    int srcWidth  = source->width;
    int srcHeight = source->height;
    int srcStride = source->stride;
    int pixelSize = ( target->format == XPixelFormatGrayscale8 ) ? 1 : ( target->format == XPixelFormatRGB24 ) ? 3 : 4;
    int y;

    uint8_t* srcPtr = source->data;
    uint8_t* dstPtr = target->data;

    if ( interpolate == false )
    {
        #pragma omp parallel for schedule(static) shared( transMatrix, srcWidth, srcHeight, srcStride, tx1, tx2, ty1, ty2, dstStride, pixelSize, srcPtr, dstPtr )
        for ( y = ty1; y <= ty2; y++ )
        {
            float factor;
            int   x, i, srcX, srcY;

            uint8_t* dstRow = dstPtr + y * dstStride + tx1 * pixelSize;
            uint8_t* p;

            for ( x = tx1; x <= tx2; x++ )
            {
                factor =           transMatrix.m31 * x + transMatrix.m32 * y + transMatrix.m33;
                srcX   = (int) ( ( transMatrix.m11 * x + transMatrix.m12 * y + transMatrix.m13 ) / factor );
                srcY   = (int) ( ( transMatrix.m21 * x + transMatrix.m22 * y + transMatrix.m23 ) / factor );

                if ( ( srcX >= 0 ) && ( srcY >= 0 ) && ( srcX < srcWidth ) && ( srcY < srcHeight ) )
                {
                    // get pointer to the pixel in the source image
                    p = srcPtr + srcY * srcStride + srcX * pixelSize;
                    // copy pixel's values
                    for ( i = 0; i < pixelSize; i++, dstRow++, p++ )
                    {
                        *dstRow = *p;
                    }
                }
                else
                {
                    if ( fillSpaces == true )
                    {
                        for ( i = 0; i < pixelSize; i++, dstRow++ )
                        {
                            *dstRow = 0;
                        }
                    }
                    else
                    {
                        dstRow += pixelSize;
                    }
                }
            }
        }
    }
    else
    {
        int srcWidthM1  = source->width - 1;
        int srcHeightM1 = source->height - 1;

        #pragma omp parallel for schedule(static) shared( transMatrix, srcWidth, srcHeight, srcWidthM1, srcHeightM1, srcStride, tx1, tx2, ty1, ty2, dstStride, pixelSize, srcPtr, dstPtr )
        for( y = ty1; y <= ty2; y++ )
        {
            float factor, srcX, srcY;
            float dx1, dy1, dx2, dy2;
            int   sx1, sy1, sx2, sy2;
            int   x, i;

            uint8_t* dstRow = dstPtr + y * dstStride + tx1 * pixelSize;
            uint8_t  *p1, *p2, *p3, *p4;

            for ( x = tx1; x <= tx2; x++ )
            {
                factor =  transMatrix.m31 * x + transMatrix.m32 * y + transMatrix.m33;
                srcX  = ( transMatrix.m11 * x + transMatrix.m12 * y + transMatrix.m13 ) / factor;
                srcY  = ( transMatrix.m21 * x + transMatrix.m22 * y + transMatrix.m23 ) / factor;

                sx1 = (int) srcX;
                sy1 = (int) srcY;

                if ( ( sx1 >= 0 ) && ( sy1 >= 0 ) && ( sx1 < srcWidth ) && ( sy1 < srcHeight ) )
                {
                    // get rid of possible negative fractions
                    if ( srcX < 0 ) srcX = 0.0f;
                    if ( srcY < 0 ) srcY = 0.0f;

                    sx2 = ( sx1 == srcWidthM1 ) ? sx1 : sx1 + 1;
                    dx1 = srcX - sx1;
                    dx2 = 1.0f - dx1;

                    sy2 = ( sy1 == srcHeightM1 ) ? sy1 : sy1 + 1;
                    dy1 = srcY - sy1;
                    dy2 = 1.0f - dy1;

                    // get four points
                    p1 = p2 = srcPtr + sy1 * srcStride;
                    p1 += sx1 * pixelSize;
                    p2 += sx2 * pixelSize;

                    p3 = p4 = srcPtr + sy2 * srcStride;
                    p3 += sx1 * pixelSize;
                    p4 += sx2 * pixelSize;

                    // interpolate using 4 points
                    for ( i = 0; i < pixelSize; i++, dstRow++, p1++, p2++, p3++, p4++ )
                    {
                        *dstRow = (uint8_t) (
                            dy2 * ( dx2 * ( *p1 ) + dx1 * ( *p2 ) ) +
                            dy1 * ( dx2 * ( *p3 ) + dx1 * ( *p4 ) ) );
                    }
                }
                else
                {
                    if ( fillSpaces == true )
                    {
                        for ( i = 0; i < pixelSize; i++, dstRow++ )
                        {
                            *dstRow = 0;
                        }
                    }
                    else
                    {
                        dstRow += pixelSize;
                    }
                }
            }
        }
    }
}

// Embed source image into target using the specified 4 quadrilateral points
XErrorCode EmbedQuadrilateral( ximage* target, const ximage* source, const xpoint* targetQuadrilateral, bool interpolate )
{
    XErrorCode ret = SuccessCode;

    if ( ( target == 0 ) || ( source == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( target->format != XPixelFormatGrayscale8 ) &&
              ( target->format != XPixelFormatRGB24 ) &&
              ( target->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( target->format != source->format )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        int  minX, maxX, minY, maxY;

        // get bounding rectangle of the quadrilateral
        minX = XMIN( targetQuadrilateral[0].x, targetQuadrilateral[1].x );
        if ( targetQuadrilateral[2].x < minX ) minX = targetQuadrilateral[2].x;
        if ( targetQuadrilateral[3].x < minX ) minX = targetQuadrilateral[3].x;
        maxX = XMAX( targetQuadrilateral[0].x, targetQuadrilateral[1].x );
        if ( targetQuadrilateral[2].x > maxX ) maxX = targetQuadrilateral[2].x;
        if ( targetQuadrilateral[3].x > maxX ) maxX = targetQuadrilateral[3].x;
        minY = XMIN( targetQuadrilateral[0].y, targetQuadrilateral[1].y );
        if ( targetQuadrilateral[2].y < minY ) minY = targetQuadrilateral[2].y;
        if ( targetQuadrilateral[3].y < minY ) minY = targetQuadrilateral[3].y;
        maxY = XMAX( targetQuadrilateral[0].y, targetQuadrilateral[1].y );
        if ( targetQuadrilateral[2].y > maxY ) maxY = targetQuadrilateral[2].y;
        if ( targetQuadrilateral[3].y > maxY ) maxY = targetQuadrilateral[3].y;

        // make sure there is overlap with the image
        if ( ( maxX >= 0 ) && ( maxY >= 0 ) && ( minX < target->width ) && ( minY < target->height ) )
        {
            xpoint   sourceQuadrilateral[4] = { { 0 } };
            xmatrix3 transMatrix;

            // clip the bounding rectangle
            if ( minX < 0 ) minX = 0;
            if ( minY < 0 ) minY = 0;
            if ( maxX >= target->width )  maxX = target->width  - 1;
            if ( maxY >= target->height ) maxY = target->height - 1;

            // set source quadrilateral
            sourceQuadrilateral[2].x = sourceQuadrilateral[1].x = source->width  - 1;
            sourceQuadrilateral[2].y = sourceQuadrilateral[3].y = source->height - 1;

            // calculate transformation matrix
            if ( MapQuadToQuad( targetQuadrilateral, sourceQuadrilateral, &transMatrix ) != 0 )
            {
                ret = ErrorFailed;
            }
            else
            {
                TransformQuadrilateral( source, target, &transMatrix, minX, minY, maxX, maxY, interpolate, false );
            }
        }
    }

    return ret;
}

// Extract specified quadrilateral from source image into target (the target's image size specifies the result size)
XErrorCode ExtractQuadrilateral( const ximage* source, ximage* target, const xpoint* sourceQuadrilateral, bool interpolate )
{
    XErrorCode ret = SuccessCode;

    if ( ( target == 0 ) || ( source == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( target->format != XPixelFormatGrayscale8 ) &&
              ( target->format != XPixelFormatRGB24 ) &&
              ( target->format != XPixelFormatRGBA32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( target->format != source->format )
    {
        ret = ErrorImageParametersMismatch;
    }
    else
    {
        xpoint   targetQuadrilateral[4] = { { 0 } };
        xmatrix3 transMatrix;

        // set taregt quadrilateral
        targetQuadrilateral[2].x = targetQuadrilateral[1].x = target->width  - 1;
        targetQuadrilateral[2].y = targetQuadrilateral[3].y = target->height - 1;

        // calculate transformation matrix
        if ( MapQuadToQuad( targetQuadrilateral, sourceQuadrilateral, &transMatrix ) != 0 )
        {
            ret = ErrorFailed;
        }
        else
        {
            TransformQuadrilateral( source, target, &transMatrix, 0, 0, target->width - 1, target->height - 1, interpolate, true );
        }
    }

    return ret;
}
