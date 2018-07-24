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

#include <memory.h>
#include "ximaging.h"

// Perform relabelling and merging of the connected components
static XErrorCode BcPerformRelabelling( uint32_t* mapPtr, uint32_t mapSize, const uint32_t* labels, uint32_t labelsCount, uint32_t* objectsCountFound )
{
    XErrorCode ret          = SuccessCode;
    uint32_t*  reLabels     = (uint32_t*) malloc( ( labelsCount + 1 ) * sizeof( uint32_t ) );
    uint32_t   objectsCount = 0;
    uint32_t   i, lt;

    if ( reLabels == 0 )
    {
        ret = ErrorOutOfMemory;
    }
    else
    {
        // count objects and prepare remapping array
        reLabels[0] = 0;
        for ( i = 1; i <= labelsCount; i++ )
        {
            if ( labels[i] == i )
            {
                // increase objects count
                reLabels[i] = ++objectsCount;
            }
        }
        // second pass to complete remapping
        for ( i = 1; i <= labelsCount; i++ )
        {
            // make sure we get the final label
            lt = labels[i];
            while ( lt != labels[lt] )
            {
                lt = labels[lt];
            }

            reLabels[i] = reLabels[lt];
        }

        // repair object labels
        for ( i = 0; i < mapSize; i++, mapPtr++ )
        {
            *mapPtr = reLabels[*mapPtr];
        }

        if ( objectsCountFound )
        {
            *objectsCountFound = objectsCount;
        }

        free( reLabels );
    }

    return ret;
}

// Build map of disconnected objects and count them
XErrorCode BcBuildObjectsMap( const ximage* image, ximage* map, uint32_t* objectsCountFound, uint32_t* tempLabelsMap, uint32_t tempLabelsMapSize )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( map == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( ( image->format != XPixelFormatGrayscale8 ) && ( image->format != XPixelFormatRGB24 ) && ( image->format != XPixelFormatRGBA32 ) ) ||
              ( map->format != XPixelFormatGrayscale32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->width != map->width ) || ( image->height != map->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( image->width <= 1 )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( map->stride != map->width * 4 )
    {
        // to simplify things we want map's stride to be equal its line size
        ret = ErrorFailed; // not too helpful error message, but at least something for now
    }
    else if ( ( tempLabelsMap != 0 ) && ( tempLabelsMapSize < (uint32_t) ( ( ( image->width / 2 ) + 1 ) * ( ( image->height / 2 ) + 1 ) + 1 ) ) )
    {
        ret = ErrorTooSmallBuffer;
    }
    else
    {
        int       width        = image->width;
        int       widthM1      = width - 1;
        int       mWidth       = -width;
        int       mWidthP1     = -width + 1;
        int       mWidthM1     = -width - 1;
        int       height       = image->height;
        int       stride       = image->stride;
        int       mStride      = -stride;
        int       mStrideP1    = -stride + 1;
        int       mStrideM1    = -stride - 1;
        int       pixelSize    = ( image->format == XPixelFormatGrayscale8 ) ? 1 : ( ( image->format == XPixelFormatRGB24 ) ? 3 : 4 );
        int       offset       = stride - width * pixelSize;
        uint32_t  labelsCount  = 0;
        uint32_t  maxObjects   = ( ( width / 2 ) + 1 ) * ( ( height / 2 ) + 1 ) + 1;
        uint8_t*  ptr          = image->data;
        uint32_t* mp           = (uint32_t*) map->data;

        uint32_t* labels       = ( tempLabelsMap != 0 ) ? tempLabelsMap : (uint32_t*) malloc( maxObjects * sizeof( uint32_t ) );

        int       x, y;
        uint32_t  i, l1, l2, l1t, l2t;

        if ( labels == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            memset( mp, 0, width * height * sizeof( uint32_t ) );

            // --------------------------------------
            // initially map all labels to themself
            for ( i = 0; i < maxObjects; i++ )
            {
                labels[i] = i;
            }

            if ( image->format == XPixelFormatGrayscale8 )
            {
                // Grayscale images

                // --------------------------------------
                // 1 - for pixels of the first row
                if ( *ptr != 0 )
                {
                    *mp = ++labelsCount;
                }

                mp++;
                ptr++;

                // process the rest of the first row
                for ( x = 1; x < width; x++, ptr++, mp++ )
                {
                    // check if we need to label current pixel
                    if ( *ptr != 0 )
                    {
                        // check if the previous pixel already was labeled
                        if ( ptr[-1] != 0 )
                        {
                            // label current pixel, as the previous
                            *mp = mp[-1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                }
                ptr += offset;

                // --------------------------------------
                // 2 - for all other rows
                for ( y = 1; y < height; y++ )
                {
                    // for the first pixel of the row, we need to check
                    // only upper and upper-right pixels
                    if ( *ptr != 0 )
                    {
                        // check surrounding pixels
                        if ( ptr[mStride] != 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else if ( ptr[mStrideP1] != 0 )
                        {
                            // label current pixel, as the above right
                            *mp = mp[mWidthP1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    ++ptr;
                    ++mp;

                    // check left pixel and three upper pixels for the rest of pixels
                    for ( x = 1; x < widthM1; x++, ptr++, mp++ )
                    {
                        if ( *ptr != 0 )
                        {
                            // check surrounding pixels
                            if ( ptr[-1] != 0 )
                            {
                                // label current pixel, as the left
                                *mp = mp[-1];
                            }
                            else if ( ptr[mStrideM1] != 0 )
                            {
                                // label current pixel, as the above left
                                *mp = mp[mWidthM1];
                            }
                            else if ( ptr[mStride] != 0 )
                            {
                                // label current pixel, as the above
                                *mp = mp[mWidth];
                            }

                            if ( ptr[mStrideP1] != 0 )
                            {
                                if ( *mp == 0 )
                                {
                                    // since the pixel is not yet labeled, label it as the above right
                                    *mp = mp[mWidthP1];
                                }
                                else
                                {
                                    // get two labels ...
                                    l1  = *mp;
                                    l2  = mp[mWidthP1];
                                    // ... and their targets
                                    l1t = labels[l1];
                                    l2t = labels[l2];

                                    if ( ( l1 != l2 ) && ( l1t != l2t ) )
                                    {
                                        // merge
                                        if ( l1t == l1 )
                                        {
                                            // get the final target
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }
                                            // map left value to the right
                                            labels[l1] = l2t;
                                        }
                                        else if ( l2t == l2 )
                                        {
                                            // get the final target
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            // map right value to the left
                                            labels[l2] = l1t;
                                        }
                                        else
                                        {
                                            // get the final targets
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }

                                            // propogate remapping
                                            for ( i = 1; i <= labelsCount; i++ )
                                            {
                                                if ( labels[i] == l1t )
                                                {
                                                    labels[i] = l2t;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // label the object if it is not yet
                            if ( *mp == 0 )
                            {
                                // create new label
                                *mp = ++labelsCount;
                            }
                        }
                    }

                    // for the last pixel of the row, we need to check
                    // only upper and upper-left pixels
                    if ( *ptr != 0 )
                    {
                        // check surrounding pixels
                        if ( ptr[-1] != 0 )
                        {
                            // label current pixel, as the left
                            *mp = mp[-1];
                        }
                        else if ( ptr[mStrideM1] != 0 )
                        {
                            // label current pixel, as the above left
                            *mp = mp[mWidthM1];
                        }
                        else if ( ptr[mStride] != 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    ++ptr;
                    ++mp;

                    ptr += offset;
                }
            }
            else
            {
                // RGB(A) images

                // --------------------------------------
                // 1 - for pixels of the first row
                if ( ( ptr[RedIndex] != 0 ) && ( ptr[GreenIndex] != 0 ) && ( ptr[BlueIndex] != 0 ) )
                {
                    *mp = ++labelsCount;
                }

                mp++;
                ptr += pixelSize;

                // process the rest of the first row
                for ( x = 1; x < width; x++, ptr += pixelSize, mp++ )
                {
                    // check if we need to label current pixel
                    if ( ( ptr[RedIndex] != 0 ) && ( ptr[GreenIndex] != 0 ) && ( ptr[BlueIndex] != 0 ) )
                    {
                        // check if the previous pixel already was labeled
                        if ( mp[-1] != 0 )
                        {
                            // label current pixel, as the previous
                            *mp = mp[-1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                }
                ptr += offset;

                // --------------------------------------
                // 2 - for all other rows
                for ( y = 1; y < height; y++ )
                {
                    // for the first pixel of the row, we need to check
                    // only upper and upper-right pixels
                    if ( ( ptr[RedIndex] != 0 ) && ( ptr[GreenIndex] != 0 ) && ( ptr[BlueIndex] != 0 ) )
                    {
                        // check surrounding pixels
                        if ( mp[mWidth] != 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else if ( mp[mWidthP1] != 0 )
                        {
                            // label current pixel, as the above right
                            *mp = mp[mWidthP1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    ptr += pixelSize;
                    mp++;

                    // check left pixel and three upper pixels for the rest of pixels
                    for ( x = 1; x < widthM1; x++, ptr += pixelSize, mp++ )
                    {
                        if ( ( ptr[RedIndex] != 0 ) && ( ptr[GreenIndex] != 0 ) && ( ptr[BlueIndex] != 0 ) )
                        {
                            // check surrounding pixels
                            if ( mp[-1] != 0 )
                            {
                                // label current pixel, as the left
                                *mp = mp[-1];
                            }
                            else if ( mp[mWidthM1] != 0 )
                            {
                                // label current pixel, as the above left
                                *mp = mp[mWidthM1];
                            }
                            else if ( mp[mWidth] != 0 )
                            {
                                // label current pixel, as the above
                                *mp = mp[mWidth];
                            }

                            if ( mp[mWidthP1] != 0 )
                            {
                                if ( *mp == 0 )
                                {
                                    // since the pixel is not yet labeled, label it as the above right
                                    *mp = mp[mWidthP1];
                                }
                                else
                                {
                                    // get two labels ...
                                    l1 = *mp;
                                    l2 = mp[mWidthP1];
                                    // ... and their targets
                                    l1t = labels[l1];
                                    l2t = labels[l2];

                                    if ( ( l1 != l2 ) && ( l1t != l2t ) )
                                    {
                                        // merge
                                        if ( l1t == l1 )
                                        {
                                            // get the final target
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }
                                            // map left value to the right
                                            labels[l1] = l2t;
                                        }
                                        else if ( l2t == l2 )
                                        {
                                            // get the final target
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            // map right value to the left
                                            labels[l2] = l1t;
                                        }
                                        else
                                        {
                                            // get the final targets
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }

                                            // propogate remapping
                                            for ( i = 1; i <= labelsCount; i++ )
                                            {
                                                if ( labels[i] == l1t )
                                                {
                                                    labels[i] = l2t;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // label the object if it is not yet
                            if ( *mp == 0 )
                            {
                                // create new label
                                *mp = ++labelsCount;
                            }
                        }
                    }

                    // for the last pixel of the row, we need to check
                    // only upper and upper-left pixels
                    if ( ( ptr[RedIndex] != 0 ) && ( ptr[GreenIndex] != 0 ) && ( ptr[BlueIndex] != 0 ) )
                    {
                        // check surrounding pixels
                        if ( mp[-1] != 0 )
                        {
                            // label current pixel, as the left
                            *mp = mp[-1];
                        }
                        else if ( mp[mWidthM1] != 0 )
                        {
                            // label current pixel, as the above left
                            *mp = mp[mWidthM1];
                        }
                        else if ( mp[mWidth] != 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    mp++;

                    ptr += ( pixelSize + offset );
                }
            }

            // --------------------------------------
            // perform final labelling
            ret = BcPerformRelabelling( (uint32_t*) map->data, width * height, labels, labelsCount, objectsCountFound );

            if ( tempLabelsMap == 0 )
            {
                free( labels );
            }
        }
    }

    return ret;
}

// Build map of disconnected background areas and count them
XErrorCode BcBuildBackgroundMap( const ximage* image, ximage* map, uint32_t* objectsCountFound, uint32_t* tempLabelsMap, uint32_t tempLabelsMapSize )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( map == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( ( image->format != XPixelFormatGrayscale8 ) && ( image->format != XPixelFormatRGB24 ) && ( image->format != XPixelFormatRGBA32 ) ) ||
              ( map->format != XPixelFormatGrayscale32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->width != map->width ) || ( image->height != map->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( image->width <= 1 )
    {
        ret = ErrorImageIsTooSmall;
    }
    else if ( map->stride != map->width * 4 )
    {
        // to simplify things we want map's stride to be equal its line size
        ret = ErrorFailed; // not too helpful error message, but at least something for now
    }
    else if ( ( tempLabelsMap != 0 ) && ( tempLabelsMapSize < (uint32_t) ( ( ( image->width / 2 ) + 1 ) * ( ( image->height / 2 ) + 1 ) + 1 ) ) )
    {
        ret = ErrorTooSmallBuffer;
    }
    else
    {
        int       width        = image->width;
        int       widthM1      = width - 1;
        int       mWidth       = -width;
        int       mWidthP1     = -width + 1;
        int       mWidthM1     = -width - 1;
        int       height       = image->height;
        int       stride       = image->stride;
        int       mStride      = -stride;
        int       mStrideP1    = -stride + 1;
        int       mStrideM1    = -stride - 1;
        int       pixelSize    = ( image->format == XPixelFormatGrayscale8 ) ? 1 : ( ( image->format == XPixelFormatRGB24 ) ? 3 : 4 );
        int       offset       = stride - width * pixelSize;
        uint32_t  labelsCount  = 0;
        uint32_t  maxObjects   = ( ( width / 2 ) + 1 ) * ( ( height / 2 ) + 1 ) + 1;
        uint8_t*  ptr          = image->data;
        uint32_t* mp           = (uint32_t*) map->data;

        uint32_t* labels       = ( tempLabelsMap != 0 ) ? tempLabelsMap : (uint32_t*) malloc( maxObjects * sizeof( uint32_t ) );

        int       x, y;
        uint32_t  i, l1, l2, l1t, l2t;

        if ( labels == 0 )
        {
            ret = ErrorOutOfMemory;
        }
        else
        {
            memset( mp, 0, width * height * sizeof( uint32_t ) );

            // --------------------------------------
            // initially map all labels to themself
            for ( i = 0; i < maxObjects; i++ )
            {
                labels[i] = i;
            }

            if ( image->format == XPixelFormatGrayscale8 )
            {
                // Grayscale images

                // --------------------------------------
                // 1 - for pixels of the first row
                if ( *ptr == 0 )
                {
                    *mp = ++labelsCount;
                }

                mp++;
                ptr++;

                // process the rest of the first row
                for ( x = 1; x < width; x++, ptr++, mp++ )
                {
                    // check if we need to label current pixel
                    if ( *ptr == 0 )
                    {
                        // check if the previous pixel already was labeled
                        if ( ptr[-1] == 0 )
                        {
                            // label current pixel, as the previous
                            *mp = mp[-1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                }
                ptr += offset;

                // --------------------------------------
                // 2 - for all other rows
                for ( y = 1; y < height; y++ )
                {
                    // for the first pixel of the row, we need to check
                    // only upper and upper-right pixels
                    if ( *ptr == 0 )
                    {
                        // check surrounding pixels
                        if ( ptr[mStride] == 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else if ( ptr[mStrideP1] == 0 )
                        {
                            // label current pixel, as the above right
                            *mp = mp[mWidthP1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    ++ptr;
                    ++mp;

                    // check left pixel and three upper pixels for the rest of pixels
                    for ( x = 1; x < widthM1; x++, ptr++, mp++ )
                    {
                        if ( *ptr == 0 )
                        {
                            // check surrounding pixels
                            if ( ptr[-1] == 0 )
                            {
                                // label current pixel, as the left
                                *mp = mp[-1];
                            }
                            else if ( ptr[mStrideM1] == 0 )
                            {
                                // label current pixel, as the above left
                                *mp = mp[mWidthM1];
                            }
                            else if ( ptr[mStride] == 0 )
                            {
                                // label current pixel, as the above
                                *mp = mp[mWidth];
                            }

                            if ( ptr[mStrideP1] == 0 )
                            {
                                if ( *mp == 0 )
                                {
                                    // since the pixel is not yet labeled, label it as the above right
                                    *mp = mp[mWidthP1];
                                }
                                else
                                {
                                    // get two labels ...
                                    l1 = *mp;
                                    l2 = mp[mWidthP1];
                                    // ... and their targets
                                    l1t = labels[l1];
                                    l2t = labels[l2];

                                    if ( ( l1 != l2 ) && ( l1t != l2t ) )
                                    {
                                        // merge
                                        if ( l1t == l1 )
                                        {
                                            // get the final target
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }
                                            // map left value to the right
                                            labels[l1] = l2t;
                                        }
                                        else if ( l2t == l2 )
                                        {
                                            // get the final target
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            // map right value to the left
                                            labels[l2] = l1t;
                                        }
                                        else
                                        {
                                            // get the final targets
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }

                                            // propogate remapping
                                            for ( i = 1; i <= labelsCount; i++ )
                                            {
                                                if ( labels[i] == l1t )
                                                {
                                                    labels[i] = l2t;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // label the object if it is not yet
                            if ( *mp == 0 )
                            {
                                // create new label
                                *mp = ++labelsCount;
                            }
                        }
                    }

                    // for the last pixel of the row, we need to check
                    // only upper and upper-left pixels
                    if ( *ptr == 0 )
                    {
                        // check surrounding pixels
                        if ( ptr[-1] == 0 )
                        {
                            // label current pixel, as the left
                            *mp = mp[-1];
                        }
                        else if ( ptr[mStrideM1] == 0 )
                        {
                            // label current pixel, as the above left
                            *mp = mp[mWidthM1];
                        }
                        else if ( ptr[mStride] == 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    ++ptr;
                    ++mp;

                    ptr += offset;
                }
            }
            else
            {
                // RGB(A) images

                // --------------------------------------
                // 1 - for pixels of the first row
                if ( ( ptr[RedIndex] == 0 ) && ( ptr[GreenIndex] == 0 ) && ( ptr[BlueIndex] == 0 ) )
                {
                    *mp = ++labelsCount;
                }

                mp++;
                ptr += pixelSize;

                // process the rest of the first row
                for ( x = 1; x < width; x++, ptr += pixelSize, mp++ )
                {
                    // check if we need to label current pixel
                    if ( ( ptr[RedIndex] == 0 ) && ( ptr[GreenIndex] == 0 ) && ( ptr[BlueIndex] == 0 ) )
                    {
                        // check if the previous pixel already was labeled
                        if ( mp[-1] != 0 )
                        {
                            // label current pixel, as the previous
                            *mp = mp[-1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                }
                ptr += offset;

                // --------------------------------------
                // 2 - for all other rows
                for ( y = 1; y < height; y++ )
                {
                    // for the first pixel of the row, we need to check
                    // only upper and upper-right pixels
                    if ( ( ptr[RedIndex] == 0 ) && ( ptr[GreenIndex] == 0 ) && ( ptr[BlueIndex] == 0 ) )
                    {
                        // check surrounding pixels
                        if ( mp[mWidth] != 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else if ( mp[mWidthP1] != 0 )
                        {
                            // label current pixel, as the above right
                            *mp = mp[mWidthP1];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    ptr += pixelSize;
                    mp++;

                    // check left pixel and three upper pixels for the rest of pixels
                    for ( x = 1; x < widthM1; x++, ptr += pixelSize, mp++ )
                    {
                        if ( ( ptr[RedIndex] == 0 ) && ( ptr[GreenIndex] == 0 ) && ( ptr[BlueIndex] == 0 ) )
                        {
                            // check surrounding pixels
                            if ( mp[-1] != 0 )
                            {
                                // label current pixel, as the left
                                *mp = mp[-1];
                            }
                            else if ( mp[mWidthM1] != 0 )
                            {
                                // label current pixel, as the above left
                                *mp = mp[mWidthM1];
                            }
                            else if ( mp[mWidth] != 0 )
                            {
                                // label current pixel, as the above
                                *mp = mp[mWidth];
                            }

                            if ( mp[mWidthP1] != 0 )
                            {
                                if ( *mp == 0 )
                                {
                                    // since the pixel is not yet labeled, label it as the above right
                                    *mp = mp[mWidthP1];
                                }
                                else
                                {
                                    // get two labels ...
                                    l1 = *mp;
                                    l2 = mp[mWidthP1];
                                    // ... and their targets
                                    l1t = labels[l1];
                                    l2t = labels[l2];

                                    if ( ( l1 != l2 ) && ( l1t != l2t ) )
                                    {
                                        // merge
                                        if ( l1t == l1 )
                                        {
                                            // get the final target
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }
                                            // map left value to the right
                                            labels[l1] = l2t;
                                        }
                                        else if ( l2t == l2 )
                                        {
                                            // get the final target
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            // map right value to the left
                                            labels[l2] = l1t;
                                        }
                                        else
                                        {
                                            // get the final targets
                                            while ( l1t != labels[l1t] ) { l1t = labels[l1t]; }
                                            while ( l2t != labels[l2t] ) { l2t = labels[l2t]; }

                                            // propogate remapping
                                            for ( i = 1; i <= labelsCount; i++ )
                                            {
                                                if ( labels[i] == l1t )
                                                {
                                                    labels[i] = l2t;
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            // label the object if it is not yet
                            if ( *mp == 0 )
                            {
                                // create new label
                                *mp = ++labelsCount;
                            }
                        }
                    }

                    // for the last pixel of the row, we need to check
                    // only upper and upper-left pixels
                    if ( ( ptr[RedIndex] == 0 ) && ( ptr[GreenIndex] == 0 ) && ( ptr[BlueIndex] == 0 ) )
                    {
                        // check surrounding pixels
                        if ( mp[-1] != 0 )
                        {
                            // label current pixel, as the left
                            *mp = mp[-1];
                        }
                        else if ( mp[mWidthM1] != 0 )
                        {
                            // label current pixel, as the above left
                            *mp = mp[mWidthM1];
                        }
                        else if ( mp[mWidth] != 0 )
                        {
                            // label current pixel, as the above
                            *mp = mp[mWidth];
                        }
                        else
                        {
                            // create new label
                            *mp = ++labelsCount;
                        }
                    }
                    mp++;

                    ptr += ( pixelSize + offset );
                }
            }

            // --------------------------------------
            // perform final labelling
            ret = BcPerformRelabelling( (uint32_t*) map->data, width * height, labels, labelsCount, objectsCountFound );

            if ( tempLabelsMap == 0 )
            {
                free( labels );
            }
        }
    }

    return ret;
}

// Create colored map (colors are not unique) from the map of disconnected objects
XErrorCode BcDoColorLabelling( const ximage* map, ximage* image )
{
    const static xargb colors[] = { { 0xFFFF0000 }, { 0xFF00FF00 }, { 0xFF0000FF }, { 0xFFFFFF00 }, { 0xFF00FFFF }, { 0xFFFF00FF }, { 0xFFFFFFFF },
                                    { 0xFFFFC0C0 }, { 0xFFC0FFC0 }, { 0xFFC0C0FF }, { 0xFFFFFFC0 }, { 0xFFC0FFFF }, { 0xFFFFC0FF }, { 0xFFC0C0C0 },
                                    { 0xFFFF8080 }, { 0xFF80FF80 }, { 0xFF8080FF }, { 0xFFFFFF80 }, { 0xFF80FFFF }, { 0xFFFF80FF }, { 0xFF808080 },
                                    { 0xFFD04040 }, { 0xFF40D040 }, { 0xFF4040D0 }, { 0xFFD0D040 }, { 0xFF40D0D0 }, { 0xFFD040D0 }, { 0xFF404040 } };
    const int colorsCount = XARRAY_SIZE( colors );

    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( map == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( image->format != XPixelFormatRGB24 ) ||
              ( map->format != XPixelFormatGrayscale32 ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->width != map->width ) || ( image->height != map->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( map->stride != map->width * 4 )
    {
        ret = ErrorFailed;
    }
    else
    {
        int      width  = image->width;
        int      height = image->height;
        int      stride = image->stride;
        int      offset = stride - width * 3;
        uint8_t* ptr    = image->data;

        int      x, y;
        uint32_t mapValue;
        xargb    c;

        for ( y = 0; y < height; y++ )
        {
            uint32_t* mp = (uint32_t*) ( map->data + y * map->stride );

            for ( x = 0; x < width; x++, mp++, ptr += 3 )
            {
                mapValue = *mp;

                if ( mapValue == 0 )
                {
                    ptr[0] = ptr[1] = ptr[2] = 0;
                }
                else
                {
                    c = colors[( mapValue - 1 ) % colorsCount];

                    ptr[RedIndex]   = c.components.r;
                    ptr[GreenIndex] = c.components.g;
                    ptr[BlueIndex]  = c.components.b;
                }
            }

            ptr += offset;
        }

    }

    return ret;
}

// Find bouding rectangles of all objects. Rectangles array must be preallocated for objectsCount items.
XErrorCode BcGetObjectsRectangles( const ximage* map, uint32_t objectsCount, xrect* rectangles )
{
    XErrorCode ret = SuccessCode;

    if ( ( map == 0 ) || ( rectangles == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( map->format != XPixelFormatGrayscale32 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width    = map->width;
        int      height   = map->height;
        int      widthM1  = width - 1;
        int      heightM1 = height - 1;
        int      x, y;
        uint32_t i;
        xrect*   rect;

        // initialize rectangles
        for ( i = 0; i < objectsCount; i++ )
        {
            rectangles[i].x1 = widthM1;
            rectangles[i].y1 = heightM1;
            rectangles[i].x2 = 0;
            rectangles[i].y2 = 0;
        }

        for ( y = 0; y < height; y++ )
        {
            uint32_t* mp = (uint32_t*) ( map->data + y * map->stride );

            for ( x = 0; x < width; x++, mp++ )
            {
                if ( *mp != 0 )
                {
                    rect = &( rectangles[*mp - 1] );

                    if ( x < rect->x1 ) rect->x1 = x;
                    if ( x > rect->x2 ) rect->x2 = x;
                    if ( y < rect->y1 ) rect->y1 = y;
                    if ( y > rect->y2 ) rect->y2 = y;
                }
            }
        }
    }

    return ret;
}

// Find areas of all objects (number of pixels in every object). Areas array must be preallocated for objectsCount values.
XErrorCode BcGetObjectsArea( const ximage* map, uint32_t objectsCount, uint32_t* areas, uint32_t* totalArea )
{
    XErrorCode ret = SuccessCode;

    if ( ( map == 0 ) || ( areas == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( map->format != XPixelFormatGrayscale32 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width    = map->width;
        int      height   = map->height;
        int      x, y;
        uint32_t i, total = 0;

        // initialize rectangles
        for ( i = 0; i < objectsCount; i++ )
        {
            areas[i] = 0;
        }

        for ( y = 0; y < height; y++ )
        {
            uint32_t* mp = (uint32_t*) ( map->data + y * map->stride );

            for ( x = 0; x < width; x++, mp++ )
            {
                if ( *mp != 0 )
                {
                    total++;

                    areas[*mp - 1]++;
                }
            }
        }

        if ( totalArea != 0 )
        {
            *totalArea = total;
        }
    }

    return ret;
}

// Find bouding rectangles of all objects and their area. Rectangles and areas arrays must be preallocated for objectsCount items.
XErrorCode BcGetObjectsRectanglesAndArea( const ximage* map, uint32_t objectsCount, xrect* rectangles, uint32_t* areas )
{
    XErrorCode ret = SuccessCode;

    if ( ( map == 0 ) || ( rectangles == 0 ) || ( areas == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( map->format != XPixelFormatGrayscale32 )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else
    {
        int      width    = map->width;
        int      height   = map->height;
        int      widthM1  = width - 1;
        int      heightM1 = height - 1;
        int      x, y;
        uint32_t i, id;
        xrect*   rect;

        // initialize arrays
        for ( i = 0; i < objectsCount; i++ )
        {
            rectangles[i].x1 = widthM1;
            rectangles[i].y1 = heightM1;
            rectangles[i].x2 = 0;
            rectangles[i].y2 = 0;

            areas[i] = 0;
        }

        for ( y = 0; y < height; y++ )
        {
            uint32_t* mp = (uint32_t*) ( map->data + y * map->stride );

            for ( x = 0; x < width; x++, mp++ )
            {
                if ( *mp != 0 )
                {
                    id = *mp - 1;

                    rect = &( rectangles[id] );

                    if ( x < rect->x1 ) rect->x1 = x;
                    if ( x > rect->x2 ) rect->x2 = x;
                    if ( y < rect->y1 ) rect->y1 = y;
                    if ( y > rect->y2 ) rect->y2 = y;

                    areas[id]++;
                }
            }
        }
    }

    return ret;
}

// Fill objects specified by the fill map. Fill map is array of size objectsCount+1, which contains 1 or 0 to indicate if object must be filled or not.
XErrorCode BcFillObjects( ximage* image, const ximage* map, const uint8_t* fillMap, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( map == 0 ) || ( fillMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( map->format != XPixelFormatGrayscale32 ) ||
            ( ( image->format != XPixelFormatGrayscale8 ) && ( image->format != XPixelFormatRGB24 ) && ( image->format != XPixelFormatRGBA32 ) ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->width != map->width ) || ( image->height != map->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( map->stride != map->width * 4 )
    {
        ret = ErrorFailed;
    }
    else
    {
        int      width     = map->width;
        int      height    = map->height;
        int      pixelSize = ( image->format == XPixelFormatGrayscale8 ) ? 1 : ( ( image->format == XPixelFormatRGB24 ) ? 3 : 4 );
        int      offset    = image->stride - width * pixelSize;
        int      x, y;

        uint8_t*  imagePtr = image->data;
        uint32_t* mapPtr   = (uint32_t*) map->data;

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) ( RGB_TO_GRAY( fillColor.components.r, fillColor.components.g, fillColor.components.b ) * fillColor.components.a / 255 );

            for ( y = 0; y < height; y++ )
            {
                for ( x = 0; x < width; x++, imagePtr++, mapPtr++ )
                {
                    if ( fillMap[*mapPtr] )
                    {
                        *imagePtr = fillValue;
                    }
                }

                imagePtr += offset;
            }
        }
        else if ( image->format == XPixelFormatRGB24 )
        {
            uint8_t fillR = (uint8_t) ( fillColor.components.r * fillColor.components.a / 255 );
            uint8_t fillG = (uint8_t) ( fillColor.components.g * fillColor.components.a / 255 );
            uint8_t fillB = (uint8_t) ( fillColor.components.b * fillColor.components.a / 255 );

            for ( y = 0; y < height; y++ )
            {
                for ( x = 0; x < width; x++, imagePtr += 3, mapPtr++ )
                {
                    if ( fillMap[*mapPtr] )
                    {
                        imagePtr[RedIndex]   = fillR;
                        imagePtr[GreenIndex] = fillG;
                        imagePtr[BlueIndex]  = fillB;
                    }
                }

                imagePtr += offset;
            }
        }
        else
        {
            for ( y = 0; y < height; y++ )
            {
                for ( x = 0; x < width; x++, imagePtr += 4, mapPtr++ )
                {
                    if ( fillMap[*mapPtr] )
                    {
                        imagePtr[RedIndex]   = fillColor.components.r;
                        imagePtr[GreenIndex] = fillColor.components.g;
                        imagePtr[BlueIndex]  = fillColor.components.b;
                        imagePtr[AlphaIndex] = fillColor.components.a;
                    }
                }

                imagePtr += offset;
            }
        }
    }

    return ret;
}

// Fill all object except the one with the specified ID
XErrorCode BcKeepObjectIDOnly( ximage* image, const ximage* map, uint32_t blobId, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( ( image == 0 ) || ( map == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( ( map->format != XPixelFormatGrayscale32 ) ||
            ( ( image->format != XPixelFormatGrayscale8 ) && ( image->format != XPixelFormatRGB24 ) && ( image->format != XPixelFormatRGBA32 ) ) )
    {
        ret = ErrorUnsupportedPixelFormat;
    }
    else if ( ( image->width != map->width ) || ( image->height != map->height ) )
    {
        ret = ErrorImageParametersMismatch;
    }
    else if ( map->stride != map->width * 4 )
    {
        ret = ErrorFailed;
    }
    else
    {
        int      width     = map->width;
        int      height    = map->height;
        int      pixelSize = ( image->format == XPixelFormatGrayscale8 ) ? 1 : ( ( image->format == XPixelFormatRGB24 ) ? 3 : 4 );
        int      offset    = image->stride - width * pixelSize;
        int      x, y;

        uint8_t*  imagePtr = image->data;
        uint32_t* mapPtr   = (uint32_t*) map->data;

        if ( image->format == XPixelFormatGrayscale8 )
        {
            uint8_t fillValue = (uint8_t) ( RGB_TO_GRAY( fillColor.components.r, fillColor.components.g, fillColor.components.b ) * fillColor.components.a / 255 );

            for ( y = 0; y < height; y++ )
            {
                for ( x = 0; x < width; x++, imagePtr++, mapPtr++ )
                {
                    if ( *mapPtr != blobId )
                    {
                        *imagePtr = fillValue;
                    }
                }

                imagePtr += offset;
            }
        }
        else if ( image->format == XPixelFormatRGB24 )
        {
            uint8_t fillR = (uint8_t) ( fillColor.components.r * fillColor.components.a / 255 );
            uint8_t fillG = (uint8_t) ( fillColor.components.g * fillColor.components.a / 255 );
            uint8_t fillB = (uint8_t) ( fillColor.components.b * fillColor.components.a / 255 );

            for ( y = 0; y < height; y++ )
            {
                for ( x = 0; x < width; x++, imagePtr += 3, mapPtr++ )
                {
                    if ( *mapPtr != blobId )
                    {
                        imagePtr[RedIndex]   = fillR;
                        imagePtr[GreenIndex] = fillG;
                        imagePtr[BlueIndex]  = fillB;
                    }
                }

                imagePtr += offset;
            }
        }
        else
        {
            for ( y = 0; y < height; y++ )
            {
                for ( x = 0; x < width; x++, imagePtr += 4, mapPtr++ )
                {
                    if ( *mapPtr != blobId )
                    {
                        imagePtr[RedIndex]   = fillColor.components.r;
                        imagePtr[GreenIndex] = fillColor.components.g;
                        imagePtr[BlueIndex]  = fillColor.components.b;
                        imagePtr[AlphaIndex] = fillColor.components.a;
                    }
                }

                imagePtr += offset;
            }
        }
    }

    return ret;
}

// Initialize fill map, which can be used to fill non edge objects
XErrorCode BcBuildFillMapNonEdgeObjects( int imageWidth, int imageHeight, uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap, uint32_t* objectsToFillCount )
{
    XErrorCode ret = SuccessCode;

    if ( ( rectangles == 0 ) || ( fillMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int      widthM1  = imageWidth  - 1;
        int      heightM1 = imageHeight - 1;
        uint32_t i, toFill = 0;

        fillMap[0] = 0;
        for ( i = 0; i < objectsCount; i++ )
        {
            if ( ( rectangles[i].x1 == 0 ) || ( rectangles[i].x2 == widthM1 ) ||
                 ( rectangles[i].y1 == 0 ) || ( rectangles[i].y2 == heightM1 ) )
            {
                fillMap[i + 1] = 0;
            }
            else
            {
                fillMap[i + 1] = 1;
                toFill++;
            }
        }

        if ( objectsToFillCount )
        {
            *objectsToFillCount = toFill;
        }
    }

    return ret;
}

// Initialize fill map, which can be used to fill non edge objects smaller than the specified size
XErrorCode BcBuildFillMapNonEdgeBySizeObjects( int imageWidth, int imageHeight, uint32_t minWidth, uint32_t minHeight, bool coupleFiltering,
                                               uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap,
                                               uint32_t* objectsToFillCount, uint32_t* objectsToStayCount )
{
    XErrorCode ret = SuccessCode;

    if ( ( rectangles == 0 ) || ( fillMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int      imageWidthM1  = imageWidth  - 1;
        int      imageHeightM1 = imageHeight - 1;
        uint32_t width, height, i, toFill = 0, toStay = 0;

        fillMap[0] = 0;
        for ( i = 0; i < objectsCount; i++ )
        {
            if ( ( rectangles[i].x1 == 0 ) || ( rectangles[i].x2 == imageWidthM1 ) ||
                 ( rectangles[i].y1 == 0 ) || ( rectangles[i].y2 == imageHeightM1 ) )
            {
                fillMap[i + 1] = 0;
            }
            else
            {
                width  = (uint32_t) ( rectangles[i].x2 - rectangles[i].x1 + 1 );
                height = (uint32_t) ( rectangles[i].y2 - rectangles[i].y1 + 1 );


                if ( ( ( coupleFiltering == true )  &&   ( width < minWidth ) && ( height < minHeight ) ) ||
                     ( ( coupleFiltering == false ) && ( ( width < minWidth ) || ( height < minHeight ) ) ) )
                {
                    fillMap[i + 1] = 1;
                    toFill++;
                }
                else
                {
                    fillMap[i + 1] = 0;
                    toStay++;
                }
            }
        }

        if ( objectsToFillCount )
        {
            *objectsToFillCount = toFill;
        }
        if ( objectsToStayCount )
        {
            *objectsToStayCount = toStay;
        }
    }

    return ret;
}

// Initialize fill map, which can be used to fill non edge objects having area smaller than the specified limit
XErrorCode BcBuildFillMapNonEdgeByAreaObjects( int imageWidth, int imageHeight, uint32_t minArea,
                                               uint32_t objectsCount, const xrect* rectangles, const uint32_t* areas, uint8_t* fillMap,
                                               uint32_t* objectsToFillCount, uint32_t* objectsToStayCount )
{
    XErrorCode ret = SuccessCode;

    if ( ( rectangles == 0 ) || ( areas == 0 ) || ( fillMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int      imageWidthM1  = imageWidth  - 1;
        int      imageHeightM1 = imageHeight - 1;
        uint32_t i, toFill = 0, toStay = 0;

        fillMap[0] = 0;
        for ( i = 0; i < objectsCount; i++ )
        {
            if ( ( rectangles[i].x1 == 0 ) || ( rectangles[i].x2 == imageWidthM1 ) ||
                 ( rectangles[i].y1 == 0 ) || ( rectangles[i].y2 == imageHeightM1 ) )
            {
                fillMap[i + 1] = 0;
            }
            else
            {
                if ( areas[i] < minArea )
                {
                    fillMap[i + 1] = 1;
                    toFill++;
                }
                else
                {
                    fillMap[i + 1] = 0;
                    toStay++;
                }
            }
        }

        if ( objectsToFillCount )
        {
            *objectsToFillCount = toFill;
        }
        if ( objectsToStayCount )
        {
            *objectsToStayCount = toStay;
        }
    }

    return ret;
}

// Initialize fill map, which can be used to fill edge objects
XErrorCode BcBuildFillMapEdgeObjects( int imageWidth, int imageHeight, uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap )
{
    XErrorCode ret = SuccessCode;

    if ( ( rectangles == 0 ) || ( fillMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        int      widthM1  = imageWidth - 1;
        int      heightM1 = imageHeight - 1;
        uint32_t i;

        fillMap[0] = 0;
        for ( i = 0; i < objectsCount; i++ )
        {
            fillMap[i + 1] = ( ( rectangles[i].x1 == 0 ) || ( rectangles[i].x2 == widthM1 ) ||
                               ( rectangles[i].y1 == 0 ) || ( rectangles[i].y2 == heightM1 ) ) ? 1 : 0;
        }
    }

    return ret;
}

// Initialize fill map, which can be used to fill objects smaller than minimum size or bigger than maximum size.
// If coupled filtering is used, then both with and height must be smaller or bigger.
XErrorCode BcBuildFillMapOutOfSizeObjects( uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight, bool coupleFiltering,
                                           uint32_t objectsCount, const xrect* rectangles, uint8_t* fillMap, uint32_t* blobsLeft )
{
    XErrorCode ret = SuccessCode;

    if ( ( rectangles == 0 ) || ( fillMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        uint32_t width, height, i;
        uint32_t objectsRemoved = 0;

        // build the fill map
        fillMap[0] = 0;
        for ( i = 0; i < objectsCount; i++ )
        {
            width  = (uint32_t) ( rectangles[i].x2 - rectangles[i].x1 + 1 );
            height = (uint32_t) ( rectangles[i].y2 - rectangles[i].y1 + 1 );

            if ( coupleFiltering == true )
            {
                fillMap[i + 1] = ( ( ( width < minWidth ) && ( height < minHeight ) ) ||
                                   ( ( width > maxWidth ) && ( height > maxHeight ) ) ) ? 1 : 0;
            }
            else
            {
                fillMap[i + 1] = ( ( width < minWidth ) || ( height < minHeight ) ||
                                   ( width > maxWidth ) || ( height > maxHeight ) ) ? 1 : 0;
            }

            objectsRemoved += fillMap[i + 1];
        }

        if ( blobsLeft )
        {
            *blobsLeft = objectsCount - objectsRemoved;
        }
    }

    return ret;
}

// Initialize fill map, which can be used to fill objects having smaller or bigger area than the specified limit
XErrorCode BcBuildFillMapOutOfAreaObjects( uint32_t minArea, uint32_t maxArea,
                                           uint32_t objectsCount, const uint32_t* areas, uint8_t* fillMap, uint32_t* blobsLeft )
{
    XErrorCode ret = SuccessCode;

    if ( ( areas == 0 ) || ( fillMap == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        uint32_t i, objectsRemoved = 0;

        // build the fill map
        fillMap[0] = 0;
        for ( i = 0; i < objectsCount; i++ )
        {
            if ( ( areas[i] < minArea ) || ( areas[i] > maxArea ) )
            {
                fillMap[i + 1] = 1;
                objectsRemoved++;
            }
            else
            {
                fillMap[i + 1] = 0;
            }
        }

        if ( blobsLeft )
        {
            *blobsLeft = objectsCount - objectsRemoved;
        }
    }

    return ret;
}

// Fill objects not touching image's edge
XErrorCode BcFillNonEdgeObjects( ximage* image, const ximage* map, uint32_t objectsCount, const xrect* rectangles, xargb fillColor, uint32_t* objectsFilledCount )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( objectsCount != 0 )
        {
            uint8_t* fillMap = (uint8_t*) malloc( objectsCount + 1 );

            if ( fillMap == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                ret = BcBuildFillMapNonEdgeObjects( image->width, image->height, objectsCount, rectangles, fillMap, objectsFilledCount );

                if ( ret == SuccessCode )
                {
                    ret = BcFillObjects( image, map, fillMap, fillColor );
                }

                free( fillMap );
            }
        }
    }

    return ret;
}

// Fill objects touching image's edge
XErrorCode BcFillEdgeObjects( ximage* image, const ximage* map, uint32_t objectsCount, const xrect* rectangles, xargb fillColor )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( objectsCount != 0 )
        {
            uint8_t* fillMap = (uint8_t*) malloc( objectsCount + 1 );

            if ( fillMap == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                ret = BcBuildFillMapEdgeObjects( image->width, image->height, objectsCount, rectangles, fillMap );

                if ( ret == SuccessCode )
                {
                    ret = BcFillObjects( image, map, fillMap, fillColor );
                }

                free( fillMap );
            }
        }
    }

    return ret;
}

// Fill objects smaller or bigger than the specified min/max sizes
XErrorCode BcFilterBySizeObjects( ximage* image, const ximage* map, uint32_t objectsCount, const xrect* rectangles, xargb fillColor,
                                  uint32_t minWidth, uint32_t minHeight, uint32_t maxWidth, uint32_t maxHeight, bool coupleFiltering, uint32_t* blobsLeft )
{
    XErrorCode ret = SuccessCode;

    if ( image == 0 )
    {
        ret = ErrorNullParameter;
    }
    else
    {
        if ( objectsCount != 0 )
        {
            uint8_t* fillMap = (uint8_t*) malloc( objectsCount + 1 );

            if ( fillMap == 0 )
            {
                ret = ErrorOutOfMemory;
            }
            else
            {
                ret = BcBuildFillMapOutOfSizeObjects( minWidth, minHeight, maxWidth, maxHeight, coupleFiltering, objectsCount, rectangles, fillMap, blobsLeft );

                if ( ret == SuccessCode )
                {
                    ret = BcFillObjects( image, map, fillMap, fillColor );
                }

                free( fillMap );
            }
        }
    }

    return ret;
}

// Collect object's points on its left/right edges. Each edge array must be at least object's height in size.
XErrorCode BcGetObjectLeftRightEdges( const ximage* blobsMap, uint32_t blobId, xrect blobRect, uint32_t edgeArraysSize, xpoint* leftEdge, xpoint* rightEdge, uint32_t* avgThickness )
{
    uint32_t   blobHeight = (uint32_t) ( blobRect.y2 - blobRect.y1 + 1 );
    XErrorCode ret        = SuccessCode;

    if ( ( blobsMap == 0 ) || ( leftEdge == 0 ) || ( rightEdge == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( blobsMap->format != XPixelFormatGrayscale32 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( edgeArraysSize < blobHeight )
    {
        ret = ErrorTooSmallBuffer;
    }
    else
    {
        int32_t  startX = blobRect.x1;
        int32_t  startY = blobRect.y1;
        int32_t  endX   = blobRect.x2;
        int32_t  endY   = blobRect.y2;
        int32_t  stride = blobsMap->stride;
        int32_t  xs, xe, y, i;
        uint32_t thickness = 0;

        uint8_t*  ptr = blobsMap->data;
        uint32_t* row;

        for ( y = startY, i = 0; y <= endY; y++, i++ )
        {
            row = (uint32_t*) ( ptr + y * stride );

            // left edge
            xs = startX;
            while ( row[xs] != blobId )
            {
                xs++;
            }
            leftEdge[i].x = xs;
            leftEdge[i].y = y;

            // right edge
            xe = endX;
            while ( row[xe] != blobId )
            {
                xe--;
            }
            rightEdge[i].x = xe;
            rightEdge[i].y = y;

            thickness += ( xe - xs );
        }

        if ( avgThickness )
        {
            // thickness should be calculated as (x2-x1)+1, which is not done above.
            // so we add blobHeight to compensate for all the missing 1's.
            *avgThickness = ( thickness + blobHeight ) / blobHeight;
        }
    }

    return ret;
}

// Collect object's points on its top/bottom edges. Each edge array must be at least object's width in size.
XErrorCode BcGetObjectTopBottomEdges( const ximage* blobsMap, uint32_t blobId, xrect blobRect, uint32_t edgeArraysSize, xpoint* topEdge, xpoint* bottomEdge, uint32_t* avgThickness )
{
    uint32_t   blobWidth = (uint32_t) ( blobRect.x2 - blobRect.x1 + 1 );
    XErrorCode ret       = SuccessCode;

    if ( ( blobsMap == 0 ) || ( topEdge == 0 ) || ( bottomEdge == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( blobsMap->format != XPixelFormatGrayscale32 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( edgeArraysSize < blobWidth )
    {
        ret = ErrorTooSmallBuffer;
    }
    else
    {
        int32_t  startX = blobRect.x1;
        int32_t  startY = blobRect.y1;
        int32_t  endX   = blobRect.x2;
        int32_t  endY   = blobRect.y2;
        int32_t  stride = blobsMap->stride;
        int32_t  x, ys, ye, i;
        uint32_t thickness = 0;

        uint8_t* ptr = blobsMap->data;
        uint8_t* column;

        for ( x = startX, i = 0; x <= endX; x++, i++ )
        {
            // top edge
            ys = startY;
            column = ( ptr + ys * stride + x * 4 );
            while ( *( (uint32_t*) column ) != blobId )
            {
                ys++;
                column += stride;
            }
            topEdge[i].x = x;
            topEdge[i].y = ys;

            // bottom edge
            ye = endY;
            column = ( ptr + ye * stride + x * 4 );
            while ( *( (uint32_t*) column ) != blobId )
            {
                ye--;
                column -= stride;
            }
            bottomEdge[i].x = x;
            bottomEdge[i].y = ye;

            thickness += ( ye - ys );
        }

        if ( avgThickness )
        {
            // thickness should be calculated as (y2-y1)+1, which is not done above.
            // so we add blobWidth to compensate for all the missing 1's.
            *avgThickness = ( thickness + blobWidth ) / blobWidth;
        }
    }

    return ret;
}

// Collect object's points. The edge array must be at least ( width + height ) * 2 in size. First left edge's points are put into the
// array, then right edge's, finally top/bottom edge points are mixed (to make sure they don't repeat left/right edges).
XErrorCode BcGetObjectEdgePoints( const ximage* blobsMap, uint32_t blobId, xrect blobRect, uint32_t edgeArraysSize, xpoint* edgePoints, uint32_t* edgePointsCount, uint32_t* avgVerticalThickness )
{
    uint32_t   blobWidth  = (uint32_t) ( blobRect.x2 - blobRect.x1 + 1 );
    uint32_t   blobHeight = (uint32_t) ( blobRect.y2 - blobRect.y1 + 1 );
    XErrorCode ret        = SuccessCode;

    if ( ( blobsMap == 0 ) || ( edgePoints == 0 ) || ( edgePointsCount == 0 ) )
    {
        ret = ErrorNullParameter;
    }
    else if ( blobsMap->format != XPixelFormatGrayscale32 )
    {
        ret = ErrorInvalidArgument;
    }
    else if ( edgeArraysSize < ( blobHeight + blobWidth * 2 ) )
    {
        ret = ErrorTooSmallBuffer;
    }
    else
    {
        int32_t  startX = blobRect.x1;
        int32_t  startY = blobRect.y1;
        int32_t  endX   = blobRect.x2;
        int32_t  endY   = blobRect.y2;
        int32_t  stride = blobsMap->stride;
        int32_t  xs, xe, x, y, i, y0;
        uint32_t verticalThickness = 0;

        xpoint* leftEdge  = edgePoints;
        xpoint* rightEdge = leftEdge + blobHeight;

        uint8_t*  ptr = blobsMap->data;
        uint8_t*  column;
        uint32_t* row;

        uint32_t  totalPointsCollected = blobHeight * 2;

        // collect left/right edges first
        for ( y = startY, i = 0; y <= endY; y++, i++ )
        {
            row = (uint32_t*) ( ptr + y * stride );

            // left edge
            xs = startX;
            while ( row[xs] != blobId )
            {
                xs++;
            }
            leftEdge[i].x = xs;
            leftEdge[i].y = y;

            // right edge
            xe = endX;
            while ( row[xe] != blobId )
            {
                xe--;
            }
            rightEdge[i].x = xe;
            rightEdge[i].y = y;

            verticalThickness += ( xe - xs );
        }

        if ( avgVerticalThickness )
        {
            *avgVerticalThickness = ( verticalThickness + blobHeight ) / blobHeight;
        }

        // collect missing top/bottom edges
        for ( x = startX, i = 0; x <= endX; x++ )
        {
            // top edge
            y = startY;
            column = ( ptr + y * stride + x * 4 );
            while ( *( (uint32_t*) column ) != blobId )
            {
                y++;
                column += stride;
            }
            y0 = y - startY;

            if ( ( leftEdge[y0].x != x ) && ( rightEdge[y0].x != x ) )
            {
                edgePoints[totalPointsCollected].x = x;
                edgePoints[totalPointsCollected].y = y;
                totalPointsCollected++;
            }

            // bottom edge
            y = endY;
            column = ( ptr + y * stride + x * 4 );
            while ( *( (uint32_t*) column ) != blobId )
            {
                y--;
                column -= stride;
            }
            y0 = y - startY;

            if ( ( leftEdge[y0].x != x ) && ( rightEdge[y0].x != x ) )
            {
                edgePoints[totalPointsCollected].x = x;
                edgePoints[totalPointsCollected].y = y;
                totalPointsCollected++;
            }
        }

        *edgePointsCount = totalPointsCollected;
    }

    return ret;

}
