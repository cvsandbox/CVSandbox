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

// Calculates distance transformation of a binary image, which represents a distance map -
// shortest distance from non-background pixel to object's edge.
XErrorCode DistanceTransformation( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( dst->width  != src->width  ) ||
              ( dst->height != src->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( dst->format != XPixelFormatGrayscale16 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        int width     = src->width;
        int height    = src->height;
        int widthM1   = width - 1;
        int srcStride = src->stride;
        int dstStride = dst->stride;
        int x, y;

        uint8_t*  srcPtr = src->data;
        uint8_t*  dstPtr = dst->data;
        uint8_t*  srcRow;
        uint16_t* dstRow;
        uint16_t* dstRowPrev;
        uint16_t  value, temp;

        // --- going from top/left to bottom/right ---

        // --- first row
        dstRow = (uint16_t*) dstPtr;
        srcRow = srcPtr;

        for ( x = 0; x < width; x++ )
        {
            *dstRow = ( *srcRow == 0 ) ? 0 : 1;

            dstRow++;
            srcRow++;
        }

        // --- rest of the rows
        for ( y = 1; y < height; y++ )
        {
            srcRow     = srcPtr + y * srcStride;
            dstRow     = (uint16_t*) ( dstPtr + y * dstStride );
            dstRowPrev = (uint16_t*) ( dstPtr + ( y - 1 ) * dstStride );

            // first pixel of the row
            *dstRow = ( *srcRow == 0 ) ? 0 : 1;
            // last pixel of the row
            dstRow[widthM1] = ( srcRow[widthM1] == 0 ) ? 0 : 1;

            // pixels between first and last
            srcRow++;
            dstRow++;
            dstRowPrev++;

            // pixels between first and last
            for ( x = 1; x < widthM1; x++ )
            {
                if ( *srcRow == 0 )
                {
                    *dstRow = 0;
                }
                else
                {
                    value = dstRow[-1];

                    temp = dstRowPrev[-1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = *dstRowPrev;
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = dstRowPrev[1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    *dstRow = value + 1;
                }

                srcRow++;
                dstRow++;
                dstRowPrev++;
            }
        }

        // --- going from bottom/right to top/left ---

        // --- last row
        dstRow = (uint16_t*) ( dstPtr + ( height - 1 ) * dstStride + widthM1 * 2 );

        for ( x = 0; x < width; x++ )
        {
            if ( *dstRow != 0 )
            {
                // if source is 0, the destination is set to 0 already before. so check if non 0 only
                *dstRow = 1;
            }

            dstRow--;
        }

        // --- rest of the rows
        for ( y = height - 2; y >= 0; y-- )
        {
            dstRow     = (uint16_t*) ( dstPtr + y * dstStride + ( widthM1 - 1 ) * 2 );
            dstRowPrev = (uint16_t*) ( dstPtr + ( y + 1 ) * dstStride + ( widthM1 - 1 ) * 2 );

            // pixels between last and first
            for ( x = 1; x < widthM1; x++ )
            {
                if ( *dstRow != 0 )
                {
                    value = dstRow[1];

                    temp = dstRowPrev[1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = *dstRowPrev;
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = dstRowPrev[-1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    if ( value < *dstRow )
                    {
                        *dstRow = value + 1;
                    }
                }

                dstRow--;
                dstRowPrev--;
            }
        }
    }

    return ret;
}

// Calculates distance transformation of background in a binary image, which represents a distance map -
// shortest distance from background pixel to an object.
XErrorCode BackgroundDistanceTransformation( const ximage* src, ximage* dst )
{
    XErrorCode ret = SuccessCode;

    if ( ( src == 0 ) || ( dst == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( src->format != XPixelFormatGrayscale8 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( dst->width  != src->width  ) ||
              ( dst->height != src->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( dst->format != XPixelFormatGrayscale16 )
    {
        ret = ErrorInvalidArgument;
    }
    else
    {
        int width     = src->width;
        int height    = src->height;
        int widthM1   = width - 1;
        int srcStride = src->stride;
        int dstStride = dst->stride;
        int x, y;

        uint8_t*  srcPtr = src->data;
        uint8_t*  dstPtr = dst->data;
        uint8_t*  srcRow;
        uint16_t* dstRow;
        uint16_t* dstRowPrev;
        uint16_t  value, temp;
        uint16_t  mapMaxValue = (uint16_t) XMAX( width, height );

        // --- going from top/left to bottom/right ---

        // --- first row
        dstRow = (uint16_t*) dstPtr;
        srcRow = srcPtr;

        *dstRow = ( *srcRow != 0 ) ? 0 : mapMaxValue; // first pixel
        dstRow++;
        srcRow++;

        for ( x = 1; x < width; x++ )
        {
            if ( *srcRow != 0 )
            {
                *dstRow = 0;
            }
            else
            {
                *dstRow = ( dstRow[-1] == mapMaxValue ) ? mapMaxValue : dstRow[-1] + 1;
            }

            dstRow++;
            srcRow++;
        }

        // --- rest of the rows
        for ( y = 1; y < height; y++ )
        {
            srcRow     = srcPtr + y * srcStride;
            dstRow     = (uint16_t*) ( dstPtr + y * dstStride );
            dstRowPrev = (uint16_t*) ( dstPtr + ( y - 1 ) * dstStride );

            // first pixel of the row
            if ( *srcRow != 0 )
            {
                *dstRow = 0;
            }
            else
            {
                value = *dstRowPrev;

                temp = dstRowPrev[1];
                if ( temp < value )
                {
                    value = temp;
                }

                *dstRow = value + 1;
            }

            // pixels between first and last
            srcRow++;
            dstRow++;
            dstRowPrev++;

            // pixels between first and last
            for ( x = 1; x < widthM1; x++ )
            {
                if ( *srcRow != 0 )
                {
                    *dstRow = 0;
                }
                else
                {
                    value = dstRow[-1];

                    temp = dstRowPrev[-1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = *dstRowPrev;
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = dstRowPrev[1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    *dstRow = value + 1;
                }

                srcRow++;
                dstRow++;
                dstRowPrev++;
            }

            // last pixel of the row
            if ( *srcRow != 0 )
            {
                *dstRow = 0;
            }
            else
            {
                value = dstRow[-1];

                temp = dstRowPrev[-1];
                if ( temp < value )
                {
                    value = temp;
                }

                temp = *dstRowPrev;
                if ( temp < value )
                {
                    value = temp;
                }

                *dstRow = value + 1;
            }
        }

        // --- going from bottom/right to top/left ---

        // --- last row
        // skip the very last pixel, since it should get correct distance in any case
        dstRow = (uint16_t*) ( dstPtr + ( height - 1 ) * dstStride + ( widthM1 - 1 ) * 2 );

        for ( x = 1; x < width; x++ )
        {
            if ( *dstRow != 0 )
            {
                if ( dstRow[1] < *dstRow )
                {
                    *dstRow = dstRow[1] + 1;
                }
            }

            dstRow--;
        }

        // --- rest of the rows
        for ( y = height - 2; y >= 0; y-- )
        {
            dstRow     = (uint16_t*) ( dstPtr + y * dstStride + widthM1 * 2 );
            dstRowPrev = (uint16_t*) ( dstPtr + ( y + 1 ) * dstStride + widthM1 * 2 );

            // last pixel of the row
            if ( *dstRow != 0 )
            {
                value = *dstRowPrev;

                temp = dstRowPrev[-1];
                if ( temp < value )
                {
                    value = temp;
                }

                if ( value < *dstRow )
                {
                    *dstRow = value + 1;
                }
            }

            dstRow--;
            dstRowPrev--;

            // pixels between last and first
            for ( x = 1; x < widthM1; x++ )
            {
                if ( *dstRow != 0 )
                {
                    value = dstRow[1];

                    temp = dstRowPrev[1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = *dstRowPrev;
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    temp = dstRowPrev[-1];
                    if ( temp < value )
                    {
                        value = temp;
                    }

                    if ( value < *dstRow )
                    {
                        *dstRow = value + 1;
                    }
                }

                dstRow--;
                dstRowPrev--;
            }

            // first pixel of the row
            if ( *dstRow != 0 )
            {
                value = dstRow[1];

                temp = dstRowPrev[1];
                if ( temp < value )
                {
                    value = temp;
                }

                temp = *dstRowPrev;
                if ( temp < value )
                {
                    value = temp;
                }

                if ( value < *dstRow )
                {
                    *dstRow = value + 1;
                }
            }
        }
    }

    return ret;
}

// Removes specified amount of objects' edges in a segmented grayscale image
XErrorCode ObjectsThinning( ximage* src, ximage* tempDistanceMap, uint16_t thinningAmount )
{
    XErrorCode ret = DistanceTransformation( src, tempDistanceMap );

    if ( ret == SuccessCode )
    {
        int width     = src->width;
        int height    = src->height;
        int srcStride = src->stride;
        int mapStride = tempDistanceMap->stride;
        int y;

        uint8_t* srcPtr = src->data;
        uint8_t* mapPtr = tempDistanceMap->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, mapPtr, width, height, srcStride, mapStride, thinningAmount )
        for ( y = 0; y < height; y++ )
        {
            uint8_t*  srcRow = srcPtr + y * srcStride;
            uint16_t* mapRow = (uint16_t*) ( mapPtr + y * mapStride );
            int       x;

            for ( x = 0; x < width; x++ )
            {
                *srcRow = ( *mapRow <= thinningAmount ) ? 0 : 255;
                srcRow++;
                mapRow++;
            }
        }
    }

    return ret;
}

// Grow objects' edges by the specified amount in a segmented grayscale image
XErrorCode ObjectsThickening( ximage* src, ximage* tempDistanceMap, uint16_t growingAmount )
{
    XErrorCode ret = BackgroundDistanceTransformation( src, tempDistanceMap );

    if ( ret == SuccessCode )
    {
        int width     = src->width;
        int height    = src->height;
        int srcStride = src->stride;
        int mapStride = tempDistanceMap->stride;
        int y;

        uint8_t* srcPtr = src->data;
        uint8_t* mapPtr = tempDistanceMap->data;

        #pragma omp parallel for schedule(static) shared( srcPtr, mapPtr, width, height, srcStride, mapStride, growingAmount )
        for ( y = 0; y < height; y++ )
        {
            uint8_t*  srcRow = srcPtr + y * srcStride;
            uint16_t* mapRow = (uint16_t*) ( mapPtr + y * mapStride );
            int       x;

            for ( x = 0; x < width; x++ )
            {
                *srcRow = ( *mapRow <= growingAmount ) ? 255 : 0;
                srcRow++;
                mapRow++;
            }
        }
    }

    return ret;
}

// Help function to keep only edge value of the distance map, which produce either objects' edges or
// outliers depending which distance map is calculated
static void KeepEdgeOnly( ximage* src, const ximage* tempDistanceMap, uint16_t edgeThickness, uint16_t gap )
{
    int width     = src->width;
    int height    = src->height;
    int srcStride = src->stride;
    int mapStride = tempDistanceMap->stride;
    int y;

    uint16_t min = gap;
    uint16_t max = gap + edgeThickness;

    uint8_t* srcPtr = src->data;
    uint8_t* mapPtr = tempDistanceMap->data;

    #pragma omp parallel for schedule(static) shared( srcPtr, mapPtr, width, height, srcStride, mapStride, min, max )
    for ( y = 0; y < height; y++ )
    {
        uint8_t*  srcRow = srcPtr + y * srcStride;
        uint16_t* mapRow = (uint16_t*) ( mapPtr + y * mapStride );
        int       x;

        for ( x = 0; x < width; x++ )
        {
            *srcRow = ( ( *mapRow <= min ) || ( *mapRow > max ) ) ? 0 : 255;
            srcRow++;
            mapRow++;
        }
    }
}

// Keeps only specified thickness of objects edges removing anything esle inside (works on segmented grayscale images)
XErrorCode ObjectsEdges( ximage* src, ximage* tempDistanceMap, uint16_t edgeThickness )
{
    XErrorCode ret = DistanceTransformation( src, tempDistanceMap );

    if ( ret == SuccessCode )
    {
        KeepEdgeOnly( src, tempDistanceMap, edgeThickness, 0 );
    }

    return ret;
}

// Similar to object edges, but produces outline instead, which is around the object (not the edges of the object itself)
XErrorCode ObjectsOutline( ximage* src, ximage* tempDistanceMap, uint16_t outlineThickness, uint16_t outlineGap )
{
    XErrorCode ret = BackgroundDistanceTransformation( src, tempDistanceMap );

    if ( ret == SuccessCode )
    {
        KeepEdgeOnly( src, tempDistanceMap, outlineThickness, outlineGap );
    }

    return ret;
}
